# Qualify the OS in separate layers

![Umicom](../../framework/resources/brand/umicom-logo.svg)

[Umicom application icon](../../framework/resources/brand/umicom.ico)

The OS repository contains two different builds. The root CMake project checks
Framework-backed user-space applications. `boot/foundation` builds the independent
PID1, configuration parser, service supervisor and recovery console. A successful
user-space build does not establish that PID1 or an operating-system image works.

1. Check the `OS user-space contracts` and `Independent PID1 and recovery contracts`
   jobs separately in the repository's Actions page.
2. Download their evidence artifacts, including failed runs. Read configure and
   build logs before interpreting test failures. A test executable that was never
   built cannot provide runtime evidence.
3. Read the recorded OS and Framework revisions. Reproduce a failure against those
   sources before deciding whether a later change fixed it.
4. Inspect skipped tests in the JUnit report and CTest log. A green job does not
   turn skipped tests into passes. The foundation identity transition runs
   separately with the privileges its private chroot needs; exit 77 fails that
   step rather than silently providing identity coverage.
5. After host checks, separately build and boot normal and recovery images using
   the [native image workflow](native-image-workflow.md). These CI jobs do not
   boot QEMU, install an OS or certify hardware support.

## Run just the foundation

Use an ordinary Linux account or WSL with a native Linux compiler. From the OS
repository root:

```sh
cmake -S boot/foundation -B build/foundation -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DUMICOM_OS_FRAMEWORK_SOURCE="$PWD/framework"
cmake --build build/foundation --parallel 2
ctest --test-dir build/foundation --output-on-failure --no-tests=error --output-log foundation-tests.log
```

Stop if configuration or compilation fails. Most cases are unprivileged. The
identity case reports a skip without the required privileges; keep that result
visible rather than disabling the test or treating it as a pass. CI isolates
that case into an explicitly privileged step.

The existing Python image-format and console checks remain in this independent
suite. The production boot programs are C23. Test transport, archive validation,
guest boot and production release approval are different evidence categories.

Artifacts are retained according to GitHub repository policy. They contain logs
from the checked-out public source, not a copy of a developer's private workspace.

## Inspect the accepted service plan

In the guest recovery console, type `services`. It lists the accepted dependency
order, executable paths and deadlines. This is a configuration view: use `status`
and `log SERVICE` for outcomes from the current boot. Forced recovery can show a
valid plan even though it deliberately started none of those services. If the
configuration was rejected, the console reports that no accepted plan exists.

Before booting, the host `umicom-os-boot-plan` tool can inspect the same grammar.
Pass a configuration file and optionally `--cmdline "umicom.recovery=1"`.
A rejected declaration reports its line on standard error; whole-plan errors
such as cycles use a general diagnostic. Neither inspection surface starts a
service. Host admission checks also verify that requested recovery does not
hide failed prerequisites such as hostname application.

## Inspect a captured guest report from the OS application

The normal `umicom-os` command keeps its existing user-space startup check. Pass
`--boot-report` to inspect an explicitly selected captured report instead:

```powershell
& "C:\umicom\installed\os\bin\umicom-os.exe" --boot-report "C:\umicom\_local\os-evidence\boot.report"
```

Use the path where you actually installed the OS program. The input is the
`UMICOM_BOOT_REPORT 1` record itself, such as the guest's `/run/umicom/boot.report`,
not an entire serial transcript containing surrounding markers. The command
shows mode, state, completed/planned services, reason and source fingerprint.
An optional `--source` followed by the expected 64-character lowercase fingerprint
rejects a report from a different captured source set.

The Framework reader accepts a bounded regular file through the shared native
file API. It does not modify the file, start the application lifecycle, launch a
guest or require administrative rights. Malformed, oversized or mismatching input
returns failure without printing an accepted report. A well-formed report may
still describe a failed boot; successful inspection is not successful booting.
A captured report is not proof of current system health or publisher identity.

### Verify a report copy by its exact bytes

1. When you first review a captured report, record the bytes' SHA-256 using
   `Get-FileHash -Algorithm SHA256` in PowerShell or `sha256sum` on Linux.
   Keep that value with your reviewed evidence. Store it as 64 lowercase
   hexadecimal characters.
2. When reviewing a copy, pass the saved value in `$reviewedCaptureSha256`.
   If you also recorded the source manifest identity, add `--source` and its
   lowercase SHA-256 value.

```powershell
& "C:\umicom\installed\os\bin\umicom-os.exe" --boot-report-identity `
    "C:\umicom\_local\os-evidence\boot.report" --capture $reviewedCaptureSha256
```

3. Confirm the displayed `capture`, `source`, `bytes` and recorded `state`.
   The `capture` value covers the complete file. A change to the field order
   changes that value even if the recorded fields still compare equal.
4. A process exit code of 2 indicates malformed command options. Exit code 1
   with `status=2` means one of the supplied identities did not match. Retrieve
   the reviewed capture before updating saved evidence. An older Framework
   package may report that capture identity is unavailable. Its existing report
   readers remain available until the selected SDK includes the feature.

## Check the packaged service environment

The guest's platform-check service measures its real and effective user/group
IDs, supplementary group count, no-new-privileges setting, soft and hard resource
limits, private work directory and proc/sys/run/tmp mounts. It refuses an
incomplete observation. Each runtime mount must have its expected filesystem
type and the minimal image's nosuid, nodev and noexec restrictions.

This check complements the supervisor: configuring a restriction and observing
it from the child are separate responsibilities. A failed check exits the
service, so the existing startup controller records a service failure and enters
recovery. Its service log explains which environment property was rejected.

The independent host suite includes `os.foundation.guest.*` policy cases.
They supply synthetic observations and never change host identities or mounts.
The existing Linux supervisor identity test separately exercises real credential
changes inside a private chroot, including effective group, supplementary groups,
no-new-privileges, address-space/process limits and current working directory.
Neither category substitutes for a normal and recovery guest boot.

The platform check is for trusted packaged services in the minimal image.
It is not an arbitrary-code sandbox, an image authentication mechanism or a
claim that the whole operating system is production ready.

For machine-readable host inspection, see [Inspect an Umicom OS boot plan](boot-plan-inspection.md). The JSON view separates configured and scheduled counts and never claims services were executed.
