#!/usr/bin/env python3
"""Figures for the week-6 deck (06_schedulability.md).

    python3 rta_figs.py       # writes rta_iteration.svg, rta_window.svg, rta_inversion.svg here

All are computed: the RTA iterates Buttazzo Table 4.3 and Fig. 4.13's τ2; the inversion is simulated.
"""
import math
from pathlib import Path

from figlib import C, GRID, INK, MUTED, T, line, poly, rect, save, svg, text
from sc_figs import TASKS

HERE = Path(__file__).resolve().parent

TABLE_4_3 = [(1, 4, 3), (1, 5, 4), (2, 6, 5), (1, 11, 10)]  # (C, T, D), deadline-monotonic order
FIG_4_13 = [(c, p, p) for _, c, p, _ in TASKS]


def rta_steps(tasks, i):
    c = tasks[i][0]
    steps = [c]
    while True:
        r = c + sum(math.ceil(steps[-1] / t) * ch for ch, t, _ in tasks[:i])
        steps.append(r)
        if r == steps[-2] or r > tasks[i][2]:
            return steps


def iteration():
    steps = rta_steps(TABLE_4_3, 3)
    d = TABLE_4_3[3][2]
    x0, x1, y0, y1 = 110, 760, 380, 50
    sx = lambda s: x0 + (x1 - x0) * s / (len(steps) - 1)
    sy = lambda r: y0 - (y0 - y1) * r / 12
    body = [line(x0, y0, x1 + 20, y0), line(x0, y0, x0, y1)]
    for r in range(0, 13, 2):
        body += [line(x0 - 8, sy(r), x0, sy(r)), text(x0 - 14, sy(r) + 6, str(r), 18, 400, MUTED, "end")]
    pts = []
    for s, r in enumerate(steps):
        pts += [(sx(s), sy(r))]
        body.append(text(sx(s), y0 + 30, str(s), 18, 400, MUTED, "middle"))
    body.append(line(x0, sy(d), x1 + 20, sy(d), C["c5"], 3, "8 6"))
    body.append(text(x0 + 12, sy(d) - 10, f"D₄ = {d}", 20, 700, C["c5"]))
    body.append(poly(pts, C["c2"], 4))
    for s, r in enumerate(steps):
        body.append(f'<circle cx="{sx(s):.1f}" cy="{sy(r):.1f}" r="7" fill="{C["c2"]}"/>')
        body.append(text(sx(s) + 14, sy(r) + 26, str(r), 20, 700, C["c2"]))
    body.append(text((x0 + x1) / 2, y0 + 62, "iteration step s", 20, 400, MUTED, "middle"))
    body.append(text(x0, y1 - 20, "R₄ guess", 20, 400, MUTED))
    tx = 830
    rs = [rta_steps(TABLE_4_3, i)[-1] for i in range(len(TABLE_4_3))]
    body.append(text(tx, 80, "Buttazzo Table 4.3", 22, 700, INK))
    for j, h in enumerate(("", "C", "T", "D", "R")):
        body.append(text(tx + 40 + j * 62, 120, h, 20, 700, C["c2"] if h == "R" else MUTED, "middle"))
    for i, ((c, t, dd), r) in enumerate(zip(TABLE_4_3, rs)):
        y = 155 + i * 36
        for j, v in enumerate((f"τ{i + 1}", c, t, dd, r)):
            body.append(text(tx + 40 + j * 62, y, str(v), 20, 700 if j in (0, 4) else 400,
                             C["c2"] if j == 4 else INK, "middle"))
    body.append(text(tx, 330, "every R ≤ D: schedulable", 22, 700, C["c3"]))
    body.append(text(tx, 362, f"τ4 settles at R₄ = {steps[-1]} = D₄", 20, 400, MUTED))
    return svg(1160, 470, body)


def window():
    """RTA on Fig. 4.13's τ2, one row per step: the window R and the τ1 releases it holds."""
    (c1, t1, _), (c2, _, d2) = FIG_4_13
    col1, col2 = TASKS[0][3], TASKS[1][3]
    steps = rta_steps(FIG_4_13, 1)
    x0, unit, row, top, span = 190, 52, 104, 96, 10
    sx = lambda t: x0 + t * unit

    def release(x, y0, y1, col, sw):  # the open chevron of sc_fig413.svg
        return (line(x, y0, x, y1, col, sw)
                + f'<path d="M{x - 6} {y1 + 9} L{x} {y1} L{x + 6} {y1 + 9}" fill="none" stroke="{col}" stroke-width="{sw}"/>')

    body = [text(20, 36, f"τ1: C = {c1}, T = {t1}", 22, 700, C[col1]),
            text(300, 36, f"τ2: C = {c2}, T = D = {d2}", 22, 700, C[col2])]
    tx = sx(span) + 50
    for k, r in enumerate(steps):
        ly = top + k * row
        axis = ly + 58
        late = r > d2
        body.append(text(x0 - 30, ly + 40, f"R = {r}", 24, 700, C["c5"] if late else C[col2], "end"))
        body.append(rect(sx(0), ly + 12, r * unit, 40, T[col2], C[col2], 2, 6))
        body.append(line(sx(0), axis, sx(span), axis, GRID, 2))
        for a in range(0, span + 1, t1):
            inside = a < r
            body.append(release(sx(a), axis, ly + 4, C[col1] if inside else MUTED, 4 if inside else 2))
        if late:
            body.append(text(tx, ly + 32, f"{r} > D₂ = {d2}: stop", 24, 700, C["c5"]))
            body.append(text(tx, ly + 60, "τ2 misses, as on the RM timeline", 20, 400, C["c5"]))
        else:
            n = math.ceil(r / t1)
            body.append(text(tx, ly + 32, f"{c2} + ⌈{r}/{t1}⌉·{c1} = {steps[k + 1]}", 24, 700, INK))
            body.append(text(tx, ly + 60, f"{n} release{'s' if n > 1 else ''} of τ1 inside", 20, 400, MUTED))
    bottom = top + len(steps) * row - 30
    body.append(line(sx(d2), top - 6, sx(d2), bottom, C["c5"], 3, "8 6"))
    body.append(text(sx(d2), top - 14, f"D₂ = {d2}", 20, 700, C["c5"], "middle"))
    for t in range(0, span + 1, t1):
        body.append(text(sx(t), bottom + 26, str(t), 18, 400, MUTED, "middle"))
    body.append(text(x0, bottom + 64, "↑ τ1 release · shaded: τ2's window · the ceiling counts the arrows inside it",
                     18, 400, MUTED))
    return svg(1160, bottom + 76, body)


def simulate(tasks, inherit, horizon):
    """Fixed-priority preemptive, one mutex. tasks: name -> dict(prio, release, prog)."""
    state = {n: {"pc": 0, "left": None, "done": False} for n in tasks}
    owner, out = None, {n: [] for n in tasks}
    for t in range(horizon):
        changed = True
        while changed:  # lock/unlock take no time: settle them before scheduling
            changed = False
            for n, tk in sorted(tasks.items(), key=lambda kv: -kv[1]["prio"]):
                st = state[n]
                while not st["done"] and t >= tk["release"] and st["left"] is None:
                    if st["pc"] >= len(tk["prog"]):
                        st["done"] = changed = True
                        break
                    op = tk["prog"][st["pc"]]
                    if op == "lock" and owner is None:
                        owner, st["pc"], changed = n, st["pc"] + 1, True
                    elif op == "unlock":
                        owner, st["pc"], changed = None, st["pc"] + 1, True
                    elif isinstance(op, int):
                        st["left"], changed = op, True
                    else:
                        break
        def blocked(n):
            st = state[n]
            return st["left"] is None and not st["done"] and tasks[n]["prog"][st["pc"]] == "lock"
        def prio(n):
            p = tasks[n]["prio"]
            if inherit and n == owner:
                p = max([p] + [tasks[m]["prio"] for m in tasks if blocked(m)])
            return p
        ready = [n for n in tasks if t >= tasks[n]["release"] and not state[n]["done"] and not blocked(n)]
        run = max(ready, key=prio) if ready else None
        for n in tasks:
            if t < tasks[n]["release"] or state[n]["done"]:
                out[n].append(None)
            elif n == run:
                out[n].append("crit" if owner == n else "run")
            else:
                out[n].append("blocked" if blocked(n) else "ready")
        if run:
            st = state[run]
            st["left"] -= 1
            if st["left"] == 0:
                st["left"], st["pc"] = None, st["pc"] + 1
    return out


SCENARIO = {"H": {"prio": 3, "release": 2, "prog": [1, "lock", 2, "unlock", 1]},
            "M": {"prio": 2, "release": 4, "prog": [6]},
            "L": {"prio": 1, "release": 0, "prog": [1, "lock", 4, "unlock", 1]}}
ROLES = {"H": ("control", "c5"), "M": ("telemetry", "c3"), "L": ("console", "c6")}


def finish(lane):
    return max(i for i, s in enumerate(lane) if s in ("run", "crit")) + 1


def inversion():
    horizon, x0, unit, lane_h = 17, 250, 48, 46
    body, y = [], 20
    for inherit, title in ((False, "k_sem used as a lock: no owner, no inheritance"),
                           (True, "k_mutex: the holder inherits the waiter's priority")):
        out = simulate(SCENARIO, inherit, horizon)
        body.append(text(x0, y + 24, title, 24, 700, INK))
        for i, n in enumerate(("H", "M", "L")):
            role, col = ROLES[n]
            ly = y + 40 + i * lane_h
            body.append(text(x0 - 14, ly + 28, f"{n} · {role}", 22, 700, C[col], "end"))
            body.append(line(x0, ly + 34, x0 + horizon * unit, ly + 34, GRID, 2))
            for t, s in enumerate(out[n]):
                if s in ("run", "crit"):
                    body.append(rect(x0 + t * unit, ly + 6, unit, 28, C[col] if s == "crit" else T[col], C[col]))
                elif s == "blocked":
                    body.append(rect(x0 + t * unit, ly + 14, unit, 12, "#f1f5f9", MUTED, 1, 0, "4 3"))
        hb = [t for t, s in enumerate(out["H"]) if s == "blocked"]
        bx, by = x0 + hb[0] * unit, y + 40 + 3 * lane_h + 8
        body.append(line(bx, by, x0 + (hb[-1] + 1) * unit, by, C["c5"], 3))
        body.append(text(x0 + (hb[-1] + 1) * unit + 10, by + 7, f"H waits {len(hb)}, finishes at t = {finish(out['H'])}",
                         21, 700, C["c5"]))
        y = by + 34
    body.append(text(x0, y, "filled = holds the lock · dashed = blocked on it", 20, 400, MUTED))
    return svg(x0 + horizon * unit + 280, y + 14, body)


if __name__ == "__main__":
    assert rta_steps(TABLE_4_3, 3) == [1, 5, 6, 7, 9, 10, 10]
    assert [rta_steps(TABLE_4_3, i)[-1] for i in range(4)] == [1, 2, 4, 10], "book: every task meets its deadline"
    assert rta_steps(FIG_4_13, 1) == [4, 6, 8], "Fig. 4.13: τ2's R passes D = 7 at the third step"
    sem, pip = simulate(SCENARIO, False, 17), simulate(SCENARIO, True, 17)
    assert sum(s == "blocked" for s in pip["H"]) == 3 and finish(pip["H"]) == 9, "PIP: H waits for L's critical section only"
    assert sum(s == "blocked" for s in sem["H"]) == 9 and finish(sem["H"]) == 15, "no PIP: M's 6 units land on H"
    assert finish(sem["H"]) > finish(pip["H"])
    save(HERE, "rta_iteration.svg", iteration())
    save(HERE, "rta_window.svg", window())
    save(HERE, "rta_inversion.svg", inversion())
    print("rta_iteration.svg rta_window.svg rta_inversion.svg")
