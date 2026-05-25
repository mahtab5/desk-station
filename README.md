# Desk Productivity & Comfort Station

A smart desk companion built on an Arduino Uno, controlled entirely from a Linux terminal. It monitors your room's temperature and humidity in real time, calculates a comfort score, and helps you manage your work sessions with a countdown timer and stopwatch.

---

## Features

- **Real-time environment monitoring** — temperature, humidity, and a comfort score out of 100
- **Countdown timer** — set any duration, use Pomodoro (25 min) or break (5 min) presets
- **Stopwatch** — independent reading/session tracker on a dedicated 4-digit display
- **LED indicators** — green, yellow, or red based on comfort score
- **Buzzer alerts** — beeps on timer start, pause, and completion
- **Full Linux control** — single `desk` command controls everything from the terminal

---

## Hardware

| Component | Details |
|---|---|
| Arduino Uno | Main microcontroller |
| DHT22 | Temperature & humidity sensor |
| 16x2 LCD + I2C backpack | Displays sensor data and timer |
| TM1637 4-digit display (HW-069) | Dedicated stopwatch |
| Red, Yellow, Green LEDs | Visual comfort indicator |
| Passive piezo buzzer | Audio timer alerts |
| 220Ω resistors x3 | LED current limiting |
| 10kΩ resistors x2 | I2C SDA/SCL pull-ups |

### Wiring

| Component | Arduino Pin |
|---|---|
| DHT22 data | D2 |
| TM1637 CLK | D3 |
| TM1637 DIO | D4 |
| Green LED | D5 |
| Yellow LED | D6 |
| Red LED | D7 |
| Buzzer | D8 |
| LCD SDA | A4 / SDA |
| LCD SCL | A5 / SCL |

> **Note:** Add 10kΩ pull-up resistors between SDA→5V and SCL→5V. Without them the I2C bus will hang.

---

## LCD Display Layout

```
Row 0:  T:23.5C H:61%        ← always shown
Row 1:  Comfort:72/100        ← no timer active
Row 1:  C:72/100 24:59        ← timer running
```

The TM1637 display always shows the stopwatch in `MM:SS` format independently.

---

## Arduino Libraries Required

Install via Arduino IDE → Library Manager:

- `DHT sensor library` — Adafruit
- `Adafruit Unified Sensor` — Adafruit
- `LiquidCrystal I2C` — Frank de Brabander
- `TM1637Display` — Avishay Orpaz

---

## Linux Setup

### Requirements

- Python 3
- `pyserial` library

```bash
pip3 install pyserial
```

### Find your Arduino port

```bash
dmesg | grep tty
```

The port is usually `/dev/ttyACM0` or `/dev/ttyACM1`. Update the port in `control.py` if needed:

```python
ser = serial.Serial("/dev/ttyACM1", 9600, timeout=1)
```

### Grant serial port permission

```bash
sudo usermod -a -G dialout $USER
# log out and back in, or run:
sudo chmod 666 /dev/ttyACM1
```

### Install the desk command

```bash
sudo cp desk /usr/local/bin/desk
sudo chmod +x /usr/local/bin/desk
```

---

## Usage

```bash
desk pomodoro       # set and start a 25-minute Pomodoro timer
desk break          # set and start a 5-minute break timer
desk set 2500       # set a custom timer (25 min 00 sec)
desk start          # start the current timer
desk stop           # pause the current timer
desk reset          # reset the current timer
desk status         # print full system status
desk sensor         # print temperature, humidity, comfort score
desk watch          # start the stopwatch
desk watchstop      # pause the stopwatch
desk watchreset     # reset the stopwatch
desk led_on         # enable LEDs
desk led_off        # turn LEDs off
desk buzzer_on      # enable buzzer
desk buzzer_off     # silence buzzer
```

### Interactive mode

You can also run the Python script directly for an interactive prompt:

```bash
python3 control.py
> sensor
> pomodoro
> start
```

---

## File Structure

```
.
├── desk_station.ino    # Arduino sketch
├── control.py          # Python serial communication script
├── desk                # Bash wrapper — install to /usr/local/bin/desk
└── README.md
```

---

## How It Works

The `desk` bash script pipes commands through `control.py` to the Arduino over USB serial at 9600 baud. The Python script uses `sys.stdin.isatty()` to detect whether it is receiving piped input or running interactively, allowing it to serve both the bash wrapper and direct terminal use with the same script.

The Arduino sketch uses a non-blocking loop — all timing is handled with `millis()` so the countdown timer, stopwatch, sensor reads, and display updates run simultaneously without blocking each other.

---

## Comfort Score

The comfort score is calculated from the sensor readings with an ideal of **22°C** and **50% RH**:

- Temperature penalty: −3 points per °C away from 22°C
- Humidity penalty: −0.8 points per % away from 50% RH
- Score clamped between 0 and 100

| Score | LED | Meaning |
|---|---|---|
| 70 – 100 | 🟢 Green | Comfortable |
| 40 – 69 | 🟡 Yellow | Moderate |
| 0 – 39 | 🔴 Red | Uncomfortable |
