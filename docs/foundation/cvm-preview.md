# Umicom C preview and the OS boot path

Umicom OS has two distinct activities: building native user-space tools and booting a Linux-based guest image. Running a tool on Windows does not boot Umicom OS.

The optional `tools/cvm-preview` project consumes the shared Framework source-preview service and an installed Umicom C SDK. It supports a small arithmetic program and prints its result. Select the Framework source with `UMICOM_OS_FRAMEWORK_SOURCE`; publish shared changes once and update clean dependency checkouts through Git.

The independent `boot/cvm-probe` project builds the arithmetic runtime from `UMICOM_C_SOURCE_DIR`. The probe accepts no paths, commands or network input. A native host execution expects `UMICOM_CVM_PROBE_READY value=42`. A cross build registers no host execution test.

## Moving from host to guest

A Linux guest requires a Linux-target binary. The Windows executable is not usable inside it. The static guest option `UMICOM_OS_CVM_STATIC` requires a Linux compiler target and a static C library for that target.

The probe is deliberately not inserted into the existing root-filesystem inventory or PID-1 service configuration. Installing it to a staging directory does not make it part of a boot image. The existing image inventory is restrictive; changing it requires reviewing the package manifest, dependency ordering, archive validation and guest acceptance together.

To understand the supported boot route, follow [Boot a small Umicom system](first-boot.html) and [the native image workflow](native-image-workflow.md). Keep normal boot, forced recovery and service-failure evidence separate. A built archive, a launched desktop and a guest boot are different observations.

Umicom Kernel's CVM profile is a separate freestanding RISC-V environment. Its ELF program uses the Kernel native EXIT interface rather than Linux system calls. Neither binary can be substituted for the other.
