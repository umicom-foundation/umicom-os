# Native designer project host tools

Umicom OS · Sammy Hegab, Umicom Foundation · MIT

This composition builds the Framework native project exporter and its model checks. It does not build or boot an OS guest, change the kernel, install a desktop or enable physical-device writes. The shared implementation remains in the Framework checkout.

From Linux or WSL, with the two Windows checkout roots mounted under `/mnt/c/umicom`:

```sh
set -eu
cmake -S /mnt/c/umicom/umicomOS/tools/designer-project \
    -B "$HOME/umicom-builds/designer-project" -G Ninja \
    -DUMICOM_OS_FRAMEWORK_SOURCE=/mnt/c/umicom/Umicom-Applications/framework \
    -DCMAKE_BUILD_TYPE=Release -DUMICOM_DESIGNER_NATIVE_GTK4=OFF
cmake --build "$HOME/umicom-builds/designer-project" --parallel 2
ctest --test-dir "$HOME/umicom-builds/designer-project" --parallel 2 --no-tests=error --output-on-failure
"$HOME/umicom-builds/designer-project/bin/umicom-designer-project" --self-test
```

The public lesson is `framework/docs/learning/native-designer-projects.html`. A GUI build needs an actual GTK SDK and separate graphical acceptance. Generation always uses a new output directory; no existing project is overwritten.
