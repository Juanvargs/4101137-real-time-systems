#!/usr/bin/env python3
"""Run fixed-priority response-time and utilization analyses from JSON."""

import argparse
import json
from math import ceil
from pathlib import Path


def response_time(task, higher_priority):
    """Return the RTA fixed point and iteration history in milliseconds."""
    c_ms = task["c_ms"]
    b_ms = task.get("blocking_ms", 0.0)
    r_ms = c_ms + b_ms
    history = [r_ms]
    for _ in range(1000):
        next_ms = c_ms + b_ms + sum(
            ceil(r_ms / hp["period_ms"]) * hp["c_ms"]
            for hp in higher_priority
        )
        history.append(next_ms)
        if abs(next_ms - r_ms) < 1e-12:
            return next_ms, history
        if next_ms > task["deadline_ms"]:
            return next_ms, history
        r_ms = next_ms
    raise RuntimeError(f"RTA did not converge for {task['name']}")


def task_set_missing(spec):
    missing = []
    for task in spec.get("tasks", []):
        for key in ("c_ms", "period_ms", "deadline_ms"):
            if task.get(key) is None:
                missing.append(f"{task['name']}.{key}")
    return missing


def analyze(spec):
    print(f"\n{spec['name']}")
    missing = task_set_missing(spec)
    if missing:
        print("  PENDING: fill measured/model parameters: " + ", ".join(missing))
        return

    tasks = spec["tasks"]  # high-to-low priority order for fixed-priority RTA
    if spec.get("utilization_tests", True):
        utilization = sum(task["c_ms"] / task["period_ms"] for task in tasks)
        n = len(tasks)
        ll_bound = n * (2 ** (1 / n) - 1) if n else 0.0
        product = 1.0
        for task in tasks:
            product *= 1 + task["c_ms"] / task["period_ms"]
        print(f"  U={utilization:.6f}")
        print(f"  Liu-Layland n={n}: bound={ll_bound:.6f}; "
              f"{'PASS' if utilization <= ll_bound else 'inconclusive'}")
        print(f"  Hyperbolic product={product:.6f}; "
              f"{'PASS' if product <= 2 else 'inconclusive'}")
        if all(task["deadline_ms"] == task["period_ms"] for task in tasks):
            print(f"  EDF implicit-deadline test: "
                  f"{'PASS' if utilization <= 1 else 'FAIL'} (U <= 1)")

    for index, task in enumerate(tasks):
        r_ms, history = response_time(task, tasks[:index])
        verdict = "PASS" if r_ms <= task["deadline_ms"] else "FAIL"
        print(f"  {task['name']}: C={task['c_ms']:.6f} ms, "
              f"B={task.get('blocking_ms', 0.0):.6f} ms, "
              f"R={r_ms:.6f} ms, D={task['deadline_ms']:.6f} ms -> {verdict}")
        print("    iteration: " + " -> ".join(f"{value:g}" for value in history))
        if "observed_response_ms" in task:
            observed = task["observed_response_ms"]
            print(f"    observed R={observed:.6f} ms; "
                  f"within calculated envelope: {observed <= r_ms}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--input", type=Path,
        default=Path(__file__).with_name("week06_rta_input.json"),
        help="JSON file containing task sets (default: week06_rta_input.json)",
    )
    parser.add_argument("--set", dest="set_name", help="analyze one named task set")
    args = parser.parse_args()

    data = json.loads(args.input.read_text(encoding="utf-8"))
    task_sets = data["task_sets"]
    if args.set_name:
        task_sets = [spec for spec in task_sets if spec["name"] == args.set_name]
        if not task_sets:
            parser.error(f"task set not found: {args.set_name}")
    print(f"Input: {args.input}")
    for spec in task_sets:
        analyze(spec)


if __name__ == "__main__":
    main()
