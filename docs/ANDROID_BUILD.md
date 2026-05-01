Supercell Wx Android build and run
=================================

This guide is the shortest path from a clean checkout to a runnable Android build.

Prerequisites
-------------

- Android SDK 33+ with platform-tools and cmdline-tools
- Android NDK r25 or newer
- Java JDK 11+
- Qt 6.11.x for Android installed via Qt Maintenance Tool or `aqtinstall`
- Conan 2.x
- CMake 3.24+

Environment
-----------

Set these variables in your shell before configuring:

```bash
export ANDROID_SDK_ROOT=/path/to/android-sdk
export ANDROID_NDK_HOME=/path/to/android-ndk
export JAVA_HOME=/path/to/jdk
```

Build
-----

For arm64-v8a, use the provided preset and setup script:

```bash
cd /path/to/supercell-wx
./tools/setup-android.sh build-android-arm64 .venv arm64-v8a
cmake --preset android-arm64-v8a-release
cmake --build build/android-arm64-v8a-release --target supercell-wx
```

For the 32-bit ARM target, swap the preset and ABI:

```bash
cmake --preset android-armeabi-v7a-release
cmake --build build/android-armeabi-v7a-release --target supercell-wx
```

For emulator testing (CI-compatible), use x86_64:

```bash
cmake --preset android-x86_64-release
cmake --build build/android-x86_64-release --target supercell-wx
```

Run on a device or emulator
---------------------------

If Qt deployment has generated an APK, install it with `adb`:

```bash
adb install -r /path/to/supercell-wx.apk
adb shell am start -n net.supercellwx.app/org.qtproject.qt.android.bindings.QtActivity
adb logcat | grep supercell-wx
```

Packaging notes
---------------

- Debug builds are intended for local device testing.
- Release builds should use a signed keystore.
- See [APK_SIGNING.md](APK_SIGNING.md) for keystore and signing workflow.

Troubleshooting
---------------

- If CMake cannot find Qt for Android, confirm the Android Qt kit is installed and the Qt path is on `CMAKE_PREFIX_PATH`.
- If `androiddeployqt` cannot find the NDK, verify `ANDROID_NDK_HOME` points to the NDK root.
- If the app launches but shows a black screen, check OpenGL ES support and device logs via `adb logcat`.
- If map rendering is slow, try a newer device or reduce overlay density for mobile builds.

Useful references
-----------------

- [ANDROID_PORT.md](ANDROID_PORT.md)
- [TOUCH_ADAPTATION.md](TOUCH_ADAPTATION.md)
- [MAPLIBRE_ANDROID.md](MAPLIBRE_ANDROID.md)
- [APK_SIGNING.md](APK_SIGNING.md)
