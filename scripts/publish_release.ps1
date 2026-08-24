#requires -Version 7.2

[CmdletBinding(SupportsShouldProcess, ConfirmImpact = "High")]
param(
    [Parameter(Mandatory)]
    [ValidatePattern('^[0-9]+\.[0-9]+\.[0-9]+$')]
    [string]$Version,

    [string]$Preset,
    [string]$Branch = "master",
    [string]$Remote = "origin",
    [switch]$SkipConan,
    [switch]$Push
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Invoke-Checked {
    param(
        [Parameter(Mandatory)]
        [string]$Command,
        [string[]]$ArgumentList = @()
    )

    Write-Host "> $Command $($ArgumentList -join ' ')"
    & $Command @ArgumentList
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed with exit code ${LASTEXITCODE}: $Command"
    }
}

function Get-CheckedOutput {
    param(
        [Parameter(Mandatory)]
        [string]$Command,
        [string[]]$ArgumentList = @()
    )

    $output = & $Command @ArgumentList
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed with exit code ${LASTEXITCODE}: $Command"
    }
    return ($output -join "`n").Trim()
}

$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
$isWindowsPlatform = [System.Runtime.InteropServices.RuntimeInformation]::IsOSPlatform(
    [System.Runtime.InteropServices.OSPlatform]::Windows)
if (-not $Preset) {
    $Preset = if ($isWindowsPlatform) { "release" } else { "ninja-release" }
}
if ($Preset -notin @("release", "ninja-release")) {
    throw "Release preparation supports only 'release' and 'ninja-release' presets"
}

$packagePreset = if ($Preset.StartsWith("ninja-")) { "ninja-pkg" } else { "pkg" }
$packageBuildDirectory = if ($packagePreset -eq "ninja-pkg") {
    "out/build/ninja-pkg"
} else {
    "out/build/vs/pkg"
}
$tag = "v$Version"
$versionFiles = @("VERSION", "algorithms/stlpp.h", "ports/stlpp/vcpkg.json")
$cmake = (Get-Command cmake -ErrorAction Stop).Source
$ctest = (Get-Command ctest -ErrorAction Stop).Source
$git = (Get-Command git -ErrorAction Stop).Source
$conan = if ($SkipConan) { $null } else { Get-Command conan -ErrorAction SilentlyContinue }
if (-not $SkipConan -and -not $conan) {
    throw "Conan 2 is required for release verification. Install it or pass -SkipConan."
}
if ($conan) {
    $conanVersion = Get-CheckedOutput $conan.Source @("--version")
    if ($conanVersion -notmatch '^Conan version 2\.') {
        throw "Conan 2 is required; found '$conanVersion'"
    }
}

Push-Location $repoRoot
try {
    $currentBranch = Get-CheckedOutput $git @("branch", "--show-current")
    if ($currentBranch -ne $Branch) {
        throw "Release preparation must run on '$Branch'; current branch is '$currentBranch'"
    }

    $status = Get-CheckedOutput $git @("status", "--porcelain=v1", "--untracked-files=normal")
    if ($status) {
        throw "Release preparation requires a clean worktree.`n$status"
    }

    $currentVersion = (Get-Content -Raw VERSION).Trim()
    if ([version]$Version -le [version]$currentVersion) {
        throw "New version $Version must be greater than current version $currentVersion"
    }

    & $git show-ref --verify --quiet "refs/tags/$tag"
    if ($LASTEXITCODE -eq 0) {
        throw "Local tag $tag already exists"
    }
    if ($LASTEXITCODE -notin @(0, 1)) {
        throw "Could not check whether tag $tag exists"
    }

    if ($Push) {
        [void](Get-CheckedOutput $git @("remote", "get-url", $Remote))
    }

    $packageName = "stlpp-$Version"
    $packageRoot = Join-Path $repoRoot "out/package"
    $installPrefix = Join-Path $packageRoot $packageName
    $releaseOutputs = @($installPrefix, "${installPrefix}.tar.gz", "${installPrefix}.zip")
    foreach ($path in $releaseOutputs) {
        if (Test-Path -LiteralPath $path) {
            throw "Release output already exists: $path"
        }
    }

    $action = "bump $currentVersion to $Version, verify packages, commit, and tag $tag"
    if ($Push) {
        $action += ", then atomically push branch and tag to $Remote"
    }
    if (-not $PSCmdlet.ShouldProcess($repoRoot, $action)) {
        return
    }

    $versionUpdated = $false
    $commitCreated = $false
    try {
        Invoke-Checked $cmake @("-DSTLPP_VERSION=$Version", "-P", "cmake/SetVersion.cmake")
        $versionUpdated = $true
        Invoke-Checked $cmake @("-DSTLPP_GIT_TAG=$tag", "-P", "cmake/ValidateVersion.cmake")

        & (Join-Path $PSScriptRoot "workflow.ps1") `
            -Preset $Preset -Fresh -CleanFirst -WarningsAsErrors

        Invoke-Checked $cmake @("--preset", $packagePreset, "--fresh")
        Invoke-Checked $cmake @("--build", "--preset", $packagePreset)
        $installArguments = @("--install", $packageBuildDirectory, "--prefix", $installPrefix)
        if ($packagePreset -eq "pkg") {
            $installArguments += @("--config", "Release")
        }
        Invoke-Checked $cmake $installArguments

        Push-Location $packageRoot
        try {
            Invoke-Checked $cmake @("-E", "tar", "czf", "$packageName.tar.gz", $packageName)
            Invoke-Checked $cmake @("-E", "tar", "cf", "$packageName.zip", "--format=zip", $packageName)
            Invoke-Checked $cmake @("-E", "tar", "tf", "$packageName.tar.gz")
            Invoke-Checked $cmake @("-E", "tar", "tf", "$packageName.zip")
        } finally {
            Pop-Location
        }

        if (-not $SkipConan) {
            Invoke-Checked $conan.Source @("create", ".", "--build=missing", "-s", "compiler.cppstd=20")
        }

        Invoke-Checked $git @("diff", "--check")
        $changedFiles = @(& $git diff --name-only)
        if ($LASTEXITCODE -ne 0) {
            throw "Could not list release changes"
        }
        $unexpected = @(Compare-Object ($versionFiles | Sort-Object) ($changedFiles | Sort-Object))
        if ($unexpected.Count -ne 0) {
            throw "Version bump changed unexpected files:`n$($changedFiles -join "`n")"
        }

        Invoke-Checked $git (@("add", "--") + $versionFiles)
        Invoke-Checked $git @("diff", "--cached", "--check")
        Invoke-Checked $git @("commit", "-m", "Prepare $tag")
        $commitCreated = $true
        Invoke-Checked $git @("tag", "-a", $tag, "-m", "stlpp $tag")

        if ($Push) {
            Invoke-Checked $git @(
                "push",
                "--atomic",
                $Remote,
                "HEAD:refs/heads/$Branch",
                "refs/tags/$tag"
            )
            Write-Host "Published $tag. GitHub Actions will create the release."
        } else {
            Write-Host "Prepared commit and annotated tag $tag. Nothing was pushed."
            Write-Host "Push both atomically when ready:"
            Write-Host "  git push --atomic $Remote HEAD:refs/heads/$Branch refs/tags/$tag"
        }
    } catch {
        if ($versionUpdated -and -not $commitCreated) {
            Write-Warning "Release preparation failed before commit; restoring $currentVersion"
            & $git restore --staged -- @versionFiles 2>$null
            & $cmake "-DSTLPP_VERSION=$currentVersion" -P cmake/SetVersion.cmake | Out-Host
            foreach ($path in $releaseOutputs) {
                if (Test-Path -LiteralPath $path) {
                    Remove-Item -LiteralPath $path -Recurse -Force
                }
            }
        }
        throw
    }
} finally {
    Pop-Location
}
