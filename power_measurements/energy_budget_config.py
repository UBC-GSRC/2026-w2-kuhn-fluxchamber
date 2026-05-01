
#!/usr/bin/env python3
"""
energy_budget_config.py — Edit the PARAMETERS section, then run the file.

This script estimates:
- Days of autonomy with zero sun
- Daily energy balance with solar (surplus/deficit)
- Days until empty with solar (if deficit)
- Required battery nameplate Wh for a target number of sunless days

HOW TO USE:
1) Open this file and edit the values in the PARAMETERS section.
2) Save, then run:  python energy_budget_config.py
3) Read the printed summary.

NOTES:
- Currents should be battery-side (after regulator losses if possible).
- PANEL_W × SUN_HOURS ≈ raw daily solar Wh; effective charge is reduced by CHARGE_EFF.
- ROUNDTRIP_EFF models battery losses; keeping it at 0.9 is a conservative default.
- DOD is usable fraction of the battery (0.8 = 80% use).
"""

from dataclasses import dataclass

# ===========================
# PARAMETERS (EDIT THESE)
# ===========================

# Battery
BATTERY_V = 12.0        # Battery nominal voltage (V)
BATTERY_AH = 30.0       # Battery capacity (Ah)
BATTERY_WH = BATTERY_V * BATTERY_AH
DOD = 0.80              # Usable fraction (Depth of Discharge, 0..1)
ROUNDTRIP_EFF = 0.90    # Battery round-trip efficiency (0..1)

# Load profile (battery-side currents)
ACTIVE_CURRENT_mA = 350     # Active-mode current draw (mA)
SLEEP_CURRENT_mA  = 1.2     # Sleep-mode current draw (mA)

ACTIVE_HOURS_PER_DAY = 6.0  # Hours per day in active mode (0..24)

# Solar
PANEL_W = 25.0          # Solar panel rating (W)
SUN_HOURS = 3.0         # Peak sun hours per day (h)
CHARGE_EFF = 0.75       # Panel/controller charging efficiency (0..1)

# Sizing target
TARGET_SUNLESS_DAYS = 7  # Desired days with zero sun for battery sizing

# ===========================
# END OF PARAMETERS
# ===========================

# ---- Derived load calculations ----
ACTIVE_CURRENT_A = ACTIVE_CURRENT_mA / 1000.0
SLEEP_CURRENT_A  = SLEEP_CURRENT_mA  / 1000.0

ACTIVE_FRACTION = max(0.0, min(ACTIVE_HOURS_PER_DAY / 24.0, 1.0))

AVERAGE_CURRENT_A = (
    ACTIVE_CURRENT_A * ACTIVE_FRACTION +
    SLEEP_CURRENT_A  * (1.0 - ACTIVE_FRACTION)
)

LOAD_W = AVERAGE_CURRENT_A * BATTERY_V


@dataclass
class EnergyModel:
    load_w: float
    battery_wh: float
    dod: float = 0.8
    roundtrip_eff: float = 0.9
    panel_w: float = 0.0
    sun_hours: float = 0.0
    charge_eff: float = 0.85

    def usable_wh(self) -> float:
        return self.battery_wh * self.dod

    def daily_load_wh(self) -> float:
        return 24.0 * self.load_w

    def daily_solar_wh(self) -> float:
        return self.panel_w * self.sun_hours * self.charge_eff

    def dark_autonomy_days(self) -> float:
        if self.load_w <= 0:
            return float('inf')
        effective_usable = self.usable_wh() * self.roundtrip_eff
        return effective_usable / self.daily_load_wh()

    def net_daily_delta_wh(self) -> float:
        return self.daily_solar_wh() - self.daily_load_wh()

    def days_until_empty_with_solar(self) -> float:
        delta = self.net_daily_delta_wh()
        if delta >= 0:
            return float('inf')
        effective_usable = self.usable_wh() * self.roundtrip_eff
        return effective_usable / (-delta)

    def required_battery_wh_for_days(self, target_days: float) -> float:
        if target_days <= 0:
            return 0.0
        needed_wh_at_load = self.daily_load_wh() * target_days
        required_stored_wh = needed_wh_at_load / max(self.roundtrip_eff, 1e-6)
        return required_stored_wh / max(self.dod, 1e-6)


def format_days(d: float) -> str:
    if d == float('inf'):
        return 'infinite'
    return f"{d:.2f}"


def main():
    model = EnergyModel(
        load_w=LOAD_W,
        battery_wh=BATTERY_WH,
        dod=DOD,
        roundtrip_eff=ROUNDTRIP_EFF,
        panel_w=PANEL_W,
        sun_hours=SUN_HOURS,
        charge_eff=CHARGE_EFF,
    )

    print("\n========= ENERGY BUDGET SUMMARY =========\n")

    print("Inputs:")
    print(f"  Battery voltage (V):           {BATTERY_V:.2f}")
    print(f"  Battery capacity (Ah):         {BATTERY_AH:.2f}")
    print(f"  Battery nameplate (Wh):        {BATTERY_WH:.2f}")
    print(f"  Usable DoD fraction:           {DOD:.2f}")
    print(f"  Battery round-trip eff:        {ROUNDTRIP_EFF:.2f}")

    print("\nLoad profile:")
    print(f"  Active current (mA):           {ACTIVE_CURRENT_mA:.2f}")
    print(f"  Sleep current (mA):            {SLEEP_CURRENT_mA:.2f}")
    print(f"  Active hours/day:              {ACTIVE_HOURS_PER_DAY:.2f}")
    print(f"  Avg battery current (mA):      {AVERAGE_CURRENT_A * 1000:.3f}")
    print(f"  Avg load power (W):            {model.load_w:.3f}")

    print("\nSolar:")
    print(f"  Panel rating (W):              {model.panel_w:.2f}")
    print(f"  Peak sun hours (h):            {model.sun_hours:.2f}")
    print(f"  Charge/controller eff:         {model.charge_eff:.2f}")

    print("\nResults:")
    print(f"  Usable energy (Wh):            {model.usable_wh():.2f}")
    print(f"  Daily load (Wh/day):           {model.daily_load_wh():.2f}")
    print(f"  Daily solar to battery (Wh):   {model.daily_solar_wh():.2f}")

    delta = model.net_daily_delta_wh()
    print(f"  Net daily delta (Wh):          {delta:+.2f} "
          f"({'surplus' if delta >= 0 else 'deficit'})")

    print("\nKey Metrics:")
    print(f"  Autonomy with zero sun (days): {format_days(model.dark_autonomy_days())}")

    days_with_solar = model.days_until_empty_with_solar()
    if days_with_solar == float('inf'):
        print("  Days until empty w/ solar:     infinite (net daily surplus)")
    else:
        print(f"  Days until empty w/ solar:     {days_with_solar:.2f}")

    req_wh = model.required_battery_wh_for_days(TARGET_SUNLESS_DAYS)
    print(f"  Required battery for {TARGET_SUNLESS_DAYS} sunless days:")
    print(f"    {req_wh:.2f} Wh  (~{req_wh / BATTERY_V:.2f} Ah @ {BATTERY_V:.1f} V)")


if __name__ == '__main__':
    main()
