#!/usr/bin/env python3
"""Week 6 schedulability checks using the lab's measured parameters."""

from math import ceil, sqrt


def response_time(c_ms, b_ms=0.0, higher_priority=()):
    """Fixed-priority RTA: R = C + B + sum(ceil(R/T_h) * C_h)."""
    r_ms = c_ms + b_ms
    history = [r_ms]
    for _ in range(1000):
        next_ms = c_ms + b_ms + sum(
            ceil(r_ms / period_ms) * execution_ms
            for execution_ms, period_ms in higher_priority
        )
        history.append(next_ms)
        if abs(next_ms - r_ms) < 1e-12:
            return next_ms, history
        r_ms = next_ms
    raise RuntimeError("RTA did not converge")


def report_task(name, c_ms, deadline_ms, b_ms=0.0, higher_priority=()):
    r_ms, history = response_time(c_ms, b_ms, higher_priority)
    verdict = "PASS" if r_ms <= deadline_ms else "FAIL"
    print(f"{name}: C={c_ms:.5f} ms, B={b_ms:.5f} ms, "
          f"R={r_ms:.5f} ms, D={deadline_ms:.5f} ms -> {verdict}")
    print("  iteration: " + " -> ".join(f"{value:g}" for value in history))
    return r_ms


def main():
    print("Fig. 4.13, RM response-time analysis")
    c1_ms, t1_ms = 2.0, 5.0
    c2_ms, t2_ms = 4.0, 7.0
    report_task("tau1", c1_ms, t1_ms)
    report_task("tau2", c2_ms, t2_ms,
                 higher_priority=((c1_ms, t1_ms),))

    u1 = c1_ms / t1_ms
    u2 = c2_ms / t2_ms
    utilization = u1 + u2
    ll_bound = 2 * (2 ** (1 / 2) - 1)
    hyperbolic_product = (1 + u1) * (1 + u2)
    print("\nUtilization checks for Fig. 4.13")
    print(f"U={utilization:.6f}; Liu-Layland n=2 bound={ll_bound:.6f} "
          f"({ 'PASS' if utilization <= ll_bound else 'inconclusive' })")
    print(f"Hyperbolic product={hyperbolic_product:.6f} "
          f"(bound <= 2; { 'PASS' if hyperbolic_product <= 2 else 'inconclusive' })")
    print(f"EDF implicit-deadline test: U <= 1 is "
          f"{ 'PASS' if utilization <= 1 else 'FAIL' }")

    print("\nTask B, measured control C and lock-held B (ms)")
    c_control_ms = 0.93830
    r_baseline = report_task("baseline", c_control_ms, 10.0)
    r_sem = report_task("binary semaphore (observed B)", c_control_ms,
                        10.0, b_ms=51.604)
    r_mutex = report_task("mutex PI", c_control_ms, 10.0,
                          b_ms=28.148)
    print("\nMeasured control responses: baseline=0.93830 ms, "
          "semaphore=50.466 ms, mutex PI=27.001 ms")
    print(f"Observed <= calculated envelope: baseline {0.93830 <= r_baseline}, "
          f"semaphore {50.466 <= r_sem}, mutex PI {27.001 <= r_mutex}")
    print("The semaphore B is this capture's wall-clock lock-held width, "
          "including medium-thread interference; it is not a protocol-level "
          "blocking guarantee.")


if __name__ == "__main__":
    main()
