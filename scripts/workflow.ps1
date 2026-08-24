#requires -Version 7.2

[CmdletBinding()]
param(
    [string]$Preset,
    [switch]$Fresh,
    [switch]$CleanFirst,
    [switch]$WarningsAsErrors,
    [switch]$NoTest,
    [ValidateRange(1, 256)]
    [int]$Jobs
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

$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
$isWindowsPlatform = [System.Runtime.InteropServices.RuntimeInformation]::IsOSPlatform(
    [System.Runtime.InteropServices.OSPlatform]::Windows)

if (-not $Preset) {
    $Preset = if ($isWindowsPlatform) { "debug" } else { "ninja-debug" }
}

$testablePresets = @(
    "debug",
    "release",
    "relwithdebinfo",
    "sanitize",
    "ninja-debug",
    "ninja-release",
    "ninja-relwithdebinfo",
    "ninja-sanitize"
)
if ($Preset -notin $testablePresets) {
    throw "Preset '$Preset' is not a build-and-test preset. Choose: $($testablePresets -join ', ')"
}

$cmake = (Get-Command cmake -ErrorAction Stop).Source
$ctest = (Get-Command ctest -ErrorAction Stop).Source

Push-Location $repoRoot
try {
    $configureArguments = @("--preset", $Preset)
    if ($Fresh) {
        $configureArguments += "--fresh"
    }
    if ($WarningsAsErrors) {
        $configureArguments += "-DSTLPP_WARNINGS_AS_ERRORS=ON"
    }
    Invoke-Checked $cmake $configureArguments

    $buildArguments = @("--build", "--preset", $Preset)
    if ($CleanFirst) {
        $buildArguments += "--clean-first"
    }
    if ($Jobs) {
        $buildArguments += @("--parallel", $Jobs.ToString())
    }
    Invoke-Checked $cmake $buildArguments

    if (-not $NoTest) {
        Invoke-Checked $ctest @("--preset", $Preset, "--output-on-failure")
    }
} finally {
    Pop-Location
}
