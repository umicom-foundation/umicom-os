# Market-tape host tools

This entry builds the Framework-owned observation laboratory. It neither adds a
market feed to the operating-system guest nor changes a boot profile.

```sh
cmake -S tools/market-tape -B "$HOME/umicom-builds/market-tape" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DUMICOM_OS_FRAMEWORK_SOURCE=/path/to/Umicom-Applications/framework
cmake --build "$HOME/umicom-builds/market-tape" --parallel 2
ctest --test-dir "$HOME/umicom-builds/market-tape" --no-tests=error --output-on-failure
```

Read the shared guide at `framework/docs/learning/market-tape.html`. The command
and C lesson use fictional data in memory. No broker, network, files or database
is opened by those examples. The optional GTK window is built only with its real
development dependency and needs a usable display for graphical tests.
