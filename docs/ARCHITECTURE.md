# Umicom OS architecture

The Linux kernel is the hardware/process/network/filesystem substrate. The
minimal recovery userland stays usable without Umicom Framework. The normal
Umicom OS user-space installs Framework as a core platform package.

The desktop shell is a Framework application. It consumes toolkit-neutral UI
components and explicitly links the GTK4 adapter for the desktop build.

No GTK types belong in OS models/controllers. GTK types remain in
`applications/os/src/gtk4/` and Framework `Umicom::ui_gtk4`.
