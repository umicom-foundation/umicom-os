![Umicom](../../framework/resources/brand/umicom-logo.svg)

# Umicom OS Web Preview

This native console hosts selected static files on the local loopback address through Umicom Framework. It is a development tool, not an Internet-facing server or a complete WAMP/LAMP replacement. Framework owns parsing, file serving, request admission, connection deadlines and signal cleanup; the OS executable owns only native arguments and the process result.

## Build with an installed SDK

First install a qualified Framework SDK that includes umicom/web/preview_command.h and its matching web library. From this directory, choose an unused build directory and your actual SDK prefix. Stop on any error.

~~~powershell
cmake -S . -B build/sdk -G Ninja -DUMICOM_SDK_ROOT=C:/umicom/_local/install/framework
cmake --build build/sdk --target umicom-os-web-preview
ctest --test-dir build/sdk -R os.web_preview --output-on-failure
& ./build/sdk/bin/umicom-os-web-preview.exe --help
~~~

No Framework clone belongs inside this tool. Maintainers can select an existing reviewed checkout with -DUMICOM_OS_WEB_PREVIEW_FRAMEWORK_SOURCE=C:/umicom/Umicom-Applications/framework in a separate build directory instead. Never set both source and SDK modes. Source configuration may register Framework tests too; the test selection above targets only this console.

## Preview a directory

Create a directory containing your own index.html. Use an absolute path and an explicit unused port from 1024 through 65535. First inspect the plan:

~~~powershell
& ./build/sdk/bin/umicom-os-web-preview.exe --root C:/umicom/projects/my-site --port 8080 --dry-run
~~~

Dry-run checks argument syntax only. It does not open the directory, reserve the port or start a browser. After reviewing the directory you intend to expose, omit --dry-run, then open http://127.0.0.1:8080 yourself. Stop with Ctrl+C. Only local connections are accepted; GET and HEAD serve complete static files up to the Framework one-MiB limit. Paths with spaces must be quoted.

This console is for Windows/Linux user space. Its presence in the OS repository does not make it a freestanding Kernel server. TLS, authentication, upload handling, dynamic application execution, database provisioning, public deployment and background service installation remain outside this tool.

## Troubleshoot

Exit 2 indicates invalid arguments; inspect duplicates, unknown flags, relative roots and port range. Exit 1 means an operational or console-output failure. A bind failure may mean another process owns the selected port. A dry-run success cannot rule that out. Read the exact error before selecting another port. Failed startup destroys the preview object and restores installed signal handlers.

The branded executable uses the Framework SVG resource catalogue and Windows ICO helper. Actual icon rendering, installed-library identity and live HTTP serving require qualification on your selected system. The six console tests do not open a listener; the shared Framework has separate session, signal and HTTP regression coverage.
