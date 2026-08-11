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
