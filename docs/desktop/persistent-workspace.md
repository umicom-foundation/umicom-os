# Persistent desktop workspaces

This update composes the shared Framework notebook and presentation preferences.
It is one user-space building block for the graphical/persistent desktop roadmap,
not a completed graphical operating-system image.

## Run on an existing desktop

Build the Applications checkout and open Umicom Desk. Its new **Open persistent
desktop workspace** entry opens the shared GTK window. Storage opens only after
**Open workspace**. The current user's application-data path is displayed.

The existing System Centre, saved application groups, taskbar, context strip and
Linux session handoff remain unchanged. There is no automatic execution of a
saved application or stored note.

## Focused native tools

Configure `desktop/workspace` with `UMICOM_OS_FRAMEWORK_SOURCE` pointing to the
updated Framework checkout. This builds the same native services and tests,
without copying the implementation into this repository.

```
cmake -S desktop/workspace -B build/desktop-workspace -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/desktop-workspace --parallel 2
ctest --test-dir build/desktop-workspace --parallel 2 --no-tests=error --output-on-failure
```

## Guest boundary

The existing diskless foundation still has a RAM-only runtime. Installing these
source files does not create persistent user storage, accounts, display-server
support, a graphical login, a bootable desktop image or system update/rollback.
A qualified graphical OS profile must supply a persistent volume and an existing
graphical session before this workspace can preserve user work across guest boots.

Use the public beginner guide at
`framework/docs/learning/desktop-workspace.html`. Keep physical-media writing
disabled while the separate media writer remains unqualified.

Sammy Hegab · Umicom Foundation · MIT
