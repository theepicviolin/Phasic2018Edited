#include <SD.h>
#include <SPI.h>
#include <LiquidCrystal.h>

const int lcdPin = 39;
const int rs = 32, en = 30, d4 = 37, d5 = 35, d6 = 33, d7 = 31;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  //initialize SD
  Serial.begin(9600);
  lcd.begin(16, 2);
  pinMode(lcdPin, OUTPUT);
  digitalWrite(lcdPin, HIGH);
  if (!SD.begin(53)) {
    Serial.println("SD Failed");
    lcd.print("SD Failed");
  } else {
    Serial.println("Fam ya did it");
    lcd.print("Fam ya did it");
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
