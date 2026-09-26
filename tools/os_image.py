#!/usr/bin/env python3
"""Umicom OS Foundation image construction and explicit QEMU boot checks.

Sammy Hegab, Umicom Foundation. Licence: MIT.

This host tool snapshots selected source files, invokes a pinned Buildroot
checkout, and packages a whitelist of static guest programs. It never writes a
block device, mounts an image, changes the host bootloader, or uses a shell to
execute a constructed command. Kernel and recovery ownership stays in umicom-os.
"""
from __future__ import annotations

import argparse
import dataclasses
import gzip
import hashlib
import io
import json
import os
from pathlib import Path, PurePosixPath
import re
import selectors
import shutil
import signal
import stat
import struct
import subprocess
import sys
import time
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
MAX_ARCHIVE = 64 * 1024 * 1024
MAX_FILE = 32 * 1024 * 1024
ARCHES = {"riscv64": (243, "Image", "qemu-system-riscv64"),
          "x86_64": (62, "bzImage", "qemu-system-x86_64")}
DIRECTORIES = {"dev": 0o755, "proc": 0o555, "sys": 0o555, "run": 0o755,
               "tmp": 0o1777, "etc": 0o755, "etc/umicom": 0o755, "root": 0o700,
               "usr": 0o755, "usr/libexec": 0o755, "usr/share": 0o755,
               "usr/share/umicom": 0o755}
OVERLAY = ("etc/umicom/boot.conf", "etc/os-release", "etc/passwd", "etc/group", "etc/shadow")
PROGRAMS = ("init", "usr/libexec/umicom-platform-check", "usr/libexec/umicom-framework-probe")


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def json_bytes(value: Any) -> bytes:
    return (json.dumps(value, sort_keys=True, indent=2, ensure_ascii=True) + "\n").encode()


def read_json(path: Path) -> Any:
    def unique(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
        result: dict[str, Any] = {}
        for key, value in pairs:
            if key in result:
                raise ValueError(f"Duplicate JSON field: {key}")
            result[key] = value
        return result
    return json.loads(regular_bytes(path), object_pairs_hook=unique)


def regular_bytes(path: Path, maximum: int = MAX_FILE) -> bytes:
    info = path.lstat()
    if not stat.S_ISREG(info.st_mode) or info.st_size > maximum:
        raise ValueError(f"Expected a bounded regular file, not a link or device: {path}")
    with path.open("rb") as stream:
        data = stream.read(maximum + 1)
    if len(data) > maximum:
        raise ValueError(f"File exceeds its size limit: {path}")
    return data


def relative_name(name: str) -> str:
    path = PurePosixPath(name)
    if name in ("", ".", "TRAILER!!!") or name != path.as_posix() or path.is_absolute() or ".." in path.parts:
        raise ValueError(f"Unsafe archive path: {name!r}")
    if any(ord(c) < 33 or ord(c) > 126 or c in "\\:" for c in name):
        raise ValueError(f"Unsupported archive path: {name!r}")
    return name


def source_bytes(root: Path, name: str) -> bytes:
    relative_name(name)
    current = root
    for part in PurePosixPath(name).parts:
        current = current / part
        if current.is_symlink():
            raise ValueError(f"Source links are not followed: {current}")
    return regular_bytes(current)


def exclusive_file(path: Path, data: bytes) -> None:
    with path.open("xb") as stream:
        stream.write(data)


def new_directory(path: Path) -> Path:
    path = path.expanduser().absolute()
    if path.exists() or path.is_symlink():
        raise ValueError(f"Use a new output directory. Existing data is not replaced: {path}")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.mkdir(mode=0o755)
    return path


def run(arguments: list[str], **kwargs: Any) -> subprocess.CompletedProcess:
    print("Running:", " ".join(repr(arg) for arg in arguments), flush=True)
    return subprocess.run(arguments, check=True, **kwargs)


def git_identity(root: Path) -> dict[str, Any]:
    try:
        commit = subprocess.check_output(["git", "-C", str(root), "rev-parse", "HEAD"],
                                         stderr=subprocess.DEVNULL, text=True).strip()
        dirty = bool(subprocess.check_output(["git", "-C", str(root), "status", "--porcelain"],
                                            stderr=subprocess.DEVNULL))
        return {"commit": commit, "dirty": dirty}
    except (subprocess.CalledProcessError, FileNotFoundError):
        return {"commit": None, "dirty": None}


def check_buildroot(root: Path, versions: dict) -> None:
    identity = git_identity(root)
    if identity["commit"] != versions["buildroot_commit"] or identity["dirty"] is not False:
        raise ValueError("Buildroot must be a clean checkout of the pinned commit in image/versions.json")
    makefile = regular_bytes(root / "Makefile").decode()
    if "EXTRAVERSION = .18" not in makefile and "2025.02.18" not in makefile:
        # The commit is the authority. This check is only a basic tree sanity test.
        if "VERSION = 2025" not in makefile:
            raise ValueError("Buildroot Makefile does not match the selected release family")


def elf_static(data: bytes, arch: str) -> None:
    if len(data) < 64 or data[:7] != b"\x7fELF\x02\x01\x01":
        raise ValueError("Guest programs must be little-endian ELF64 files")
    elf_type, machine = struct.unpack_from("<HH", data, 16)
    if elf_type != 2 or machine != ARCHES[arch][0]:
        raise ValueError("Guest executable architecture or ELF type is wrong")
    phoff = struct.unpack_from("<Q", data, 32)[0]
    phsize, phcount = struct.unpack_from("<HH", data, 54)
    if phsize != 56 or not 1 <= phcount <= 128 or phoff + phsize * phcount > len(data):
        raise ValueError("Invalid ELF program header table")
    entrypoint = struct.unpack_from("<Q", data, 24)[0]
    if struct.unpack_from("<I", data, 20)[0] != 1 or struct.unpack_from("<H", data, 52)[0] != 64:
        raise ValueError("Invalid ELF header version or size")
    executable_entry = False
    for index in range(phcount):
        kind, flags, offset, address, _, filesz, memsz, _ = struct.unpack_from("<IIQQQQQQ", data, phoff + index * phsize)
        if kind in (2, 3):
            raise ValueError("Guest program needs a dynamic linker; static linking is required")
        if kind == 1 and (offset + filesz > len(data) or memsz < filesz):
            raise ValueError("Invalid ELF load segment")
        if kind == 1 and address + memsz > (1 << 64):
            raise ValueError("ELF memory segment overflows its address space")
        if kind == 1 and flags & 1 and address <= entrypoint < address + filesz:
            executable_entry = True
        if kind == 0x6474E551 and flags & 1:
            raise ValueError("Guest programs must not request an executable stack")
    if not executable_entry:
        raise ValueError("ELF entry point is not in a file-backed executable segment")


@dataclasses.dataclass(frozen=True)
class Entry:
    name: str
    mode: int
    data: bytes = b""
    major: int = 0
    minor: int = 0

    def metadata(self) -> dict:
        return {"mode": self.mode, "size": len(self.data), "sha256": digest(self.data),
                "major": self.major, "minor": self.minor}


def validate_entry(entry: Entry) -> None:
    relative_name(entry.name)
    kind = stat.S_IFMT(entry.mode)
    if entry.mode & ~0o177777 or entry.mode & (stat.S_ISUID | stat.S_ISGID):
        raise ValueError("Set-ID files are not accepted")
    if kind not in (stat.S_IFREG, stat.S_IFDIR, stat.S_IFCHR):
        raise ValueError("Only files, directories and the two initial console devices are accepted")
    if kind != stat.S_IFREG and entry.data:
        raise ValueError("Non-file archive entry has data")
    if kind == stat.S_IFCHR and (entry.name, entry.major, entry.minor) not in (
            ("dev/console", 5, 1), ("dev/null", 1, 3)):
        raise ValueError("Unexpected device; block devices are never included")
    if kind != stat.S_IFCHR and (entry.major or entry.minor):
        raise ValueError("Unexpected device numbers")
    if len(entry.data) > MAX_FILE:
        raise ValueError("Archive entry is too large")


def cpio(entries: list[Entry]) -> bytes:
    """Write Linux's newc format without root privileges or host device nodes."""
    if len(entries) > 128:
        raise ValueError("Too many root filesystem entries")
    output = bytearray()
    names: dict[str, int] = {}
    ordered = sorted(entries, key=lambda e: e.name.encode("ascii"))
    for index, entry in enumerate(ordered, 1):
        validate_entry(entry)
        if entry.name in names:
            raise ValueError("Duplicate archive entry")
        names[entry.name] = entry.mode
        parent = str(PurePosixPath(entry.name).parent)
        if parent != "." and (parent not in names or not stat.S_ISDIR(names[parent])):
            raise ValueError(f"Missing parent directory: {parent}")
        name = entry.name.encode("ascii") + b"\0"
        if len(output) + len(name) + len(entry.data) + 1024 > MAX_ARCHIVE:
            raise ValueError("Root filesystem exceeds the memory-image budget")
        fields = [index, entry.mode, 0, 0, 2 if stat.S_ISDIR(entry.mode) else 1,
                  0, len(entry.data), 0, 0, entry.major, entry.minor, len(name), 0]
        output.extend(b"070701" + b"".join(f"{value:08x}".encode() for value in fields))
        output.extend(name)
        output.extend(b"\0" * (-len(output) % 4))
        output.extend(entry.data)
        output.extend(b"\0" * (-len(output) % 4))
    name = b"TRAILER!!!\0"
    fields = [0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, len(name), 0]
    output.extend(b"070701" + b"".join(f"{value:08x}".encode() for value in fields) + name)
    output.extend(b"\0" * (-len(output) % 512))
    if len(output) > MAX_ARCHIVE:
        raise ValueError("Root filesystem exceeds the memory-image budget")
    compressed = io.BytesIO()
    with gzip.GzipFile(fileobj=compressed, mode="wb", filename="", mtime=0, compresslevel=9) as stream:
        stream.write(output)
    return compressed.getvalue()


def read_cpio(compressed: bytes) -> list[Entry]:
    with gzip.GzipFile(fileobj=io.BytesIO(compressed)) as stream:
        data = stream.read(MAX_ARCHIVE + 1)
    if len(data) > MAX_ARCHIVE:
        raise ValueError("Expanded archive exceeds its limit")
    cursor = 0
    result: list[Entry] = []
    while cursor < len(data):
        if cursor + 110 > len(data) or data[cursor:cursor + 6] != b"070701":
            raise ValueError("Invalid newc header")
        raw = data[cursor + 6:cursor + 110]
        if not re.fullmatch(b"[0-9a-fA-F]{104}", raw):
            raise ValueError("Invalid newc hexadecimal field")
        fields = [int(raw[i:i + 8], 16) for i in range(0, 104, 8)]
        _, mode, uid, gid, links, mtime, size, devmajor, devminor, major, minor, name_size, check = fields
        cursor += 110
        if not 1 <= name_size <= 192 or cursor + name_size > len(data) or uid or gid or mtime or check or devmajor or devminor:
            raise ValueError("Invalid newc metadata")
        raw_name = data[cursor:cursor + name_size]
        if raw_name[-1:] != b"\0" or b"\0" in raw_name[:-1]:
            raise ValueError("Invalid newc filename")
        name = raw_name[:-1].decode("ascii")
        cursor += name_size
        padding = -cursor % 4
        if any(data[cursor:cursor + padding]):
            raise ValueError("Non-zero archive name padding")
        cursor += padding
        if size > MAX_FILE or cursor + size > len(data):
            raise ValueError("Invalid newc file size")
        content = data[cursor:cursor + size]
        cursor += size
        padding = -cursor % 4
        if any(data[cursor:cursor + padding]):
            raise ValueError("Non-zero archive content padding")
        cursor += padding
        if name == "TRAILER!!!":
            if mode or size or major or minor or links != 1 or any(data[cursor:]):
                raise ValueError("Invalid newc trailer")
            return result
        if links != (2 if stat.S_ISDIR(mode) else 1):
            raise ValueError("Hardlinks are not accepted")
        entry = Entry(name, mode, content, major, minor)
        validate_entry(entry)
        if any(e.name == name for e in result):
            raise ValueError("Duplicate archive filename")
        parent = str(PurePosixPath(name).parent)
        if parent != "." and not any(e.name == parent and stat.S_ISDIR(e.mode) for e in result):
            raise ValueError("Archive parent must be an earlier directory")
        result.append(entry)
        if len(result) > 128:
            raise ValueError("Too many root filesystem entries")
    raise ValueError("Missing archive trailer")


def input_manifest(output: Path) -> dict:
    manifest = read_json(output / "input-manifest.json")
    if (not isinstance(manifest, dict) or set(manifest) != {"schema", "arch", "versions", "os", "framework", "files"}
            or manifest["schema"] != 1 or manifest["arch"] not in ARCHES
            or not isinstance(manifest["files"], dict) or not manifest["files"]):
        raise ValueError("Unsupported frozen-input manifest")
    for name, expected in manifest["files"].items():
        if digest(source_bytes(output / "inputs", name)) != expected:
            raise ValueError(f"A frozen build input changed: {name}")
    return manifest


def prepare(args: argparse.Namespace) -> None:
    if sys.platform != "linux":
        raise ValueError("Prepare and Build run in Linux or WSL, not MSYS2")
    buildroot, framework = args.buildroot.expanduser().resolve(), args.framework.expanduser().resolve()
    versions = read_json(ROOT / "image/versions.json")
    for root in (buildroot, framework, args.output.expanduser().absolute()):
        if any(c.isspace() for c in str(root)):
            raise ValueError("Buildroot paths must not contain spaces or control characters")
    check_buildroot(buildroot, versions)
    source_names = read_json(ROOT / "image/framework-files.json")
    captured: dict[str, bytes] = {}
    for name in source_names:
        captured["framework/" + name] = source_bytes(framework, name)
    # The OS repository may also contain an entire Linux source checkout.
    # Snapshot only this profile's declared inputs, never recurse through it.
    os_names = read_json(ROOT / "image/os-files.json")
    if (not isinstance(os_names, list) or not 1 <= len(os_names) <= 128 or
            len(set(os_names)) != len(os_names)):
        raise ValueError("Invalid OS source inventory")
    for name in os_names:
        captured["umicom-os/" + name] = source_bytes(ROOT, name)
    output = args.output.expanduser().absolute()
    for protected in (ROOT, framework, buildroot):
        if output == protected or protected in output.parents or output in protected.parents:
            raise ValueError("Output must be outside the source and Buildroot directories")
    output = new_directory(output)
    manifest = {"schema": 1, "arch": args.arch, "versions": versions,
                "os": git_identity(ROOT), "framework": git_identity(framework),
                "files": {name: digest(data) for name, data in sorted(captured.items())}}
    for name, data in captured.items():
        path = output / "inputs" / name
        path.parent.mkdir(parents=True, exist_ok=True)
        exclusive_file(path, data)
    exclusive_file(output / "input-manifest.json", json_bytes(manifest))
    exclusive_file(output / "build-location.json", json_bytes({"buildroot": str(buildroot)}))
    run(["make", "-C", str(buildroot), "O=" + str(output / "build"),
         "BR2_EXTERNAL=" + str(output / "inputs/umicom-os/image"),
         f"umicom_{args.arch}_defconfig"])
    print("Inputs captured. No image has been built or booted.")


def check_config(output: Path, manifest: dict) -> None:
    text = regular_bytes(output / "build/.config").decode()
    required = ("BR2_INIT_NONE=y", "BR2_STATIC_LIBS=y", "BR2_TOOLCHAIN_BUILDROOT_MUSL=y",
                "BR2_PACKAGE_UMICOM_OS_INIT=y", "BR2_PACKAGE_UMICOM_FRAMEWORK_PROBE=y",
                "BR2_DOWNLOAD_FORCE_CHECK_HASHES=y",
                f'BR2_LINUX_KERNEL_CUSTOM_VERSION_VALUE="{manifest["versions"]["linux"]}"')
    for line in required:
        if line not in text.splitlines():
            raise ValueError(f"The configured image lost a required option: {line}")


def build(args: argparse.Namespace) -> None:
    if sys.platform != "linux" or os.geteuid() == 0:
        raise ValueError("Run Buildroot as a normal Linux/WSL user, not root")
    output = args.output.expanduser().resolve()
    manifest = input_manifest(output)
    buildroot = Path(read_json(output / "build-location.json")["buildroot"])
    check_buildroot(buildroot, manifest["versions"])
    check_config(output, manifest)
    run(["make", "-C", str(buildroot), "O=" + str(output / "build"), f"BR2_JLEVEL={args.jobs}"])
    input_manifest(output)
    print("Build finished. Use pack, then verify and boot. This is not yet a boot result.")


def kernel_header(data: bytes, arch: str) -> None:
    if arch == "x86_64":
        if (len(data) < 4096 or data[0x1FE:0x200] != b"\x55\xaa" or data[0x202:0x206] != b"HdrS"
                or struct.unpack_from("<H", data, 0x206)[0] < 0x20C
                or not struct.unpack_from("<H", data, 0x236)[0] & 1):
            raise ValueError("Expected a 64-bit x86 bzImage Linux boot header")
    else:
        # Linux documents the legacy magic at offset 48 and the current
        # RSC\x05 magic at offset 56. Both sit within the 64-byte header.
        if (len(data) < 4096 or data[48:56] != b"RISCV\0\0\0" or
                data[56:60] != b"RSC\x05" or struct.unpack_from("<Q", data, 24)[0] & 1):
            raise ValueError("Expected a little-endian RISC-V Linux Image boot header")
        size = struct.unpack_from("<Q", data, 16)[0]
        if size < 4096:
            raise ValueError("Invalid RISC-V kernel memory-size field")


def entries_for(target: Path, overlay: Path, manifest: dict, source_id: str) -> list[Entry]:
    entries = [Entry(name, stat.S_IFDIR | mode) for name, mode in DIRECTORIES.items()]
    entries += [Entry("dev/console", stat.S_IFCHR | 0o600, major=5, minor=1),
                Entry("dev/null", stat.S_IFCHR | 0o666, major=1, minor=3)]
    for name in PROGRAMS:
        data = source_bytes(target, name)
        elf_static(data, manifest["arch"])
        entries.append(Entry(name, stat.S_IFREG | 0o755, data))
    for name in OVERLAY:
        mode = 0o400 if name == "etc/shadow" else 0o444
        entries.append(Entry(name, stat.S_IFREG | mode, source_bytes(overlay, name)))
    entries.append(Entry("etc/umicom/source-id", stat.S_IFREG | 0o444, (source_id + "\n").encode()))
    packages = {"profile": "diskless-foundation", "architecture": manifest["arch"],
                "kernel": manifest["versions"]["linux"], "buildroot": manifest["versions"]["buildroot_tag"],
                "runtime_libc": "statically linked musl from the selected Buildroot release",
                "programs": {e.name: e.metadata() for e in entries if e.name in PROGRAMS},
                "notice": "No GUI, disk installer, persistent user storage or network service is included."}
    entries.append(Entry("usr/share/umicom/packages.json", stat.S_IFREG | 0o444, json_bytes(packages)))
    return entries


def create_bundle(target: Path, overlay: Path, kernel: Path, manifest: dict, destination: Path) -> Path:
    source_data = json_bytes(manifest)
    source_id = digest(source_data)
    entries = entries_for(target, overlay, manifest, source_id)
    kernel_data = regular_bytes(kernel)
    kernel_header(kernel_data, manifest["arch"])
    archive = cpio(entries)
    # Validate before creating the destination; rejection must not replace files.
    if {e.name: e.metadata() for e in read_cpio(archive)} != {e.name: e.metadata() for e in entries}:
        raise ValueError("Archive round-trip validation failed")
    destination = new_directory(destination)
    kernel_name = ARCHES[manifest["arch"]][1]
    files = {kernel_name: kernel_data, "umicom-rootfs.cpio.gz": archive, "source-manifest.json": source_data}
    for name, data in files.items():
        exclusive_file(destination / name, data)
    release = {"schema": 1, "arch": manifest["arch"], "source_id": source_id,
               "files": {name: {"size": len(data), "sha256": digest(data)} for name, data in files.items()},
               "rootfs": {e.name: e.metadata() for e in entries}, "boot_status": "not-run"}
    exclusive_file(destination / "image-manifest.json", json_bytes(release))
    return destination


def pack(args: argparse.Namespace) -> None:
    output = args.output.expanduser().resolve()
    manifest = input_manifest(output)
    check_config(output, manifest)
    kernel_dir = output / "build/build" / ("linux-" + manifest["versions"]["linux"])
    config = regular_bytes(kernel_dir / ".config").decode().splitlines()
    for required in ("CONFIG_BLK_DEV_INITRD=y", "CONFIG_RD_GZIP=y", "CONFIG_BINFMT_ELF=y",
                     "CONFIG_PROC_FS=y", "CONFIG_SYSFS=y", "CONFIG_TMPFS=y", "CONFIG_DEVTMPFS=y",
                     "CONFIG_SERIAL_8250_CONSOLE=y", "# CONFIG_MODULES is not set", "# CONFIG_NET is not set"):
        if required not in config:
            raise ValueError(f"Kernel configuration is missing: {required}")
    destination = args.bundle or output / "bundle"
    create_bundle(output / "build/target", output / "inputs/umicom-os/rootfs/foundation",
                  output / "build/images" / ARCHES[manifest["arch"]][1], manifest, destination)
    print("Boot bundle assembled. It has not been booted. Run verify, then boot.")


def verify_bundle(bundle: Path) -> dict:
    manifest = read_json(bundle / "image-manifest.json")
    if set(manifest) != {"schema", "arch", "source_id", "files", "rootfs", "boot_status"} or manifest["schema"] != 1 or manifest["arch"] not in ARCHES:
        raise ValueError("Unsupported image manifest")
    if manifest["boot_status"] != "not-run" or not re.fullmatch("[0-9a-f]{64}", manifest["source_id"]):
        raise ValueError("Image identity or build/boot separation is invalid")
    required = {ARCHES[manifest["arch"]][1], "umicom-rootfs.cpio.gz", "source-manifest.json"}
    if set(manifest["files"]) != required:
        raise ValueError("Unexpected bundle file list")
    for name in required:
        data = regular_bytes(bundle / name)
        if manifest["files"][name] != {"size": len(data), "sha256": digest(data)}:
            raise ValueError(f"Image file changed: {name}")
    if digest(regular_bytes(bundle / "source-manifest.json")) != manifest["source_id"]:
        raise ValueError("Source identity mismatch")
    source_manifest = read_json(bundle / "source-manifest.json")
    if source_manifest.get("arch") != manifest["arch"] or source_manifest.get("schema") != 1:
        raise ValueError("Source and image architecture differ")
    kernel_header(regular_bytes(bundle / ARCHES[manifest["arch"]][1]), manifest["arch"])
    entries = read_cpio(regular_bytes(bundle / "umicom-rootfs.cpio.gz"))
    actual = {entry.name: entry.metadata() for entry in entries}
    if actual != manifest["rootfs"]:
        raise ValueError("Root filesystem differs from its inventory")
    expected_names = set(DIRECTORIES) | set(PROGRAMS) | set(OVERLAY) | {
        "dev/console", "dev/null", "etc/umicom/source-id", "usr/share/umicom/packages.json"}
    if set(actual) != expected_names:
        raise ValueError("The diskless image contains unexpected or missing entries")
    for entry in entries:
        if entry.name in PROGRAMS:
            elf_static(entry.data, manifest["arch"])
            if entry.mode != stat.S_IFREG | 0o755:
                raise ValueError("A guest executable lost its intended permissions")
        elif entry.name in DIRECTORIES and entry.mode != stat.S_IFDIR | DIRECTORIES[entry.name]:
            raise ValueError("A guest directory has unexpected permissions")
        elif entry.name in OVERLAY:
            mode = 0o400 if entry.name == "etc/shadow" else 0o444
            if entry.mode != stat.S_IFREG | mode:
                raise ValueError("A guest configuration file has unexpected permissions")
        elif entry.name in ("etc/umicom/source-id", "usr/share/umicom/packages.json"):
            if entry.mode != stat.S_IFREG | 0o444:
                raise ValueError("Generated metadata has unexpected permissions")
        elif entry.name == "dev/console" and entry.mode != stat.S_IFCHR | 0o600:
            raise ValueError("Console device has unexpected permissions")
        elif entry.name == "dev/null" and entry.mode != stat.S_IFCHR | 0o666:
            raise ValueError("Null device has unexpected permissions")
        if entry.name == "etc/umicom/source-id" and entry.data != (manifest["source_id"] + "\n").encode():
            raise ValueError("Guest source identity mismatch")
    return manifest


def qemu_arguments(bundle: Path, manifest: dict, qemu: str, mode: str, interactive: bool = False) -> list[str]:
    arch = manifest["arch"]
    board = ["-M", "virt", "-bios", "default"] if arch == "riscv64" else ["-M", "q35"]
    commandline = "console=ttyS0 rdinit=/init panic=-1"
    commandline += " umicom.recovery=" + ("1" if mode == "recovery" else "0")
    commandline += " umicom.autopoweroff=" + ("0" if interactive else "1")
    return [qemu, *board, "-accel", "tcg", "-m", "256M", "-smp", "1", "-display", "none",
            "-monitor", "none", "-serial", "stdio", "-nic", "none", "-no-reboot",
            "-kernel", str(bundle / ARCHES[arch][1]), "-initrd", str(bundle / "umicom-rootfs.cpio.gz"),
            "-append", commandline]


def evaluate_boot(log: bytes, manifest: dict, mode: str, returncode: int) -> dict:
    clean = log.replace(b"\r\n", b"\n")
    if returncode != 0 or b"Kernel panic" in clean or b"UMICOM_INIT pid=1" not in clean or b"UMICOM_SHUTDOWN" not in clean:
        raise ValueError("The guest did not complete a clean init-to-poweroff run")
    reports = re.findall(rb"UMICOM_REPORT_BEGIN\n(.*?)UMICOM_REPORT_END", clean, re.S)
    if len(reports) != 1:
        raise ValueError("Expected exactly one final boot report")
    lines = reports[0].decode("ascii").splitlines()
    if not lines or lines[0] != "UMICOM_BOOT_REPORT 1":
        raise ValueError("Unrecognised guest report")
    fields: dict[str, str] = {}
    for line in lines[1:]:
        if "=" not in line:
            raise ValueError("Malformed report line")
        key, value = line.split("=", 1)
        if key in fields:
            raise ValueError("Duplicate report field")
        fields[key] = value
    expected = {"mode": mode, "state": "ready" if mode == "normal" else "recovery",
                "planned": "2", "completed": "2" if mode == "normal" else "0",
                "reason": "none" if mode == "normal" else "requested", "source": manifest["source_id"]}
    if fields != expected:
        raise ValueError(f"Unexpected guest state: {fields}")
    started = re.findall(rb"UMICOM_SERVICE start=([^\n]+)", clean)
    if started != ([b"platform-check", b"framework-probe"] if mode == "normal" else []):
        raise ValueError("Service launch evidence does not match the selected mode")
    return {"status": "passed", "mode": mode, "report": fields, "returncode": returncode}


def boot(args: argparse.Namespace) -> int:
    if sys.platform != "linux":
        raise ValueError("Run the boot harness in Linux or WSL")
    bundle = args.bundle.expanduser().resolve()
    manifest = verify_bundle(bundle)
    qemu = shutil.which(args.qemu or ARCHES[manifest["arch"]][2])
    if not qemu:
        print("NOT RUN: the required QEMU executable is unavailable. No boot success is recorded.", file=sys.stderr)
        return 77
    command = qemu_arguments(bundle, manifest, qemu, args.mode)
    if args.interactive:
        command = qemu_arguments(bundle, manifest, qemu, args.mode, True)
        print("No host disk or network device is attached. Type poweroff to leave the guest.")
        return subprocess.call(command)
    if not args.log:
        raise ValueError("Automated boot requires --log with a new transcript filename")
    log_path = args.log.expanduser().absolute()
    result_path = log_path.with_suffix(log_path.suffix + ".json")
    if result_path.exists() or result_path.is_symlink():
        raise ValueError("Use a new result filename")
    log_path.parent.mkdir(parents=True, exist_ok=True)
    transcript = bytearray()
    result: dict[str, Any] = {"status": "failed", "mode": args.mode, "arguments": command}
    with log_path.open("xb") as log:
        process = subprocess.Popen(command, stdin=subprocess.DEVNULL, stdout=subprocess.PIPE,
                                   stderr=subprocess.STDOUT, start_new_session=True)
        selector = selectors.DefaultSelector()
        try:
            assert process.stdout is not None
            selector.register(process.stdout, selectors.EVENT_READ)
            deadline = time.monotonic() + args.timeout
            while selector.get_map():
                if time.monotonic() >= deadline:
                    raise TimeoutError("QEMU boot deadline expired")
                for key, _ in selector.select(0.2):
                    data = os.read(key.fileobj.fileno(), 65536)
                    if not data:
                        selector.unregister(key.fileobj)
                        continue
                    if len(transcript) + len(data) > 4 * 1024 * 1024:
                        raise ValueError("QEMU transcript exceeded its limit")
                    transcript.extend(data)
                    log.write(data)
            code = process.wait(timeout=max(0.1, deadline - time.monotonic()))
            result.update(evaluate_boot(bytes(transcript), manifest, args.mode, code))
        except (TimeoutError, ValueError, subprocess.TimeoutExpired) as error:
            result["reason"] = str(error)
        finally:
            selector.close()
            if process.poll() is None:
                os.killpg(process.pid, signal.SIGKILL)
                process.wait(timeout=5)
            process.stdout.close()
    result["log_sha256"] = digest(bytes(transcript))
    exclusive_file(result_path, json_bytes(result))
    print(json.dumps(result, indent=2))
    return 0 if result["status"] == "passed" else 1


def legal(args: argparse.Namespace) -> None:
    output = args.output.expanduser().resolve()
    manifest = input_manifest(output)
    root = Path(read_json(output / "build-location.json")["buildroot"])
    check_buildroot(root, manifest["versions"])
    run(["make", "-C", str(root), "O=" + str(output / "build"), "legal-info"])
    print("Inspect Buildroot's legal-info output and all warnings before redistributing binaries.")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    p = sub.add_parser("prepare")
    p.add_argument("--buildroot", type=Path, required=True)
    p.add_argument("--framework", type=Path, required=True)
    p.add_argument("--output", type=Path, required=True)
    p.add_argument("--arch", choices=ARCHES, default="riscv64")
    for name in ("build", "pack", "legal-info"):
        p = sub.add_parser(name)
        p.add_argument("--output", type=Path, required=True)
        if name == "build":
            p.add_argument("--jobs", type=int, choices=range(1, 65), default=2)
        if name == "pack":
            p.add_argument("--bundle", type=Path)
    p = sub.add_parser("verify")
    p.add_argument("--bundle", type=Path, required=True)
    p = sub.add_parser("boot")
    p.add_argument("--bundle", type=Path, required=True)
    p.add_argument("--mode", choices=("normal", "recovery"), default="normal")
    p.add_argument("--qemu")
    p.add_argument("--log", type=Path)
    p.add_argument("--timeout", type=int, choices=range(5, 601), default=90)
    p.add_argument("--interactive", action="store_true")
    args = parser.parse_args()
    try:
        if args.command == "prepare": prepare(args)
        elif args.command == "build": build(args)
        elif args.command == "pack": pack(args)
        elif args.command == "legal-info": legal(args)
        elif args.command == "verify":
            manifest = verify_bundle(args.bundle.expanduser().resolve())
            print(f'Bundle integrity verified for {manifest["arch"]}. This does not prove that it boots.')
        elif args.command == "boot": return boot(args)
    except (OSError, ValueError, KeyError, TypeError, EOFError, subprocess.CalledProcessError) as error:
        print(f"Stopped: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
