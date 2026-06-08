import serial
import struct
import time
from datetime import datetime

# ---- CONFIG ----
COM_PORT = 'COM13'       # Change to your Arduino port
BAUD_RATE = 9600
START_MARKER = 255

# Struct format (must match Arduino)

STRUCT_FORMAT = "<7i"  # cmd_id, year, month, day, hour, minute, second

# ---- SERIAL SETUP ----
ser = serial.Serial(COM_PORT, BAUD_RATE)

cmd_id = 8

def build_packet(cmd_id, year, month, day, hour, minute, second):
    return struct.pack(STRUCT_FORMAT, cmd_id, year, month, day, hour, minute, second)

def read_response():
    """Read and print all available incoming serial data."""
    time.sleep(0.1)  # give Arduino time to respond

    while ser.in_waiting:
        try:
            line = ser.readline().decode(errors='ignore').strip()
            if line:
                print(f"[ARDUINO] {line}")
        except Exception as e:
            print(f"[READ ERROR] {e}")
            break

def readSensorsContinuous():
    cmd_id = 1

def readSensorsOnce():
    cmd_id = 2

def openVent():
    cmd_id = 3
    
def closeVent():
    cmd_id = 4

def startFan():
    cmd_id = 5  

def stopFan():
    cmd_id = 6

def setDateTime():
    cmd_id = 7

def getDateTime():
    cmd_id = 8

try:
    print(f"Connected to {COM_PORT}")
    ser.reset_input_buffer()

    while True:
        now = datetime.now()
        # ---- Build and send packet ----
        packet = build_packet(cmd_id, now.year, now.month, now.day, now.hour, now.minute, now.second)

        ser.flushInput()  # Clear input buffer before sending
        ser.write(bytes([START_MARKER]))
        ser.write(packet)

        print(f"[SENT] ID={cmd_id} Time={now.strftime('%Y-%m-%d %H:%M:%S')}")

        # ---- Listen for Arduino response ----
        read_response()

        if cmd_id < 9:
            cmd_id += 1
        else:
            cmd_id = 0
        time.sleep(5)

except KeyboardInterrupt:
    print("Stopping...")

finally:
    if ser.is_open:
        ser.close()
        print("Serial port closed.")
