#include "ESPAsyncWebServer.h"
#include <credentials_P10.h>
#include <time.h>

#define PLAYERPIN 5
#define NTPSERVER "hu.pool.ntp.org"
#define TIMEZONE "CET-1CEST,M3.5.0/02,M10.5.0/03"
#define TIMEOUT 5000     //5 sec
#define TIMEREFRESH 28000  //28 sec
#define NROFCLASSES 8
#define SOUNDLENGHT 15000   //15 sec
#define MILLISPERMIN 60000  //1min
#define MAXMIN 59
#define MAXHOUR 23

unsigned int startOfClass[NROFCLASSES];
int startHours[NROFCLASSES] = { 9, 10, 10, 11, 11, 12, 13, 14 };
int startMinutes[NROFCLASSES] = { 0, 15, 30, 30, 45, 45, 30, 30 };
unsigned eventServed = NROFCLASSES;
const char index_html[] PROGMEM = "<!DOCTYPE html><head><title>Iskolacsengo</title></head><body><p>1.ora: %1PH%<br />2.ora: %2PH%<br />3.ora: %3PH%<br />4.ora: %4PH%<br />5.ora: %5PH%<br />6.ora: %6PH%<br />7.ora: %7PH%<br />8.ora: %8PH%</p></p></body></html>";
struct tm dateTime;
time_t now;

//Enums
enum MAIN_SM {
  INIT = 0,
  SOUND_OFF = 1,
  SOUND_ON = 2,
  ERROR = 3,
  RESERVED = 4,
};

AsyncWebServer server(80);

int CheckEvents() {
  int eventMatch = -1;

  for (int i = 0; i < NROFCLASSES; i++) {
    if ((startHours[i] == dateTime.tm_hour) && (startMinutes[i] == dateTime.tm_min)) {
      eventMatch = i;
      if (eventServed == eventMatch) {
        eventMatch = -1;
      } else {
        Serial.print("Event found: ");
        Serial.println(i);
        Serial.print(startHours[i]);
        Serial.print(":");
        Serial.println(startMinutes[i]);
      }
    }
  }
  return eventMatch;
}

void SoundRingBell(bool value) {
  // No sound on Sunday or Saturday
  if ((dateTime.tm_wday == 6) || (dateTime.tm_wday == 0)) {
    digitalWrite(PLAYERPIN, LOW);
    return;
  }
  // No sound on July and August
  if ((dateTime.tm_mon + 1 == 7) || (dateTime.tm_mon + 1 == 8)) {
    digitalWrite(PLAYERPIN, LOW);
    return;
  }
  // No sound on Friday durin workshop sessions
  if ((dateTime.tm_wday == 5) && (eventServed > 2)) {
    digitalWrite(PLAYERPIN, LOW);
    return;
  }

  digitalWrite(LED_BUILTIN, !value);  //LOW/HIGH
  digitalWrite(PLAYERPIN, value);     //LOW/HIGH
}

bool RefreshDateTime() {
  bool timeIsValid = true;
  time(&now);                    // read the current time
  localtime_r(&now, &dateTime);  // update the structure tm with the current

  if ((dateTime.tm_year > 135) || (dateTime.tm_year < 123)) {
    timeIsValid = false;
  }
  return (timeIsValid);
}

String processor(const String& var) {
  String tempvar = var;
  tempvar.remove(2);
  int numberofPH = tempvar.toInt() - 1;
  return (String(startHours[numberofPH]) + ":" + String(startMinutes[numberofPH]));
}

void WaitForMinute(int waitfromSecond) {
  int storedMin = dateTime.tm_min;
  int waitingTime = MILLISPERMIN - (1000 * waitfromSecond);
  delay(waitingTime);
  dateTime.tm_min = storedMin + 1;
  TimeLimiter();
}

void TimeLimiter() {
  if (dateTime.tm_min > MAXMIN) {
    dateTime.tm_min = 0;
    dateTime.tm_hour++;
    if (dateTime.tm_hour > MAXHOUR) {
      dateTime.tm_hour = 0;
    }
  } else {
    return;
  }
}

void setup() {
  pinMode(PLAYERPIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PLAYERPIN, LOW);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait for connection
  unsigned long wifitimeout = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (millis() - wifitimeout > TIMEOUT) {
      break;
    }
  }

  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    int paramsNr = request->params();

    for (int i = 0; i < paramsNr; i++) {

      AsyncWebParameter* p = request->getParam(i);
      startOfClass[i] = p->value().toInt();
      startHours[i] = startOfClass[i] / 100;
      startMinutes[i] = startOfClass[i] % 100;
    }
    request->send_P(200, "text/html", index_html, processor);
  });
  server.begin();

  configTime(TIMEZONE, NTPSERVER);
}

void loop() {
  static unsigned long lastRefresh;
  static unsigned long soundStarted;
  static MAIN_SM state = INIT;
  int eventFound;

  switch (state) {
    case INIT:
      {
        while (!RefreshDateTime()) {
          delay(100);
        }
        lastRefresh = millis();
        state = SOUND_OFF;
        Serial.println("System inited");
        break;
      }
    case SOUND_OFF:
      {
        if (millis() - lastRefresh > TIMEREFRESH) {
          while (!RefreshDateTime()) {
            delay(100);
          }
          Serial.println("Time refreshed");
          lastRefresh = millis();
        }
        WaitForMinute(dateTime.tm_sec);
        Serial.print("Time: ");
        Serial.print(dateTime.tm_hour);
        Serial.print(":");
        Serial.println(dateTime.tm_min);
        eventFound = CheckEvents();
        if (0 <= eventFound) {
          eventServed = eventFound;
          SoundRingBell(true);
          soundStarted = millis();
          state = SOUND_ON;
          Serial.println("Sound started");
        }
        break;
      }
    case SOUND_ON:
      {
        if (millis() - soundStarted > SOUNDLENGHT) {
          SoundRingBell(false);
          state = SOUND_OFF;
          Serial.println("Sound stopped");
        }
        break;
      }
  }
}