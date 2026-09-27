# Inspect and practise a media transfer

Umicom Foundation — Sammy Hegab — MIT

The implementation belongs to `Umicom::boot_media` in the root Framework submodule. This repository contributes composition, not another device writer.

From Linux/WSL, select the Applications Framework checkout explicitly while reviewing an uncommitted overlay:

```bash
cmake -S /mnt/c/umicom/umicomOS/tools/boot-media \
  -B "$HOME/umicom-builds/boot-media" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DUMICOM_OS_FRAMEWORK_SOURCE=/mnt/c/umicom/Umicom-Applications/framework \
  -DUMICOM_BOOT_MEDIA_ENABLE_DEVICE_WRITES=OFF
cmake --build "$HOME/umicom-builds/boot-media" --parallel 2
ctest --test-dir "$HOME/umicom-builds/boot-media" --parallel 2 --no-tests=error --output-on-failure
```

The programs appear in that build's `bin` directory. `umicom-media-image-example` is deliberately non-bootable test data. Do not transfer it to a physical device. `umicom-media inspect` observes a supported complete image, not a kernel/rootfs pair.

The experimental physical backend stays disabled pending platform/device qualification. This delivery does not burn optical discs, safely eject devices, or prove any Umicom OS boot. Native image construction, direct-kernel QEMU checks, ISO creation and physical-device boot testing are distinct operations.
