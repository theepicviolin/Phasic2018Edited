#include <WS2812FX.h>
#include <SD.h>
#include <SPI.h>
#include <LiquidCrystal.h>

#define LED_COUNT 76
#define LED_PIN 43
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
int s = 700; //speed, realized as delay time in ms
const int nSegs = 10;
int segLengths[nSegs] = {7, 7, 9, 8, 7, 7, 8, 9, 7, 7};

const int lcdPin = 29;
const int rs = 32, en = 30, d4 = 37, d5 = 35, d6 = 33, d7 = 31;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int pirPin = 34;
const int piezoPin = A0;

void setup() {
  Serial.begin(9600);
  setupNeoPixels();
  
  pinMode(piezoPin, INPUT);

  lcd.begin(20, 4);
  pinMode(lcdPin, OUTPUT);
  digitalWrite(lcdPin, HIGH);

  pinMode(pirPin, INPUT);
  digitalWrite(pirPin, LOW); // Turn it on
  Serial.print("Calibrating PIR...");
  lcd.setCursor(0, 0);
  lcd.print("Calibrating PIR");
  pinMode(lcdPin, OUTPUT);
  delay(1000);
  Serial.println(" done!");
  lcd.setCursor(15, 0);
  lcd.print("done");
  lcd.setCursor(0, 1);
  if (!SD.begin(53)) {
    Serial.println("SD Failed");
    lcd.print("SD Failed");
  } else {
    Serial.println("SD initialized");
    lcd.print("SD initialized");
  }
  delay(2000);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(pirPin)) {
    ws2812fx.service();
  }
  lcd.setCursor(0, 2);
  lcd.print("PIR: " + String(digitalRead(pirPin)));
  lcd.setCursor(0, 3);
  lcd.print("Piezo: " + String(analogRead(piezoPin)));
  Serial.println(analogRead(piezoPin));
}


void setupNeoPixels() {
  ws2812fx.init();
  ws2812fx.setBrightness(10);
  ws2812fx.setColor(00, 128, 255); 
  int curIdx = 0;
  bool a = true;
  bool b = false;
  for (int segIdx = 0; segIdx < nSegs; segIdx++) {
    ws2812fx.setSegment(segIdx, curIdx, curIdx + segLengths[segIdx] - 1, FX_MODE_COMET,  0x0080FF, s + random(80) - 40, a);
    curIdx += segLengths[segIdx];
    a = a ^ b; 
    b = !b;
  }
  ws2812fx.start();
}

