param(
    [string]$UERoot = "D:\Epic Games\UE_5.7",
    [string]$Configuration = "Development",
    [string]$ArchiveRoot = "",
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$ProjectFile = Join-Path $RepoRoot "BlackStatic.uproject"
$EditorCmd = Join-Path $UERoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$RunUAT = Join-Path $UERoot "Engine\Build\BatchFiles\RunUAT.bat"

if ([string]::IsNullOrWhiteSpace($ArchiveRoot)) {
    $ArchiveRoot = Join-Path $RepoRoot "Artifacts\Windows\$(Get-Date -Format 'yyyyMMdd-HHmmss')"
}

& (Join-Path $PSScriptRoot "bootstrap.ps1") -UERoot $UERoot -Quiet

if (-not $SkipBuild) {
    & (Join-Path $PSScriptRoot "build.ps1") -UERoot $UERoot -Configuration $Configuration
}

$BootstrapArgs = @(
    "`"$ProjectFile`"",
    "-run=BlackStaticBenchmark",
    "-BootstrapAssets",
    "-BootstrapOnly",
    "-unattended",
    "-nop4",
    "-nosplash",
    "-NullRHI",
    "-NoSound"
)

Write-Host "[package] Ensuring benchmark assets and map wiring exist"
& $EditorCmd $BootstrapArgs
if ($LASTEXITCODE -ne 0) {
    throw "Asset bootstrap failed with exit code $LASTEXITCODE."
}

New-Item -ItemType Directory -Force -Path $ArchiveRoot | Out-Null

$UatArgs = @(
    "BuildCookRun",
    "-project=`"$ProjectFile`"",
    "-noP4",
    "-platform=Win64",
    "-clientconfig=$Configuration",
    "-serverconfig=$Configuration",
    "-build",
    "-cook",
    "-stage",
    "-pak",
    "-archive",
    "-archivedirectory=`"$ArchiveRoot`"",
    "-map=/Game/Benchmarks/Maps/MAP_Benchmark+/Game/MAP_TutorialRoad_P"
)

Write-Host "[package] Packaging Windows build to $ArchiveRoot"
& $RunUAT $UatArgs
if ($LASTEXITCODE -ne 0) {
    throw "Packaging failed with exit code $LASTEXITCODE."
}

Write-Host "[package] Packaging completed successfully."

