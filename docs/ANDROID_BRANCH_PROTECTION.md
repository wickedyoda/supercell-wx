# Android Port Branch Protection Setup

This document describes how to configure GitHub branch protection rules for the `android-port-build` branch to ensure all APK builds pass automated checks before deployment.

## Overview

The `android-port-build` branch is the integration point where Android port code is built and packaged into APK/AAB files. To maintain code quality and build reliability:

- **Required Status Check**: The `android-build` CI job must pass before any PR can merge
- **Push Protection**: Direct pushes are blocked; all changes must go through pull requests
- **Review Requirements**: Stale reviews are dismissed on new commits
- **Automatic Triggering**: Successful merges to `android-port-build` automatically trigger the APK build workflow

## Branch Protection Rules

### Required Configuration

| Setting | Value | Purpose |
|---------|-------|---------|
| **Require a pull request before merging** | ✅ Yes | Enforce code review workflow |
| **Dismiss stale pull request approvals when new commits are pushed** | ✅ Yes | Require re-review after changes |
| **Require status checks to pass before merging** | ✅ Yes | Enforce CI validation |
| **Required status checks** | `android-build` | Requires APK build job to pass |
| **Require branches to be up to date before merging** | ✅ Yes | Prevent merge conflicts |
| **Require dismissal of stale reviews** | ✅ Yes | Enforce fresh reviews |
| **Block direct pushes** | ✅ Yes (implicit) | All changes via PR only |

### Optional Configuration

| Setting | Recommendation | Purpose |
|---------|---|---------|
| **Require reviews from code owners** | ⚠️ Suggested | Code owners must review Android changes |
| **Number of approvals required** | 1 | At least one approval before merge |
| **Require conversation resolution before merging** | ✅ Yes | Resolve all review comments |
| **Include administrators** | ✅ Yes | Admins must also follow rules |
| **Restrict who can push to matching branches** | ⚠️ Optional | Limit to @dpaulat or core team |

## Setup Instructions

### Option 1: GitHub UI (Recommended for First-Time Setup)

1. **Navigate to Repository Settings**
   - Go to your GitHub repository: `https://github.com/dpaulat/supercell-wx`
   - Click **Settings** tab
   - In left sidebar, click **Branches**

2. **Add Branch Protection Rule**
   - Click **Add rule**
   - Branch name pattern: `android-port-build`

3. **Configure Protection Rules**
   - ✅ Check: **Require a pull request before merging**
     - Set: Require 1 approval (optional but recommended)
     - Check: Dismiss stale pull request approvals
     - Check: Require review from code owners (if CODEOWNERS exists)
   
   - ✅ Check: **Require status checks to pass before merging**
     - Search for and select: `android-build` (the job name from CI workflow)
     - Check: Require branches to be up to date before merging
   
   - ✅ Check: **Require conversation resolution before merging**
   
   - ✅ Check: **Restrict who can push to matching branches** (optional)
     - Add: @dpaulat (or core Android team members)

4. **Save Protection Rule**
   - Click **Create**
   - Verify rule appears in Branches settings

### Option 2: GitHub CLI (Fastest)

```bash
# Requires: GitHub CLI installed and authenticated
# See: https://cli.github.com/

# List current branch protection rules
gh api repos/dpaulat/supercell-wx/branches/android-port-build/protection

# Create/Update branch protection rule
gh api repos/dpaulat/supercell-wx/branches/android-port-build/protection \
  --input - <<EOF
{
  "required_status_checks": {
    "strict": true,
    "contexts": ["android-build"]
  },
  "required_pull_request_reviews": {
    "required_approving_review_count": 1,
    "dismiss_stale_reviews": true
  },
  "enforce_admins": true,
  "required_linear_history": false,
  "allow_force_pushes": false,
  "allow_deletions": false
}
EOF
```

### Option 3: GitHub API (Programmatic)

```bash
# Using curl (requires GITHUB_TOKEN environment variable)
curl -X PUT \
  -H "Authorization: token $GITHUB_TOKEN" \
  -H "Accept: application/vnd.github.v3+json" \
  https://api.github.com/repos/dpaulat/supercell-wx/branches/android-port-build/protection \
  -d '{
    "required_status_checks": {
      "strict": true,
      "contexts": ["android-build"]
    },
    "required_pull_request_reviews": {
      "required_approving_review_count": 1,
      "dismiss_stale_reviews": true
    },
    "enforce_admins": true,
    "required_linear_history": false,
    "allow_force_pushes": false,
    "allow_deletions": false
  }'
```

## Verification

After setting up branch protection, verify the rules are active:

1. **Check GitHub UI**
   - Go to Settings → Branches
   - Verify `android-port-build` has the lock icon (🔒)
   - Click rule to view current configuration

2. **Verify CI Job Names**
   - Look at `.github/workflows/ci.yml`
   - Confirm job ID is `android-build` (appears at line ~457)
   - This must match the status check requirement

3. **Test Branch Protection**
   - Attempt to push directly to `android-port-build`: Should fail with "remote rejected"
   - Create a test PR with failing CI: PR should not be mergeable
   - Create a test PR with passing CI: PR should be mergeable (if approved)

## Workflow After Branch Protection

### Developer Workflow

```bash
# 1. Work on feature in android-port
git checkout android-port
git pull origin android-port
# ... make changes ...
git commit -m "Android: Add feature X"

# 2. Push to android-port
git push origin android-port

# 3. GitHub creates CI workflow run
# -> android-build job validates the changes
# -> Emulator smoke test runs automatically

# 4. Create pull request android-port → android-port-build
# -> Requires CI to pass
# -> Requires 1 approval
# -> Auto-triggers when merged
```

### Build Workflow

```
android-port
    ↓
  (PR created to android-port-build)
    ↓
  (CI: android-build job runs)
    ↓
  (CI: Emulator smoke test)
    ↓
  (Status: Pass/Fail)
    ↓
  (If Pass + Approved: Merge to android-port-build)
    ↓
  (CI: Automatically builds APK)
    ↓
  (Artifacts: APK/AAB ready for release)
```

## Status Check Details

### android-build Job

The `android-build` job in `.github/workflows/ci.yml`:

- **Trigger**: Runs on push to `android-port-build` OR manual workflow dispatch
- **Environment**: Ubuntu 24.04 LTS
- **Steps**:
  1. Install Java 17 (required for Android build tools)
  2. Install Qt 6.11.0 for Android
  3. Install Android SDK/NDK components
  4. Configure CMake with `android-x86_64-release` preset
  5. Build `supercell-wx` target
  6. Package APK using `androiddeployqt`
  7. Run Android emulator smoke test
  8. Upload APK artifact

- **Pass Condition**: All steps succeed, emulator smoke test passes
- **Fail Condition**: Any step fails or emulator test shows runtime errors

## Troubleshooting

### "No status checks are available"

- **Cause**: CI workflow hasn't run yet on the branch
- **Fix**: Merge a PR to `android-port-build` with passing CI first
- **Resolution**: After first CI run, status check becomes available

### "Status check context not found"

- **Cause**: Job name mismatch (job ID vs job name)
- **Fix**: Ensure rule uses `android-build` (the job ID, not display name)
- **Verify**: Check `.github/workflows/ci.yml` line ~457

### "Cannot merge despite passing CI"

- **Cause**: Rule requires stale reviews be dismissed or other requirement not met
- **Fix**: 
  - Request re-review from approver
  - Wait for new review after latest commit
  - Check all required conditions in PR UI

### CI Job Not Triggering

- **Cause**: CI workflow condition not met
- **Fix**: Ensure commit is on `android-port-build` branch
- **Reference**: See `.github/workflows/ci.yml` line ~459 condition

## CODEOWNERS Configuration (Optional)

For code ownership tracking, create `.github/CODEOWNERS`:

```
# Android Port Maintainers
# Any changes to android-port-build require review from these users
/scwx-qt/android/ @dpaulat
/.github/workflows/ci.yml @dpaulat

# Android-specific documentation
/docs/ANDROID*.md @dpaulat
```

Then enable "Require reviews from code owners" in branch protection rule.

## Monitoring & Maintenance

### Regular Checks

- **Weekly**: Monitor CI job success rate
- **Monthly**: Review branch protection rule settings
- **Quarterly**: Update documentation if workflow changes

### Update Branch Protection If

- Job name changes in CI workflow
- New required checks are added
- Team membership changes
- Release process modifications

## Related Documentation

- [Android Build Guide](ANDROID_BUILD.md) - Developer quick-start
- [APK Signing Guide](APK_SIGNING.md) - Release signing workflow
- [CI Workflow Details](.github/workflows/ci.yml) - Full CI configuration
- [Android Port Overview](ANDROID_PORT.md) - Architecture and planning

## References

- GitHub Docs: [About Protected Branches](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-protected-branches)
- GitHub Docs: [Defining code owners](https://docs.github.com/en/repositories/managing-your-repositorys-settings-and-features/customizing-your-repository/about-code-owners)
- GitHub CLI: [gh api command reference](https://cli.github.com/manual/gh_api)
