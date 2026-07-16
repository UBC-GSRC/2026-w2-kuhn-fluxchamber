#!/usr/bin/env python3
"""
energy_budget_config.py

Edit the PARAMETERS section, then run:

    python energy_budget_config.py

This script estimates:
- Days of autonomy with zero sun
- Daily energy balance with solar (surplus/deficit)
- Days until empty with solar (if deficit)
- Required battery nameplate Wh for a target number of sunless days

Load profile supports independent operating states:
- Opening
- Fan
- Closing
- Measuring
- Sleep
"""

from dataclasses import dataclass

# ===========================
# PARAMETERS (EDIT THESE)
# ===========================

# Battery
BATTERY_V = 12.0
BATTERY_AH = 30.0
BATTERY_WH = BATTERY_V * BATTERY_AH

DOD = 0.80
ROUNDTRIP_EFF = 0.90

# ---------------------------
# Load Profile
# ---------------------------

# Currents (battery-side, mA)

OPENING_CURRENT_mA = 1000
FAN_CURRENT_mA = 155
CLOSING_CURRENT_mA = 950
MEASURING_CURRENT_mA = 100
SLEEP_CURRENT_mA = 82

# Time spent in each state (hours/day)

OPENING_HOURS_PER_DAY = 5 / 60 / 60 * 24
FAN_HOURS_PER_DAY = 30 / 60 / 60 * 24
CLOSING_HOURS_PER_DAY = 6 / 60 / 60 * 24
MEASURING_HOURS_PER_DAY = 5 / 60 * 24 


# Sleep automatically fills the remaining time

SLEEP_HOURS_PER_DAY = 24.0 - (
    OPENING_HOURS_PER_DAY +
    FAN_HOURS_PER_DAY +
    CLOSING_HOURS_PER_DAY +
    MEASURING_HOURS_PER_DAY
)
print(f"DEBUG: Total operating hours/day: {OPENING_HOURS_PER_DAY + FAN_HOURS_PER_DAY + CLOSING_HOURS_PER_DAY + MEASURING_HOURS_PER_DAY :.2f}")
print(f"DEBUG: Sleep hours/day: {SLEEP_HOURS_PER_DAY:.2f}")
if SLEEP_HOURS_PER_DAY < 0:
    raise ValueError(
        "Total operating hours exceed 24 hours/day."
    )

# ---------------------------
# Solar
# ---------------------------

PANEL_W = 25.0
SUN_HOURS = 3.0
CHARGE_EFF = 0.75

# ---------------------------
# Battery Sizing Target
# ---------------------------

TARGET_SUNLESS_DAYS = 7

# ===========================
# END OF PARAMETERS
# ===========================

# ---- Derived load calculations ----

OPENING_CURRENT_A = OPENING_CURRENT_mA / 1000.0
FAN_CURRENT_A = FAN_CURRENT_mA / 1000.0
CLOSING_CURRENT_A = CLOSING_CURRENT_mA / 1000.0
MEASURING_CURRENT_A = MEASURING_CURRENT_mA / 1000.0
SLEEP_CURRENT_A = SLEEP_CURRENT_mA / 1000.0

DAILY_AH = (
    OPENING_CURRENT_A * OPENING_HOURS_PER_DAY +
    FAN_CURRENT_A * FAN_HOURS_PER_DAY +
    CLOSING_CURRENT_A * CLOSING_HOURS_PER_DAY +
    MEASURING_CURRENT_A * MEASURING_HOURS_PER_DAY +
    SLEEP_CURRENT_A * SLEEP_HOURS_PER_DAY
)

AVERAGE_CURRENT_A = DAILY_AH / 24.0

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
            return float("inf")

        effective_usable = self.usable_wh() * self.roundtrip_eff
        return effective_usable / self.daily_load_wh()

    def net_daily_delta_wh(self) -> float:
        return self.daily_solar_wh() - self.daily_load_wh()

    def days_until_empty_with_solar(self) -> float:
        delta = self.net_daily_delta_wh()

        if delta >= 0:
            return float("inf")

        effective_usable = self.usable_wh() * self.roundtrip_eff
        return effective_usable / (-delta)

    def required_battery_wh_for_days(self, target_days: float) -> float:
        if target_days <= 0:
            return 0.0

        needed_wh_at_load = self.daily_load_wh() * target_days
        required_stored_wh = needed_wh_at_load / max(
            self.roundtrip_eff, 1e-6
        )

        return required_stored_wh / max(self.dod, 1e-6)


def format_days(d: float) -> str:
    if d == float("inf"):
        return "infinite"

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

    print(f"  Opening current (mA):          {OPENING_CURRENT_mA:.2f}")
    print(f"  Opening hours/day:             {OPENING_HOURS_PER_DAY:.2f}")

    print(f"  Fan current (mA):              {FAN_CURRENT_mA:.2f}")
    print(f"  Fan hours/day:                 {FAN_HOURS_PER_DAY:.2f}")

    print(f"  Closing current (mA):          {CLOSING_CURRENT_mA:.2f}")
    print(f"  Closing hours/day:             {CLOSING_HOURS_PER_DAY:.2f}")

    print(f"  Measuring current (mA):        {MEASURING_CURRENT_mA:.2f}")
    print(f"  Measuring hours/day:           {MEASURING_HOURS_PER_DAY:.2f}")

    print(f"  Sleep current (mA):            {SLEEP_CURRENT_mA:.2f}")
    print(f"  Sleep hours/day:               {SLEEP_HOURS_PER_DAY:.2f}")

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

    print(
        f"  Net daily delta (Wh):          {delta:+.2f} "
        f"({'surplus' if delta >= 0 else 'deficit'})"
    )

    print("\nKey Metrics:")
    print(
        f"  Autonomy with zero sun (days): "
        f"{format_days(model.dark_autonomy_days())}"
    )

    days_with_solar = model.days_until_empty_with_solar()

    if days_with_solar == float("inf"):
        print(
            "  Days until empty w/ solar:     "
            "infinite (net daily surplus)"
        )
    else:
        print(
            f"  Days until empty w/ solar:     "
            f"{days_with_solar:.2f}"
        )

    req_wh = model.required_battery_wh_for_days(
        TARGET_SUNLESS_DAYS
    )

    print(
        f"  Required battery for "
        f"{TARGET_SUNLESS_DAYS} sunless days:"
    )

    print(
        f"    {req_wh:.2f} Wh "
        f"(~{req_wh / BATTERY_V:.2f} Ah @ {BATTERY_V:.1f} V)"
    )


if __name__ == "__main__":
    main()