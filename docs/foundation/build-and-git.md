# Build, test and publish the Umicom OS Foundation source

Author: Sammy Hegab, Umicom Foundation. Licence: MIT.

This batch has two destinations, not one replacement folder:

- `Umicom-Applications/framework` → `C:\umicom\Umicom-Applications\framework`
- `umicom-os` → the separate OS checkout, shown here as `C:\umicom\umicomOS`

The existing `applications\os` module in Umicom Applications is not modified.
Keep `Delivery-Evidence` and the delivery's top-level files outside the repos.
Merge the supplied full files without deleting destination-only files. Compare
against any newer or uncommitted work before accepting a whole-file replacement.

The Windows suite tests the shared report reader. Kernel and guest-image builds
run in Linux/WSL, not MSYS2. Native component tests do not prove a guest boots.
Do not proceed to publication while required local acceptance checks are failing.

## 1. Configure the Windows applications

```powershell
Set-Location "C:\umicom\Umicom-Applications"

$ErrorActionPreference = "Stop"
$env:Path = "C:\msys64\ucrt64\bin;$env:Path"

& "C:\msys64\ucrt64\bin\cmake.exe" `
    --preset windows-ucrt64-all-debug `
    -DBUILD_TESTING=ON `
    -DUMICOM_BUILD_NATIVE_TOOL=ON

if ($LASTEXITCODE -ne 0) {
    throw "Configuration failed. Stop here."
}
```

## 2. Full build and tests

```powershell
Set-Location "C:\umicom\Umicom-Applications"

$env:Path = "C:\msys64\ucrt64\bin;$env:Path"

& "C:\msys64\ucrt64\bin\cmake.exe" `
    --build `
    --preset windows-ucrt64-all-debug `
    --parallel 2 `
    -- -k 0

if ($LASTEXITCODE -ne 0) {
    throw "Full build failed. Do not proceed to tests or commits."
}
```

After the full build succeeds:

```powershell
& "C:\msys64\ucrt64\bin\ctest.exe" `
    --preset windows-ucrt64-all-debug `
    --parallel 2 `
    --no-tests=error `
    --output-on-failure

if ($LASTEXITCODE -ne 0) {
    throw "The full test suite failed. Review the failures before committing."
}
```

The following is an additional focused selection, not a replacement for the full suite:

```powershell
& "C:\msys64\ucrt64\bin\ctest.exe" `
    --preset windows-ucrt64-all-debug `
    -R '^framework\.os_foundation\.' `
    --parallel 2 `
    --no-tests=error `
    --output-on-failure

if ($LASTEXITCODE -ne 0) {
    throw "The OS boot-report regression tests failed."
}
```

The standalone Framework probe is deliberately not added to every application
build. Its independent CMake project is exercised in the Linux commands below.

## 3. Linux/WSL host checks

Open the existing Debian or Ubuntu WSL distribution. The public HTML guide
covers installation of prerequisites and the difference between host and guest.
Do not run these Bash commands in PowerShell or MSYS2.

```bash
set -eu
export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
OS="/mnt/c/umicom/umicomOS"
FW="/mnt/c/umicom/Umicom-Applications/framework"
BR="$HOME/src/buildroot-umicom"
RUN="$HOME/umicom-builds/riscv64-foundation"

test "$(id -u)" -ne 0 || { echo "Use your normal Linux account, not root."; exit 1; }
test -f "$OS/tools/os_image.py"
test -f "$FW/include/umicom/platform/boot_report.h"

cmake -S "$OS/boot/foundation" \
    -B "$HOME/umicom-checks/os-foundation" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DUMICOM_OS_FRAMEWORK_SOURCE="$FW"
cmake --build "$HOME/umicom-checks/os-foundation" --parallel 2
ctest --test-dir "$HOME/umicom-checks/os-foundation" \
    --parallel 2 --no-tests=error --output-on-failure

cmake -S "$FW/examples/os_foundation" \
    -B "$HOME/umicom-checks/framework-probe" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release
cmake --build "$HOME/umicom-checks/framework-probe" --parallel 2
ctest --test-dir "$HOME/umicom-checks/framework-probe" \
    --no-tests=error --output-on-failure
```

Inspect skips. The optional chroot identity test needs privileges and a suitable
host per-UID process budget. Its ASan variant is deliberately skipped. Do not
use sudo just to turn that optional case green; the real guest checks still
need to prove the production identity and filesystem behaviour.

## 4. Buildroot and QEMU acceptance

For Debian 13, install host dependencies in the Linux terminal. Ubuntu package
names vary; older Ubuntu releases use `qemu-system-misc` for RISC-V rather than
`qemu-system-riscv`. Check your distribution's package and the actual executable.

```bash
sudo apt update
sudo apt install build-essential git cmake ninja-build python3 file cpio \
    rsync unzip bc bzip2 gzip xz-utils wget curl patch perl \
    libncurses-dev libssl-dev libelf-dev flex bison \
    qemu-system-riscv qemu-system-x86 opensbi

command -v qemu-system-riscv64
qemu-system-riscv64 --version

mkdir -p "$HOME/src"
git clone --depth 1 --branch 2025.02.18 \
    https://github.com/buildroot/buildroot.git "$BR"
git -C "$BR" rev-parse HEAD
```

The expected dependency commit is `d030e36bbc9669230c015be971b14b6e062cfdde`.
Reuse a dependency checkout only if it is clean and at that exact commit. The
image helper enforces this. Do not discard local dependency changes to continue.

Run prepare/build without sudo. The output directory must be new and outside
the source/dependency directories; the captured inputs must not be edited.

```bash
python3 "$OS/tools/os_image.py" prepare \
    --buildroot "$BR" --framework "$FW" \
    --output "$RUN" --arch riscv64

python3 "$OS/tools/os_image.py" build --output "$RUN" --jobs 2
python3 "$OS/tools/os_image.py" pack --output "$RUN"
python3 "$OS/tools/os_image.py" verify --bundle "$RUN/bundle"

python3 "$OS/tools/os_image.py" boot --bundle "$RUN/bundle" \
    --mode normal --log "$RUN/normal-boot.log"
python3 "$OS/tools/os_image.py" boot --bundle "$RUN/bundle" \
    --mode recovery --log "$RUN/recovery-boot.log"
```

Both commands must actually boot QEMU and return zero for their acceptance
result to pass. Normal expects ready, two completed services and no failure.
Forced recovery expects zero completed services and reason requested. Exit 77
means QEMU was unavailable: NOT RUN, not passed. The archive's boot_status
stays not-run; the separate per-run JSON file is the execution result.

For interactive inspection:

```bash
python3 "$OS/tools/os_image.py" boot --bundle "$RUN/bundle" \
    --mode recovery --interactive
```

The guest commands are `help`, `status`, `packages`, `log platform-check`,
`log framework-probe`, `poweroff` and `reboot`. There is no shell, arbitrary
command execution, disk installer or service restart. A forced-recovery run
has no service logs because it has not launched those checks.

For x86-64, prepare another new RUN directory using `--arch x86_64`, and repeat
build/pack/verify and both boot modes. Do not relabel a RISC-V bundle as x86-64.
Before distributing built binaries, run and inspect:

```bash
python3 "$OS/tools/os_image.py" legal-info --output "$RUN"
```

Do not copy a compiled kernel, firmware or runtime into Git. Preserve the
upstream licence/source information, input/image manifests, QEMU version and
boot results with a release candidate. This source delivery contains no
third-party kernel or firmware binary and no claim of a completed guest boot.

## 5. Commit Framework first

Return to PowerShell. Review all changes: `git add -A` stages unrelated work as
well. Keep generated images, personal data and secrets out of the source commit.

```powershell
$ErrorActionPreference = "Stop"
Set-Location "C:\umicom\Umicom-Applications\framework"

if ((git branch --show-current) -ne "main") {
    throw "Framework is not on main. Review its branch state."
}

git status --short --branch
git diff --check
if ($LASTEXITCODE -ne 0) { throw "Resolve the reported formatting errors." }

git add -A
if ($LASTEXITCODE -ne 0) { throw "Framework staging failed." }
git diff --cached --stat

git commit -m "feat(framework): add bounded OS boot-report contract and user-space probe"
if ($LASTEXITCODE -ne 0) { throw "Framework commit failed." }

git push
if ($LASTEXITCODE -ne 0) { throw "Framework push failed. Stop before updating repository pins." }
```

## 6. Align the separate OS repository's Framework pin

The OS checkout is shown as `C:\umicom\umicomOS`. Change only `$OsRoot` below
when your existing checkout has another location. Do not copy the new Framework
source into that submodule as an independent change: fetch the Framework commit
that you just published.

This block refuses a dirty OS Framework checkout, a non-main branch or detached
work beyond the OS repository's currently recorded pin. A clean recorded
submodule checkout may switch from detached HEAD to the canonical main branch.
It never force-resets or creates a feature branch.

```powershell
$ErrorActionPreference = "Stop"
$OsRoot = "C:\umicom\umicomOS"
$ApplicationsFramework = "C:\umicom\Umicom-Applications\framework"
$OsFramework = Join-Path $OsRoot "framework"

$FrameworkCommit = git -C $ApplicationsFramework rev-parse HEAD
if ($LASTEXITCODE -ne 0) { throw "Could not read the published Framework commit." }

if ((git -C $OsRoot branch --show-current) -ne "main") {
    throw "The OS repository is not on main. Stop and review."
}

$Dirty = git -C $OsFramework status --porcelain
if ($LASTEXITCODE -ne 0) { throw "Could not inspect the OS Framework checkout." }
if ($Dirty) { throw "The OS Framework checkout has local changes. Preserve them before updating." }

$OsFrameworkHead = git -C $OsFramework rev-parse HEAD
if ($LASTEXITCODE -ne 0) { throw "Could not read the OS Framework HEAD." }
$RecordedPin = git -C $OsRoot rev-parse HEAD:framework
if ($LASTEXITCODE -ne 0) { throw "Could not read the OS Framework pin." }
$Branch = git -C $OsFramework branch --show-current
if ($LASTEXITCODE -ne 0) { throw "Could not inspect the OS Framework branch." }

if ($Branch -and $Branch -ne "main") {
    throw "The OS Framework checkout is on another branch. Stop and review."
}
if (-not $Branch -and $OsFrameworkHead -ne $RecordedPin) {
    throw "Detached Framework work differs from the OS pin. Preserve it before switching."
}

git -C $OsFramework fetch origin
if ($LASTEXITCODE -ne 0) { throw "OS Framework fetch failed." }

git -C $OsFramework switch main
if ($LASTEXITCODE -ne 0) { throw "Could not select Framework main. Stop and review." }

git -C $OsFramework merge --ff-only $FrameworkCommit
if ($LASTEXITCODE -ne 0) { throw "The OS Framework checkout cannot fast-forward. Do not reset it." }

$AlignedHead = git -C $OsFramework rev-parse HEAD
if ($LASTEXITCODE -ne 0) { throw "Could not verify the aligned Framework commit." }
if ($AlignedHead -ne $FrameworkCommit) {
    throw "OS Framework is not at the reviewed commit. Stop and reconcile the difference."
}

git -C $OsRoot diff --submodule=log
```

The earlier root-level OS user-space presets were not changed or validated by
this batch. The new boot project is independent of them. The Framework pin
alignment is explicit so later OS user-space work starts from the same reviewed
Framework source, rather than silently retaining the older checkout.

## 7. Commit the OS repository second

```powershell
Set-Location "C:\umicom\umicomOS"

if ((git branch --show-current) -ne "main") {
    throw "The OS repository is not on main. Review its branch state."
}

git status --short --branch
git diff --check
if ($LASTEXITCODE -ne 0) { throw "Resolve the reported formatting errors." }

git add -A
if ($LASTEXITCODE -ne 0) { throw "OS staging failed." }
git diff --cached --stat

git commit -m "feat(os): add diskless Linux foundation and independent recovery"
if ($LASTEXITCODE -ne 0) { throw "OS commit failed." }

git push --recurse-submodules=check origin main
if ($LASTEXITCODE -ne 0) { throw "OS push failed. Check the Framework push first." }

git submodule foreach --recursive 'git status --short --branch'
if ($LASTEXITCODE -ne 0) { throw "OS submodule inspection failed." }
```

## 8. Applications parent last

Only the Framework pin changes here; umicom-os is a separate repository, not a
new nested module in the Applications parent. Regenerate the lock after the
Framework commit has been pushed.

```powershell
$ErrorActionPreference = "Stop"
Set-Location "C:\umicom\Umicom-Applications"
$env:Path = "C:\msys64\ucrt64\bin;$env:Path"

if ((git branch --show-current) -ne "main") {
    throw "The parent repository is not on main. Review its branch state."
}

& ".\build\windows-ucrt64-all-debug\bin\umicom.exe" repo lock .
if ($LASTEXITCODE -ne 0) { throw "Repository locking failed. Do not commit stale pins." }

git diff --submodule=log
git diff --check
if ($LASTEXITCODE -ne 0) { throw "Resolve the reported formatting errors." }

git add -A
if ($LASTEXITCODE -ne 0) { throw "Parent staging failed." }
git diff --cached --stat

git commit -m "chore(applications): pin OS foundation report support"
if ($LASTEXITCODE -ne 0) { throw "Parent commit failed." }

git push --recurse-submodules=check origin main
if ($LASTEXITCODE -ne 0) { throw "Parent push failed. Check the Framework submodule push." }

git submodule foreach --recursive 'git status --short --branch'
if ($LASTEXITCODE -ne 0) { throw "Submodule status inspection failed." }
```

Publication order: **Framework → independent OS repository → Applications parent**.
No remote commit, push, OS installation or user-machine change was performed by
the source delivery.
