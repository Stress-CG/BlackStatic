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

Write-Host "[phase0] Opening MAP_Phase0_Prototype in Unreal Editor"
& $EditorExe "`"$ProjectFile`"" "/Game/Phase0/Maps/MAP_Phase0_Prototype"
