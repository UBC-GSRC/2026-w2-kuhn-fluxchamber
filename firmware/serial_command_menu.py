
import serial
import struct
import time
from datetime import datetime

# ---- CONFIG ----
COM_PORT = 'COM13'       # Change to your Arduino port
BAUD_RATE = 9600
START_MARKER = 255

STRUCT_FORMAT = "<7i"  # cmd_id, year, month, day, hour, minute, second

# ---- SERIAL SETUP ----
ser = serial.Serial(COM_PORT, BAUD_RATE)

# ---- COMMAND DESCRIPTIONS ----
COMMANDS = {
    0: "Do nothing / idle",
    1: "Read sensors continuously",
    2: "Read sensors once",
    3: "Open vent",
    4: "Close vent",
    5: "Start fan",
    6: "Stop fan",
    7: "Set date/time",
    8: "Get date/time",
    9: "Reserved / test"
}

# ---- FUNCTIONS ----
def build_packet(cmd_id, year, month, day, hour, minute, second):
    return struct.pack(STRUCT_FORMAT, cmd_id, year, month, day, hour, minute, second)

def read_response():
    time.sleep(0.1)
    while ser.in_waiting:
        try:
            line = ser.readline().decode(errors='ignore').strip()
            if line:
                print(f"[ARDUINO] {line}")
        except Exception as e:
            print(f"[READ ERROR] {e}")
            break

def print_menu():
    print("\n==== COMMAND MENU ====")
    for key in sorted(COMMANDS.keys()):
        print(f"{key}: {COMMANDS[key]}")
    print("======================")

def wait_for_reading():
    i = 0 
    while i < 26:
        read_response()
        i += 1
        time.sleep(1)

def send_serial_command(cmd_id):
    ser.reset_input_buffer()
    ser.write(bytes([START_MARKER]))
    ser.write(packet)

    print(f"[SENT] ID={cmd_id} ({COMMANDS[cmd_id]}) "
        f"Time={now.strftime('%Y-%m-%d %H:%M:%S')}")
    
def get_user_choice():
    while True:
        try:
            choice = int(input("Enter command (0-9): "))
            if 0 <= choice <= 9:
                return choice
            else:
                print("Invalid range. Choose 0–9.")

        except ValueError:
            print("Invalid input. Enter a number.")

# ---- MAIN LOOP ----
try:
    print(f"Connected to {COM_PORT}")
    ser.reset_input_buffer()

    while True:
        print_menu()
        cmd_id = get_user_choice()

        now = datetime.now()

        packet = build_packet(
            cmd_id,
            now.year,
            now.month,
            now.day,
            now.hour,
            now.minute,
            now.second
        )

        if cmd_id == 1:
            print("\nSensor is reading continuously... press Ctrl+C to stop.\n")
            try:
                print("\nSensor is reading... please wait 30 seconds between each reading.\n")
                while True:
                    send_serial_command(cmd_id)
                    wait_for_reading()
            except KeyboardInterrupt:
                print("\nStopping continuous read...")

        elif cmd_id == 2:
            send_serial_command(cmd_id)
            print("\nSensor is reading... please wait 30 seconds.\n")

            wait_for_reading()
        else:
            send_serial_command(cmd_id)
            read_response()

except KeyboardInterrupt:
    print("\nStopping...")

finally:
    if ser.is_open:
        ser.close()
        print("Serial port closed.")
