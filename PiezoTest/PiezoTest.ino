#include <LiquidCrystal.h>

const int piezoPin = A0;

const int lcdPin = 39;
const int rs = 32, en = 30, d4 = 37, d5 = 35, d6 = 33, d7 = 31;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  Serial.begin(9600);
  lcd.begin(20, 4);
  pinMode(lcdPin, OUTPUT);
  digitalWrite(lcdPin, HIGH);
  pinMode(piezoPin, INPUT);
}

void loop() {
  /*
   * PRESS CTRL+SHIFT+M TO SEE THE SERIAL MONITOR
   */
   Serial.println(analogRead(piezoPin));
   lcd.setCursor(0,0);
   lcd.print(analogRead(piezoPin));
   /*
    * Default value should be around 512, higher flow rates will show high amplitude oscilations around that value
    */
}

