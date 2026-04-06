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

Open the benchmark demo map in the editor:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\demo-editor.ps1
```

Create a packaged Windows build:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\package.ps1
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
