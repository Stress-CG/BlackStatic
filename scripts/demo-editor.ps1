param(
    [string]$UERoot = "D:\Epic Games\UE_5.7",
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$ProjectFile = Join-Path $RepoRoot "BlackStatic.uproject"
$EditorExe = Join-Path $UERoot "Engine\Binaries\Win64\UnrealEditor.exe"

& (Join-Path $PSScriptRoot "bootstrap.ps1") -UERoot $UERoot -Quiet

if (-not $SkipBuild) {
    & (Join-Path $PSScriptRoot "build.ps1") -UERoot $UERoot
}

Write-Host "[demo] Opening MAP_Benchmark in Unreal Editor"
& $EditorExe "`"$ProjectFile`"" "/Game/Benchmarks/Maps/MAP_Benchmark"
