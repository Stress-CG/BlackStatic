param(
    [string]$UERoot = "D:\Epic Games\UE_5.7",
    [string]$OutputRoot = "",
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$ProjectFile = Join-Path $RepoRoot "BlackStatic.uproject"
$EditorCmd = Join-Path $UERoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$Timestamp = Get-Date -Format "yyyyMMdd-HHmmss"

if ([string]::IsNullOrWhiteSpace($OutputRoot)) {
    $OutputRoot = Join-Path $RepoRoot "Saved\Benchmarks\$Timestamp"
}

& (Join-Path $PSScriptRoot "bootstrap.ps1") -UERoot $UERoot -Quiet

if (-not $SkipBuild) {
    & (Join-Path $PSScriptRoot "build.ps1") -UERoot $UERoot
}

New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null

$CommandArgs = @(
    "`"$ProjectFile`"",
    "-run=BlackStaticBenchmark",
    "-BootstrapAssets",
    "-Mode=all",
    "-OutputDir=`"$OutputRoot`"",
    "-unattended",
    "-nop4",
    "-nosplash",
    "-NullRHI",
    "-NoSound"
)

Write-Host "[benchmark] Output: $OutputRoot"
& $EditorCmd $CommandArgs
if ($LASTEXITCODE -ne 0) {
    throw "Benchmark commandlet failed with exit code $LASTEXITCODE."
}

Write-Host "[benchmark] Completed successfully."

