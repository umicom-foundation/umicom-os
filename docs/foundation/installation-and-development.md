# Using Umicom OS and preparing a development environment

There are two current ways to explore this repository: a user-space application
on your existing system, and a small Linux-based image booted in QEMU. This
guide explains both. Neither is currently an installer for replacing your
computer's operating system.

## 1. Choose the right system

| Component | Runs where | Current purpose |
| --- | --- | --- |
| Umicom OS host application | Existing Windows/Linux host with its dependencies | Desktop/system-service development |
| Umicom OS Foundation image | QEMU RISC-V or x86-64 guest | Boot, two service probes and recovery inspection |
| Umicom Kernel | Separate repository, RISC-V QEMU guest | Native kernel development |

The Foundation image uses Linux. Umicom Kernel is a separate native kernel;
their executable formats, boot procedures and system interfaces are different.

```text
development host -> image tool -> Linux kernel + root filesystem archive
                                            -> QEMU -> /init -> service checks
                                                             -> recovery console
```

The **root filesystem** contains the first files available inside a guest.
`/init` is the first user-space process. The host's files are not automatically
the guest's files. A source repository is not itself an installable disk image.

## 2. Build the host application on Windows

Use the established MSYS2 UCRT64 toolchain and GTK4 development package for the
graphical preset. The Framework dependency must already be present and aligned
through the repository's normal Git workflow. Keep local changes when updating it.

Run each command separately, inspecting `$LASTEXITCODE` and stopping on errors:

```powershell
Set-Location "C:\umicom\umicomOS"
$env:Path = "C:\msys64\ucrt64\bin;$env:Path"
$LogRoot = "C:\umicom\_local\logs\umicom-os-host-$(Get-Date -Format yyyyMMdd-HHmmss)"
New-Item -ItemType Directory -Force $LogRoot
cmake --preset gtk4-debug 2>&1 | Tee-Object "$LogRoot\configure.log"
$LASTEXITCODE
cmake --build --preset gtk4-debug --parallel 2 2>&1 | Tee-Object "$LogRoot\build.log"
$LASTEXITCODE
ctest --preset gtk4-debug --output-on-failure --no-tests=error --output-log "$LogRoot\tests.log"
$LASTEXITCODE
```

The executable location depends on the Framework's output-directory settings.
Locate the result without guessing an installed path:

```powershell
Get-ChildItem ".\build\gtk4-debug" -Recurse -Filter "umicom-os-shell.exe" | Select-Object FullName
```

Launch the single executable from this build by using `&` followed by its quoted
full path. For example, if the listing shows `build\gtk4-debug\bin`:

```powershell
& ".\build\gtk4-debug\bin\umicom-os-shell.exe" 2>&1 | Tee-Object "$LogRoot\desktop-run.log"
```

This application runs on Windows; opening its window does not boot a new OS.
Use `headless-debug` instead for non-GUI host checks. Do not use the Applications
preset name in this repository; its committed presets are `gtk4-debug` and
`headless-debug`.

## 3. Prepare a Linux build host

Building the Foundation image requires Linux or an appropriate Linux environment
under WSL. WSL supplies a Linux development host on Windows. It is not Umicom OS.
Follow [Microsoft's WSL installation guide](https://learn.microsoft.com/en-us/windows/wsl/install)
if it is not installed. On first launch, complete the Linux distribution's
username/password setup. That creates a host account, not an Umicom guest account.

Keep source, tools, output and logs in locations you choose. Windows
`C:\umicom` is usually visible in WSL as `/mnt/c/umicom`. Buildroot output should
be on a suitable Linux filesystem with ordinary Unix permissions and names.
The following example uses `/work/umicom-builds`, a directory your Linux user
must own. Select an accessible alternative if it is not available; do not run
the image build as root merely to bypass permissions.

Install the host prerequisites described in the
[Buildroot manual](https://buildroot.org/downloads/manual/manual.html#requirement),
plus CMake, Ninja, a C23 compiler, Git and QEMU system emulators. Record their
versions. Native Windows MSYS2 builds and Linux image builds are different
environments; do not share their generated build directories.

## 4. Build the native image tool

In the Linux terminal, set paths to existing checkouts and a new build location:

```bash
set -eu
OS=/mnt/c/umicom/umicomOS
FW=/mnt/c/umicom/Umicom-Applications/framework
STAMP=$(date -u +%Y%m%dT%H%M%SZ)
BUILD=/work/umicom-builds/image-tool-$STAMP
cmake -S "$OS/tools/os-image" -B "$BUILD" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DUMICOM_OS_FRAMEWORK_SOURCE="$FW"
cmake --build "$BUILD" --parallel 2
ctest --test-dir "$BUILD" --output-on-failure --no-tests=error
TOOL="$BUILD/bin/umicom-os-image"
"$TOOL" profile --file "$OS/image/native/profile.umi"
```

`set -eu` stops this sequence after a failed command or an unset variable.
The profile names the dependency versions and selected source files. A profile
display is not a successful guest build. The reusable image engine lives in
Framework; the OS repository supplies boot policy and image recipes.

## 5. Prepare, build and inspect a guest bundle

Use a dedicated Buildroot checkout at the exact revision declared in
`image/versions.json` and `image/native/profile.umi`. The existing
[native image workflow](native-image-workflow.md) links the full preparation
lesson, including obtaining that dependency. Preserve its pinned revision.
Set `BR` to that clean checkout, then use a new output directory:

```bash
BR=/work/umicom-dependencies/buildroot
RUN=/work/umicom-builds/os-riscv64-$STAMP
"$TOOL" prepare --os "$OS" --framework "$FW" --buildroot "$BR" \
  --output "$RUN" --arch riscv64 --git /usr/bin/git
"$TOOL" configure --output "$RUN" --make /usr/bin/make
"$TOOL" build --output "$RUN" --make /usr/bin/make --jobs 2
"$TOOL" pack --output "$RUN" --bundle "$RUN/bundle"
"$TOOL" verify --bundle "$RUN/bundle"
```

Expected files include `Image`, `umicom-rootfs.cpio.gz`, `input-manifest.umi`
and `image.umi`. The manifest describes inputs and their digests. Verification
checks the bundle's bytes and structure; it does not prove it boots. A changed
input needs a fresh prepared workspace, because earlier inputs are preserved.

## 6. Boot and inspect the recovery console

Run the automatic checks first. Each result directory must be new:

```bash
"$TOOL" boot --bundle "$RUN/bundle" --qemu /usr/bin/qemu-system-riscv64 \
  --mode normal --result "$RUN/boot-normal" --timeout 90
"$TOOL" boot --bundle "$RUN/bundle" --qemu /usr/bin/qemu-system-riscv64 \
  --mode recovery --result "$RUN/boot-recovery" --timeout 90
```

Inspect `console.log` and `result.umi` in each result directory. Normal mode
expects two completed service checks; forced recovery expects neither to run.
A missing emulator or timeout is not a boot pass. These commands shut down
automatically; they are not interactive sessions.

To explore the same verified RISC-V bundle interactively, use the direct guest
command below. The image tool's automated boot acceptance is separate from
this manual session. No host disk or network interface is attached:

```bash
/usr/bin/qemu-system-riscv64 -M virt -bios default -accel tcg \
  -m 256M -smp 1 -display none -monitor none -serial stdio -nic none -no-reboot \
  -kernel "$RUN/bundle/Image" -initrd "$RUN/bundle/umicom-rootfs.cpio.gz" \
  -append 'console=ttyS0 rdinit=/init panic=-1 umicom.recovery=1 umicom.autopoweroff=0'
```

At `umicom>` enter `help`, `status`, `services`, `packages`, then `poweroff`.
`services` lists configured order; it does not assert that a service ran.
`log platform-check` reads the named service's log if this boot produced one.
Backspace/Delete can remove an accepted input byte; a damaged or overlong line
remains rejected until Enter. Arrow-key cursor editing is not implemented.

## 7. Create users, log in and build applications

Those are development milestones, not current Foundation commands. The image
has no user database, password authentication, login shell, package installer,
compiler or graphical session. `useradd`, `passwd`, `cmake` and a desktop IDE
cannot be used inside this recovery console. Do not enter credentials there.

Build applications on the development host today, using the Applications
repository's preset for that host. For Windows:

```powershell
Set-Location "C:\umicom\Umicom-Applications"
$env:Path = "C:\msys64\ucrt64\bin;$env:Path"
cmake --preset windows-ucrt64-all-debug -DBUILD_TESTING=ON -DUMICOM_BUILD_NATIVE_TOOL=ON -DUMICOM_APPLICATIONS_BUILD_ALL_MODULES=ON -DUMICOM_APPLICATIONS_BUILD_SHARED_GTK4=ON
cmake --build --preset windows-ucrt64-all-debug --parallel 2
ctest --preset windows-ucrt64-all-debug --output-on-failure --no-tests=error
```

Run one step at a time and retain its output in the same timestamped logging
pattern as section 2. Existing Windows executables cannot simply be copied
into a RISC-V Linux guest. Target architecture, executable format, libraries,
graphics services and permissions all have to match.

## 8. Burn a CD or create a bootable USB

The current bundle is for direct-kernel QEMU boot. It is **not an ISO or USB
disk image**. Renaming it or copying the archive to a USB stick will not make
it bootable. There is no supported physical installation procedure yet.

A future media release needs a bootloader for a named BIOS/UEFI or board,
partition/filesystem recipes, a documented image format, hardware drivers,
recovery and independent boot tests. Only then can the guide specify the image
writer and the exact device-selection steps. Burning an ISO writes its boot
structure; copying an ISO file as ordinary data does not do that. Writing a
disk image replaces data on the chosen device, so media instructions must be
specific to a qualified release and identified destination.

Use QEMU for the current edition. No instruction in this guide formats a drive,
changes firmware settings or disables Secure Boot.

## 9. Learn from the source

Read `boot/foundation/init.c` for startup, `config.c` for the accepted service
plan, `supervisor.c` for service execution and `console.c` for recovery commands.
`tests/foundation/test_console.py` drives the real console in an ordinary
process; its driver reports shutdown requests instead of powering off a host.
This separation makes editing tests practical without pretending they prove
the whole OS boots.

**Exercise:** in forced recovery, why can `services` list two services while
`status` reports zero completed? **Answer:** a configuration is a plan; recovery
deliberately did not execute it. Check `log SERVICE` for evidence from that boot,
not the existence of a configured name.
