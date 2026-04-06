param(
    [string]$UERoot = "D:\Epic Games\UE_5.7",
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$ProjectFile = Join-Path $RepoRoot "BlackStatic.uproject"
$EditorCmd = Join-Path $UERoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$TestRoot = Join-Path $RepoRoot "Saved\Tests\$(Get-Date -Format 'yyyyMMdd-HHmmss')"
$BaselineDir = Join-Path $TestRoot "baseline"
$ImprovedDir = Join-Path $TestRoot "improved"
$CompareScript = Join-Path $RepoRoot "tools\compare_runs.py"

& (Join-Path $PSScriptRoot "bootstrap.ps1") -UERoot $UERoot -Quiet

if (-not $SkipBuild) {
    & (Join-Path $PSScriptRoot "build.ps1") -UERoot $UERoot
}

New-Item -ItemType Directory -Force -Path $BaselineDir, $ImprovedDir | Out-Null

$AutomationArgs = @(
    "`"$ProjectFile`"",
    "-ExecCmds=`"Automation RunTests BlackStatic.Benchmark;Quit`"",
    "-TestExit=`"Automation Test Queue Empty`"",
    "-unattended",
    "-nop4",
    "-nosplash",
    "-NullRHI",
    "-NoSound"
)

Write-Host "[test] Running Unreal automation tests"
& $EditorCmd $AutomationArgs
if ($LASTEXITCODE -ne 0) {
    throw "Automation tests failed with exit code $LASTEXITCODE."
}

$BaselineArgs = @(
    "`"$ProjectFile`"",
    "-run=BlackStaticBenchmark",
    "-BootstrapAssets",
    "-Mode=baseline",
    "-OutputDir=`"$BaselineDir`"",
    "-unattended",
    "-nop4",
    "-nosplash",
    "-NullRHI",
    "-NoSound"
)

$ImprovedArgs = @(
    "`"$ProjectFile`"",
    "-run=BlackStaticBenchmark",
    "-BootstrapAssets",
    "-Mode=improved",
    "-OutputDir=`"$ImprovedDir`"",
    "-unattended",
    "-nop4",
    "-nosplash",
    "-NullRHI",
    "-NoSound"
)

Write-Host "[test] Running baseline benchmark suite"
& $EditorCmd $BaselineArgs
if ($LASTEXITCODE -ne 0) {
    throw "Baseline benchmark suite failed with exit code $LASTEXITCODE."
}

Write-Host "[test] Running improved benchmark suite"
& $EditorCmd $ImprovedArgs
if ($LASTEXITCODE -ne 0) {
    throw "Improved benchmark suite failed with exit code $LASTEXITCODE."
}

Write-Host "[test] Comparing benchmark output bundles"
python $CompareScript --baseline $BaselineDir --improved $ImprovedDir --enforce
if ($LASTEXITCODE -ne 0) {
    throw "Benchmark comparison failed with exit code $LASTEXITCODE."
}

Write-Host "[test] Test suite completed successfully."

