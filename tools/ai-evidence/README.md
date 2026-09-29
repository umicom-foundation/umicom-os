# AI evidence host tools

This entry composes `framework/examples/ai_evidence`. All implementation belongs to Framework.
It does not change the OS kernel, boot profile, root filesystem, image builder or guest services.

From a normal Linux/WSL account:

```sh
OS="/mnt/c/umicom/umicomOS"
FW="/mnt/c/umicom/Umicom-Applications/framework"
BUILD="$HOME/umicom-builds/ai-evidence"
cmake -S "$OS/tools/ai-evidence" -B "$BUILD" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DUMICOM_OS_FRAMEWORK_SOURCE="$FW"
cmake --build "$BUILD" --parallel 2
ctest --test-dir "$BUILD" --parallel 2 --no-tests=error --output-on-failure
"$BUILD/bin/umicom-ai-evidence" --self-test
```

Stop after any failing command. Real model inference and a GUI are not part of the memory
lesson. Consult Framework's public AI evidence guide for the deliberately opted-in local request.
