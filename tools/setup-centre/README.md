# Umicom Setup Centre

Sammy Hegab, Umicom Foundation. MIT.

This is a thin build entry point. Framework owns the installer library, Windows
bootstrap, package parser, QEMU launch plans and media-source preparation.

```sh
cmake -S /absolute/path/umicomOS/tools/setup-centre \
  -B "$HOME/umicom-builds/setup-centre" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DUMICOM_OS_FRAMEWORK_SOURCE=/absolute/path/Umicom-Applications/framework \
  -DUMICOM_SETUP_CANONICAL_PROCESS=ON
cmake --build "$HOME/umicom-builds/setup-centre" --parallel 2
ctest --test-dir "$HOME/umicom-builds/setup-centre" --parallel 2 --no-tests=error --output-on-failure
```

Read `docs/install-and-try-umicom.html` in this repository. Windows suite packaging
uses the existing Applications runtime catalogue and the native
`umicom-native-installer` target. The Linux focused build is not a Windows
cross-build or an OS-image build. QEMU and GRUB are separate native providers;
no virtual machine, kernel, initramfs, USB writer or optical burner is included.

Old OS-image and session scripts remain unchanged alternatives. The new
installer and VM/media orchestration code is C23 and does not call Python.
