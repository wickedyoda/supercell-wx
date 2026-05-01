Android port — plan and requirements
=================================

Decision
--------
- **Approach**: Reuse the existing Qt/C++ codebase and target Android via Qt for Android. This preserves existing logic in `wxdata/` and `scwx-qt/` while leveraging Qt Android deployment.
- **Status**: ✅ **CORE TOOLCHAIN COMPLETE** (steps 1-5 finished). Steps 6-8 in progress.

Targets
-------
- Android API targets: Latest (Android 14 / API 34) and one version back (Android 13 / API 33).
  - Initial build target: **API 33 (Android 13)**, compatible with both.
- Screen support: **phones and tablets** — responsive layouts and touch-friendly controls.
- Supported ABIs: **`arm64-v8a`** (64-bit ARM) and **`armeabi-v7a`** (32-bit ARM, optional).
- CI emulator ABI: **`x86_64`** (for GitHub-hosted Android emulator compatibility).

Completed Work
--------------

### ✅ Step 1: Choose Approach
- **Decision**: Qt for Android (reuse C++ core, maximize code reuse)
- **Rationale**: Fastest path to market, leverage existing wxdata/ and scwx-qt/ codebases

### ✅ Step 2: Add Android Toolchain & CMake Presets
- **CMakePresets.json**: Added 8 new presets
  - `android-arm64-v8a-release` / `android-arm64-v8a-debug`
  - `android-armeabi-v7a-release` / `android-armeabi-v7a-debug`
  - Base presets: `android-base`, `android-arm64-v8a-base`, `android-armeabi-v7a-base`

- **Conan profiles** (tools/conan/profiles/):
  - `scwx-android_arm64-v8a` / `scwx-android_arm64-v8a-debug`
  - `scwx-android_armeabi-v7a` / `scwx-android_armeabi-v7a-debug`
  - All target Android API 33 with clang 17, C++20, c++_shared STL

- **Setup script** (tools/setup-android.sh):
  - Interactive environment validation
  - Conan profile installation
  - CMake configuration suggestions

### ✅ Step 3: Integrate Qt Android Deployment
- **scwx-qt/scwx-qt.cmake**: Added Android-specific executable configuration
  - Sets Qt Android properties (API levels, package source)
  - Configures app as Qt Android app with proper settings

- **scwx-qt/android/**: Complete Android package source
  - **AndroidManifest.xml**: App manifest with permissions
    - INTERNET, NETWORK_STATE (data downloads)
    - FINE/COARSE_LOCATION (GPS, optional)
    - STORAGE READ/WRITE (placefiles, cache)
  - **res/values/strings.xml**: App strings (name, description)
  - **res/values/styles.xml**: App theming (fullscreen for maps)
  - **README.md**: Android resources guide

### ✅ Step 4: UI Touch Adaptation
- **docs/TOUCH_ADAPTATION.md**: Comprehensive strategy document
  - **Gestures**: Pinch zoom ✅ (implemented), pan ✅, double-tap ⏳, long-press ⏳
  - **Button sizing**: 44-48 dp recommendations (Material Design)
  - **Responsive layouts**: Phone/tablet/large-tablet breakpoints
  - **Code examples**: Touch handler patterns for MapWidget and MainWindow
  - **Phase-based roadmap**: 4 phases from core gestures to mobile features

### ✅ Step 5: MapLibre Qt Android Integration
- **docs/MAPLIBRE_ANDROID.md**: Complete integration guide
  - **Graphics API**: OpenGL ES 3.0 support (built-in to MapLibre)
  - **Dependencies**: All Conan packages configured for Android
  - **Build steps**: Detailed CMake + Conan workflow
  - **Known issues**: Vulkan (optional), memory optimization, GLES context
  - **Testing checklist**: Emulator and device testing procedures
  - **Performance profiling**: GPU/memory/battery monitoring
  - **Deployment**: APK/AAB creation and signing for Play Store

In Progress
-----------

### ⏳ Step 6: Configure APK/AAB Packaging & Signing
- **Status**: In progress (Gradle defaults and signing template added)
- **Tasks**:
  - Create gradle.properties for APK configuration
  - Set up keystore for release signing
  - Implement androiddeployqt integration in CMake
  - Document APK/AAB generation workflow

  - Added `scwx-qt/android/gradle.properties`
  - Added `scwx-qt/android/release-signing.properties.example`
  - Added `docs/APK_SIGNING.md`

### ⏳ Step 7: Set up CI and Device/Emulator Testing
- **Status**: In progress (Android CI job + emulator smoke test scaffold added)
- **Tasks**:
  - Add Android test job to CI matrix (.github/workflows/ci.yml)
  - Configure Android emulator in CI (arm64-v8a, API 33)
  - Add APK/AAB artifact collection
  - Implement basic smoke tests (app launch, map render)

  - Added standalone `android-build` job in `.github/workflows/ci.yml`
  - Added APK packaging via `androiddeployqt`
  - Added emulator smoke test step (`reactivecircus/android-emulator-runner`) using `x86_64` APK
  - Added Android artifact upload (`.apk`, `.aab`, `.so`)

### ⏳ Step 8: Write Android Build & Run Docs
- **Status**: In progress (supporting docs complete, need integration guide)
- **Tasks**:
  - Create ANDROID_BUILD.md with quick-start for developers
  - Document emulator setup (avdmanager, emulator launch)
  - Add troubleshooting guide (build errors, runtime issues)
  - Provide APK testing workflow

Quick Start
-----------

### For Developers: Build and Run on Emulator

**1. Install prerequisites** (one-time):

```bash
# Install Android SDK, NDK, Java JDK
brew install android-sdk android-ndk openjdk@11  # macOS
# Or download from https://developer.android.com/studio

# Install Qt 6.11.0 for Android
pip install aqtinstall
aqt install-qt android android 6.11.0 android_armv8 -m qtimageformats qtmultimedia
```

**2. Set environment** (each session):

```bash
export ANDROID_SDK_ROOT=/path/to/android-sdk
export ANDROID_NDK_HOME=/path/to/android-ndk
export JAVA_HOME=/path/to/jdk  # or /usr/libexec/java_home on macOS
```

**3. Run setup script**:

```bash
cd /path/to/supercell-wx
./tools/setup-android.sh build-android-arm64 .venv arm64-v8a
cd build-android-arm64
```

**4. Configure CMake**:

```bash
cmake .. --preset android-arm64-v8a-release
```

**5. Build**:

```bash
cmake --build . --target supercell-wx
```

**6. Create APK**:

```bash
androiddeployqt --input AndroidDeploymentSettings.json \
                --output android-build \
                --android-platform android-33 \
                --apk
```

**7. Deploy to emulator**:

```bash
# If emulator is running:
adb install -r android-build/apk/supercell-wx.apk
adb shell am start -n net.supercellwx.app/.MainActivity
adb logcat | grep supercell-wx
```

Files Created/Modified
----------------------

### New Files
| Path | Purpose |
|------|---------|
| `tools/conan/profiles/scwx-android_arm64-v8a` | Conan profile (arm64-v8a, Release) |
| `tools/conan/profiles/scwx-android_arm64-v8a-debug` | Conan profile (arm64-v8a, Debug) |
| `tools/conan/profiles/scwx-android_armeabi-v7a` | Conan profile (armeabi-v7a, Release) |
| `tools/conan/profiles/scwx-android_armeabi-v7a-debug` | Conan profile (armeabi-v7a, Debug) |
| `tools/setup-android.sh` | Setup script for developers |
| `scwx-qt/android/AndroidManifest.xml` | Android app manifest |
| `scwx-qt/android/res/values/strings.xml` | Android strings (app name, etc.) |
| `scwx-qt/android/res/values/styles.xml` | Android app theme |
| `scwx-qt/android/README.md` | Android package source guide |
| `docs/ANDROID_PORT.md` | This file |
| `docs/TOUCH_ADAPTATION.md` | Touch UI strategy & code examples |
| `docs/MAPLIBRE_ANDROID.md` | MapLibre Android integration guide |

### Modified Files
| Path | Change |
|------|--------|
| `CMakePresets.json` | Added 8 Android configure + 4 Android build presets |
| `scwx-qt/scwx-qt.cmake` | Added Android-specific executable configuration |

Dependencies
------------

### Required Tools
- **Android SDK** (API 33+)
- **Android NDK** (r25+)
- **Java JDK** (11+)
- **Qt 6.11.0 for Android** (install via aqtinstall or Qt Maintenance Tool)
- **Conan 2.x** (for package management)
- **CMake 3.24+**

### Android Platforms
- **Min SDK**: API 33 (Android 13)
- **Target SDK**: API 34 (Android 14)
- **Recommended devices for testing**:
  - Phone: Pixel 6/7, Moto G Power/Pro
  - Tablet: iPad Pro, Samsung Tab S6+

Next Steps (Roadmap)
--------------------

### Near Term (This Sprint)
1. ⏳ **Step 6**: Create gradle.properties and keystore setup
2. ⏳ **Step 7**: Add CI emulator job with basic smoke tests
3. ⏳ **Step 8**: Write ANDROID_BUILD.md quick-start guide

### Medium Term (Next Sprint)
1. Test on real Android devices (Pixel 6, Moto G)
2. Implement Phase 2 touch features (double-tap, long-press)
3. Optimize MapLibre memory footprint for phones
4. Profile battery drain and cooling

### Long Term
1. Phase 3-4: Responsive layouts, mobile-specific features
2. Submit to Google Play Store
3. Release beta via Firebase App Distribution
4. Collect user feedback and iterate

Known Limitations
-----------------

1. **Gestures**: Double-tap zoom and long-press not yet implemented (documented in TOUCH_ADAPTATION.md)
2. **UI Layout**: Currently desktop-optimized; phone layouts need adaptation (documented in TOUCH_ADAPTATION.md)
3. **Performance**: Not yet profiled on mid-range devices (Moto G); may require optimization
4. **Testing**: Not yet tested on real devices or emulator (CI job pending)
5. **Store Submission**: Release process not yet automated (manual for now)

Documentation References
------------------------

- [ANDROID_PORT.md](ANDROID_PORT.md) — This file (overview and roadmap)
- [TOUCH_ADAPTATION.md](TOUCH_ADAPTATION.md) — Touch UI strategy, code examples, testing
- [MAPLIBRE_ANDROID.md](MAPLIBRE_ANDROID.md) — MapLibre integration, build, testing, optimization
- [AGENTS.md](../AGENTS.md) — Project architecture (Qt + C++20 framework overview)

Resources
---------

- [Qt for Android Documentation](https://doc.qt.io/qt-6/android.html)
- [Android SDK Setup](https://developer.android.com/studio/install)
- [Android NDK Downloads](https://developer.android.com/studio/projects/install-ndk)
- [CMake Android Toolchain](https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html#android)
- [Conan Android Support](https://docs.conan.io/2.0/reference/settings/os/android.html)
- [Material Design](https://material.io/design/) (UI/UX best practices)
- [MapLibre Native](https://maplibre.org/maplibre-native/docs/) (map engine)


