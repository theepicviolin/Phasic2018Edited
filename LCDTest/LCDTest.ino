#include <LiquidCrystal.h>

/*
 * IF SOMETHING IS GOING WRONG LOOK HERE PLZZZZZ
 * ALSO TRY RUNNING THE ARDUINO USING THE POWER THING AND NOT THE COMPUTER THING
 */
const int lcdPin = 39;
const int rs = 32, en = 30, d4 = 37, d5 = 35, d6 = 33, d7 = 31;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  pinMode(lcdPin, OUTPUT);
  digitalWrite(lcdPin, HIGH);
  // Print a message to the LCD.
  lcd.print("hello, world!");
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis() / 1000);
}

