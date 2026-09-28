# Component integration host

This directory composes the Framework-owned configuration laboratory. It does
not build a kernel, start a guest, connect to a broker or implement a second
trading service. The three source subsets are the same canonical C code used
by the existing tools.

```sh
cmake -S . -B "$HOME/umicom-builds/component-integration" -G Ninja \
  -DUMICOM_OS_FRAMEWORK_SOURCE=/path/to/Umicom-Applications/framework \
  -DCMAKE_BUILD_TYPE=Release
cmake --build "$HOME/umicom-builds/component-integration" --parallel 2
ctest --test-dir "$HOME/umicom-builds/component-integration" \
  --parallel 2 --no-tests=error --output-on-failure
```

Use an ordinary writable build directory outside the source checkout. Native
fixtures use new directories and retain logs. The Windows commands, complete
learning exercise and installation check are in Framework's
`docs/learning/build-integrated-components.html` and the batch build guide.
