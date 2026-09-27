# Native build-log review on an OS development host

Sammy Hegab, Umicom Foundation — MIT

This host-side tool uses the same Framework review and parser as Studio. It does not boot a guest, change an image, create a database or start a compiler.

```sh
cmake -S /mnt/c/umicom/umicomOS/tools/build-review -B "$HOME/umicom-builds/build-review" -G Ninja \
  -DUMICOM_OS_FRAMEWORK_SOURCE=/mnt/c/umicom/Umicom-Applications/framework -DCMAKE_BUILD_TYPE=Release
cmake --build "$HOME/umicom-builds/build-review" --parallel 2
ctest --test-dir "$HOME/umicom-builds/build-review" --parallel 2 --no-tests=error --output-on-failure
"$HOME/umicom-builds/build-review/bin/umicom-build-review" --self-test
```

Use `umicom-build-review log <plain-text-file>` for an explicitly selected log. Exit zero means inspection completed; the operation described by the text remains unrecorded. See Framework's `docs/learning/read-build-results.html` for the public lesson and `docs/development/build-review-contract.md` for ownership and bounds.
