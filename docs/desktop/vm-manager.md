# Umicom OS in the shared Virtual Machine Manager

Sammy Hegab · Umicom Foundation · MIT

The reusable manager is in Framework. `tools/vm-manager` composes its native C23 libraries, CLI and (on Windows) GUI. Set `UMICOM_OS_FRAMEWORK_SOURCE` to the canonical Framework checkout when it is not this repository's root `framework` submodule.

Begin with Framework's `docs/learning/run-umicom-in-a-virtual-machine.html`. Run the memory-only profile lesson first. A real guest requires an explicitly prepared native QEMU runtime and a complete native OS-image bundle. Saving settings never starts a virtual machine. Starting creates a new run directory and connects while paused; Resume is separate.

The current manager supplies a serial text console, not a graphical OS desktop. The diskless foundation does not automatically gain persistent storage just because a blank disk is attached. QMP process state is not a guest service-health result. Keep the native image builder's normal and recovery boot checks as separate acceptance requirements.

The earlier Setup Centre QEMU workflow and script alternatives remain available and unchanged. This source delivery contains no QEMU distribution, firmware, prebuilt kernel, root filesystem or ISO. Optional runtime packaging and release qualification are explained in the public guide.


## Standalone guest images

The shared command also plans and runs standalone guests. In the VM tools use
the qemu subcommand with targets, plan, review or run. The main native umicom
command exposes the same service. The packaged-image workflow above remains
available with its runtime inventory and managed-disk requirements.

Read [Boot Kernel and Linux images](../../framework/docs/learning/boot-kernels-and-linux-with-qemu.html)
before selecting a target. The umicom-kernel target loads an explicitly supplied
umicom-system.elf as RV64 firmware with one CPU and ACLINT disabled. Linux targets
cover direct x86-64, RV64 and AArch64 kernel boots. BIOS-compatible x86 ISO and
raw-disk targets require serial-capable guests in this interface.

Boot profiles are local Data Server configuration records. A save does not start
a process, and a fresh review is needed before each start. Standalone raw disks
use temporary overlays; this workflow does not perform a persistent installation.
Graphics, additional boards and persistent boot-profile disks still need separate
integration and guest qualification.
