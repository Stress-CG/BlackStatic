param(
    [switch]$VerboseOutput
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$folders = @(
    "Content/Art/Library/_Vendors",
    "Content/Art/Library/Architecture/Houses",
    "Content/Art/Library/Architecture/Garages",
    "Content/Art/Library/Architecture/Commercial",
    "Content/Art/Library/Architecture/Industrial",
    "Content/Art/Library/Architecture/Settlement",
    "Content/Art/Library/Architecture/Utilities",
    "Content/Art/Library/Vehicles/Civilian",
    "Content/Art/Library/Vehicles/Industrial",
    "Content/Art/Library/Vehicles/Wrecks",
    "Content/Art/Library/Nature/Foliage/Trees",
    "Content/Art/Library/Nature/Foliage/Shrubs",
    "Content/Art/Library/Nature/Foliage/Grass",
    "Content/Art/Library/Nature/Foliage/GroundCover",
    "Content/Art/Library/Nature/Fauna",
    "Content/Art/Library/Props/Roadside",
    "Content/Art/Library/Props/Industrial",
    "Content/Art/Library/Props/Interior",
    "Content/Art/Library/Props/Settlement",
    "Content/Art/Library/Materials/Master",
    "Content/Art/Library/Materials/Instances",
    "Content/Art/Library/Decals",
    "Content/Art/Library/Prefabs/Settlement",
    "Content/Art/Library/Prefabs/Suburban",
    "Content/Art/Library/Prefabs/Urban",
    "Content/Art/Library/Prefabs/Military",
    "Content/Art/Library/Prefabs/Airport",
    "docs/asset-packs"
)

$created = @()
foreach ($relativeFolder in $folders) {
    $absoluteFolder = Join-Path $projectRoot $relativeFolder
    if (-not (Test-Path -LiteralPath $absoluteFolder)) {
        New-Item -ItemType Directory -Path $absoluteFolder -Force | Out-Null
        $created += $relativeFolder
    }
    elseif ($VerboseOutput) {
        Write-Host "[asset-library] Exists: $relativeFolder"
    }
}

Write-Host "[asset-library] Library folders verified: $($folders.Count)"
if ($created.Count -gt 0) {
    Write-Host "[asset-library] Created:"
    foreach ($relativeFolder in $created) {
        Write-Host "  - $relativeFolder"
    }
}
else {
    Write-Host "[asset-library] No new folders were needed."
}
