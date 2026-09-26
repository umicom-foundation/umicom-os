# Native application-file maintenance

Sammy Hegab · Umicom Foundation · MIT

This host-side build composes the Framework C23 implementation. It does not
update an operating-system image, erase a drive or change an OS boot profile.
The complete public lesson is `docs/maintain-umicom-applications.html` in this OS
repository; the canonical copy belongs to Framework.

```sh
cmake -S /absolute/path/umicomOS/tools/setup-maintenance \
  -B "$HOME/umicom-builds/setup-maintenance" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DUMICOM_OS_FRAMEWORK_SOURCE=/absolute/path/Umicom-Applications/framework
cmake --build "$HOME/umicom-builds/setup-maintenance" --parallel 2
ctest --test-dir "$HOME/umicom-builds/setup-maintenance" \
  --parallel 2 --no-tests=error --output-on-failure
```

The primary tool is `bin/umicom-maintain`, implemented in C rather than invoking
an interpreter. Use `--help` and `--self-test` first. Review and apply are separate
operations. Use only a trusted local native-installer receipt and matching
release payload. Recover unfinished transactions before ordinary application
use. Original files and unknown personal files are not recursively deleted.
