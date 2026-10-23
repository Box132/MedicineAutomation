#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

const char* wifiName = "Iphone";
const char* wifiPass = "higuysss123";
const char* apiKey   = "AQ.Ab8RN6Ky75buCn-ebxJH4f7G9vtfeMgiRhbk9uxdhDuKsg9ktQ";

String meds[] = {"Biogesic", "Medicol", "Neozep"};
int servoPins[] = {18, 19, 23};
int servoOff = 0;
int servoOn = 90;
int dropTime = 600;

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo servos[3];

String askGemini(String input) {
  input.replace("\"", "");

  String body = "{\"contents\":[{\"parts\":[{\"text\":\""
    "You are a medicine dispenser. Available: Biogesic (fever, headache), "
    "Medicol (body pain, headache), Neozep (colds, runny nose). "
    "The user typed: " + input + ". "
    "Reply with the medicine name only, or NONE if unclear."
    "\"}]}]}";

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.begin(client, "https://generativelanguage.googleapis.com/v1beta/models/gemini-flash-lite-latest:generateContent");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("x-goog-api-key", apiKey);
  http.setTimeout(20000);
  http.POST(body);
  String reply = http.getString();
  http.end();
  return reply;
}

void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.print("Connecting...");

  for (int i = 0; i < 3; i++) {
    servos[i].setPeriodHertz(50);
    servos[i].attach(servoPins[i], 500, 2400);
    servos[i].write(servoOff);
  }

  WiFi.begin(wifiName, wifiPass);
  while (WiFi.status() != WL_CONNECTED) delay(300);

  lcd.clear();
  lcd.print("Type a medicine");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input == "") return;

    lcd.clear();
    lcd.print("Thinking...");
    String reply = askGemini(input);

    lcd.clear();
    lcd.print("Not recognized");

    for (int i = 0; i < 3; i++) {
      if (reply.indexOf(meds[i]) >= 0) {
        lcd.clear();
        lcd.print("Dropping down:");
        lcd.setCursor(0, 1);
        lcd.print(meds[i]);
        servos[i].write(servoOn);
        delay(dropTime);
        servos[i].write(servoOff);
        break;
      }
    }

    delay(2000);
    lcd.clear();
    lcd.print("Type a medicine");
  }
}