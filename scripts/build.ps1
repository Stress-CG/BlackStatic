param(
    [string]$UERoot = "D:\Epic Games\UE_5.7",
    [string]$Configuration = "Development",
    [string]$Target = "BlackStaticEditor"
)

$ErrorActionPreference = "Stop"

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$ProjectFile = Join-Path $RepoRoot "BlackStatic.uproject"
$VsDevCmd = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
$BuildBat = Join-Path $UERoot "Engine\Build\BatchFiles\Build.bat"

& (Join-Path $PSScriptRoot "bootstrap.ps1") -UERoot $UERoot -Quiet

$BuildCommand = "call `"$VsDevCmd`" && call `"$BuildBat`" $Target Win64 $Configuration `"$ProjectFile`" -WaitMutex -NoHotReloadFromIDE"

Write-Host "[build] Building $Target $Configuration"
cmd.exe /c $BuildCommand
if ($LASTEXITCODE -ne 0) {
    throw "Unreal build failed with exit code $LASTEXITCODE."
}

Write-Host "[build] Build completed successfully."
