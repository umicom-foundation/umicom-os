# Umicom OS

Umicom OS is the Linux-based operating-system project for the Umicom ecosystem.

The Linux kernel and minimal recovery userland remain independent of Umicom
Framework. Normal Umicom OS user-space, the desktop shell, settings/control
applications, developer tooling and bundled Umicom applications depend on the
root-level `framework/` Git submodule.

## Canonical repository structure

```text
framework/                  root Git submodule -> umicom-framework
applications/os/            Umicom OS user-space application
kernel/                     future Linux source/configuration/patches
rootfs/                     future root filesystem profiles
packages/                   future Umicom package manifests
profiles/                   OS build/runtime profiles
toolchains/                 x86_64/RISC-V/CHERI toolchain definitions
tests/                      OS-level tests
```

## First headless build

Start with [Using Umicom OS and preparing a development environment](docs/foundation/installation-and-development.md)
for the distinction between the host application and the Linux-based QEMU image,
their build commands, recovery console, and current installation/account limits.

```powershell
cmake --preset headless-debug
cmake --build --preset headless-debug
ctest --preset headless-debug
```

## GTK4 desktop build

```powershell
cmake --preset gtk4-debug
cmake --build --preset gtk4-debug
ctest --preset gtk4-debug
```

The GTK4 desktop uses reusable Framework UI contracts and the Framework
`Umicom::ui_gtk4` implementation rather than creating a second OS-only widget
library.

## Author and Organisation

- Author: Sammy Hegab
- Organisation: Umicom Foundation
- Licence: MIT

House rule: commit directly to `main`. Keep source comments educational and
preserve the stable C ABI at Framework/product boundaries.

## Diskless foundation and recovery profile

The earlier user-space builds above remain unchanged. The new, independent
`boot/foundation` project supplies a small C23 PID-1 controller and a restricted
serial recovery console. Its kernel and recovery code do not depend on
Framework. A normal user-space probe uses the actual Framework Data Server.

Begin with [Boot a small Umicom system](docs/foundation/first-boot.html).
The source recipes select QEMU RISC-V `virt` first, with a separate x86-64
profile. They build a Linux kernel and a whitelisted RAM-only root filesystem.
There is no host-disk installer, login shell, network service or GUI in this
profile. Its two one-shot checks are not a replacement for a desktop service
manager.

The image build and the two real-QEMU boot modes are separate acceptance
steps. Native component tests and archive integrity checks do not prove that
a kernel boots. See [architecture and validation boundaries](docs/foundation/architecture.md).
Use the new standalone host test project, not the earlier root-level preset,
to test this boot layer independently of the legacy user-space composition.

## Native image workflow

The primary host image workflow is now the C23 `umicom-os-image` tool.
Build it through `tools/os-image` and read
[Build and check Umicom OS](docs/foundation/native-image-workflow.md).
The existing `tools/os_image.py` is retained unchanged as an alternative.
Native source snapshots and image metadata use their own strict `.umi` format;
archives describe the same diskless guest. A verified archive is not a boot pass.

The user-space GTK desktop composes Framework’s branded titlebar and searchable application catalogue. Its native launch activity shows process starts and exits; this is separate from bootable-image readiness. See [Opening applications and checking launch activity](framework/docs/learning/application-launch-activity.html).

## Installed Framework SDK for OS user space

The source-based commands above remain the default. An independent OS user-space
build can instead select a qualified installed SDK without compiling its
`framework/` submodule. This does not build or install a kernel, root filesystem
or boot image. Use a separate build directory and matching toolchain.

```powershell
cmake -S . -B build/sdk-debug -G Ninja -DUMICOM_OS_USE_INSTALLED_FRAMEWORK=ON -DUMICOM_SDK_ROOT=C:/SDKs/Umicom -DUMICOM_OS_GUI=OFF -DCMAKE_BUILD_TYPE=Debug
cmake --build build/sdk-debug
ctest --test-dir build/sdk-debug --output-on-failure
```

Stop on errors. The optional GUI requires installed `ui_gtk4`,
`desktop_workspace_gtk4` and `desktop_system_gtk4` components and current consumer
helpers. Consult `share/umicom/framework/docs/learning/sdk-product-composition.html`
in the SDK for the walkthrough and deployment limits. Configuration, linking,
execution and operating-system boot qualification are separate results.
