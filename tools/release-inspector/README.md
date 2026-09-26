# Inspect a Windows release using native Framework services

Sammy Hegab · Umicom Foundation · MIT

This is a thin host-tool composition. It does not build or boot Umicom OS,
write media, install a VM, or create an independent PE/installer implementation.

```sh
cmake -S /absolute/path/umicomOS/tools/release-inspector \
  -B "$HOME/umicom-builds/release-inspector" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DUMICOM_OS_FRAMEWORK_SOURCE=/absolute/path/Umicom-Applications/framework
cmake --build "$HOME/umicom-builds/release-inspector" --parallel 2
ctest --test-dir "$HOME/umicom-builds/release-inspector" --parallel 2 \
  --no-tests=error --output-on-failure
```

`bin/umicom-release-inspect release --root /absolute/path/to/release` reads a
native Setup Centre catalogue and Windows payload files. The result describes
static inspection only. Windows-system dependencies remain deferred even when
the host happens to be Windows; application startup needs its own test.

The public lesson is `docs/check-a-windows-release.html` in umicomOS. API and
ownership documentation lives in Framework's `docs/development/release-inspection.md`.
All previous setup, native launcher, image-building and recovery tools remain.
