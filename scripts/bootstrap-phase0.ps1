param(
    [string]$UERoot = "D:\Epic Games\UE_5.7",
    [switch]$SkipBuild,
    [switch]$AssetsOnly,
    [switch]$MapOnly,
    [switch]$PrototypeOnly,
    [switch]$MainMapsOnly,
    [string[]]$TargetMap
)

$ErrorActionPreference = "Stop"

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$ProjectFile = Join-Path $RepoRoot "BlackStatic.uproject"
$EditorCmd = Join-Path $UERoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

& (Join-Path $PSScriptRoot "bootstrap.ps1") -UERoot $UERoot -Quiet

if (-not $SkipBuild) {
    & (Join-Path $PSScriptRoot "build.ps1") -UERoot $UERoot
}

$CommandArgs = @(
    "`"$ProjectFile`"",
    "-run=BSPhase0Bootstrap",
    "-unattended",
    "-nop4",
    "-nosplash",
    "-NullRHI",
    "-NoSound"
)

if ($AssetsOnly) {
    $CommandArgs += "-AssetsOnly"
}

if ($MapOnly) {
    $CommandArgs += "-MapOnly"
}

if ($PrototypeOnly) {
    $CommandArgs += "-PrototypeOnly"
}

if ($MainMapsOnly) {
    $CommandArgs += "-MainMapsOnly"
}

if ($TargetMap) {
    $CommandArgs += "-TargetMap=$($TargetMap -join ',')"
}

Write-Host "[phase0] Bootstrapping Phase 0 assets and gameplay maps"
& $EditorCmd $CommandArgs
if ($LASTEXITCODE -ne 0) {
    throw "Phase 0 bootstrap commandlet failed with exit code $LASTEXITCODE."
}

Write-Host "[phase0] Bootstrap completed successfully."
