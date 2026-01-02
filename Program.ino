#include <Servo.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------------- PIN CONFIG ----------------
#define TRIG 7
#define ECHO 6
#define SERVO_PIN 5
#define DHTPIN 2
#define DHTTYPE DHT11

// ---------------- OBJECTS ----------------
Servo gate;
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);   // If blank, change to 0x3F

// ---------------- VARIABLES ----------------
int distance = 0;
bool gateOpen = false;

unsigned long gateOpenedAt = 0;
unsigned long lastUltraRead = 0;
unsigned long lastDHTRead = 0;
unsigned long lastLCDUpdate = 0;

const int openAngle = 90;
const int closeAngle = 0;

const unsigned long autoCloseTime = 3000;     // 3 seconds
const unsigned long ultraInterval = 200;      // 200ms
const unsigned long dhtInterval = 1000;       // 1 second
const unsigned long lcdInterval = 500;        // LCD every 0.5 sec

float tempC = 0;
float humidity = 0;

void setup() {
  Serial.begin(9600);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  gate.attach(SERVO_PIN);
  gate.write(closeAngle);

  dht.begin();

  lcd.init();
  lcd.backlight();
  lcd.print("SMART SYSTEM");
  delay(1000);
  lcd.clear();
}

void loop() {
  unsigned long now = millis();

  // ------------------------ ULTRASONIC ------------------------
  if (now - lastUltraRead >= ultraInterval) {
    lastUltraRead = now;

    digitalWrite(TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);

    long duration = pulseIn(ECHO, HIGH, 30000);
    distance = duration * 0.034 / 2;

    Serial.print("Distance: ");
    Serial.println(distance);

    if (distance > 0 && distance < 10) {
      if (!gateOpen) {
        gate.write(openAngle);
        gateOpen = true;
        gateOpenedAt = now;
        Serial.println("Gate Open");
      }
    }
  }

  // ------------------------ AUTO CLOSE ------------------------
  if (gateOpen && (now - gateOpenedAt >= autoCloseTime)) {
    gate.write(closeAngle);
    gateOpen = false;
    Serial.println("Gate Closed");
  }

  // ------------------------ DHT SENSOR ------------------------
  if (now - lastDHTRead >= dhtInterval) {
    lastDHTRead = now;

    humidity = dht.readHumidity();
    tempC = dht.readTemperature();

    Serial.print("Temp: ");
    Serial.print(tempC);
    Serial.print(" °C   Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  }

  // ------------------------ LCD UPDATE ONLY TEMP + HUM ------------ 
  if (now - lastLCDUpdate >= lcdInterval) {
    lastLCDUpdate = now;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print((int)tempC);
    lcd.print("C");

    lcd.setCursor(0, 1);
    lcd.print("Hum : ");
    lcd.print((int)humidity);
    lcd.print("%");
  }
}
