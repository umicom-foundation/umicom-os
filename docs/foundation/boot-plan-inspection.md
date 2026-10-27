# Inspect an Umicom OS boot plan

A boot plan describes which services the OS intends to start. A service is a program with a particular job, such as checking the guest environment. Some services depend on others: a dependency must finish successfully before the next service is admitted.

The host tool `umicom-os-boot-plan` reads the same configuration grammar as the OS startup program. It validates the file and lists the accepted order. It never starts those services, changes the hostname, reboots a computer or writes a disk image.

## 1. Build the inspection tool

On Windows, open PowerShell in an existing Umicom OS checkout. These commands build the small independent foundation project; they do not build the root Framework-backed OS application.

```powershell
Set-Location "C:\umicom\umicomOS"
$env:Path = "C:\msys64\ucrt64\bin;$env:Path"
New-Item -ItemType Directory -Force "C:\umicom\_local\logs\os-foundation"
cmake -S .\boot\foundation -B .\build\foundation-host -G Ninja -DCMAKE_C_COMPILER=C:/msys64/ucrt64/bin/gcc.exe -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON 2>&1 | Tee-Object "C:\umicom\_local\logs\os-foundation\configure.log"
cmake --build .\build\foundation-host --parallel 2 2>&1 | Tee-Object "C:\umicom\_local\logs\os-foundation\build.log"
ctest --test-dir .\build\foundation-host -R "os.foundation.plan" --output-on-failure --no-tests=error --output-log "C:\umicom\_local\logs\os-foundation\plan-tests.log"
```

Run each command separately and stop on an error. PowerShell does not automatically stop after a failed native program. A successful Windows host test does not establish that the Linux guest service supervisor or PID1 has run. See [qualification layers](qualification-layers.md).

## 2. Read a normal plan

```powershell
& ".\build\foundation-host\umicom-os-boot-plan.exe" ".\tests\foundation\plans\ordered.conf"
```

The text begins with `UMICOM_OS_PLAN 1`. The fixture configures two services. Although the source declarations may be in a different order, the accepted plan places `platform-check` before `framework-probe` because the second depends on the first. The final field is `executed=false`: this is an inspection, not a boot result.

## 3. Request structured output

JSON is a text format that gives names to values. It lets a desktop panel or another tool read a plan without guessing what each line means.

```powershell
& ".\build\foundation-host\umicom-os-boot-plan.exe" ".\tests\foundation\plans\ordered.conf" --json
```

The output has the schema `umicom.os.boot-plan` and version `1`. Consumers should check both before using the fields.

| Field | Meaning |
| --- | --- |
| hostname | Declared hostname; it was not applied to this host. |
| mode | `normal` or `recovery`. |
| configured | Number of accepted service declarations. |
| scheduled | Number of services selected by the requested mode. |
| autopoweroff | Whether startup controls request poweroff; this tool never powers off. |
| executed | Always `false` for this inspection tool. |
| services | Dependency-ordered services: id, executable, dependency and timeoutMs. |

A service without a dependency uses JSON `null`. A timeout of 1000 milliseconds means one second; it is configuration data, not an observed duration. Unknown future fields may be ignored, but an unsupported schema version should be rejected.

## 4. Compare recovery mode

```powershell
& ".\build\foundation-host\umicom-os-boot-plan.exe" ".\tests\foundation\plans\ordered.conf" --cmdline "umicom.recovery=1" --json
```

Recovery retains `configured=2` but reports `scheduled=0` and an empty services array. Recovery is deliberately available without launching the normal service chain. Both option orders are supported; duplicate options are rejected.

## 5. Understand a refusal

A missing file, invalid service path, dependency cycle or malformed control prevents an accepted plan from being printed. Exit code 1 means inspection failed. Exit code 2 means command usage was invalid. A successful inspection exits with 0. Standard error carries the diagnostic; standard output contains only an accepted text or JSON plan.

Inspect the reported line, correct the configuration, and rerun inspection before attempting a guest boot. Increasing a timeout cannot fix a dependency cycle. A valid plan still needs separate normal/recovery boot tests and captured service outcomes.

## Follow the implementation

- [CLI and JSON boundary](../../boot/foundation/plan.c)
- [Shared configuration parser](../../boot/foundation/config.c)
- [Structured-output regressions](../../tests/foundation/check_plan_json.cmake)
- [Native image workflow](native-image-workflow.md)
