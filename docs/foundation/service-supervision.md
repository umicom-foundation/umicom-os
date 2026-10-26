# One-shot service launch ownership

The independent boot supervisor starts a packaged executable without a shell. The caller chooses a deadline, supplies a writable log descriptor and remains the exclusive owner of child waiting while the synchronous operation runs.

## What is checked before launch

1. The executable path and deadline satisfy the existing service contract.
2. The caller's `SIGCHLD` disposition does not explicitly ignore child signals and does not request `SA_NOCLDWAIT`.
3. The log descriptor is valid and writable.
4. A private close-on-exec log copy and nonblocking error pipe can be allocated.

Internal descriptors remain above the conventional input, output and error descriptors, even if those streams were closed by the caller. The child clears close-on-exec on standard input after redirection; `dup2(0, 0)` alone would leave it set.

The supervisor does not change the caller's signal policy or close its log descriptor. A read-only log is rejected with `EBADF`. Automatic-reaping policy is rejected with `EINVAL`. These are launch failures; no successful service result is recorded.

## Cleanup has an ownership precondition

The existing completion observation keeps the direct child unreaped while its group is terminated. If waiting fails, the supervisor reports lost ownership and does not signal the numeric PID or group. The caller must not reap that child concurrently or change its signal disposition during the call.

This remains a trusted packaged-service launcher. Process groups contain ordinary descendants, not hostile programs that deliberately detach. Lost ownership is an error requiring diagnosis, not a guarantee that every descendant was cleaned up.

The prior implementation and stdin redirection remain in disabled C blocks with the replacement reason. They are available for engineering review.

## Focused Linux-host checks

Use the independent foundation build and the existing documented compiler prerequisites:

```sh
cmake -S boot/foundation -B build/foundation-host -DBUILD_TESTING=ON
cmake --build build/foundation-host --target umicom-os-supervisor-admission-tests umicom-os-test-child
ctest --test-dir build/foundation-host -R '^os\.foundation\.supervisor_owner\.' --output-on-failure --no-tests=error
```

These authored checks cover eight scenarios: ignored child signals, automatic reaping, read-only logs, closed logs, closed standard streams, execution failure with closed streams, a log at descriptor 0, and continued parent access to its log. They require Linux and are not registered for a cross-compiled guest test run.

The test closes standard descriptors only inside a forked probe. It uses private temporary files and does not modify host users, mounts or `/run`. The presence of test source does not imply a successful run. Normal and recovery guest boots still need separate evidence.

Read [qualification layers](qualification-layers.md) before treating a host result as release evidence.
