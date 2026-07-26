# MapLibre Native Qt - Android Integration Guide

## Overview

MapLibre Native Qt is the core map rendering engine for Supercell Wx. This document explains how to build and integrate MapLibre for Android (arm64-v8a and armeabi-v7a ABIs).

## Architecture

**MapLibre Native Qt stack**:
- **maplibre-native-qt** (Qt bindings layer)
  ↓
- **maplibre-native** (C++20 core, map rendering with OpenGL)
  ↓
- **Vulkan Loader** / **OpenGL ES** (graphics API)
- **Qt 6.11.0 for Android** (platform layer)

## Android-Specific Considerations

### Graphics API

- **Desktop**: OpenGL 3.0+
- **Android**: OpenGL ES 2.0/3.0 (OpenGL ES 3.1+ recommended for best performance)
- **MapLibre**: Supports both OpenGL and OpenGL ES via a common interface

**Status**: MapLibre Native supports OpenGL ES 3.0 out-of-the-box. The Qt bindings should work with Android's GLES implementation.

### Platform Dependencies

Required for Android builds via Conan:

- `boost/1.90.0` ✅ (Conan manages)
- `zlib/1.3.1` ✅ (Conan manages)
- `openssl/3.6.0` ✅ (Conan manages)
- `libcurl/8.17.0` ✅ (Conan manages)
- `libjpeg/9f` ✅ (Conan manages)
- `libpng/1.6.54` ✅ (Conan manages)
- `libtiff/4.7.1` ✅ (Conan manages)
- `libzip/1.11.4` ✅ (Conan manages)
- `sqlite3/3.51.0` ✅ (Conan manages)
- `vulkan-loader/1.4.313.0` ⚠️ (Optional, not needed for GLES)
- `zlib/1.3.1` ✅ (Conan manages)

**Qt modules required**:
- QtCore
- QtGui
- QtNetwork
- QtOpenGL (or QtOpenGLWidgets for Qt6)

**Status**: All Conan dependencies are configured in `conanfile.py`. Vulkan is optional; MapLibre will fall back to OpenGL ES.

## Building MapLibre for Android

### Step 1: Install Prerequisites

Ensure your environment is properly set up (see `docs/ANDROID_PORT.md` and run `tools/setup-android.sh`):

```bash
export ANDROID_SDK_ROOT=/path/to/android-sdk
export ANDROID_NDK_HOME=/path/to/android-ndk
export JAVA_HOME=/path/to/jdk
```

### Step 2: Configure Conan for Android

Conan profiles for Android are in `tools/conan/profiles/scwx-android_*.` They automatically configure:
- Android API level (33 = Android 13)
- C++ standard library (`c++_shared`)
- Compiler (clang 17)

### Step 3: Install CMake and Dependencies

Using CMake presets with Conan provider:

```bash
cd /path/to/build-android-arm64-v8a
cmake .. -G Ninja \
  --preset android-arm64-v8a-release \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake
```

This will:
1. Invoke Conan with the `scwx-android_arm64-v8a` profile
2. Download/build Android-compatible Conan packages
3. Configure CMake with Android toolchain

### Step 4: Build MapLibre (part of full build)

```bash
cmake --build build-android-arm64-v8a --target supercell-wx
```

This builds the entire project including MapLibre Native Qt. 

**Note**: First build may take 30-60 minutes depending on whether Conan packages are available or need to be built from source.

## Known Issues and Workarounds

### Issue 1: Vulkan Loader on Android

**Problem**: `vulkan-loader/1.4.313.0` is designed for desktop systems. Android has built-in Vulkan support but different loader mechanics.

**Status**: Not blocking. MapLibre defaults to OpenGL ES 3.0 when Vulkan is unavailable. This is preferred on mobile anyway.

**Workaround**: Disable Vulkan explicitly in conanfile.py:

```python
# In conanfile.py configure() method
if self.settings.os == "Android":
    self.options["vulkan-loader"].shared = False  # Don't use Vulkan on Android
```

### Issue 2: OpenGL Context Creation

**Problem**: Qt on Android manages OpenGL context differently than desktop. QOpenGLWidget may have issues with multi-threaded rendering.

**Status**: Not yet tested. Likely works since MapLibre-Qt uses standard Qt OpenGL APIs.

**Solution**: If issues occur:
1. Use QOpenGLWidget (already done in map_widget.hpp)
2. Ensure all OpenGL calls happen in the Qt render thread
3. Consider using QSurfaceFormat to specify GLES 3.0

**Code to test**:

```cpp
// In MapWidget constructor or initialization
QSurfaceFormat format;
format.setRenderableType(QSurfaceFormat::OpenGLES);
format.setVersion(3, 0);  // OpenGL ES 3.0
QOpenGLWidget::setFormat(format);
```

### Issue 3: Memory and Performance

**Problem**: Android devices have limited RAM and GPU bandwidth compared to desktop. Large tile loads or complex layers may cause:
- Out-of-memory crashes
- Frame rate drops
- Battery drain

**Mitigations**:
1. Reduce tile cache size for Android
2. Limit layer complexity on phones
3. Implement texture streaming for large overlays
4. Profile on real devices (Pixel 6, Moto G, tablets)

**Implementation** (future):

```cpp
// In MapWidget::Initialize() or settings
#ifdef Q_OS_ANDROID
const size_t TILE_CACHE_SIZE = 50 * 1024 * 1024;  // 50 MB instead of 100 MB
const int MAX_CONCURRENT_TILE_REQUESTS = 8;  // Lower on mobile
#endif
```

### Issue 4: Touch Gesture Conflicts

**Problem**: Qt's gesture system and MapLibre's mouse/touch event handling may conflict on Android.

**Status**: Pinch gesture already handled in MapWidget.

**Solution**: Ensure QGestureEvent is processed before QMouseEvent:

```cpp
// Already done in map_widget.cpp
case QEvent::Type::Gesture:
    gestureEvent(static_cast<QGestureEvent*>(e));
    break;
```

## Testing MapLibre on Android

### Emulator Testing

1. **Create an emulator**:
   ```bash
   sdkmanager "system-images;android-34;google_apis;arm64-v8a"
   avdmanager create avd -n Pixel6-API34 \
     -k "system-images;android-34;google_apis;arm64-v8a"
   ```

2. **Launch emulator**:
   ```bash
   emulator -avd Pixel6-API34 -gpu on
   ```

3. **Deploy APK**:
   ```bash
   adb install -r build-android-arm64-v8a/android-build/apk/supercell-wx.apk
   ```

4. **Run and check logs**:
   ```bash
   adb logcat | grep supercell-wx
   ```

### Real Device Testing

1. **Enable Developer Mode**: Go to Settings → About → Build Number (tap 7x)
2. **Connect via USB**: `adb devices`
3. **Install APK**: `adb install supercell-wx.apk`
4. **Monitor logs**: `adb logcat`

### Testing Checklist

- ✅ App launches without crashes
- ✅ Map renders with satellite/street/terrain styles
- ✅ Pan/zoom gestures work smoothly
- ✅ Radar product loads and displays
- ✅ Color table updates correctly
- ✅ No memory leaks (monitor `adb shell dumpsys meminfo`)
- ✅ Battery drain acceptable (profile with Android Studio)
- ✅ Works on both arm64-v8a and armeabi-v7a (if supporting 32-bit)

## Performance Optimization

### Profile on Android

```bash
# Using Android Studio Profiler
adb forward tcp:5037 tcp:5037
# Launch Android Studio → Profiler → Profile 'supercell-wx'
```

Key metrics to monitor:
- **GPU rendering**: Should be < 16.67 ms per frame (60 FPS)
- **Memory**: Should stay < 500 MB (phones), < 1 GB (tablets)
- **Battery drain**: Should match typical map apps (5-10% per hour at active use)

### Optimization Strategies

1. **Reduce refresh rate on battery power**:
   ```cpp
   if (QGuiApplication::applicationState() & Qt::ApplicationState::ApplicationActive) {
       // Active: 60 FPS
       timer->setInterval(16);
   } else {
       // Background: 10 FPS
       timer->setInterval(100);
   }
   ```

2. **Simplify layers on phones**:
   ```cpp
   if (QScreen::availableGeometry().width() < 600) {
       // Hide optional overlay layers
       hideLayer("detailed_warnings");
       hideLayer("storm_tracks");
   }
   ```

3. **Enable texture compression**:
   MapLibre should auto-detect and use ETC2/ASTC compression on GLES 3.0+.

## Deployment

### Creating Release APK

```bash
# Build in Release mode
cmake --preset android-arm64-v8a-release
cmake --build build-android-arm64-v8a --target supercell-wx

# Generate APK with Qt deployment tool
androiddeployqt --input build-android-arm64-v8a/AndroidDeploymentSettings.json \
                --output build-android-arm64-v8a/android-build \
                --android-platform android-33 \
                --apk \
                --release
```

### Play Store Submission

Generate signed AAB (Android App Bundle):

```bash
jarsigner -verbose -sigalg SHA256withRSA -digestalg SHA-256 \
  -keystore /path/to/release.keystore \
  build-android-arm64-v8a/android-build/supercell-wx.apk \
  release_alias
```

## References

- [MapLibre Native Documentation](https://maplibre.org/maplibre-native/docs/)
- [Qt for Android OpenGL](https://doc.qt.io/qt-6/qopenglwidget.html)
- [Android OpenGL ES](https://developer.android.com/guide/topics/graphics/opengl)
- [Qt QOpenGLWidget and GLES](https://doc.qt.io/qt-6/android.html#opengl)

## Next Steps

1. ✅ Build MapLibre for arm64-v8a on CI (when emulator testing is added)
2. ✅ Test map rendering on Android emulator
3. ✅ Test touch gestures (pinch, pan) on device
4. ✅ Profile GPU/memory usage
5. ✅ Optimize for lower-end devices (if needed)
6. ✅ Add support for armeabi-v7a (32-bit ARM)
