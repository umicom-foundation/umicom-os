# Native broker connection host tools

This is a thin host-build entry point into the canonical Framework implementation. It does not add a kernel service or broker code inside Umicom OS.

```sh
cmake -S . -B "$HOME/umicom-builds/broker-connect" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DUMICOM_OS_FRAMEWORK_SOURCE=/path/to/Umicom-Applications/framework
cmake --build "$HOME/umicom-builds/broker-connect" --parallel 2
ctest --test-dir "$HOME/umicom-builds/broker-connect" --no-tests=error --output-on-failure
```

Run `bin/umicom-broker-profile-example` for the memory-only lesson. Actual endpoint access is explicit and read-only. See the Framework `docs/learning/paper-live-connections.html` guide. A TWS process running on Windows is not automatically the loopback peer of a WSL/Linux process.
