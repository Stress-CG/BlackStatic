param(
    [string]$UERoot = "D:\Epic Games\UE_5.7",
    [switch]$Quiet
)

$ErrorActionPreference = "Stop"

function Write-Step {
    param([string]$Message)
    if (-not $Quiet) {
        Write-Host "[bootstrap] $Message"
    }
}

function Assert-Path {
    param(
        [string]$PathValue,
        [string]$Label
    )

    if (-not (Test-Path $PathValue)) {
        throw "$Label not found at '$PathValue'."
    }
}

$script:RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$script:ProjectFile = Join-Path $script:RepoRoot "BlackStatic.uproject"
$script:VsDevCmd = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
$script:BuildBat = Join-Path $UERoot "Engine\Build\BatchFiles\Build.bat"
$script:RunUAT = Join-Path $UERoot "Engine\Build\BatchFiles\RunUAT.bat"
$script:EditorCmd = Join-Path $UERoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

Assert-Path $script:ProjectFile "Project file"
Assert-Path $script:VsDevCmd "Visual Studio developer shell"
Assert-Path $script:BuildBat "Unreal Build.bat"
Assert-Path $script:RunUAT "Unreal RunUAT.bat"
Assert-Path $script:EditorCmd "UnrealEditor-Cmd.exe"

Write-Step "Project: $script:ProjectFile"
Write-Step "Engine root: $UERoot"
Write-Step "Visual Studio developer shell detected."
Write-Step "Bootstrap verification complete."

