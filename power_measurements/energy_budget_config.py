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
- Set LOAD_W to your average *battery* power draw in watts (include converter losses).
- PANEL_W × SUN_HOURS ≈ raw daily solar Wh; effective charge is reduced by CHARGE_EFF.
- ROUNDTRIP_EFF models battery losses; keeping it at 0.9 is a conservative default.
- DOD is usable fraction of the battery (0.8 = 80% use).
"""
from dataclasses import dataclass

# ===========================
# PARAMETERS (EDIT THESE)
# ===========================
BATTERY_V = 12.0        # Battery nominal voltage (V)
BATTERY_AH = 30.0       # Battery capacity (Ah)
BATTERY_WH = BATTERY_V * BATTERY_AH      # Battery nameplate capacity (Wh)
DOD = 0.80              # Usable fraction (Depth of Discharge, 0..1)
ROUNDTRIP_EFF = 0.90    # Battery round-trip efficiency (0..1)

LOAD_mA = 100          # Average load current drawn from the battery (A) (e.g., 1W at 12V)
LOAD_A = LOAD_mA / 1000.0          # Average load current drawn from the battery (A) (e.g., 1W at 12V)
LOAD_W = LOAD_A * BATTERY_V           # Average load power drawn from the battery (W)

PANEL_W = 25.0          # Solar panel rating (W)
SUN_HOURS = 3.0         # Peak sun hours per day (h)
CHARGE_EFF = 0.75       # Panel/controller charging efficiency (0..1) see https://www.ecoflow.com/ca/blog/how-to-calculate-solar-panel-output

TARGET_SUNLESS_DAYS = 7 # Desired days with zero sun for sizing battery

# ===========================
# END OF PARAMETERS
# ===========================

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
        # See https://www.ecoflow.com/ca/blog/how-to-calculate-solar-panel-output
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
    print(f"  Load power (W):                {model.load_w:.3f}")
    print(f"  Battery nameplate (Wh):        {model.battery_wh:.2f}")
    print(f" Battery nominal voltage (V):     {BATTERY_V:.2f}")
    print(f"  Battery capacity (Ah):         {BATTERY_AH:.2f}")
    print(f"  Usable DoD fraction:           {model.dod:.2f}")
    print(f"  Battery round-trip eff:        {model.roundtrip_eff:.2f}")
    print(f"  Panel rating (W):              {model.panel_w:.2f}")
    print(f"  Peak sun hours (h):            {model.sun_hours:.2f}")
    print(f"  Charge/controller eff:         {model.charge_eff:.2f}")

    print("\nResults:")
    print(f"  Usable energy (Wh):            {model.usable_wh():.2f}")
    print(f"  Daily load (Wh/day):           {model.daily_load_wh():.2f}")
    print(f"  Daily solar to battery (Wh):   {model.daily_solar_wh():.2f}")
    delta = model.net_daily_delta_wh()
    print(f"  Net daily delta (Wh):          {delta:+.2f} ({'surplus' if delta>=0 else 'deficit'})")

    print("\nKey Metrics:")
    print(f"  Autonomy with zero sun (days): {format_days(model.dark_autonomy_days())}")
    days_with_solar = model.days_until_empty_with_solar()
    if days_with_solar == float('inf'):
        print("  Days until empty w/ solar:     infinite (net daily surplus)")
    else:
        print(f"  Days until empty w/ solar:     {days_with_solar:.2f}")

    req_wh = model.required_battery_wh_for_days(TARGET_SUNLESS_DAYS)
    print(f"  Required battery for {TARGET_SUNLESS_DAYS} sunless days (Wh): {req_wh:.2f} or {req_wh / BATTERY_V:.2f} Ah at {BATTERY_V}V")

if __name__ == '__main__':
    main()
