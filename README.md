# Desk Productivity & Comfort Station

A desk tool built with Arduino that monitors room temperature and humidity, and lets you run a timer or stopwatch from the Linux terminal.


## What it does

- Shows temperature, humidity and a comfort score on an LCD
- LEDs change colour based on comfort (green = good, yellow = ok, red = bad)
- Countdown timer with Pomodoro and break presets
- Stopwatch on a separate 4-digit display
- Buzzer beeps when timer starts, pauses, or finishes
- Everything controlled from Linux terminal using the `desk` command




## Hardware used

- Arduino Uno
- DHT22 sensor
- 16x2 LCD with I2C backpack
- TM1637 4-digit display
- Red, yellow, green LEDs
- Passive piezo buzzer
- 220Ω resistors (x3 for LEDs)


### Pin connections

| Component | Pin |
|---|---|
| DHT22 | D2 |
| TM1637 CLK | D3 |
| TM1637 DIO | D4 |
| Green LED | D5 |
| Yellow LED | D6 |
| Red LED | D7 |
| Buzzer | D8 |
| LCD SDA/SCL | A4/A5 |




## Arduino libraries

- DHT sensor library (Adafruit)
- Adafruit Unified Sensor
- LiquidCrystal I2C (Frank de Brabander)
- TM1637Display (Avishay Orpaz)



## Commands

```bash
desk pomodoro       # 25 min timer
desk break          # 5 min break
desk set 2500       # custom timer (mm ss)
desk start          # start timer
desk stop           # pause timer
desk reset          # reset timer
desk status         # show everything
desk sensor         # show temp/humidity
desk watch          # start stopwatch
desk watchstop      # pause stopwatch
desk watchreset     # reset stopwatch
desk led_off        # turn off LEDs
desk buzzer_off     # silence buzzer
```
