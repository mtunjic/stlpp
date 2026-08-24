# Releasing STL++

`VERSION` is the authoritative stable semantic version. CMake and Conan read it directly. The
version updater synchronizes the public C++ macros and vcpkg manifest; the validator ensures every
surface matches `VERSION`.

## Prepare a release

Start from a clean `master` branch and choose a version greater than the current `VERSION`.
Preview the guarded operation first:

```powershell
./scripts/publish_release.ps1 -Version 0.2.0 -WhatIf
```

Prepare and verify the release locally:

```powershell
./scripts/publish_release.ps1 -Version 0.2.0
```

The script runs a fresh strict Release build and all tests, validates the CMake install archives
and Conan 2 package, creates a release commit, and creates annotated tag `v0.2.0`. It does not push
by default.

Use `-SkipConan` only when Conan 2 is unavailable. GitHub CI still validates both the Conan recipe
and vcpkg overlay.

## Publish

After reviewing the prepared commit and tag, push both atomically:

```powershell
git push --atomic origin HEAD:refs/heads/master refs/tags/v0.2.0
```

Alternatively, prepare and push in one confirmed operation:

```powershell
./scripts/publish_release.ps1 -Version 0.2.0 -Push
```

Tags must be annotated and named `v<major>.<minor>.<patch>`. Pushing a valid tag starts the GitHub
Release workflow, which revalidates the tag, builds and tests the project, creates install-tree
`.tar.gz` and `.zip` archives, and publishes generated release notes.

Publishing to Conan Center or the public vcpkg registry remains a separate registry review process.
