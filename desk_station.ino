#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TM1637Display.h>

#define DHT_PIN       2
#define TM_CLK        3
#define TM_DIO        4
#define LED_GREEN     5
#define LED_YELLOW    6
#define LED_RED       7
#define BUZZER_PIN    8
#define DHT_TYPE      DHT22

DHT               dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
TM1637Display     tm(TM_CLK, TM_DIO);

#define TIMER_IDLE    0
#define TIMER_SET     1
#define TIMER_RUNNING 2
#define TIMER_PAUSED  3
#define TIMER_DONE    4

int   timerState   = TIMER_IDLE;
float temperature  = 0;
float humidity     = 0;
int   comfortScore = 0;

long timerDuration  = 0;
long timerRemaining = 0;

bool watchRunning = false;
long watchElapsed = 0;

unsigned long lastTimerTick  = 0;
unsigned long lastWatchTick  = 0;
unsigned long lastLCDUpdate  = 0;
unsigned long lastSensorRead = 0;
unsigned long lastBlink      = 0;

const unsigned long LCD_INTERVAL    = 500;
const unsigned long SENSOR_INTERVAL = 3000;

bool blinkState = false;
bool ledOff     = false;
bool buzzerOff  = false;  // true = buzzer silenced until buzzer:on

char    inputBuffer[32];
uint8_t inputLen = 0;

void setup() {
  Serial.begin(9600);
  dht.begin();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(F("Desk Station"));
  lcd.setCursor(0, 1);
  lcd.print(F("Starting..."));
  delay(1500);
  lcd.clear();

  tm.setBrightness(4);
  tm.showNumberDecEx(0, 0b01000000, true);

  pinMode(LED_GREEN,  OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED,    OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  readSensor();
  tone(BUZZER_PIN, 1500, 100);
  Serial.println(F("ready"));
}


void loop() {
  unsigned long now = millis();

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (inputLen > 0) {
        inputBuffer[inputLen] = '\0';
        handleCommand(inputBuffer);
        inputLen = 0;
      }
    } else if (inputLen < 31) {
      // NO uppercase conversion — keep lowercase like Linux
      inputBuffer[inputLen++] = c;
    }
  }

  if (timerState == TIMER_RUNNING) {
    long delta    = now - lastTimerTick;
    lastTimerTick = now;
    timerRemaining -= delta;
    if (timerRemaining <= 0) {
      timerRemaining = 0;
      timerState     = TIMER_DONE;
      lcd.setCursor(0, 1);
      lcd.print(F("  Time's up!    "));
      if (!buzzerOff) {
        for (int i = 0; i < 3; i++) { tone(BUZZER_PIN, 1000, 200); delay(400); }
        delay(500);
        for (int i = 0; i < 3; i++) { tone(BUZZER_PIN, 1000, 200); delay(400); }
        noTone(BUZZER_PIN);
      }
      delay(1000);
      lcd.clear();
    }
  } else {
    lastTimerTick = now;
  }

  if (watchRunning) watchElapsed += (now - lastWatchTick);
  lastWatchTick = now;

  if (now - lastSensorRead >= SENSOR_INTERVAL) {
    lastSensorRead = now;
    readSensor();
  }

  if (now - lastLCDUpdate >= LCD_INTERVAL) {
    lastLCDUpdate = now;
    updateLCD();
    updateTM();
    updateLEDs(now);
  }
}


void msToChars(long ms, char *buf) {
  if (ms < 0) ms = 0;
  long totalSec = ms / 1000;
  int  mins = (int)(totalSec / 60);
  int  secs = (int)(totalSec % 60);
  buf[0] = '0' + mins / 10; buf[1] = '0' + mins % 10;
  buf[2] = ':';
  buf[3] = '0' + secs / 10; buf[4] = '0' + secs % 10;
  buf[5] = '\0';
}


void handleCommand(char *cmd) {

  if (strcmp(cmd, "help") == 0) {
    Serial.println(F("--- commands ---"));
    Serial.println(F("sensor         Read sensor data"));
    Serial.println(F("status         Full system status"));
    Serial.println(F("set:mmss       Set timer (e.g. set:2500)"));
    Serial.println(F("pomodoro       25min work timer"));
    Serial.println(F("break          5min break timer"));
    Serial.println(F("start          Start timer"));
    Serial.println(F("stop           Pause timer"));
    Serial.println(F("reset          Reset timer"));
    Serial.println(F("watch:start    Start stopwatch"));
    Serial.println(F("watch:stop     Pause stopwatch"));
    Serial.println(F("watch:reset    Reset stopwatch"));
    Serial.println(F("led:off        Turn LEDs off"));
    Serial.println(F("led:on         Turn LEDs on"));
    Serial.println(F("buzzer:off     Silence buzzer"));
    Serial.println(F("buzzer:on      Enable buzzer"));
    Serial.println(F("----------------"));
    return;
  }

  if (strcmp(cmd, "sensor") == 0) {
    readSensor(); printSensorData(); return;
  }

  if (strcmp(cmd, "status") == 0) {
    printSensorData();
    Serial.print(F("Timer: "));
    switch (timerState) {
      case TIMER_IDLE: Serial.println(F("idle"));    break;
      case TIMER_SET: Serial.println(F("set"));     break;
      case TIMER_RUNNING: Serial.println(F("running")); break;
      case TIMER_PAUSED: Serial.println(F("paused"));  break;
      case TIMER_DONE: Serial.println(F("done"));    break;
    }
    char tbuf[6];
    msToChars(timerRemaining, tbuf);
    Serial.print(F("Remaining: ")); Serial.println(tbuf);
    Serial.print(F("Watch: "));
    Serial.println(watchRunning ? F("running") : F("stopped"));
    msToChars(watchElapsed, tbuf);
    Serial.print(F("Elapsed: ")); Serial.println(tbuf);
    return;
  }

  if (strncmp(cmd, "set:", 4) == 0) {
    char *val = cmd + 4;
    if (strlen(val) == 4) {
      int mins = (val[0]-'0')*10 + (val[1]-'0');
      int secs = (val[2]-'0')*10 + (val[3]-'0');
      timerDuration  = ((long)mins * 60 + secs) * 1000L;
      timerRemaining = timerDuration;
      timerState     = TIMER_SET;
      lcd.clear();
      Serial.print(F("OK: timer_set="));
      Serial.print(mins); Serial.print('m');
      Serial.print(secs); Serial.println('s');
    } else {
      Serial.println(F("err: use set:mmss e.g. set:2500"));
    }
    return;
  }

  if (strcmp(cmd, "pomodoro") == 0) {
    timerDuration = 25L*60*1000; timerRemaining = timerDuration;
    timerState = TIMER_SET; lcd.clear();
    Serial.println(F("OK: pomodoro=25min ready, waiting to start.."));
    return;
  }

  if (strcmp(cmd, "break") == 0) {
    timerDuration = 5L*60*1000; timerRemaining = timerDuration;
    timerState = TIMER_SET; lcd.clear();
    Serial.println(F("OK: break of 5min ready, waiting to start.."));
    return;
  }

  if (strcmp(cmd, "start") == 0) {
    if (timerState == TIMER_IDLE) { Serial.println(F("err:set a timer first")); return; }
    if (timerState == TIMER_DONE) timerRemaining = timerDuration;
    timerState = TIMER_RUNNING; lastTimerTick = millis();
    if (!buzzerOff) tone(BUZZER_PIN, 1500, 100);
    Serial.println(F("OK: timer started")); return;
  }

  if (strcmp(cmd, "stop") == 0) {
    if (timerState == TIMER_RUNNING) {
      timerState = TIMER_PAUSED;
      if (!buzzerOff) { tone(BUZZER_PIN, 800, 100); delay(200); tone(BUZZER_PIN, 800, 100); }
      Serial.println(F("OK: Timer paused"));
    } else { Serial.println(F("err: Timer not running")); }
    return;
  }

  if (strcmp(cmd, "reset") == 0) {
    timerState = timerDuration > 0 ? TIMER_SET : TIMER_IDLE;
    timerRemaining = timerDuration; lcd.clear();
    Serial.println(F("OK: Timer reset")); return;
  }

  if (strcmp(cmd, "watch:start") == 0) {
    watchRunning = true; lastWatchTick = millis();
    Serial.println(F("OK: Stopwatch started")); return;
  }

  if (strcmp(cmd, "watch:stop") == 0) {
    watchRunning = false;
    Serial.println(F("OK: Stopwatch paused")); return;
  }

  if (strcmp(cmd, "watch:reset") == 0) {
    watchRunning = false; watchElapsed = 0;
    tm.showNumberDecEx(0, 0b01000000, true);
    Serial.println(F("OK: Stopwatch reset")); return;
  }

  if (strcmp(cmd, "led:off") == 0) {
    ledOff = true;
    digitalWrite(LED_GREEN, LOW); digitalWrite(LED_YELLOW, LOW); digitalWrite(LED_RED, LOW);
    Serial.println(F("OK: LEDs off")); return;
  }

  if (strcmp(cmd, "led:on") == 0) {
    ledOff = false; updateLEDs(millis());
    Serial.println(F("OK:LEDs on")); return;
  }

  if (strcmp(cmd, "buzzer:off") == 0) {
    buzzerOff = true;
    noTone(BUZZER_PIN);
    Serial.println(F("OK: Buzzer is off")); return;
  }

  if (strcmp(cmd, "buzzer:on") == 0) {
    buzzerOff = false;
    Serial.println(F("OK: Buzzer is on")); return;
  }

  Serial.print(F("err: Unknown command '"));
  Serial.print(cmd);
  Serial.println(F("' — type 'help'"));
}


void readSensor() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t) && !isnan(h)) {
    temperature = t; humidity = h;
    comfortScore = calcComfort(t, h);
  }
}

void printSensorData() {
  Serial.print(F("Temp: "));    Serial.print(temperature, 1); Serial.println('C');
  Serial.print(F("Humid: "));   Serial.print(humidity, 1);    Serial.println('%');
  Serial.print(F("Comfort: ")); Serial.print(comfortScore);   Serial.println(F("/100"));
}

// calculate the comfort
int calcComfort(float t, float h) {
  float tP = abs(t - 22.0) * 3.0;
  float hP = abs(h - 50.0) * 1.0;
  return constrain(100 - (int)(tP + hP), 0, 100);
}


void updateLCD() {
  char buf[17]; char tbuf[6];
  int tWhole = (int)temperature;
  int tDec   = (int)((temperature - tWhole) * 10);
  if (tDec < 0) tDec = -tDec;
  lcd.setCursor(0, 0);
  snprintf(buf, sizeof(buf), "T:%d.%dC H:%d%%  ", tWhole, tDec, (int)humidity);
  lcd.print(buf);
  lcd.setCursor(0, 1);
  if (timerState == TIMER_IDLE || timerState == TIMER_DONE) {
    snprintf(buf, sizeof(buf), "Comfort:%d/100   ", comfortScore);
  } else {
    msToChars(timerRemaining, tbuf);
    snprintf(buf, sizeof(buf), "C:%d/100 %s  ", comfortScore, tbuf);
  }
  lcd.print(buf);
}

void updateTM() {
  long totalSec = watchElapsed / 1000;
  int mins = (int)(totalSec / 60);
  int secs = (int)(totalSec % 60);
  if (mins > 99) mins = 99;
  tm.showNumberDecEx(mins * 100 + secs, 0b01000000, true);
}

void updateLEDs(unsigned long now) {
  if (ledOff) return;
  if (timerState == TIMER_DONE) {
    if (now - lastBlink >= 400) { lastBlink = now; blinkState = !blinkState; }
    digitalWrite(LED_RED, blinkState ? HIGH : LOW);
    digitalWrite(LED_GREEN, LOW); digitalWrite(LED_YELLOW, LOW);
    static unsigned long timerDoneAt = 0;
    if (timerDoneAt == 0) timerDoneAt = now;
    if (now - timerDoneAt >= 5000) { timerState = TIMER_IDLE; timerDoneAt = 0; }
    return;
  }
  if (comfortScore >= 72) {
    digitalWrite(LED_GREEN, HIGH); digitalWrite(LED_YELLOW, LOW); digitalWrite(LED_RED, LOW);
  } else if (comfortScore >= 45) {
    digitalWrite(LED_GREEN, LOW); digitalWrite(LED_YELLOW, HIGH); digitalWrite(LED_RED, LOW);
  } else {
    digitalWrite(LED_GREEN, LOW); digitalWrite(LED_YELLOW, LOW); digitalWrite(LED_RED, HIGH);
  }
}
