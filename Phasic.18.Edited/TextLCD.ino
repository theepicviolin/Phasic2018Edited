

void setupCustomChars() {
    // All byte functions delcared below are for the custom characters
  // for printing the big text in the active phase
  //create all custom characters for BIG TEXT
  
  byte drop[8] = {
    0b00000,
    0b00100,
    0b01110,
    0b11111,
    0b11111,
    0b01110,
    0b00000,
    0b00000
  };

  byte Full[8] = {
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111
  };
  byte MCenter[8] = {
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b01110,
    0b00100,
    0b00000
  };
  byte MTop[8] = {
    0b00000,
    0b10001,
    0b11011,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111
  };
  byte WBottom[8] = {
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11011,
    0b10001,
    0b00000
  };
  byte Wcenter[8] = {
    0b00000,
    0b00100,
    0b01110,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111
  };
  byte GRight[8] = {
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b11111,
    0b11111,
    0b11111,
    0b11111
  };
  byte ECenter[8] = {
    0b00000,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b00000
  };
  
  lcd.createChar((byte)0, Full);
  lcd.createChar(1, MCenter);
  lcd.createChar(2, MTop);
  lcd.createChar(3, WBottom);
  lcd.createChar(4, Wcenter);
  lcd.createChar(5, GRight);
  lcd.createChar(6, ECenter);
  lcd.createChar(7, drop);
}

//This function prints OFF LOW MED or HIGH to the lcd screen in 3x3 letters
//OFF if arg is 0 LOW if arg is 1 or 2, med if arg is 3 HIGH if arg is 4 or 5
void displayBigtext(FlowType flowtype) {
  //create an array of function pointers
  void (*fcnPtrArray[])() = {&printOff, &printLow, &printLow, &printMed, &printHigh, &printHigh};
  fcnPtrArray[(byte)flowtype](); //index the array based on flownumber, and call the function
}

void printOff() {
  //write O
  lcd.setCursor(0, 1);
  lcd.write((byte)0);
  lcd.write((byte)0);
  lcd.write((byte)0);
  lcd.setCursor(0, 2);
  lcd.write((byte)0);
  lcd.write(" ");
  lcd.write((byte)0);
  lcd.setCursor(0, 3);
  lcd.write((byte)0);
  lcd.write((byte)0);
  lcd.write((byte)0);
  //Write F
  lcd.setCursor(4, 1);
  lcd.write((byte)0);
  lcd.write((byte)0);
  lcd.write((byte)0);
  lcd.setCursor(4, 2);
  lcd.write((byte)0);
  lcd.write((byte)6);
  lcd.write(" ");
  lcd.setCursor(4, 3);
  lcd.write((byte)0);
  lcd.write(" ");
  lcd.write(" ");
  //Write F
  lcd.setCursor(8, 1);
  lcd.write((byte)0);
  lcd.write((byte)0);
  lcd.write((byte)0);
  lcd.setCursor(8, 2);
  lcd.write((byte)0);
  lcd.write((byte)6);
  lcd.write(" ");
  lcd.setCursor(8, 3);
  lcd.write((byte)0);
  lcd.write(" ");
  lcd.write(" ");
}

void printMed() {
  //write M
  lcd.setCursor(0, 1);
  lcd.write((byte)0);
  lcd.write((byte)2);
  lcd.write((byte)0);
  lcd.setCursor(0, 2);
  lcd.write((byte)0);
  lcd.write((byte)1);
  lcd.write((byte)0);
  lcd.setCursor(0, 3);
  lcd.write((byte)0);
  lcd.write(" ");
  lcd.write((byte)0);
  //Write E
  lcd.setCursor(4, 1);
  lcd.write((byte)0);
  lcd.write((byte)0);
  lcd.write((byte)0);
  lcd.setCursor(4, 2);
  lcd.write((byte)0);
  lcd.write((byte)6);
  lcd.write(" ");
  lcd.setCursor(4, 3);
  lcd.write((byte)0);
  lcd.write((byte)0);
  lcd.write((byte)0);
  //Write D
  lcd.setCursor(8, 1);
  lcd.write((byte)0);
  lcd.write((byte)0);
  lcd.write(" ");
  lcd.setCursor(8, 2);
  lcd.write((byte)0);
  lcd.write(" ");
  lcd.write((byte)0);
  lcd.setCursor(8, 3);
  lcd.write((byte)0);
  lcd.write((byte)0);
  lcd.write(" ");
}

void printHigh() {
  //write H
  lcd.setCursor(0, 1);
  lcd.write((byte)0);
  lcd.write(" ");
  lcd.write((byte)0);
  lcd.setCursor(0, 2);
  lcd.write((byte)0);
  lcd.write((byte)0);
  lcd.write((byte)0);
  lcd.setCursor(0, 3);
  lcd.write((byte)0);
  lcd.write(" ");
  lcd.write((byte)0);

  //write I, no serif
  lcd.setCursor(4, 1);
  lcd.write((byte)0);
  lcd.setCursor(4, 2);
  lcd.write((byte)0);
  lcd.setCursor(4, 3);
  lcd.write((byte)0);

  //write G
  lcd.setCursor(6, 1);
  lcd.write((byte)0);
  lcd.write((byte)0);
  lcd.write((byte)0);
  lcd.setCursor(6, 2);
  lcd.write((byte)0);
  lcd.write(" ");
  lcd.write((byte)5);
  lcd.setCursor(6, 3);
  lcd.write((byte)0);
  lcd.write((byte)0);
  lcd.write((byte)0);

  //write H
  lcd.setCursor(10, 1);
  lcd.write((byte)0);
  lcd.write(" ");
  lcd.write((byte)0);
  lcd.setCursor(10, 2);
  lcd.write((byte)0);
  lcd.write((byte)0);
  lcd.write((byte)0);
  lcd.setCursor(10, 3);
  lcd.write((byte)0);
  lcd.write(" ");
  lcd.write((byte)0);
}

void printLow() {
  //write L
  lcd.setCursor(0, 1);
  lcd.write((byte)0);
  lcd.write("  ");
  lcd.setCursor(0, 2);
  lcd.write((byte)0);
  lcd.write("  ");
  lcd.setCursor(0, 3);
  lcd.write((byte)0);
  lcd.write((byte)0);
  lcd.write((byte)0);
  //Write O
  lcd.setCursor(4, 1);
  lcd.write((byte)0);
  lcd.write((byte)0);
  lcd.write((byte)0);
  lcd.setCursor(4, 2);
  lcd.write((byte)0);
  lcd.write(" ");
  lcd.write((byte)0);
  lcd.setCursor(4, 3);
  lcd.write((byte)0);
  lcd.write((byte)0);
  lcd.write((byte)0);
  //Write W
  lcd.setCursor(8, 1);
  lcd.write((byte)0);
  lcd.write(" ");
  lcd.write((byte)0);
  lcd.setCursor(8, 2);
  lcd.write((byte)0);
  lcd.write((byte)4);
  lcd.write((byte)0);
  lcd.setCursor(8, 3);
  lcd.write((byte)0);
  lcd.write((byte)3);
  lcd.write((byte)0);
};

