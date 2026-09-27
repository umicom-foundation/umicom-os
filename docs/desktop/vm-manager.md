# Umicom OS in the shared Virtual Machine Manager

Sammy Hegab · Umicom Foundation · MIT

The reusable manager is in Framework. `tools/vm-manager` composes its native C23 libraries, CLI and (on Windows) GUI. Set `UMICOM_OS_FRAMEWORK_SOURCE` to the canonical Framework checkout when it is not this repository's root `framework` submodule.

Begin with Framework's `docs/learning/run-umicom-in-a-virtual-machine.html`. Run the memory-only profile lesson first. A real guest requires an explicitly prepared native QEMU runtime and a complete native OS-image bundle. Saving settings never starts a virtual machine. Starting creates a new run directory and connects while paused; Resume is separate.

The current manager supplies a serial text console, not a graphical OS desktop. The diskless foundation does not automatically gain persistent storage just because a blank disk is attached. QMP process state is not a guest service-health result. Keep the native image builder's normal and recovery boot checks as separate acceptance requirements.

The earlier Setup Centre QEMU workflow and script alternatives remain available and unchanged. This source delivery contains no QEMU distribution, firmware, prebuilt kernel, root filesystem or ISO. Optional runtime packaging and release qualification are explained in the public guide.
