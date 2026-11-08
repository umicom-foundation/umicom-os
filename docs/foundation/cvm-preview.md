# Umicom C preview and the OS boot path

Umicom OS has two distinct activities: building native user-space tools and booting a Linux-based guest image. Running a tool on Windows does not boot Umicom OS.

The optional `tools/cvm-preview` project consumes the shared Framework source-preview service and an installed Umicom C SDK. It supports a small arithmetic program and prints its result. Select the Framework source with `UMICOM_OS_FRAMEWORK_SOURCE`; publish shared changes once and update clean dependency checkouts through Git.

<!-- An explicit fixed trace mode extends the ordinary readiness probe. The previous wording is retained for engineering review.
The independent `boot/cvm-probe` project builds the arithmetic runtime from `UMICOM_C_SOURCE_DIR`. The probe accepts no paths, commands or network input. A native host execution expects `UMICOM_CVM_PROBE_READY value=42`. A cross build registers no host execution test.
-->
The independent `boot/cvm-probe` project builds the arithmetic runtime from `UMICOM_C_SOURCE_DIR`. Its default launch expects `UMICOM_CVM_PROBE_READY value=42`; the explicit `--trace-check` option runs four fixed trace contracts and expects `UMICOM_CVM_TRACE_READY cases=4`. It accepts no source, bytecode, filesystem paths or network input. A cross build registers no host execution test.

## Moving from host to guest

A Linux guest requires a Linux-target binary. The Windows executable is not usable inside it. The static guest option `UMICOM_OS_CVM_STATIC` requires a Linux compiler target and a static C library for that target.

The probe is deliberately not inserted into the existing root-filesystem inventory or PID-1 service configuration. Installing it to a staging directory does not make it part of a boot image. The existing image inventory is restrictive; changing it requires reviewing the package manifest, dependency ordering, archive validation and guest acceptance together.

To understand the supported boot route, follow [Boot a small Umicom system](first-boot.html) and [the native image workflow](native-image-workflow.md). Keep normal boot, forced recovery and service-failure evidence separate. A built archive, a launched desktop and a guest boot are different observations.

Umicom Kernel's CVM profile is a separate freestanding RISC-V environment. Its ELF program uses the Kernel native EXIT interface rather than Linux system calls. Neither binary can be substituted for the other.


## Qualify the native trace probe

Build this small project independently of the desktop and image targets. Use an updated Umicom C source checkout containing trace.h. In PowerShell, run one command at a time and stop on errors:

```powershell
Set-Location "C:\umicom\umicomOS"
$env:Path = "C:\msys64\ucrt64\bin;$env:Path"
cmake -S boot/cvm-probe -B build/cvm-probe-native -G Ninja -DCMAKE_C_COMPILER=gcc -DUMICOM_C_SOURCE_DIR=C:/umicom/umicom-c -DBUILD_TESTING=ON
cmake --build build/cvm-probe-native --parallel 2
ctest --test-dir build/cvm-probe-native --output-on-failure --no-tests=error
& ".\build\cvm-probe-native\umicom-cvm-probe.exe"
& ".\build\cvm-probe-native\umicom-cvm-probe.exe" --trace-check
```

The trace mode checks complete arithmetic, a rejected RETURN at fuel exhaustion, division by zero with unchanged operand depth, and insufficient report capacity with no execution. The runtime remains in Umicom C; this probe only checks expected outcomes. Native tests compare complete output and exit codes and reject unknown or extra arguments. These commands provide a workflow, not a recorded result.

On a Linux host use the same CMake source/build paths with a Linux C23 compiler, then launch ./build/cvm-probe-native/umicom-cvm-probe. The optional static flag requires a Linux target and its static C library. Installing to a staging root still does not alter the boot inventory or start the probe during boot.

## Use the Framework-owned source trace

The tools/cvm-preview project composes the existing Framework example directly. With a current Umicom C SDK installed at the prefix below, configure the wrapper against a Framework checkout containing cvm_trace.h:

```powershell
cmake -S tools/cvm-preview -B build/cvm-preview-native -G Ninja -DCMAKE_C_COMPILER=gcc -DCMAKE_PREFIX_PATH=C:/umicom/_local/install/umicom-c -DBUILD_TESTING=ON
cmake --build build/cvm-preview-native --parallel 2
ctest --test-dir build/cvm-preview-native --output-on-failure --no-tests=error
& ".\build\cvm-preview-native\framework-preview\umicom-cvm-preview.exe" --trace
```

By default the wrapper selects its Framework dependency directory. If that source is elsewhere, set UMICOM_OS_FRAMEWORK_SOURCE to the reviewed checkout while configuring. No source is downloaded by this option. A missing trace header or symbol means the selected Framework and Umicom C SDK need to be aligned. Read Framework's docs/learning/cvm-preview.md for the SDK installation sequence.

The default preview still prints 42. The --trace option adds four rows, then prints 42. An optional second argument supplies a complete arithmetic program as one quoted string. The source-preview path, fixed runtime probe, Kernel guest and OS boot are four separate checks. None establishes a desktop debugger, arbitrary native-code execution or complete C23 compilation.

## Optional integer comparison probe

The native probe also accepts `--comparison-check`. This selects eight fixed contracts: all six signed comparison opcodes at the integer extremes, fuel exhaustion before a comparison, and refusal of a comparison artifact labelled with the arithmetic wire version. It accepts no user source, bytecode, path or command. With the updated canonical Umicom C source, the expected successful line is `UMICOM_CVM_COMPARISON_READY cases=8`.

After completing the native probe build described above, run:

```powershell
Set-Location C:\umicom\umicomOS
& ".\build\cvm-probe-native\umicom-cvm-probe.exe" --comparison-check
ctest --test-dir build/cvm-probe-native -R "os[.]cvm[.]probe" --output-on-failure --no-tests=error
```

Stop on errors and retain the exact process output. A native build registers exact-output and extra-argument-refusal cases; a cross build deliberately registers no host launch. The no-option arithmetic readiness output and the existing `--trace-check` contract remain unchanged. No boot manifest is changed by adding this optional mode.

The source requires `UMC_COMPARISON_PROFILE`. Select matching Umicom C headers and runtime, rebuild, and retain the actual process results. Do not infer guest startup or hardware support from a host pass. A Linux-targeted static executable, root-filesystem inclusion and an actual guest launch remain separate checks.

The hosted Framework preview wrapper inherits comparison source and trace behaviour from the shared primary Framework project when that revision and the matching installed Umicom C SDK are selected. It still does not duplicate the parser. Its comparison consumer checks are separate from the fixed runtime-only probe described here.
