# One-shot service launch ownership

![Umicom](../../framework/resources/brand/umicom-logo.svg)

[Umicom icon](../../framework/resources/brand/umicom-icon.svg) · [Windows application icon](../../framework/resources/brand/umicom.ico)

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

## Read the outcome and its supporting evidence

1. Start with the `UMICOM_SERVICE end=` line in the guest transcript. It gives the established outcome, exit code and signal. Exit code `-1` means a normal exit code was unavailable.
2. If `UMICOM_SERVICE_DIAGNOSTIC` follows, match its `id` to the service. The `errno` number is a Linux error reported by the launcher or supervisor. For example, a missing executable can yield `ENOENT` and a truncated launch record yields `EPROTO`. `elapsed_ms` measures the whole supervised operation when a valid monotonic sample is available. Zero can represent a sub-millisecond operation or an unavailable sample.
3. Check the boot report for the overall transition. The existing `service-launch` reason covers failures to launch or observe the service. `service-timeout` requires a valid monotonic clock observation and successful reaping. A child can finish with exit code zero while the supervisor records a launch failure if the error channel cannot be read reliably.

The launch pipe has a write descriptor that closes on successful `execve`. An empty end of file after the child has been reaped means no launch error was reported. Before treating that as a successful one-shot check, the supervisor also requires a normal zero exit. Application readiness would require a separate message from the current running generation. The boot report continues to use version 1.

## Focused Linux-host checks

Use the independent foundation build and the existing documented compiler prerequisites:

```sh
cmake -S boot/foundation -B build/foundation-host -DBUILD_TESTING=ON
cmake --build build/foundation-host --target umicom-os-supervisor-admission-tests umicom-os-test-child
ctest --test-dir build/foundation-host -R '^os\.foundation\.supervisor_owner\.' --output-on-failure --no-tests=error
```

These authored checks cover eight scenarios: ignored child signals, automatic reaping, read-only logs, closed logs, closed standard streams, execution failure with closed streams, a log at descriptor 0, and continued parent access to its log. They require Linux and are not registered for a cross-compiled guest test run.

On a 64-bit native Linux host with GCC or Clang, build and run the fault-injection tests using the same configured directory:

```sh
cmake --build build/foundation-host --target umicom-os-supervisor-evidence-tests
ctest --test-dir build/foundation-host -R '^os\.foundation\.supervisor_evidence\.' --output-on-failure --no-tests=error
```

Expect nine cases. The test driver passes a real child to the supervisor and replaces selected calls at link time to force errors that are difficult to reproduce on demand. If a case fails, use its name and printed assertion to identify the operation. `pipe-interrupt` should still observe a normal zero exit. `pipe-short` should preserve the observed exit code and report `EPROTO`. A `wait-reap` failure should keep the high-level outcome at `service-launch` even after the deadline elapsed.

The test closes standard descriptors only inside a forked probe. It uses private temporary files and does not modify host users, mounts or `/run`. The presence of test source does not imply a successful run. Normal and recovery guest boots still need separate evidence.

Read [qualification layers](qualification-layers.md) before treating a host result as release evidence.
