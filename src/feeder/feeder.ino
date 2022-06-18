// Shamelessly stolen from alexgyver

const byte feedTime[][2] = {
  {7, 0},
  {13, 0},
  {19, 0},
  {23, 0}
};

#define DEBUG                           // comment for debug

#define EE_RESET 5                      // timer reset flag
#define BTN_PIN 2                       // button pin
#define STEPS_FRW 28                    // steps forward
#define STEPS_BKW 12                    // steps backward
#define FEED_SPEED 3000                 // stepper_delay
#define STEPER_MOTOR_DIRECTION -1       // modify direction of stepper motor
#define CHECK_TIMEOUT 500               // refresh timeout

const byte drvPins[] = {3, 4, 5, 6};    // driver pins
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

#include "EncButton.h"
#include <EEPROM.h>
#include <RTClib.h>

RTC_DS3231 rtc;
EncButton<BTN_PIN> btn;
int feedAmount = 30;

void setup() {
#ifdef DEBUG
  Serial.begin(9600);
#endif

  rtc.begin();
  // if reset flag
  if (EEPROM.read(0) != EE_RESET) {
    EEPROM.write(0, EE_RESET);
    EEPROM.put(1, feedAmount);
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  EEPROM.get(1, feedAmount);
  // set pins to output
  for (byte i = 0; i < 4; i++) pinMode(drvPins[i], OUTPUT);   
}

void loop() {
  static uint32_t tmr = 0;
  if (millis() - tmr > CHECK_TIMEOUT) {
    static byte prevMin = 0;
    tmr = millis();
    DateTime now = rtc.now();

#ifdef DEBUG
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    Serial.print("feedAmount");
    Serial.print(feedAmount, DEC);
    Serial.println();
#endif

    if (prevMin != now.minute()) {
      prevMin = now.minute();
      for (byte i = 0; i < sizeof(feedTime) / 2; i++)    // для всего расписания
        if (feedTime[i][0] == now.hour() && feedTime[i][1] == now.minute())    // пора кормить
          feed();
    }
  }

// TODO ::: remove this shit

  btn.tick();
  if (btn.isClick()) {
    feed();
  }
  // if (btn.isHold()) {
  //   int newAmount = 0;
  //   while (btn.isHold()) {
  //     btn.tick();
  //     oneRev();
  //     newAmount++;
  //   }
  //   disableMotor();
  //   feedAmount = newAmount;
  //   EEPROM.put(1, feedAmount);
  // }
}

void feed() {
  Serial.print("feedAmount");
  Serial.print(feedAmount, DEC);
  Serial.print("\t");
  
  for (int i = 0; i < feedAmount; i++) oneRev();      // крутим на количество feedAmount
  disableMotor();
}

void disableMotor() {
  for (byte i = 0; i < 4; i++) digitalWrite(drvPins[i], 0); // выключаем ток на мотор
}

void oneRev() {
  static byte val = 0;  
  for (byte i = 0; i < STEPS_BKW; i++) {
    runMotor(val--);
  }
  for (byte i = 0; i < STEPS_FRW; i++) {
    runMotor(val++);
  }
}

void runMotor(byte thisStep) {
  static const byte steps[] = {0b1010, 0b0110, 0b0101, 0b1001};
  for (byte i = 0; i < 4; i++)
    digitalWrite(drvPins[i], bitRead(steps[thisStep & 0b11], i));
  delayMicroseconds(FEED_SPEED);
}
