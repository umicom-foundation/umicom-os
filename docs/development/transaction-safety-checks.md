# Check shared transaction behaviour from the OS checkout

The `tools/data-safety` entry builds the canonical Framework host project. It does not install a second Data Server or modify a guest. Select `UMICOM_OS_FRAMEWORK_SOURCE` explicitly when using the Applications Framework checkout.

```bash
cmake -S tools/data-safety -B "$HOME/umicom-builds/data-safety" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DUMICOM_OS_FRAMEWORK_SOURCE=/absolute/path/to/Umicom-Applications/framework
cmake --build "$HOME/umicom-builds/data-safety" --parallel 2
ctest --test-dir "$HOME/umicom-builds/data-safety" --no-tests=error --output-on-failure
```

The public lesson is `framework/docs/learning/save-a-complete-change.html`; its complete C example is `framework/examples/data_safety/notes_transaction.c`. Existing native and Python image source inventories already include the changed Data Server header and source, so no inventory entry is added. Prepare a new image workspace when rebuilding changed bytes.
