void saveUsageSummary() {
  //save all of it in one nice 5 column file
  File UseSum = SD.open(F("UseSums.txt"), FILE_WRITE);
  if (UseSum) {
    thisEWMAAvg = thisEWMA / thisCounter;
    thisStdAvg = thisStd / thisCounter;
   thisAvgFlow= calculateCurrentFlow(useEWMA ? thisEWMAAvg : thisStdAvg);
   thisAvgFlow = min(thisAvgFlow, MaxFlow);
   thisAvgFlow = max(thisAvgFlow, 0);

    thisWater = thisElapsed * thisAvgFlow; //CHOOSE WHICH METHOD TO CALCULATE WATER
    //Check what day / hour it is
    UseSum.print(Today); //DAY:HR:TIME:EWMA:STDD:H2O
    UseSum.print("\t");
    UseSum.print(thisHour);
    UseSum.print("\t");
    UseSum.print(thisElapsed);
    UseSum.print("\t");
    UseSum.print(thisEWMAAvg);
    UseSum.print("\t");
    UseSum.print(thisStdAvg);
    UseSum.print("\t");
    UseSum.println(thisWater);
    UseSum.close();
    //Add values to the daily totals
    todayOnTime += thisElapsed;
    todayTimeEWMA += thisEWMAAvg * thisElapsed;
    todayStdDevTime += thisStdAvg * thisElapsed;
    todayWater += thisWater; //OR this Water, or some combo!!! <------------- DAILY H2O calc.
    todayUses++;
    if (!SaveToArrays()) { //save to arrays or show error
      lcd.clear();
      lcd.write(F("Arrays not saved,"));
      lcd.setCursor(0, 1);
      lcd.write(F("please contact"));
      lcd.setCursor(0, 2);
      lcd.write(F("gtwaterstudy@gatech.edu"));
      lcd.setCursor(0, 3);
      lcd.write(F("Or 6263908943"));
      delay(30000);
    }
  }
  else { //save use sum or show error
    lcd.clear();
    lcd.print(F("ERROR UseSum not opened"));
    delay(30000);
  }
}

byte whatDay() { //reads a file called Day and outputs the 1-2 digit int ---------------------- WHAT DAY ------------
  File Day;
  String inString = "";
  //int i = 0;
  byte LastDay = 0;
  //read what day it remembers it being from Days.txt
  Day = SD.open(daysFile);
  if (Day) {
    while (Day.available()) {
      char inDay = Day.read();
      inString += inDay;
    }
  } else {
    lcd.clear();
    lcd.print(F("Error open Days"));
    delay(30000);
  }
  Day.close();

  LastDay = (byte)inString.toFloat();
  //inString = "";
  return LastDay;
}

//Most imporatnat timing funcitons, will check if an hour has passed, a day has passed, and will
// read and write from and to the Days and Hour .txt files, this keeps track of where the program is even
// if the arduino gets turned off, so it wont lose its place in the phases or arrays
byte whatHour() {
  File Hour;
  File Day;
  String inHour = "";
  String inString = "";
  byte LastHour = 0;
  byte LastDayRead = 0;
  //read what day it remembers it being from Days.txt
  Hour = SD.open(hourFile);
  //Read hour file
  if (Hour) {
    while (Hour.available()) {
      char hourread = Hour.read();
      inHour += hourread;
    }
  } else {
    lcd.clear();
    lcd.print(F("ERROR Hour File"));
    delay(30000);
  }
  Hour.close();
  LastHour = (byte)inHour.toFloat(); //Set Last Hour to read from file
  inHour = "";
  //Has time elapsed to change the file?
  //Is there an hour difference between current millis() and last
  //HourCheck?
  if ((millis() - HourCheck) > 3600000) { //change back to 24 and delete modifier
    HourCheck = millis();
    LastHour++;
    if (LastHour == 24) { //keep it on 24hr clock, if day changed, change day
      LastHour = 0;
      Day = SD.open(daysFile); //open and read day
      if (Day) {
        while (Day.available()) {
          char inDay = Day.read();
          inString += inDay;
        }
        LastDayRead = (byte)inString.toFloat();
        inString = "";
        LastDayRead++; //change day
      } else {
        lcd.clear();
        lcd.print(F("ERROR Read Day"));
        delay(30000);
      }


      SD.remove(daysFile);
      Day = SD.open(daysFile, FILE_WRITE);
      if (Day) {
        Day.print(LastDayRead);
      } else {
        lcd.clear();
        lcd.print(F("ERROR Day File"));
        delay(3000);
      }
      Day.close();
    }
    SD.remove(hourFile); //change hour file if hour passed
    Hour = SD.open(hourFile, FILE_WRITE);
    if (Hour) {
      Hour.print(LastHour);
    }
    else {
      lcd.print(F("Error changing Hour"));
      delay(30000);
    }
    Hour.close();
  }
  return LastHour;
}

void displayText77( char OpenFile[8]) { //tip names should be in format "###.txt", and have an empty final character
  int idx = 0;
  byte y = 0;
  byte x = 0;
  char tRead;
  char tLast;
  char tPrinted;
  File tipFile;

  tipFile = SD.open(OpenFile);
  lcd.clear();
  if (tipFile) {
    while (tipFile.available()) {
      tPrinted = tLast;
      tLast = tRead;
      tRead = tipFile.read();
      if (idx >= 1) {
        if (x == 19) {
          if (tPrinted == ' ') {
            lcd.print(' ');
            x++;
          } else if (tRead != ' ' && tLast != ' ' && tPrinted != ' ') {
            lcd.print("-");
            x++;
          }
        }
        if (x == 20) {
          y++;
          x = 0;
        }
        lcd.setCursor(x, y);
        if (x == 0 && tLast == ' ') {
          ;//do nothing and skip this char
        }
        else {
          lcd.print(tLast);
          x++;
        }
      }
      idx++;
    }
    tipFile.close();
  } else {
    lcd.clear();
    lcd.print(F("File not found"));
    tipFile.close();
  }
}

/*
Saves the values stored in:
unsigned long todayOnTime;
unsigned long todayTimeEWMA;
unsigned long todayStdDevTime;
double todayWater;
int todayUses;
to the file in newArraysFile in a format where the first nDays*4 bytes are for
the first variable (todayOnTime) for each day. The next nDays*4 bytes are for
the next variable (todayTimeEWMA), and so on.
*/
bool SaveToArrays() {
  ToByte value;
  File Array = SD.open(newArraysFile, FILE_WRITE);
  if (!Array) {
    return false;
  }
  value.l = todayOnTime;
  Array.seek(4 * (Today - 1));
  Array.write(value.c, 4);
  value.l = todayTimeEWMA;
  Array.seek(4 * (nDays + Today - 1));
  Array.write(value.c, 4);
  value.l = todayStdDevTime;
  Array.seek(4 * (nDays * 2 + Today - 1));
  Array.write(value.c, 4);
  value.d = todayWater;
  Array.seek(4 * (nDays * 3 + Today - 1));
  Array.write(value.c, 4);
  value.i = todayUses;
  Array.seek(4 * (nDays * 4 + Today - 1));
  Array.write(value.c, 4);
  Array.close();
  return true;
}

/*
Reads the file in newarraysFile and saves the variables for the
current day into:
unsigned long todayOnTime;
unsigned long todayTimeEWMA;
unsigned long todayStdDevTime;
double todayWater;
int todayUses;
*/
bool ReadFromArrays() {
  ToByte value;
  File Array = SD.open(newArraysFile, FILE_WRITE);
  if (!Array) {
    return false;
  }
  Array.seek(4 * (Today - 1));
  Array.read(value.c, 4);
  todayOnTime = value.l;
  Array.seek(4 * (nDays + Today - 1));
  Array.read(value.c, 4);
  todayTimeEWMA = value.l;
  Array.seek(4 * (nDays * 2 + Today - 1));
  Array.read(value.c, 4);
  todayStdDevTime = value.l;
  Array.seek(4 * (nDays * 3 + Today - 1));
  Array.read(value.c, 4);
  todayWater = value.d;
  Array.seek(4 * (nDays * 4 + Today - 1));
  Array.read(value.c, 4);
  todayUses = value.i;
  Array.close();
  return true;
}

/*enum MetricType : byte {
  OnTime, // = 0
  EWMATime, // = 1
  StdDevTime, // = 2
  TotalWater, // = 3
  Uses // = 4
};*/

long GetPastOnTime(byte day) {
  return ReadValueFromArray(day, OnTime).l;
}

long GetPastEWMATime(byte day) {
  return ReadValueFromArray(day, EWMATime).l;
}

long GetPastStdDevTime(byte day) {
  return ReadValueFromArray(day, StdDevTime).l;
}

double GetPastTotalWater(byte day) {
  return ReadValueFromArray(day, TotalWater).d;
}

int GetPastUses(byte day) {
  return ReadValueFromArray(day, Uses).i;
}

ToByte ReadValueFromArray(byte day, MetricType metric) {
  ToByte value;
  File Array = SD.open(newArraysFile, FILE_WRITE);
  if (!Array) {
    return value;
  }
  Array.seek(4 * (nDays * (byte)metric + Today - 1));
  Array.read(value.c, 4);
  Array.close();
  return value;
}

