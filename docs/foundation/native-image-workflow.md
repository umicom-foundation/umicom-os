# Build an image with the native Umicom tools

The primary host tool is now the C23 `umicom-os-image` executable supplied by Framework. The older `tools/os_image.py` is retained unchanged as an alternative. Their preparation manifests are different; start with a new native workspace instead of mixing their output directories.

Read the shared public lesson: [Build and check a small Umicom OS image](../../framework/docs/learning/build-and-check-umicom-os.html).

From a normal Linux or WSL account:

```bash
OS="/mnt/c/umicom/umicomOS"
FW="/mnt/c/umicom/Umicom-Applications/framework"
BUILD="$HOME/umicom-builds/os-image"
cmake -S "$OS/tools/os-image" -B "$BUILD" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release -DUMICOM_OS_FRAMEWORK_SOURCE="$FW"
cmake --build "$BUILD" --parallel 2
ctest --test-dir "$BUILD" --parallel 2 --no-tests=error --output-on-failure
"$BUILD/bin/umicom-os-image" --help
```

The host tool is reusable Framework code. This repository owns the native profile inventory, existing Buildroot packages, kernel configuration, guest PID 1 and recovery policy. This entry point does not create another OS-only copy of those host services.

Image preparation, kernel building, archive verification and normal/recovery guest tests are separate steps. Unit-test fixtures and a valid archive do not prove that the selected Linux kernel boots. BIOS/UEFI, installation to a physical disk and graphical desktop work remain separate qualification tasks.
