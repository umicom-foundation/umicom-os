# Financial close review host

This directory delegates to Framework's native developer laboratory. It neither boots an OS image nor installs a banking service.

```bash
cmake -S . -B "$HOME/umicom-builds/finance-close-review" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DUMICOM_OS_FRAMEWORK_SOURCE=/absolute/path/to/Umicom-Applications/framework
cmake --build "$HOME/umicom-builds/finance-close-review" --parallel 2
ctest --test-dir "$HOME/umicom-builds/finance-close-review" --parallel 2 --no-tests=error --output-on-failure
```

Run each command only after the previous command succeeds. The command-line example uses a new memory-only book.
