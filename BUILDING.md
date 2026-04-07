# Building Black Static

## Requirements

- Unreal Engine 5.7 at `D:\Epic Games\UE_5.7`
- Visual Studio 2022 Community with the Desktop C++ workload
- PowerShell 5+ or PowerShell 7+
- Python 3 for the benchmark comparison helper

## Commands

Bootstrap:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\bootstrap.ps1
```

Build the editor target:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1
```

Run the benchmark commandlet and scaffold missing assets:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run-benchmark.ps1
```

Bootstrap the Phase 0 survival systems into the main gameplay maps plus the standalone prototype map:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\bootstrap-phase0.ps1
```

Open the benchmark demo map in the editor:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\demo-editor.ps1
```

Open the Phase 0 main gameplay map in the editor:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\demo-phase0.ps1
```

Open the isolated Phase 0 prototype map instead:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\demo-phase0.ps1 -Map Prototype
```

Create a packaged Windows build:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\package.ps1
```

Scaffold the asset-library folder layout used for vendor packs, curated assets, and worldbuilding prefabs:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\bootstrap-asset-library.ps1
```

Create a raw vendor-ingest folder for a newly acquired pack:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\new-asset-pack.ps1 -Vendor Fab -PackName Suburban_Housing_Pack
```

Run automation tests plus commandlet regression checks:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\test.ps1
```

## Notes

- The scripts explicitly enter the Visual Studio developer shell before invoking Unreal build tooling.
- Benchmark artifacts are written to `Saved\Benchmarks\`.
- Demo map runs write aggregate artifacts to `Saved\DemoRuns\`.
- The benchmark commandlet can recreate missing scenario assets and the benchmark map.
- The Phase 0 bootstrap commandlet creates data assets under `Content\Phase0\Data\`, upgrades `Content\MAP_TutorialRoad_P.umap` and `Content\FirstPerson\Lvl_FirstPerson.umap`, and keeps the isolated prototype map at `Content\Phase0\Maps\MAP_Phase0_Prototype.umap`.
- `MAP_TutorialRoad_P` is the project default map after bootstrap so opening the project drops you into the authored Black Static space instead of the engine template.
- Art-library standards and naming conventions live in `docs\ART_LIBRARY.md`.
