# Black Static Asset Library

This project now has a dedicated art-library layout under `Content/Art/Library`.

The goal is to keep three things separate:

1. Raw vendor imports that should remain close to the source pack.
2. Curated Black Static assets that have been renamed, checked, and standardized.
3. Reusable prefab sets for fast worldbuilding across settlements, suburbs, the city, military spaces, and the airport.

## Folder layout

Primary content root:

- `Content/Art/Library/_Vendors`
- `Content/Art/Library/Architecture`
- `Content/Art/Library/Vehicles`
- `Content/Art/Library/Nature`
- `Content/Art/Library/Props`
- `Content/Art/Library/Materials`
- `Content/Art/Library/Decals`
- `Content/Art/Library/Prefabs`

Architecture:

- `Content/Art/Library/Architecture/Houses`
- `Content/Art/Library/Architecture/Garages`
- `Content/Art/Library/Architecture/Commercial`
- `Content/Art/Library/Architecture/Industrial`
- `Content/Art/Library/Architecture/Settlement`
- `Content/Art/Library/Architecture/Utilities`

Vehicles:

- `Content/Art/Library/Vehicles/Civilian`
- `Content/Art/Library/Vehicles/Industrial`
- `Content/Art/Library/Vehicles/Wrecks`

Nature:

- `Content/Art/Library/Nature/Foliage/Trees`
- `Content/Art/Library/Nature/Foliage/Shrubs`
- `Content/Art/Library/Nature/Foliage/Grass`
- `Content/Art/Library/Nature/Foliage/GroundCover`
- `Content/Art/Library/Nature/Fauna`

Props:

- `Content/Art/Library/Props/Roadside`
- `Content/Art/Library/Props/Industrial`
- `Content/Art/Library/Props/Interior`
- `Content/Art/Library/Props/Settlement`

Materials and world sets:

- `Content/Art/Library/Materials/Master`
- `Content/Art/Library/Materials/Instances`
- `Content/Art/Library/Decals`
- `Content/Art/Library/Prefabs/Settlement`
- `Content/Art/Library/Prefabs/Suburban`
- `Content/Art/Library/Prefabs/Urban`
- `Content/Art/Library/Prefabs/Military`
- `Content/Art/Library/Prefabs/Airport`

## Recommended acquisition workflow

Use marketplace or scan-based content as the source, but do not dump promoted assets directly into gameplay folders.

The intended workflow is:

1. Acquire a pack through Fab, Quixel, or a DCC export pipeline.
2. Create a raw landing zone under `_Vendors/<Vendor>/<PackName>/`.
3. Keep original vendor names in the raw landing zone.
4. Curate the assets you actually want to ship.
5. Duplicate or migrate the curated assets into the standardized library folders above.
6. Rename curated assets to Black Static naming conventions.
7. Build reusable prefab Blueprints or grouped level actors in `Prefabs/*`.

## Naming standard

Keep raw imports raw. Standardize only the curated assets that the project will actively use.

Recommended prefixes:

- `SM_BS_` static meshes
- `SK_BS_` skeletal meshes
- `M_BS_` master materials
- `MI_BS_` material instances
- `T_BS_` textures
- `BP_BS_` Blueprint actors
- `FOL_BS_` foliage types
- `DA_BS_` data assets

Examples:

- `SM_BS_Car_Sedan_01`
- `SM_BS_House_Ranch_02`
- `SM_BS_Garage_Detached_01`
- `SM_BS_Sign_RoadWarning_01`
- `BP_BS_Set_SuburbanDriveway_01`
- `MI_BS_Concrete_Aged_01`

## Import standard

Every curated pack should be checked for:

- Scale: Unreal centimeters, believable player scale.
- Pivot: sensible placement pivot, especially for buildings, props, and foliage.
- Collision: simple collision for props, usable collision volumes for buildings, no accidental blocking volumes.
- LOD and Nanite: enable or preserve whichever is appropriate for the asset type and triangle count.
- Materials: collapse duplicate materials where practical and route environment materials toward Black Static master materials over time.
- Textures: keep texture resolution appropriate to view distance and importance.
- Naming: promote only the curated assets into the standardized names.

## Black Static zone tagging

When you curate or promote an asset, decide where it belongs first:

- `Settlement`
- `Suburban`
- `Urban`
- `Military`
- `Airport`
- `Rural`

Use that to decide both target folder and prefab grouping.

## Prefab guidance

Once a few packs are curated, build reusable set pieces instead of hand-assembling the same compositions repeatedly.

Recommended first prefab sets:

- `BP_BS_Set_SuburbanHouseDriveway_01`
- `BP_BS_Set_GarageWorkshop_01`
- `BP_BS_Set_RoadsideAbandonedCars_01`
- `BP_BS_Set_SettlementGeneratorCorner_01`
- `BP_BS_Set_MilitaryCheckpoint_01`
- `BP_BS_Set_AirportServiceLane_01`

## Tracking pack quality

Use `tools/asset-library-pack-template.csv` as the review sheet for incoming packs.

Suggested review fields:

- Vendor
- Pack name
- Theme
- Target zone
- Scale checked
- Collision checked
- Materials unified
- Nanite or LOD checked
- Notes

## Helper scripts

- `scripts/bootstrap-asset-library.ps1`
  Creates or repairs the folder layout for the library.
- `scripts/new-asset-pack.ps1`
  Creates a vendor ingest folder for a newly acquired pack.

## First recommended library buildout

If you want the fastest useful Black Static world kit, start in this order:

1. Nature and debris
2. Houses and garages
3. Civilian and wrecked vehicles
4. Utility and infrastructure props
5. Commercial and military sets
