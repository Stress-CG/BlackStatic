# Black Static

Black Static is now wired as an Unreal Engine 5.7 C++ + Blueprint project with two live tracks:

- the original deterministic pathing benchmark harness
- a new Phase 0 survival gameplay foundation built around settlement persistence, noise-reactive infected, and the first water-and-power restoration loop

## What this repo now contains

- A native `BlackStatic` gameplay module.
- A deterministic grid benchmark harness with baseline A* and improved Theta* planners.
- A benchmark commandlet that can scaffold scenario assets and execute benchmark runs.
- A Phase 0 gameplay framework with survivor vitals, inventory/stash persistence, reputation, infected AI, objective pickups, a settlement stash, extraction, and task board actors.
- A Phase 0 bootstrap commandlet that scaffolds item/task assets and a prototype map.
- PowerShell scripts for bootstrap, build, test, and benchmark execution on Windows.

## Quick start

1. Run `powershell -ExecutionPolicy Bypass -File .\scripts\bootstrap.ps1`
2. Run `powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1`
3. Run `powershell -ExecutionPolicy Bypass -File .\scripts\run-benchmark.ps1`
4. Run `powershell -ExecutionPolicy Bypass -File .\scripts\bootstrap-phase0.ps1`
5. Run `powershell -ExecutionPolicy Bypass -File .\scripts\test.ps1`
6. Run `powershell -ExecutionPolicy Bypass -File .\scripts\demo-editor.ps1` for the benchmark demo
7. Run `powershell -ExecutionPolicy Bypass -File .\scripts\demo-phase0.ps1` for the survival prototype map
8. Run `powershell -ExecutionPolicy Bypass -File .\scripts\package.ps1` for a packaged Windows build

## Important assumptions

- Unreal Engine 5.7 is installed at `D:\Epic Games\UE_5.7`
- Visual Studio 2022 Community with VC++ tools is installed locally
- The first milestone is editor-playable and commandlet-runnable on Windows
