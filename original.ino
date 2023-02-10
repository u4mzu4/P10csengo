#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WidgetRTC.h>
// kimbiri pw:bps
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "7nmWjwMX3Bdu6YVzoiG_u2JGhK6wsGxP";
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "BPS_Teacher";
char pass[] = "BPS_T_10";

SimpleTimer timer;
WidgetRTC rtc;

void SoundRingBell(int value) {
  // Not sound on Sunday or Saturday
  if ((weekday() == 1) or (weekday() == 7)) return;  

  // Not sound on July and August
  if ((month() == 7) or (month() == 8)) return;  
  if (value == 0) digitalWrite(5,LOW);    //0-1
  else digitalWrite(5,HIGH);             //0-1
}

BLYNK_CONNECTED() {
  rtc.begin();
}

BLYNK_WRITE(V0) {
  if (param.asInt() == 1) SoundRingBell(1);
  else SoundRingBell(0);
 }
BLYNK_WRITE(V1) {
  if (param.asInt() == 1) SoundRingBell(1);
  else SoundRingBell(0);
 }
BLYNK_WRITE(V2) {
  if (param.asInt() == 1) SoundRingBell(1);
  else SoundRingBell(0);
 }
BLYNK_WRITE(V3) {
  if (param.asInt() == 1) SoundRingBell(1);
  else SoundRingBell(0);
 }
BLYNK_WRITE(V4) {
  if (param.asInt() == 1) SoundRingBell(1);
  else SoundRingBell(0);
 }
BLYNK_WRITE(V5) {
  if (param.asInt() == 1) SoundRingBell(1);
  else SoundRingBell(0);
 }
BLYNK_WRITE(V6) {
  if (param.asInt() == 1) SoundRingBell(1);
  else SoundRingBell(0);
 }
 BLYNK_WRITE(V7) {
  if (param.asInt() == 1) SoundRingBell(1);
  else SoundRingBell(0);
 }
 BLYNK_WRITE(V8) {
  if (param.asInt() == 1) SoundRingBell(1);
  else SoundRingBell(0);
 }
 
void reconnectBlynk() {
  if (!Blynk.connected()) {
    digitalWrite(LED_BUILTIN,HIGH);
    Serial.println("Lost connection");
    if(Blynk.connect()) Serial.println("Reconnected");
    else Serial.println("Not reconnected");
  }
}

void setup()
{
  // Debug console
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN,HIGH);
  pinMode(5, OUTPUT);
  timer.setInterval(30*1000, reconnectBlynk);
  setSyncInterval(10*60);
 
  Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8080);
 
}

void loop()
{
  timer.run();
  if(Blynk.connected()) {
    Blynk.run();
    digitalWrite(LED_BUILTIN,LOW);
  }
   else  
     {digitalWrite(5,LOW);  // ha nincs wifi, csengő leáll
}
}
