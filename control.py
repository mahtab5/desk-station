import serial
import time
import sys

ser = serial.Serial("/dev/ttyACM1", 9600, timeout=1)
time.sleep(0.1)

def send(cmd):
    ser.write((cmd + "\n").encode())
    time.sleep(0.1)

    while ser.in_waiting:
        print(ser.readline().decode().strip())


if not sys.stdin.isatty():
    for line in sys.stdin:
        cmd = line.strip()
        if cmd:
            send(cmd)
    sys.exit(0)

print("Arduino ready. Type commands:")

while True:
    try:
        cmd = input("> ").strip()
        send(cmd)

    except EOFError:
        print("\nExiting...")
        break
