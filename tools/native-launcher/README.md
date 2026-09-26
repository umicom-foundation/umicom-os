# Native launcher delivery

Sammy Hegab, Umicom Foundation. MIT.

This build entry point composes Framework's C23 `umicom-session-stage` tool.
It contains no separate OS-local staging engine and starts no Python process.
The older `tools/desktop_session.py` wrapper remains an unchanged alternative.

From a normal Linux account, select the Framework checkout explicitly:

```sh
cmake -S /mnt/c/umicom/umicomOS/tools/native-launcher \
  -B "$HOME/umicom-builds/native-launcher" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DUMICOM_OS_FRAMEWORK_SOURCE=/mnt/c/umicom/Umicom-Applications/framework
cmake --build "$HOME/umicom-builds/native-launcher" --parallel 2
ctest --test-dir "$HOME/umicom-builds/native-launcher" \
  --parallel 2 --no-tests=error --output-on-failure
"$HOME/umicom-builds/native-launcher/bin/umicom-session-stage" --self-test
```

For staging, select an **already built Linux Desk installation** and the Linux
`umicom-desk-session` executable from the preceding desktop-system capability.
This focused build does not compile Desk, install a desktop, create autostart,
boot a guest or replace a compositor. Windows executables are not Linux inputs.

The `plan` operation does not create output. After review, the `stage` operation
accepts an optional `--expect-plan` SHA-256 fingerprint. It creates one new
private directory and writes the icon, desktop entry and manifest. `verify`
rechecks those files and the two referenced executables without launching them.
No operation overwrites an existing review directory. Keep partial failures for
inspection and use a new directory for a later attempt.

The source manifest format remains compatible with the earlier Python tool.
Native verification adds stricter path, ownership, inventory, PNG-envelope and
regenerated-command checks. Checksums establish byte agreement, not authorship,
trust, successful linking, a graphical-session result or OS-boot acceptance.

Follow [the public lesson](../../docs/native-launcher-delivery.html) for the
complete Windows digest and Linux staging exercises. The canonical engineering
rule is in Framework's `docs/development/native-implementation-policy.md`.
