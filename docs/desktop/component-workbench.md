# Arrange a workspace from shared components

Umicom Component Workbench is a host-side learning application. It uses the same
canonical semantic layout types as the rest of Framework, then displays a Notes
panel, a notebook list, fictional account rows and a status panel. Nothing in
this practice application saves a note, contacts a bank or starts an OS guest.

Build the command-line lesson and regression tests from a Linux terminal:

```sh
cmake -S /mnt/c/umicom/umicomOS/tools/component-workbench \
    -B "$HOME/umicom-builds/component-workbench" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DUMICOM_OS_FRAMEWORK_SOURCE=/mnt/c/umicom/Umicom-Applications/framework
cmake --build "$HOME/umicom-builds/component-workbench" --parallel 2
ctest --test-dir "$HOME/umicom-builds/component-workbench" --parallel 2 \
    --no-tests=error --output-on-failure
"$HOME/umicom-builds/component-workbench/bin/umicom-layout-example"
```

The default focused build is headless. Add `-DUMICOM_VIEWPORT_GTK4=ON` when
configuring on a host with actual GTK4 development files (4.10 or newer). This
setting fails configuration when GTK is unavailable; it does not build a
substitute GUI. Run `bin/umicom-component-workbench` in a graphical session.

Read `framework/docs/learning/arrange-a-workspace.html` for the complete lesson.
GTK/Windows execution was not performed in the Batch 30 delivery environment.
The headless geometry and dependency tests do not qualify a graphical guest.
