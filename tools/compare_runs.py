from __future__ import annotations

import argparse
import csv
import json
import math
import pathlib
import statistics
import sys


def load_summary(directory: pathlib.Path) -> dict:
    summary_path = directory / "summary.json"
    if not summary_path.exists():
        raise FileNotFoundError(f"Missing summary.json in {directory}")

    with summary_path.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def index_metrics(summary: dict) -> dict[str, dict]:
    runs = summary.get("runs", [])
    return {run["scenario_id"]: run for run in runs}


def pct_change(old_value: float, new_value: float) -> float:
    if math.isclose(old_value, 0.0):
        return 0.0 if math.isclose(new_value, 0.0) else 100.0
    return ((new_value - old_value) / old_value) * 100.0


def build_rows(baseline: dict, improved: dict) -> list[dict]:
    baseline_index = index_metrics(baseline)
    improved_index = index_metrics(improved)
    scenario_ids = sorted(set(baseline_index) & set(improved_index))
    rows: list[dict] = []

    for scenario_id in scenario_ids:
        base = baseline_index[scenario_id]
        imp = improved_index[scenario_id]
        rows.append(
            {
                "scenario_id": scenario_id,
                "baseline_success": base["success"],
                "improved_success": imp["success"],
                "baseline_time_to_goal_s": base["time_to_goal_s"],
                "improved_time_to_goal_s": imp["time_to_goal_s"],
                "baseline_executed_length": base["executed_length"],
                "improved_executed_length": imp["executed_length"],
                "baseline_illegal_traversal_count": base["illegal_traversal_count"],
                "improved_illegal_traversal_count": imp["illegal_traversal_count"],
                "baseline_collision_count": base["collision_count"],
                "improved_collision_count": imp["collision_count"],
                "baseline_planner_runtime_ms": base["planner_runtime_ms"],
                "improved_planner_runtime_ms": imp["planner_runtime_ms"],
                "time_delta_pct": pct_change(base["time_to_goal_s"], imp["time_to_goal_s"]),
                "length_delta_pct": pct_change(base["executed_length"], imp["executed_length"]),
                "planner_delta_pct": pct_change(base["planner_runtime_ms"], imp["planner_runtime_ms"]),
            }
        )

    return rows


def emit_markdown(rows: list[dict]) -> str:
    lines = [
        "| Scenario | Base Success | Improved Success | Time Delta % | Length Delta % | Planner Delta % |",
        "| --- | --- | --- | ---: | ---: | ---: |",
    ]
    for row in rows:
        lines.append(
            "| {scenario_id} | {baseline_success} | {improved_success} | {time_delta_pct:.2f} | {length_delta_pct:.2f} | {planner_delta_pct:.2f} |".format(
                **row
            )
        )
    return "\n".join(lines)


def enforce_acceptance(rows: list[dict]) -> None:
    if not rows:
        raise AssertionError("No overlapping scenarios were found between baseline and improved results.")

    scored_improvements = 0
    planner_deltas = []

    for row in rows:
        if not row["baseline_success"] or not row["improved_success"]:
            raise AssertionError(f"Scenario {row['scenario_id']} did not succeed in both modes.")
        if row["baseline_illegal_traversal_count"] != 0 or row["improved_illegal_traversal_count"] != 0:
            raise AssertionError(f"Scenario {row['scenario_id']} reported illegal traversal.")
        if row["baseline_collision_count"] != 0 or row["improved_collision_count"] != 0:
            raise AssertionError(f"Scenario {row['scenario_id']} reported collisions in the static suite.")

        if row["length_delta_pct"] <= -10.0 or row["time_delta_pct"] <= -15.0:
            scored_improvements += 1

        planner_deltas.append(row["planner_delta_pct"])

    if scored_improvements < 3:
        raise AssertionError(
            f"Improved mode only cleared the required thresholds in {scored_improvements} scenarios; expected at least 3."
        )

    median_planner_regression = statistics.median(planner_deltas)
    if median_planner_regression > 25.0:
        raise AssertionError(
            f"Median planner runtime regression was {median_planner_regression:.2f}%, which exceeds the 25% cap."
        )


def write_csv(rows: list[dict], output_path: pathlib.Path) -> None:
    with output_path.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=list(rows[0].keys()))
        writer.writeheader()
        writer.writerows(rows)


def main() -> int:
    parser = argparse.ArgumentParser(description="Compare Black Static baseline and improved benchmark bundles.")
    parser.add_argument("--baseline", required=True, type=pathlib.Path)
    parser.add_argument("--improved", required=True, type=pathlib.Path)
    parser.add_argument("--enforce", action="store_true")
    args = parser.parse_args()

    baseline_summary = load_summary(args.baseline)
    improved_summary = load_summary(args.improved)
    rows = build_rows(baseline_summary, improved_summary)

    markdown = emit_markdown(rows)
    print(markdown)

    if rows:
        write_csv(rows, args.improved / "comparison.csv")

    if args.enforce:
        enforce_acceptance(rows)

    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"compare_runs.py failed: {exc}", file=sys.stderr)
        raise
