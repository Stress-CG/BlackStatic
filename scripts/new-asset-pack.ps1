param(
    [Parameter(Mandatory = $true)]
    [string]$Vendor,

    [Parameter(Mandatory = $true)]
    [string]$PackName
)

$ErrorActionPreference = "Stop"

function Normalize-Name {
    param([string]$Value)

    return ($Value.Trim() -replace "[^A-Za-z0-9_-]", "_")
}

$projectRoot = Split-Path -Parent $PSScriptRoot
$safeVendor = Normalize-Name -Value $Vendor
$safePackName = Normalize-Name -Value $PackName

$vendorRoot = Join-Path $projectRoot "Content/Art/Library/_Vendors/$safeVendor/$safePackName"
$subfolders = @(
    "Meshes",
    "Materials",
    "Textures",
    "Blueprints",
    "Docs"
)

foreach ($subfolder in $subfolders) {
    New-Item -ItemType Directory -Path (Join-Path $vendorRoot $subfolder) -Force | Out-Null
}

$reviewNotesPath = Join-Path $projectRoot "docs/asset-packs/$safePackName.md"
if (-not (Test-Path -LiteralPath $reviewNotesPath)) {
    @"
# $PackName

- Vendor: $Vendor
- Raw root: Content/Art/Library/_Vendors/$safeVendor/$safePackName
- Intended target zones:
- Curated destination folders:
- Scale checked:
- Collision checked:
- Materials unified:
- Nanite or LOD checked:
- Notes:
"@ | Set-Content -Path $reviewNotesPath
}

Write-Host "[asset-library] Pack scaffold ready:"
Write-Host "  Raw root: Content/Art/Library/_Vendors/$safeVendor/$safePackName"
Write-Host "  Review notes: docs/asset-packs/$safePackName.md"
