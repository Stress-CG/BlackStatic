param(
    [string]$UERoot = "D:\Epic Games\UE_5.7",
    [switch]$SkipBuild,
    [ValidateSet("Main", "Prototype", "FirstPerson")]
    [string]$Map = "Main"
)

$ErrorActionPreference = "Stop"

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$ProjectFile = Join-Path $RepoRoot "BlackStatic.uproject"
$EditorExe = Join-Path $UERoot "Engine\Binaries\Win64\UnrealEditor.exe"

& (Join-Path $PSScriptRoot "bootstrap.ps1") -UERoot $UERoot -Quiet

if (-not $SkipBuild) {
    & (Join-Path $PSScriptRoot "build.ps1") -UERoot $UERoot
}

$MapPath = switch ($Map) {
    "Prototype" { "/Game/Phase0/Maps/MAP_Phase0_Prototype" }
    "FirstPerson" { "/Game/FirstPerson/Lvl_FirstPerson" }
    default { "/Game/MAP_TutorialRoad_P" }
}

Write-Host "[phase0] Opening $MapPath in Unreal Editor"
& $EditorExe "`"$ProjectFile`"" $MapPath
