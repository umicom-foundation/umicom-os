# Umicom OS diskless foundation

Author: Sammy Hegab, Umicom Foundation. Licence: MIT.

## Ownership and scope

Linux and the minimal C23 boot/recovery controller remain in `umicom-os`.
Normal user-space services consume Framework contracts. This boundary lets a
failed or absent Framework probe leave the independent recovery console
available. No GTK, database library, network client or Framework header is
linked into `/init`. The normal probe uses the actual memory Data Server,
not a replacement implementation. OS Buildroot packaging compiles selected
Framework-owned source from a frozen source snapshot; it never copies that
implementation into the OS repository as a new owner.

The first profile is QEMU's RISC-V `virt` machine with its supplied OpenSBI.
An x86-64 `q35` profile is also provided. Both use direct kernel loading and an
initramfs, not a partitioned drive. The kernel is Linux 6.12.109, with its source
hash in the pinned Buildroot 2025.02.18 release. `image/versions.json` records
the exact Buildroot commit and kernel tarball hash. QEMU/OpenSBI comes from
the host distribution and is not pinned by this source package; preserve the
boot transcript and host QEMU version with any acceptance results.

## Control flow

The minimal Master Controller is `/init`: mount the guest's virtual filesystems,
validate the complete configuration and order the one-shot service checks.
Each service has its own narrow responsibility. Platform-check verifies the
unprivileged runtime environment; Framework-probe exercises Data Server commit
and rollback. The serial recovery console is independent of both services.

The complete service graph is parsed before execution. Up to 16 named services
are permitted; each has one optional predecessor, one packaged absolute
executable path and a timeout from 100 to 30,000 milliseconds. There are no
shell arguments, substitutions, arbitrary file paths or network lookups.
Missing predecessors, cycles, duplicate IDs and malformed settings fail before
a service is started. This is a bounded one-shot boot graph, not a general
long-running service manager or parallel scheduler.

A service runs in a private process group with an empty supplementary group
list, UID/GID 1000 and `no_new_privs`. Its standard input is `/dev/null` and its
output goes to a root-owned, bounded log. Inherited descriptors are closed.
Resource limits cover CPU, file size, descriptors, address space and per-UID
process count. The supervisor distinguishes an unsuccessful exit, signal,
exec failure, privilege failure and timeout. It kills ordinary descendants in
the group while retaining the direct child's PID until the cleanup finishes.
These controls are for trusted packaged programs: they do not constitute a
sandbox for hostile code that deliberately changes sessions or escapes groups.

The initramfs is RAM-backed. `/run` and `/tmp` are bounded tmpfs mounts with
no executable files. No physical disk or host directory is attached by the
runner. Root configuration is not writable by service UID 1000. There is no
persistent state volume, encrypted storage, package updater or disk installer.
Passwords are locked, no login shell is provided, and the recovery console
accepts only help, status, packages, a known service log, poweroff and reboot.
It is a development console, not authenticated administration.

## Recovery

`umicom.recovery=1` bypasses all normal services. An invalid graph or failed
normal service also selects recovery, retaining the reason and completed count.
No automatic restart, disk repair, rollback of an installed OS or external
network recovery is implemented. Power failure loses RAM-only logs and data;
capture the serial transcript on the host. `umicom.autopoweroff=1` is reserved
for automated checks; interactive runs leave the console available.

The boot report is written through an exclusively created temporary file and
an atomic rename in `/run/umicom`. No durable-write claim is made for tmpfs.
The source identity is a checksum, not authenticity or remote attestation.

## Build and image controls

`prepare` requires a clean, commit-pinned Buildroot tree and snapshots the
actual selected Framework and OS source bytes into a new output directory.
The manifest records commit IDs and dirty state when available. An uncommitted
merge is valid development input but is not labelled a clean release. Preparing
again requires a new output directory. Buildroot works in the Linux filesystem,
not in the Windows source tree. `build --jobs` controls Buildroot's package-level
parallelism with `BR2_JLEVEL`; the tool does not enable experimental top-level
parallel builds.

`pack` verifies frozen inputs and mandatory configurations. It builds a rootfs
from an explicit whitelist of three static programs, configuration, package
inventory, directories and only the initial console/null character devices.
There are no symlinks, hardlinks, set-ID files or block devices. The bounded
newc archive is generated without creating devices on the host; names,
permissions, sizes and hashes are checked before publication. Gzip metadata
and archive order are deterministic for identical program/configuration bytes.
This does not assert complete toolchain build reproducibility across hosts.

The source and image manifests are unsigned. Checks detect mismatches against
the accompanying inventory, not a malicious replacement of all files. Use
trusted source/output directories, avoid concurrent mutation and inspect the
build configuration. The runner uses a software-emulated CPU, no virtual NIC,
no attached disk and no shared directory. It does not sandbox vulnerabilities
in QEMU itself. Only start trusted images with a maintained QEMU installation.

## Validation boundaries

Native tests exercise parsing, actual child launch/deadlines, descriptor closure,
restricted console input, the report writer and the canonical Framework reader.
Format tests use explicitly labelled synthetic kernel/ELF headers and serial
transcripts. Those fixtures are not boot evidence. The independent GNU cpio
reader checks generated archives. The actual static native programs are built
separately against this host's glibc; that is not a musl cross-build.

The identity test uses a private chroot only when privileges allow it. A host
per-UID process quota can prevent exec after switching to UID 1000; that result
is skipped, not passed. The small chroot does not support an ASan runtime, so
that branch is also skipped in sanitiser builds. A real guest boot must still
prove the production privilege drop and mounted filesystem behaviour.

Real Buildroot configuration, musl/RISC-V cross-compilation, kernel execution,
normal boot and forced recovery require the separate local image acceptance
steps. They were not executed in the source-delivery environment. Never infer
success from successful archive verification or native test counts.

## Before redistributing a built image

Run `legal-info` and inspect every warning and the licence/source records. The
OS repository's MIT licence covers its original source, not Linux, Buildroot,
musl, QEMU or firmware. Preserve upstream notices and satisfy the applicable
licence terms for any binaries that are redistributed. No upstream kernel,
firmware, model or compiler binary is included in the source batch.

## Next stage

The next OS/Desktop work can add a normal user-space service manager, persistent
storage policy and Desk integration without turning the recovery controller
into a GTK application. A physical installer, verified updates, rollback,
credential-backed administration and broader hardware portability need separate
requirements and acceptance tests; none is silently enabled by this profile.
