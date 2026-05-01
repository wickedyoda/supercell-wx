Android APK/AAB packaging and signing
=====================================

This document covers the release packaging path for the Android client.

Packaging model
---------------
- **Debug builds** use the Qt Android deployment defaults and can be installed directly on a device or emulator.
- **Release builds** should be signed with a dedicated release keystore before being uploaded to Play Console or distributed internally.
- **Target artifacts**: APK for sideloading, AAB for Play Store distribution.

Project files added
-------------------
- [scwx-qt/android/gradle.properties](../scwx-qt/android/gradle.properties)
- [scwx-qt/android/release-signing.properties.example](../scwx-qt/android/release-signing.properties.example)

Gradle defaults
---------------
The Gradle defaults keep Android builds stable and predictable:
- `android.useAndroidX=true`
- `android.nonTransitiveRClass=true`
- `org.gradle.jvmargs=-Xmx4g`

These settings are only defaults for the Android package source directory. Qt still owns the core CMake-driven build and deployment flow.

Signing workflow
----------------
1. Create a release keystore.
2. Copy `release-signing.properties.example` to `release-signing.properties`.
3. Fill in your keystore path, alias, and passwords.
4. Load those values in your packaging script or CI secrets.
5. Use `androiddeployqt` to generate the APK/AAB, then apply signing during the release packaging step.

Example keystore creation
-------------------------
```bash
keytool -genkeypair -v \
  -keystore supercell-wx-release.jks \
  -alias supercellwx \
  -keyalg RSA \
  -keysize 2048 \
  -validity 10000
```

Example local signing file
--------------------------
```properties
storeFile=/absolute/path/to/supercell-wx-release.jks
storePassword=changeit
keyAlias=supercellwx
keyPassword=changeit
```

APK/AAB generation
------------------
After a successful Android build, use Qt deployment tools to stage the package:

```bash
androiddeployqt \
  --input build-android-arm64/AndroidDeploymentSettings.json \
  --output build-android-arm64/android-build \
  --android-platform android-33 \
  --apk
```

For App Bundles, switch the deployment target from APK to AAB in your release pipeline and keep the same signing material.

CI and release notes
--------------------
- Store the signing file in CI secrets, not in the repository.
- Keep the keystore offline or in a secure secret manager.
- Use separate debug and release package names only if you need parallel installs.
- Re-check `versionCode` and `versionName` before publishing.
