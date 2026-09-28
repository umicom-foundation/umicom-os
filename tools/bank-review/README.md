# Native banking review host tools

This directory delegates to Framework's `examples/bank_review` build. It is a host-side C23 learning and regression entry, not a change to the guest OS, kernel or recovery environment. Use `UMICOM_OS_FRAMEWORK_SOURCE` to select the Framework checkout.

```sh
cmake -S /mnt/c/umicom/umicomOS/tools/bank-review -B "$HOME/umicom-builds/bank-review" -G Ninja -DCMAKE_BUILD_TYPE=Release -DUMICOM_OS_FRAMEWORK_SOURCE=/mnt/c/umicom/Umicom-Applications/framework
cmake --build "$HOME/umicom-builds/bank-review" --parallel 2
ctest --test-dir "$HOME/umicom-builds/bank-review" --parallel 2 --no-tests=error --output-on-failure
"$HOME/umicom-builds/bank-review/bin/umicom-bank-review" --self-test
```

The complete example creates a memory-only practice bank. No real money, remote account or operating-system setting is affected. See Framework's branded `docs/learning/bank-review.html` and `docs/development/bank-review.md`.
