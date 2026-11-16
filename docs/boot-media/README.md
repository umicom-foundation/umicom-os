![Umicom](../../framework/resources/brand/umicom-logo.svg)

# Inspect and practise a media transfer

Umicom Foundation — Sammy Hegab — MIT

The implementation belongs to `Umicom::boot_media` in the root Framework submodule. This repository contributes composition, not another device writer.

From Linux/WSL, select the Applications Framework checkout explicitly while reviewing an uncommitted overlay:

```bash
cmake -S /mnt/c/umicom/umicomOS/tools/boot-media \
  -B "$HOME/umicom-builds/boot-media" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DUMICOM_OS_FRAMEWORK_SOURCE=/mnt/c/umicom/Umicom-Applications/framework \
  -DUMICOM_BOOT_MEDIA_ENABLE_DEVICE_WRITES=OFF
cmake --build "$HOME/umicom-builds/boot-media" --parallel 2
ctest --test-dir "$HOME/umicom-builds/boot-media" --parallel 2 --no-tests=error --output-on-failure
```

The programs appear in that build's `bin` directory. `umicom-media-image-example` is deliberately non-bootable test data. Do not transfer it to a physical device. `umicom-media inspect` observes a supported complete image, not a kernel/rootfs pair.

The experimental physical backend stays disabled pending platform/device qualification. This delivery does not burn optical discs, safely eject devices, or prove any Umicom OS boot. Native image construction, direct-kernel QEMU checks, ISO creation and physical-device boot testing are distinct operations.


## Build the OS console without a Framework checkout

This lesson uses an installed C SDK: public headers describe the API, libraries
contain the implementation, and the CMake package identifies their components,
dependencies and producer environment. An application consumes that package
instead of compiling another copy of Framework. This does not make its external
runtime dependencies disappear.

### 1. Select a qualified package

Install a matching full UmicomFramework SDK with `boot_media` and `platform`.
The producer and consumer need compatible compiler, architecture and runtime
settings. This tool requires the installed identity, ABI and branding helpers.
The earlier focused `UmicomMediaSDK` package is a different reduced export;
its existence does not establish full Framework provenance.

Keep source and installed builds in different directories. In installed mode,
`UMICOM_OS_FRAMEWORK_SOURCE` and `UMICOM_BOOT_MEDIA_ENABLE_DEVICE_WRITES` are
refused because they would misrepresent how the linked library was produced.
The device-write flag is a producer option: setting it OFF in an application
does not remove code already compiled into its SDK. Select a producer that was
qualified with the intended policy.

### 2. Configure, build and run harmless contracts

Run each command separately and stop on an error. The source may be the whole
OS repository or a copied `tools/boot-media` folder with its `main.c` and
`tests` directory. The installed branch never enters the Framework submodule.

~~~powershell
cmake -S C:/umicom/umicomOS/tools/boot-media -B C:/umicom/umicomOS/build/boot-media-sdk -G Ninja -DUMICOM_OS_BOOT_MEDIA_USE_INSTALLED_SDK=ON -DUMICOM_SDK_ROOT=C:/SDK/Umicom -DBUILD_TESTING=ON
cmake --build C:/umicom/umicomOS/build/boot-media-sdk --target umicom-os-boot-media --parallel 2
ctest --test-dir C:/umicom/umicomOS/build/boot-media-sdk -R "^os[.]boot_media_sdk[.]" --output-on-failure --no-tests=error
& C:/umicom/umicomOS/build/boot-media-sdk/bin/umicom-os-boot-media.exe --help
~~~

These commands are for a Windows Ninja build; use the executable without
`.exe` on Linux. The expected test count is nine. Help and the pure CRC
self-check return zero; malformed commands return usage code two. The tests
do not run list, inspect, review, copy or write. A successful configuration
alone is not a successful compile, installed-runtime launch or test result.

### 3. Follow the shared process boundary

The native entry calls `UmiNativeArgumentsDispatch` once with the original
process arguments. On Windows, Framework captures the Unicode command line
and converts it to UTF-8. Its callback is the existing `UmiBootMediaMain`,
which owns command validation and operations. The OS code adds no device
parser, filesystem writer or platform-specific argument decoder.

The executable also checks console flushing before returning success. A
printed report that could not reach the output stream must not be reported
as successfully delivered. This cannot roll back a completed operation;
operation evidence and output delivery remain separate facts.

### 4. Diagnose package and runtime problems

- A missing package or component means the selected installation is incomplete
  or the wrong package was chosen. Do not add repository include directories
  to conceal a mismatched SDK.
- A provenance or ABI failure requires checking producer and consumer identity.
  Keep the reported mismatch; do not disable the check merely to configure.
- A source-option refusal means the cache mixes build modes. Choose a new build
  directory and pass only the installed-mode options.
- A loader failure after a successful link requires checking the SDK's runtime
  dependency deployment. CMake package discovery does not establish DLL or
  shared-library availability at launch.
- Verify the staged SVGs and Windows icon independently after a successful
  build. Font roles remain the shared Umicom interface/code stacks; no font
  files are downloaded or redistributed by this tool.

### 5. Keep media operations distinct from package qualification

The console exposes the existing shared command set. Use help to review it.
Installing or linking the SDK does not create a bootable ISO, burn a CD, prove
firmware support or validate a removable device. Image inspection, new-file
practice, physical transfer and actual boot remain separate qualification
steps. The source-mode Windows GUI and practice programs remain available
through the earlier composition; this installed mode adds a console consumer,
not a replacement layout.
