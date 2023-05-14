import serial
import signal
import sys
import time

def signal_handler(sig, frame):
    sys.exit(0)

def write_to_file(filename, s):
    print(filename + ": " + s)
    with open(filename, "w") as f:
        f.write(s)

signal.signal(signal.SIGINT, signal_handler)
battery = ""
speedometer = ""
temperature = ""

ser = serial.Serial(sys.argv[1], 9600, timeout=0)
while True:
    if(not ser.is_open):
        ser = serial.Serial(sys.argv[1], 9600, timeout=0)
    line = ""
    try:
        raw = ser.readline()
        if(len(raw) < 3):
            continue
        line = raw.decode('utf-8')
    except:
        continue

    fields = line.split(",")
    if(len(fields) < 3):
        continue

    battery = str(int(float(fields[0]) * 100.0))
    speedometer = str(int(float(fields[1]) * 80.0))
    temperature = fields[2]

    write_to_file(argv[2] + "/battery", battery)
    write_to_file(argv[2] + "/speed", speedometer)
    write_to_file(argv[2] + "/temperature", temperature)
    time.sleep(0.1)
