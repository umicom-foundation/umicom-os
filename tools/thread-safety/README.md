# Native thread-lifetime checks

This host compiles the canonical Framework example and regression tests. It is not
an alternative thread runtime, graphical application, kernel build or guest boot.
Select the reviewed checkout with `-DUMICOM_FRAMEWORK_SOURCE=/absolute/framework`
when not using this repository's `framework` dependency.

```sh
cmake -S tools/thread-safety -B build/thread-safety -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/thread-safety --parallel 2
ctest --test-dir build/thread-safety --parallel 2 --no-tests=error --output-on-failure
```

Read `framework/docs/learning/background-work-and-memory.html` for the complete lesson.
Run the complete application and target-platform suites separately before release.
