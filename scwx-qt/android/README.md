# Android Package Source Directory

This directory contains Android-specific resources and configuration for building Supercell Wx as an Android app using Qt.

## Directory Structure

```
android/
├── AndroidManifest.xml          # Android app manifest (permissions, activities, metadata)
├── build.gradle                 # Gradle build configuration (optional, Qt handles most)
├── gradle.properties            # Gradle properties (optional)
├── release-signing.properties.example  # Local signing config template
└── res/
    ├── values/
    │   ├── strings.xml          # App strings (name, descriptions)
    │   └── styles.xml           # App themes
    └── mipmap-*/                # App icons (to be added for different densities)
        └── ic_launcher.png
```

## Android Resources

### Strings (res/values/strings.xml)
- `app_name`: Display name shown on device
- `app_description`: Short description (used in app info)

### Styles (res/values/styles.xml)
- Base theme for the app (currently using fullscreen theme, suitable for a map viewer)

### Icons (res/mipmap-*)
- Create app launcher icons for different screen densities:
  - `mipmap-ldpi/`: Low density (~120 dpi)
  - `mipmap-mdpi/`: Medium density (~160 dpi)
  - `mipmap-hdpi/`: High density (~240 dpi)
  - `mipmap-xhdpi/`: Extra-high density (~320 dpi)
  - `mipmap-xxhdpi/`: Extra-extra-high density (~480 dpi)
  - `mipmap-xxxhdpi/`: Extra-extra-extra-high density (~640 dpi)

Each icon should be a PNG file named `ic_launcher.png`.

## Building for Android

Run the setup script from the repo root:

```bash
./tools/setup-android.sh
```

Then configure and build:

```bash
cmake --preset android-arm64-v8a-release
cmake --build build-android-arm64-v8a --target supercell-wx
```

## Deploying to APK

After a successful build, use Qt's `androiddeployqt` tool to create an APK:

```bash
androiddeployqt --input build-android-arm64-v8a/AndroidDeploymentSettings.json \
                --output build-android-arm64-v8a/android-build \
                --android-platform android-33 \
                --apk
```

## Signing

For release builds, copy `release-signing.properties.example` to `release-signing.properties` and fill in your keystore path, alias, and passwords. Keep the real file out of version control.

See [APK_SIGNING.md](../../docs/APK_SIGNING.md) for the full packaging and signing workflow.

## Permissions

The following permissions are requested in `AndroidManifest.xml`:

- `INTERNET` — Download radar data from AWS/NWS
- `ACCESS_NETWORK_STATE` — Check network connectivity
- `ACCESS_FINE_LOCATION` — GPS positioning (optional, for future location-based features)
- `ACCESS_COARSE_LOCATION` — Network-based positioning
- `READ_EXTERNAL_STORAGE` — Load custom placefiles or data
- `WRITE_EXTERNAL_STORAGE` — Cache downloaded data

For Android 6.0+ (API 23+), runtime permissions are required for dangerous permissions. The Qt app framework handles this automatically for the most critical ones.

## References

- [Qt for Android](https://doc.qt.io/qt-6/android.html)
- [Android Manifest Documentation](https://developer.android.com/guide/topics/manifest/manifest-intro)
- [Android Resource Types](https://developer.android.com/guide/topics/resources/available-resources)
- [Material Design Icons](https://material.io/resources/icons/)

## Next Steps

1. Add high-quality launcher icons for different screen densities
2. Test on emulator and real devices (Pixel 6/7, Moto G series)
3. Optimize UI for touch interactions (larger buttons, swipe gestures)
4. Configure signing keys for Play Store release builds
5. Test location permissions and network access
