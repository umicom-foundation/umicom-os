#!/usr/bin/env python3
"""Image-format and host-runner tests. No fixture below is a booted kernel.

Sammy Hegab, Umicom Foundation. Licence: MIT.
Synthetic ELF, kernel headers and transcripts deliberately isolate rejection
rules. The separate `boot` command is the only real-QEMU acceptance path.
"""
from __future__ import annotations
import argparse
import dataclasses
import gzip
import importlib.util
import io
import json
import os
from pathlib import Path
import shutil
import stat
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("os_image", ROOT / "tools/os_image.py")
image = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = image
spec.loader.exec_module(image)
D = stat.S_IFDIR
F = stat.S_IFREG
C = stat.S_IFCHR
IDENTITY = "a" * 64


def elf(arch="x86_64", kind=1, flags=5):
    """A structural header fixture: intentionally not runnable machine code."""
    data=bytearray(4096)
    data[:7]=b"\x7fELF\x02\x01\x01"
    struct.pack_into("<HHI",data,16,2,image.ARCHES[arch][0],1)
    struct.pack_into("<Q",data,24,0x400000)
    struct.pack_into("<Q",data,32,64)
    struct.pack_into("<HHH",data,52,64,56,1)
    struct.pack_into("<IIQQQQQQ",data,64,kind,flags,0,0x400000,0,4096,4096,4096)
    return bytes(data)


def kernel(arch="x86_64"):
    """Offset fixture from Linux's documented boot headers, not a kernel."""
    data=bytearray(4096)
    if arch=="x86_64":
        data[0x1fe:0x200]=b"\x55\xaa";data[0x202:0x206]=b"HdrS"
        struct.pack_into("<H",data,0x206,0x20F);struct.pack_into("<H",data,0x236,1)
    else:
        struct.pack_into("<Q",data,16,8192)
        data[48:56]=b"RISCV\0\0\0";data[56:60]=b"RSC\x05"
    return bytes(data)


def transcript(mode="normal"):
    starts=(b"UMICOM_SERVICE start=platform-check\nUMICOM_SERVICE start=framework-probe\n" if mode=="normal" else b"")
    return b"UMICOM_INIT pid=1\n"+starts+(
        "UMICOM_REPORT_BEGIN\nUMICOM_BOOT_REPORT 1\nmode="+mode+"\nstate="+
        ("ready" if mode=="normal" else "recovery")+"\nplanned=2\ncompleted="+
        ("2" if mode=="normal" else "0")+"\nreason="+("none" if mode=="normal" else "requested")+
        "\nsource="+IDENTITY+"\nUMICOM_REPORT_END\nUMICOM_SHUTDOWN\n").encode()


class ImageTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory(prefix="umicom-image-tests-")
        self.addCleanup(self.temp.cleanup)
        self.root=Path(self.temp.name)

    def entries(self):
        return [image.Entry("dev",D|0o755),image.Entry("dev/console",C|0o600,major=5,minor=1),
                image.Entry("etc",D|0o755),image.Entry("etc/notice",F|0o444,b"Umicom\n")]

    def bundle(self,arch="x86_64"):
        target=self.root/"target";target.mkdir()
        for name in image.PROGRAMS:
            file=target/name;file.parent.mkdir(parents=True,exist_ok=True);file.write_bytes(elf(arch))
        k=self.root/"kernel-fixture";k.write_bytes(kernel(arch))
        versions=image.read_json(ROOT/"image/versions.json")
        manifest={"schema":1,"arch":arch,"versions":versions,"os":{"commit":None,"dirty":None},
                  "framework":{"commit":None,"dirty":None},"files":{"fixture":image.digest(b"test")}}
        return image.create_bundle(target,ROOT/"rootfs/foundation",k,manifest,self.root/"bundle")

    def rewrite_rootfs(self,bundle,change):
        m=image.read_json(bundle/"image-manifest.json")
        entries=change(image.read_cpio((bundle/"umicom-rootfs.cpio.gz").read_bytes()))
        data=image.cpio(entries);(bundle/"umicom-rootfs.cpio.gz").write_bytes(data)
        m["files"]["umicom-rootfs.cpio.gz"]={"size":len(data),"sha256":image.digest(data)}
        m["rootfs"]={e.name:e.metadata() for e in entries}
        (bundle/"image-manifest.json").write_bytes(image.json_bytes(m))

    def test_archive_roundtrip(self):
        restored=image.read_cpio(image.cpio(self.entries()))
        self.assertEqual({e.name:e for e in restored},{e.name:e for e in self.entries()})

    def test_archive_deterministic(self):
        a=image.cpio(self.entries());b=image.cpio(list(reversed(self.entries())))
        self.assertEqual(a,b);self.assertEqual(a[4:8],b"\0"*4)

    def test_archive_gnu_reader(self):
        executable=shutil.which("cpio")
        if not executable:self.skipTest("GNU cpio is unavailable")
        result=subprocess.run([executable,"-it","--quiet"],input=gzip.decompress(image.cpio(self.entries())),capture_output=True,check=True)
        self.assertEqual(set(result.stdout.decode().splitlines()),{e.name for e in self.entries()})

    def test_archive_paths(self):
        for name in ("../etc","/etc","a/../etc","a//b","a\\b","a:b","a/./b","a/", ".", "TRAILER!!!", "", "a\nb"):
            with self.subTest(name=name),self.assertRaises(ValueError):image.relative_name(name)

    def test_archive_duplicate(self):
        with self.assertRaises(ValueError):image.cpio(self.entries()+[self.entries()[0]])

    def test_archive_parents(self):
        for entries in ([image.Entry("a/b",F|0o644)], [image.Entry("a",F|0o644),image.Entry("a/b",F|0o644)]):
            with self.assertRaises(ValueError):image.cpio(entries)

    def test_archive_file_types(self):
        for mode in (stat.S_IFLNK|0o777,stat.S_IFBLK|0o600,stat.S_IFIFO|0o600,F|0o4755,F|0o2755):
            with self.assertRaises(ValueError):image.cpio([image.Entry("x",mode)])

    def test_archive_devices(self):
        with self.assertRaises(ValueError):image.cpio([image.Entry("device",C|0o600,major=8,minor=0)])
        with self.assertRaises(ValueError):image.cpio([image.Entry("file",F|0o644,major=1)])

    def test_archive_nonfile_data(self):
        with self.assertRaises(ValueError):image.cpio([image.Entry("a",D|0o755,b"x")])

    def test_archive_budgets(self):
        with mock.patch.object(image,"MAX_FILE",2),self.assertRaises(ValueError):image.cpio([image.Entry("a",F|0o644,b"123")])
        compressed=image.cpio([])
        with mock.patch.object(image,"MAX_ARCHIVE",128),self.assertRaises(ValueError):image.read_cpio(compressed)
        with self.assertRaises(ValueError):image.cpio([image.Entry("x"+str(i),F|0o444) for i in range(129)])

    def test_archive_truncation(self):
        raw=gzip.decompress(image.cpio(self.entries()))
        for length in (0,5,109,110,111,250):
            with self.subTest(length=length),self.assertRaises((ValueError,EOFError)):image.read_cpio(gzip.compress(raw[:length]))

    def test_archive_metadata(self):
        raw=gzip.decompress(image.cpio(self.entries()))
        for index,value in ((2,1),(3,1),(4,7),(5,123),(12,1)):
            changed=bytearray(raw);changed[6+index*8:14+index*8]=f"{value:08x}".encode()
            with self.assertRaises(ValueError):image.read_cpio(gzip.compress(changed))

    def test_archive_padding(self):
        raw=bytearray(gzip.decompress(image.cpio(self.entries())))
        raw[114]=1  # first entry is 'dev': 110 + four name bytes, then padding
        with self.assertRaises(ValueError):image.read_cpio(gzip.compress(raw))

    def test_elf_architecture(self):
        image.elf_static(elf(),"x86_64");image.elf_static(elf("riscv64"),"riscv64")
        with self.assertRaises(ValueError):image.elf_static(elf(),"riscv64")

    def test_elf_dynamic(self):
        for kind in (2,3):
            with self.assertRaises(ValueError):image.elf_static(elf(kind=kind),"x86_64")

    def test_elf_stack(self):
        with self.assertRaises(ValueError):image.elf_static(elf(kind=0x6474e551,flags=7),"x86_64")

    def test_elf_bounds(self):
        for data in (elf()[:70],b"bad",elf()[::-1]):
            with self.assertRaises(ValueError):image.elf_static(data,"x86_64")
        data=bytearray(elf());struct.pack_into("<Q",data,64+32,9000)
        with self.assertRaises(ValueError):image.elf_static(data,"x86_64")
        data=bytearray(elf());struct.pack_into("<Q",data,24,0x500000)
        with self.assertRaises(ValueError):image.elf_static(data,"x86_64")

    def test_kernel_riscv_offsets(self):
        image.kernel_header(kernel("riscv64"),"riscv64")
        data=bytearray(kernel("riscv64"));data[48:64]=b"\0"*16;data[56:64]=b"RISCV\0\0\0"
        with self.assertRaises(ValueError):image.kernel_header(data,"riscv64")

    def test_kernel_endian_and_magic(self):
        for offset in (24,48,56):
            data=bytearray(kernel("riscv64"));data[offset]^=1
            with self.assertRaises(ValueError):image.kernel_header(data,"riscv64")

    def test_kernel_x86(self):
        image.kernel_header(kernel(),"x86_64")
        for data in (kernel()[:1024],kernel("riscv64"),b"bad"):
            with self.assertRaises(ValueError):image.kernel_header(data,"x86_64")

    def test_bundle_roundtrip(self):
        bundle=self.bundle();m=image.verify_bundle(bundle)
        self.assertEqual(m["boot_status"],"not-run")
        self.assertEqual(set(m["rootfs"]),set(image.DIRECTORIES)|set(image.PROGRAMS)|set(image.OVERLAY)|
                         {"dev/console","dev/null","etc/umicom/source-id","usr/share/umicom/packages.json"})

    def test_bundle_file_corruption(self):
        bundle=self.bundle();(bundle/"bzImage").write_bytes(kernel()+b"changed")
        with self.assertRaises(ValueError):image.verify_bundle(bundle)

    def test_bundle_unknown_file(self):
        bundle=self.bundle();self.rewrite_rootfs(bundle,lambda es:es+[image.Entry("unapproved",F|0o444,b"bad")])
        with self.assertRaises(ValueError):image.verify_bundle(bundle)

    def test_bundle_permissions(self):
        bundle=self.bundle();self.rewrite_rootfs(bundle,lambda es:[dataclasses.replace(e,mode=F|0o666) if e.name=="etc/umicom/boot.conf" else e for e in es])
        with self.assertRaises(ValueError):image.verify_bundle(bundle)

    def test_bundle_metadata_permissions(self):
        bundle=self.bundle();self.rewrite_rootfs(bundle,lambda es:[dataclasses.replace(e,mode=F|0o666) if e.name=="etc/umicom/source-id" else e for e in es])
        with self.assertRaises(ValueError):image.verify_bundle(bundle)

    def test_bundle_devices_permissions(self):
        bundle=self.bundle();self.rewrite_rootfs(bundle,lambda es:[dataclasses.replace(e,mode=C|0o666) if e.name=="dev/console" else e for e in es])
        with self.assertRaises(ValueError):image.verify_bundle(bundle)

    def test_bundle_guest_identity(self):
        bundle=self.bundle();self.rewrite_rootfs(bundle,lambda es:[dataclasses.replace(e,data=b"0"*64+b"\n") if e.name=="etc/umicom/source-id" else e for e in es])
        with self.assertRaises(ValueError):image.verify_bundle(bundle)

    def test_bundle_not_boot_evidence(self):
        bundle=self.bundle();m=image.read_json(bundle/"image-manifest.json");m["boot_status"]="passed"
        (bundle/"image-manifest.json").write_bytes(image.json_bytes(m))
        with self.assertRaises(ValueError):image.verify_bundle(bundle)

    def test_json_duplicates(self):
        f=self.root/"data.json";f.write_text('{"schema":1,"schema":1}')
        with self.assertRaises(ValueError):image.read_json(f)

    def test_no_overwrite(self):
        target=self.root/"protected";target.write_bytes(b"original")
        with self.assertRaises(FileExistsError):image.exclusive_file(target,b"replacement")
        with self.assertRaises(ValueError):image.new_directory(self.root)
        self.assertEqual(target.read_bytes(),b"original")

    def test_source_symlinks(self):
        f=self.root/"file";f.write_bytes(b"a");(self.root/"alias").symlink_to(f)
        with self.assertRaises(ValueError):image.source_bytes(self.root,"alias")
        (self.root/"nested").symlink_to(self.root,target_is_directory=True)
        with self.assertRaises(ValueError):image.source_bytes(self.root,"nested/file")

    def test_input_mutation(self):
        folder=self.root/"inputs/framework";folder.mkdir(parents=True);(folder/"file.c").write_bytes(b"initial")
        m={"schema":1,"arch":"riscv64","versions":{},"os":{},"framework":{},"files":{"framework/file.c":image.digest(b"initial")}}
        (self.root/"input-manifest.json").write_bytes(image.json_bytes(m));image.input_manifest(self.root)
        (folder/"file.c").write_bytes(b"changed")
        with self.assertRaises(ValueError):image.input_manifest(self.root)

    def test_input_manifest_schema(self):
        (self.root/"input-manifest.json").write_text('{"files":{}}')
        with self.assertRaises(ValueError):image.input_manifest(self.root)

    def test_declared_os_sources(self):
        names=image.read_json(ROOT/"image/os-files.json")
        self.assertLessEqual(len(names),128)
        self.assertEqual(len(names),len(set(names)))
        self.assertIn("kernel/foundation.config",names)
        for name in names:
            self.assertIsInstance(image.source_bytes(ROOT,name),bytes)
        self.assertFalse(any(name.startswith("kernel/linux/") for name in names))

    def test_qemu_isolation_arguments(self):
        for arch in image.ARCHES:
            args=image.qemu_arguments(self.root,{"arch":arch},"qemu","normal")
            self.assertEqual(args[args.index("-nic")+1],"none")
            self.assertEqual(args[args.index("-accel")+1],"tcg")
            self.assertTrue({"-drive","-blockdev","-hda","-fsdev","-virtfs","-netdev"}.isdisjoint(args))
            self.assertIn("umicom.autopoweroff=1",args[-1])

    def test_qemu_interactive(self):
        args=image.qemu_arguments(self.root,{"arch":"riscv64"},"qemu","recovery",True)
        self.assertIn("umicom.recovery=1",args[-1]);self.assertIn("umicom.autopoweroff=0",args[-1])

    def test_evidence_normal_fixture(self):
        result=image.evaluate_boot(transcript(),{"source_id":IDENTITY},"normal",0)
        self.assertEqual(result["status"],"passed")

    def test_evidence_recovery_fixture(self):
        result=image.evaluate_boot(transcript("recovery"),{"source_id":IDENTITY},"recovery",0)
        self.assertEqual(result["status"],"passed")

    def test_evidence_nonzero_exit(self):
        with self.assertRaises(ValueError):image.evaluate_boot(transcript(),{"source_id":IDENTITY},"normal",1)

    def test_evidence_missing_stages(self):
        for marker in (b"UMICOM_INIT pid=1",b"UMICOM_SHUTDOWN",b"UMICOM_SERVICE start=platform-check"):
            with self.assertRaises(ValueError):image.evaluate_boot(transcript().replace(marker,b"missing"),{"source_id":IDENTITY},"normal",0)

    def test_evidence_wrong_identity(self):
        with self.assertRaises(ValueError):image.evaluate_boot(transcript(),{"source_id":"b"*64},"normal",0)

    def test_evidence_duplicates_and_panic(self):
        for log in (transcript()+transcript(),transcript()+b"Kernel panic",transcript().replace(b"planned=2",b"planned=2\nplanned=2")):
            with self.assertRaises(ValueError):image.evaluate_boot(log,{"source_id":IDENTITY},"normal",0)

    def test_evidence_no_services_in_recovery(self):
        with self.assertRaises(ValueError):image.evaluate_boot(b"UMICOM_SERVICE start=framework-probe\n"+transcript("recovery"),{"source_id":IDENTITY},"recovery",0)

    def test_missing_qemu_not_pass(self):
        args=argparse.Namespace(bundle=self.bundle(),qemu="definitely-no-qemu",mode="normal",interactive=False,log=self.root/"boot.log",timeout=5)
        with mock.patch.object(image.shutil,"which",return_value=None):self.assertEqual(image.boot(args),77)
        self.assertFalse(args.log.exists())

    def test_runner_deadline_fixture(self):
        # A sleeping Python program tests the host timeout, not a virtual CPU.
        executable=self.root/"runner-fixture";executable.write_text("#!/usr/bin/env python3\nimport time\ntime.sleep(10)\n");executable.chmod(0o755)
        args=argparse.Namespace(bundle=self.bundle(),qemu=str(executable),mode="normal",interactive=False,log=self.root/"boot.log",timeout=0.15)
        with mock.patch.object(image.shutil,"which",return_value=str(executable)):
            self.assertEqual(image.boot(args),1)
        self.assertEqual(image.read_json(args.log.with_suffix(".log.json"))["status"],"failed")


if __name__=="__main__":
    if len(sys.argv)==2 and sys.argv[1]=="--list":
        print("\n".join(unittest.defaultTestLoader.getTestCaseNames(ImageTests)))
    else:
        unittest.main(verbosity=2)
