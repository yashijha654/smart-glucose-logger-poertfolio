#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Pin assignments
#define SDA_PIN   21
#define SCL_PIN   22
#define RED_LED   19
#define GREEN_LED 18
#define BUZZER    17
#define ACK_BTN    5
#define LOG_BTN    4

// Glucose alarm limits (mg/dL)
const int LOW_LIMIT  = 70;
const int HIGH_LIMIT = 180;
const unsigned long READING_INTERVAL = 5000;

// SIMULATED test values (not from a sensor)
const int simReadings[] = {110, 95, 65, 150, 200, 120, 88, 170};
const int simCount = sizeof(simReadings) / sizeof(simReadings[0]);

// Insulin dose times (24-hour clock)
const int DOSE_HOUR[] = {8, 20};
const int DOSE_MIN[]  = {0, 0};
const int DOSE_COUNT  = 2;

Adafruit_SSD1306 display(128, 64, &Wire, -1);
RTC_DS3231 rtc;

int currentReading = 0;
int simIndex = 0;
unsigned long lastReadingTime = 0;
bool alarmOn = false;
bool reminderOn = false;
int lastReminderKey = -1;
bool blinkState = false;
unsigned long lastBlink = 0;

void logEntry(DateTime t, int value, const char* tag) {
  char buf[90];
  sprintf(buf, "%s %04d-%02d-%02d %02d:%02d:%02d  %d mg/dL (SIMULATED)",
          tag, t.year(), t.month(), t.day(),
          t.hour(), t.minute(), t.second(), value);
  Serial.println(buf);
}

void nextSimulatedReading() {
  currentReading = simReadings[simIndex];
  simIndex = (simIndex + 1) % simCount;

  if (currentReading < LOW_LIMIT || currentReading > HIGH_LIMIT) {
    alarmOn = true;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(ACK_BTN, INPUT_PULLUP);
  pinMode(LOG_BTN, INPUT_PULLUP);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while (true) delay(100);
  }
  if (!rtc.begin()) {
    Serial.println("DS3231 not found");
    while (true) delay(100);
  }

  // Set the clock only if its battery has run out
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Startup beep to confirm the buzzer works
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);
}

void loop() {
  DateTime now = rtc.now();

  // Glucose: new test reading every READING_INTERVAL
  if (millis() - lastReadingTime >= READING_INTERVAL) {
    lastReadingTime = millis();
    nextSimulatedReading();
    logEntry(now, currentReading, "AUTO");
  }

  // Insulin reminder: fire at each scheduled dose time, once per minute
  int nowKey = now.hour() * 60 + now.minute();
  for (int i = 0; i < DOSE_COUNT; i++) {
    int doseKey = DOSE_HOUR[i] * 60 + DOSE_MIN[i];
    if (nowKey == doseKey && nowKey != lastReminderKey) {
      reminderOn = true;
      lastReminderKey = nowKey;
    }
  }

  // Log button: record the current value with a timestamp
  static bool lastLogState = HIGH;
  bool logState = digitalRead(LOG_BTN);
  if (lastLogState == HIGH && logState == LOW) {
    logEntry(now, currentReading, "LOG");
  }
  lastLogState = logState;

  // Ack button: silence the glucose alarm and the insulin reminder
  if (digitalRead(ACK_BTN) == LOW) {
    alarmOn = false;
    reminderOn = false;
  }

  // Alarm output: red LED and buzzer blink for either alert
  bool anyAlert = alarmOn || reminderOn;
  if (anyAlert) {
    digitalWrite(GREEN_LED, LOW);
    if (millis() - lastBlink > 250) {
      lastBlink = millis();
      blinkState = !blinkState;
      digitalWrite(RED_LED, blinkState);
      digitalWrite(BUZZER, blinkState);
    }
  } else {
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER, LOW);
    digitalWrite(GREEN_LED, HIGH);
  }

  // OLED: refresh when the time or reading changes
  static int lastSec = -1;
  static int lastShown = -1;
  if (now.second() != lastSec || currentReading != lastShown) {
    lastSec = now.second();
    lastShown = currentReading;

    char l1[24], l2[24], l3[24], l4[24];
    sprintf(l1, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
    sprintf(l2, "Glucose: %d mg/dL", currentReading);
    sprintf(l3, "Status: %s",
            alarmOn ? "ALARM" : (reminderOn ? "INSULIN DUE" : "OK"));
    sprintf(l4, "SIMULATED DATA");

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(l1);
    display.println(l2);
    display.println(l3);
    display.println(l4);
    display.display();
  }

  delay(20);
}
