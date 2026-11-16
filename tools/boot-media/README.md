![Umicom](../../framework/resources/brand/umicom-logo.svg)

# Native boot-media tools

Umicom Foundation — Sammy Hegab — MIT

This is a thin CMake entry point to the canonical Framework `examples/boot_media` project. Select the Framework checkout with `-DUMICOM_OS_FRAMEWORK_SOURCE=...`. The default enables inspection and new-file copies, not destructive physical writing. The earlier image builder and Setup Centre media preparation remain unchanged.

Read [the OS workflow](../../docs/boot-media/README.md) and the public Framework guide at `docs/learning/prepare-and-check-boot-media.html`.


## Use an installed Framework SDK

Source composition remains the default described above. To build only the
thin OS console against installed libraries, select the explicit SDK mode:

~~~powershell
cmake -S C:/umicom/umicomOS/tools/boot-media -B C:/umicom/umicomOS/build/boot-media-sdk -G Ninja -DUMICOM_OS_BOOT_MEDIA_USE_INSTALLED_SDK=ON -DUMICOM_SDK_ROOT=C:/SDK/Umicom -DBUILD_TESTING=ON
cmake --build C:/umicom/umicomOS/build/boot-media-sdk --target umicom-os-boot-media --parallel 2
ctest --test-dir C:/umicom/umicomOS/build/boot-media-sdk -R "^os[.]boot_media_sdk[.]" --output-on-failure --no-tests=error
~~~

Replace the SDK path with a qualified full UmicomFramework installation.
Use a separate build directory from source mode. No Framework checkout or
Framework source compilation is used by this branch. The older focused
UmicomMediaSDK export is a different package and is not accepted as a
substitute for the full identity-checked SDK.

The executable is `bin/umicom-os-boot-media.exe` on Windows. Its command parser,
image inspection, policy and transfer implementations are supplied by
`Umicom::boot_media`. Unicode argument ownership comes from
`Umicom::platform`. The installed branding helper supplies the approved SVG
resources and Windows icon. This mode does not build the existing Windows GUI
or the source practice-image programs; those remain available in source mode.

Read the [SDK dependency lesson](../../docs/boot-media/README.md#build-the-os-console-without-a-framework-checkout)
before running media commands. Nine included tests exercise help, a pure CRC
self-check and argument refusals. They do not enumerate or open devices.
Native compilation, tests, installed-library deployment, icon appearance and
real-media behavior still require qualification.
