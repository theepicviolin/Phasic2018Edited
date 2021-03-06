/*
This code was modified on October 6, 2016 by Nicholas Benzoni under the guidance of Dr. Cassandra Telenko for. The code is supposed to read from a 6.3kHz piezo
electric transducer on an ARDUINO MEGA. The transducer should be attached to the actual metal part of the faucet of a sink. The code then estimates and records
the flowrate used at the faucet, and does many other things, like give feedback to the users in various states. The code is intended to be run for at least 50 days,
and up to 80. The phases used are generally outlined in He et al.'s paper "One Size Does Not Fit All", and adapted to water usage at the primary kitchen sink in a household.

There are three main states in this code with respect to what the faucet is doing;
1. Idle: the faucet is off. The device will be displaying Tips, changing regularly (or simply the word Baseline and the day it is)
2. Active: The faucet is currently running water. The code has 6 different actions for this phase;
 i. More advanced baseline, also displaying the hour, mostly to tell if the code is recognizing on/off
 ii. Simply sayign Hello, so the user knows the device recognizes on/off in the Pre-Contemplation Phase
 iii. A declaration of what the given flow is good for be it high or low
 iv. A bar showing the relative flowrate of the sink from Off-Max
 v. Big text from OFF-HIGH, or same as iv
 vi. Same as ii.
3. Just Off: the faucet was just turned off a second or so ago, this state will do vartious things, as the active state, but most importantly shoes the esimate for how much
water was just used

I believe the code can be improved vastly and made more simple in many weays. It was my first Arduino project, so please excuse any bulkiness or inconvenience. Any questions
may be directed to my email nickbenzoni@gmail.com, though I will do my best to comment the code sufficiently enough to make such contact unnecessary.

Other hardware notes: Using a LM6132 op-amp, with a gain of about 20, achieved with a 560Ohm resistor, and a 10kOhm resistor. There is a 1MOhm resistor between the poisitive
and negative leads of the piezo transducer, and a 1kOhm resistor between the negative lead and ground. There is also a capacitor of 1-10microF between power and ground,
it is also VERY IMPORTANT to have an actual wire attached to the ground and earth ground, otherwise the signal will be very noisy when attached to only the wall-power adapter.
There is also a micro-sd card module ont he board attached to pins 50-54, and an LCD screen installed as described on adafruits website. The LCD is the 20x4 adafruit standard with
the white font on a blue background. Previous versions of this code are made for use with the adafruit screen with RGB backglight.

The PCB file for the shield should be in some provided documentation, or Mr. Ahn Nguyen has it in a fodler titled Benzoni.
There should also be included the lasercut schematic for the packaging, which is to be done on a 1/8" 12"x24" piece of acrylic and makes 2.5 housings.
*/

#define MIN_VALUE 0xFFFF
#define MAX_VALUE 0x7FFF

// include the library code:
#include <LiquidCrystal.h>
#include <SD.h>
#include <SPI.h>

/********
 *  LED SETUP
 */
 #include <WS2812FX.h>
const int nPixels = 76; // Number of LEDs in strip
const int ledPin = 43;
WS2812FX pixels = WS2812FX(nPixels, ledPin, NEO_GRB + NEO_KHZ800);
//Adafruit_NeoPixel pixels = Adafruit_NeoPixel(nPixels, ledPin, NEO_GRB + NEO_KHZ800);

const int nSegs = 10;
int segLengths[nSegs] = {7, 7, 8, 9, 7, 7, 9, 8, 7, 7};

const uint32_t color = 0x0080FF;
const int maxPixelSpeed = 400;

const int fadeDelay = 100;
const int fadeFactor = 4; //constant to multiply number associated with flow type by. 
int endBright = 0; //The end level of brightness its transitioning too when going to a new state, eventually howbright is set equal to this
long lastPixelUpdate = 0;

/*************
 * PIR SETUP
 */
int pirCalibrationTime = 1; //sec 
//the time when the sensor outputs a low impulse
long unsigned int pirLowIn;
//the amount of milliseconds the sensor has to be low
//before we assume all motion has stopped
long unsigned int pirPause = 5000;
boolean pirLockLow = true;
boolean pirTakeLowTime;
int pirPin = 34; //the digital pin connected to the PIR sensor's output

/************
 * EVERYTHING ELSE SETUP
 */

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(32, 30, 37, 35, 33, 31);
const int lcdPin = 29;

const int inputPin = A0;              // pin on board that the amplification circuit is hooked up to

const bool useSerial = true;

const byte ptpTime = 150;        //how many times the loop should run before actually outputting or changing anything, approximately proportionate to 1.6ms each
byte count = 0;                  // timer for output for the ptp output above
const byte idleSwitch = 10;      //10 = 1.6 seconds of showing per tip 1900 for 5 min
const byte activeSwitch = 2;     //to make active screen not fresh too fast
const double lam = 0.5;               //Lambda of EWMA calculations, larger = faster and less smooth, typically between 0.5-0.1

//All below values need to be set with EACH calibration

const double FlowEWMA[5] = {2, 20, 40, 60, 90}; // thresholds (EWMA or stdDEWMA) for {vlow, low, med, high, vhigh}                                 CALIBRATE[ ]
double EWMA = 0; //initialize to 'off' value                                                                                                             CAILBRATE[ ]
const double MaxFlow = 0.15; //should be in L/ms, set to MAX FLOW from sink                                                                                      CAILBRATE[ ]
double stdDEWMA = 11; //off value                                                                                                                          CAILBRATE[ ]
const bool useEWMA = false; //true for ptpEWMA , false if standard deviation (usually use standard deviation)                                                     CAILBRATE[ ]

const double thresh[2] = {30, 120}; //set to split code into one, two or three polynomials. 0-thresh[0], thresh[0]-thresh[1], thresh[1]-inf                     CAILBRATE[ ]
//set thresh[0] very high if you want to only use a single polynomial to fit the data
//the polynomial values are retrieved from you Excel file, the y axis is flow in L/s, the x axis is either EWMA (useEWMA = true)
//or stdD (standard deviation) (useEWMA = false)

//Polynomial 1 ceAx^2+ceBx+ceC=flow 0-thresh[0]                                                                                                            CAILBRATE[ ]
const double ceA = 0;
const double ceB = 0;
const double ceC = 0;

//Polynomial 2 ceA1X^2+ceB1x+ceC1 = flow //for chunking at given threshold thresh[0]-thresh[1]                                                              CAILBRATE[ ]
const double ceA1 = 0;
const double ceB1 = 0;
const double ceC1 = 0;

//Polynomial ceA1X^2+ceB1x+ceC1 = flow //for chunking at given threshold >thresh[1]                                                                          CAILBRATE[ ]
const double ceA2 = 0;
const double ceB2 = 0;
const double ceC2 = 0;

const byte stdDevReads = 20;     //Number of readings for the standard deviation fo the signal
float stdD = 0;         //all std dev only
float dev[stdDevReads]; //array fed into the standard deviation function
byte stdIndex = 0;       //used to control the above array

const String daysFile;
const String hourFile;
const String arraysFile;
const String newArraysFile;

const byte numReadings = 20;     //number of readings for the running  used in ptp calculations
bool ok = true;                     // just for trimming of beignning data during calibration, as the serial buffer will have data in it to be ignored
int readings[numReadings];      // the readings from the analog input
byte readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;              // the average
byte activeCounter = 0;          //timer for active screen
byte idleCounter = 0;            //timer for idle screen

//these valeus are used to generate a peak to peak signal of the raw sensor reading
//mxm = maximum, mnm = minimum, ptp = peak to peak of a given number of averages of readings
//this whole system could probably be improved
int mxm = MIN_VALUE;
int mnm = MAX_VALUE;
double ptp = 0;

//variables used to determine the type of flow, from Off, Very Low, Low, Med, High, Very High
enum FlowType : byte
{
	Off,        // = 0
	VeryLow,    // = 1
	Low,        // = 2
	Medium,     // = 3
	High,       // = 4
	VeryHigh    // = 5
};
FlowType lastFlow = Off;
FlowType flowNow = Off;
FlowType flow = Off;

enum ActionType : byte
{
  BaselineAction,
  Hello,
  WasteNot,
  HighFlowGoodFor,
  FlowBar,
  BigText
};

byte flowCount = 0;
byte JustOffAction = 0;

//PHASES
enum PhaseType : byte
{
  BaselinePhase,
  Precontemplation,
  Contemplation,
  Preparation,
  Action,
  Maintenance
};
PhaseType PhaseNow = BaselinePhase;


const byte PhaseDay[5] = {14, 21, 28, 35, 49}; //BL, PC, C, PR, A, M 50 days total
ActionType whichAction = BaselineAction; //0: Big text 1: Waste not want not 2: Raining
byte whichTip = 0;    //which tip from #1-#52 to display
long actionTip = 0;  //timer to change the tip every set number of hours in ms

//These are boolean values to determine if the sink is off or JUST turned off
bool justOff = false;
bool On = false;
bool tipSwitch = false;

//the "this" values are for saving the current use, which are saved in
// the UseSums file, after each off-on-off recognition, so there is a single
// line int he file for each usage
unsigned long startUse = 0;
unsigned long endUse = 0;
unsigned long thisElapsed = 0;
double thisEWMA = 0;
double thisStd = 0;
double thisEWMAAvg = 0;
double thisStdAvg = 0;
double thisAvgFlow = 0;
unsigned long thisCounter = 0;
float thisWater = 0; //water used on a single off-on-off instance

//stuff for daily/weekly breakdown
byte Today = 0;
byte thisHour = 0;
double CurrentFlow = 0; //flow calculated live while sink is on (for bar display)
long checkTime = 0; //just for knwoing when 3 minutes has passed for the program to check the hour and day

unsigned long HourCheck = 0; //for timing in the whatHour() and whatDay() functions
unsigned long DayCheck = 0;

//used to print the data on the SD card in a machine readable format
union ToByte {
	unsigned long l;
	int i;
  double d;
  char c[4];
};

//enum for each of the below variables used for daily statistics
enum MetricType : byte {
  OnTime, // = 0
  EWMATime, // = 1
  StdDevTime, // = 2
  TotalWater, // = 3
  Uses // = 4
};

// All the below variables are used to determine the daily statistics
unsigned long todayOnTime; //total time a sink is on in a given day
unsigned long todayTimeEWMA; //EWMA * total time, so one can determine the average EWMA from a day
unsigned long todayStdDevTime; //stdD * total time, so one can determine the average stdD from a day
double todayWater; //daily estimated water by the polynomial
int todayUses; //total number of off-on-off instances in a given day

const byte nDays = 80;

void setup() {
  // initialize serial communication with computer:
  if (useSerial) Serial.begin(9600);
  pinMode(lcdPin, OUTPUT);
  digitalWrite(lcdPin, HIGH);
  
  pinMode(pirPin, INPUT);
  digitalWrite(pirPin, LOW); // Turn it on
  if (useSerial) Serial.print("calibrating sensor ");
  for(int i = 0; i < pirCalibrationTime; i++){
    if (useSerial) Serial.print(".");
    delay(1000);
  }
  if (useSerial) Serial.println(" done");
  if (useSerial) Serial.println("SENSOR ACTIVE");
  delay(50);
  
   setupNeoPixels();
  
  // set up the LCD's number of columns and rows:
  lcd.begin(20, 4);

  //first display which program is being used
  lcd.print(F("Phasic 17 Cleaned Up"));
  
  daysFile = F("Days.txt");
  hourFile = F("HOUR.txt");
  arraysFile = F("Arrays.txt");
  newArraysFile = F("Data.txt");

  setupCustomChars();
  
  //initialize SD
  // value of 53 is specific to ARDUINO MEGA, will be different for other Arduino boards
  //if (!SD.begin(53)) {
  if (!SD.begin()) { //no input results in default pin set based on the board selected in build options, MEGA -> 53
    if (useSerial) Serial.println(("SD Failed"));
    lcd.clear();
    lcd.print(F("SD Failed. Try again."));
    delay(30000);
    return;
  }
  
  lcd.clear();
  lcd.print(F("SD Initialized"));
  
  
  delay(100);

  //Open file, make sure it worked
	File myFile = SD.open(F("AllOn.txt"), FILE_WRITE);
  if (myFile) {
    lcd.clear();
    lcd.write(F("File opened"));
    myFile.println(F("File opened"));
    myFile.close();
    delay(100);
  }
  else {
    lcd.clear();
    lcd.write(F("File failed to open"));
    delay(30000);
  }

  //check the last saved day and hour and set values as so
	Today = whatDay();
  thisHour = whatHour();
  lcd.clear();
//Fills in values of variables based on arrays on sd card
  if (ReadFromArrays()) {
    lcd.write(F("Memory ready."));
    delay(100);
  } else {
    lcd.write(F("Arrays not intialized."));
    delay(3000);
  }
  setPhase();
	lcd.write(F("Ready!"));
  
  delay(500);
}


void loop() {
  updatePixelParams();
  pixels.service();
   digitalWrite(lcdPin, digitalRead(pirPin));
  // TAKE READING AND CALCULATE STANDARD DEVIATION AND RUNNING AVERAGE
  //int reading = analogRead(inputPin);
  int value = analogRead(inputPin); //get pot reading 
  int reading = 512 + (random(value) - (value/2))/3; //use pot reading to simulate piezo data
  Serial.println(stdDEWMA);
  total -= readings[readIndex]; // subtract the last reading:
  readings[readIndex] = reading; // read from the sensor:
  total += readings[readIndex]; // add the reading to the total:

  average = total / numReadings; // calculate the average

  dev[stdIndex] = readings[readIndex]; //add reading to running stdDev
  stdD = standard_deviation(dev, stdDevReads); //calc stdev
  readIndex = (readIndex + 1) % numReadings;
  stdIndex = (stdIndex + 1) % stdDevReads;
  
  if (millis() - checkTime > 180000) { //just to make sure the day and hour are checked every 3 mins
    checkTime = millis();
    thisHour = whatHour();
    byte curDay = whatDay();
    if (curDay != Today) {
      todayOnTime = 0;
      todayTimeEWMA = 0;
      todayStdDevTime = 0;
      todayWater = 0;
      todayUses = 0;
      Today = curDay;
    }
  }
  
  //PTP Calculations gather max and min
  if (count <= ptpTime) {
    //check if average is max or min, cut super high
    if (average >= mxm) {
      mxm = average; // high values for ptp
    }
    if (average <= mnm) { //low values for ptp
      mnm = average;
    }
    count++; //add to count for PTP window
    return;
  }
  
  // EWMA CALCULATED ------------------------------------------------------- EWMA CALCULATED -------------------------------------------------------
  //if (count > ptpTime) {
  ptp = mxm - mnm; //set ptp
  EWMA = lam * ptp + (1 - lam) * EWMA ;//EWMA Calculation
  stdDEWMA = lam * stdD + (1 - lam) * stdDEWMA;

  if (useSerial)
  {/*
    Serial.print(millis()); //Serial output for Processing/DAQ ----- // out for real one
    Serial.print("\t");
    Serial.print(EWMA);
    Serial.print("\t");
    Serial.println(stdD);*/
  }
  //Check if flow was just TURNED ON (from off)
  if (flowNow != Off && !On && lastFlow == Off) {
    //turn flow state ON
    On = true;
    startUse = millis();
    thisStd = 0;
    thisEWMA = 0;
    //Which just off action?
  }

  //Check if flow was just TURNED OFF (from any state) -------- JUST TURNED OFF STATE CHANGE
  //Save newline for this use
  if (On && flowNow == Off) {
    //Turn state OFF
    On = false;
    justOff = true;
    //calculate length of last usage
    endUse = millis();
    thisElapsed = endUse - startUse;
		saveUsageSummary();
    //Reset all values to 0
    thisEWMA = 0;
    thisStd = 0;
    thisCounter = 0;
    startUse = 0;
    endUse = 0;
  }

  //ACTIVE STATE ====================== ACTIVE STATE ==================== ACTIVE STATE ================== ACTIVE STATE ================ ACTIVE STATE ================
  if (On) {
    thisStd += stdD; //to determine the average STD and EWMA of this usage, for calculating total used
    thisEWMA += EWMA;
    thisCounter ++;
    activeCounter++;
    if (activeCounter > activeSwitch) {
      CurrentFlow = calculateCurrentFlow(useEWMA ? EWMA : stdD);
      //Serial.println(CurrentFlow);
      CurrentFlow = min(MaxFlow * 1000,  CurrentFlow);
      CurrentFlow = max(CurrentFlow, 0);
      displayActiveAction();
      activeCounter = 0; //reset active timer, to make sure the screen does not refresh too fast.
    }
  }
  //IDLE STATE ********************* IDLE STATE ********************* IDLE STATE ********************* IDLE STATE ********************* IDLE STATE *********************
  else if (digitalRead(pirPin)) {
    idleCounter++;              //next active, random is (min, max-1)
    //JUST OFF ACTION STATE +++ +++ +++ +++ +++ JUST OFF ACTION STATE +++ +++ +++ +++ +++  JUST OFF ACTION STATE +++ +++ +++ +++ +++  JUST OFF ACTION STATE +++ +++ +++ +++ +++
    if (justOff) {
      lcd.clear();
      switch (JustOffAction) {
        case 0:
          lcd.print("Baseline Day ");
          lcd.print(Today);
          break;
        case 1:
          lcd.print("Goodbye!");
          break;
        case 2:
          lcd.write("You just used about ");
          lcd.setCursor(0, 1);
          lcd.print(thisWater);
          lcd.write( " liters.");
          break;
        case 3:
          lcd.write("Today's usage so far");
          lcd.setCursor(0, 1);
          lcd.write("about ");
          lcd.print(todayWater);
          lcd.write(" L.");
          break;
      }
      if (idleCounter > 93) { //ten seconds
        //No longer "just turned off"
        justOff = 0;
        thisElapsed = 0;
        thisEWMAAvg = 0;
        thisStdAvg = 0;
        idleCounter = 0;
      }
    } else { //if (!justOff)
      if (idleCounter > idleSwitch) { // :::::::::::::::: IDLE TIPS ::::::::::::::::::::: IDLE IDLE IDLE :::::::::::::::::::::
        char thisTip [8] = "###.TXT";
        //int whichTip = random(1, 12); //MAX+1 and Min should be range of tips for given state
        //convert random int into char array for being called filename for each tip
        if (whichTip > 0) {//convert tip number to char array and call on sd card
          thisTip[0] = whichTip / 100 + '0';
          thisTip[1] = (whichTip % 100 / 10) + '0';
          thisTip[2] = whichTip % 100 % 10 + '0';
          displayText77(thisTip);
        } else if (whichTip == 0) { //baseline
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Baseline Day ");
          lcd.print(Today);
        } else if (whichTip == -1) { //display daily use
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.write("Today about ");
          lcd.print(todayWater);
          lcd.setCursor(0, 1);
          lcd.write("have been used at");
          lcd.setCursor(0, 2);
          lcd.write("this sink.");
        }
        idleCounter = 0;
        setPhase();
      } //idle tip end
    }//closed here
  } else { //no flow and no person
    digitalWrite(lcdPin, LOW);
  }
  // classify flow into 6 categories
  flow = classifyFlow(useEWMA ? EWMA : stdDEWMA);
  //the flow has changed to a new kind, and the time spent at
  if (flow != flowNow) {
    flowCount ++;
    //make sure flow has changed for 3 periods before switching
    if (flowCount >= 3) {
      lastFlow = flowNow;
      flowNow = flow; //the current flow is registered
      flowCount = 0;
    }
  }

  mxm = MIN_VALUE; //reset PTP
  mnm = MAX_VALUE;
  count = 0; //reset count
//} //end of the output step (count)
} // ------------------------------------------------------------------------- END OF VOID LOOP -------------------------------------------------------------------

void displayActiveAction() {
  lcd.clear();
  switch (whichAction) {
    case BaselineAction:
      lcd.print("Baseline Phase");
      lcd.setCursor(0, 1);
      lcd.print("Day ");
      lcd.print(Today);
      lcd.setCursor(0, 2);
      lcd.print("Hour ");
      lcd.print(thisHour);
      break;
    case Hello:
      lcd.print("Hello!");
      lcd.setCursor(0, 1);
      lcd.print("Day ");
      lcd.print(Today);
      lcd.setCursor(0, 2);
      lcd.print("Hour ");
      lcd.print(thisHour);
      break;
    case WasteNot:
      lcd.print("Is the water running");
      lcd.setCursor(0, 1);
      lcd.print("in the background?");
      break;
    case HighFlowGoodFor:
      if (flow >= Medium) { // || flow == High || flow == VeryHigh) {
        lcd.print("High flow is best");
        lcd.setCursor(0, 1);
        lcd.print("for filling pots,");
        lcd.setCursor(0, 2);
        lcd.print("cups, and ");
        lcd.setCursor(0, 3);
        lcd.print("waterbottles");
      } else {
        lcd.print("Low flow is good");
        lcd.setCursor(0, 1);
        lcd.print("for most uses.");
      }
      break;
    case FlowBar:
      lcd.print("Current Flow:");
      lcd.setCursor(0, 2);
      lcd.print("OFF");
      //lcd.setCursor(3, 1);
      for (byte i = 3; i <= 16 * (CurrentFlow / (1000 * MaxFlow)); i++) {
        lcd.setCursor(i, 1);
        lcd.write((byte)0);
        lcd.setCursor(i, 2);
        lcd.write((byte)0);
        lcd.setCursor(i, 3);
        lcd.write((byte)0);
      }
      lcd.setCursor(17, 2);
      lcd.print("MAX");
      break;
    case BigText:
      // Populate LCD Screen with Live Usage Information
      lcd.write("Raw Signal: ");
      lcd.print(EWMA);
      lcd.setCursor(0, 1);
      //Just to print the big text
      displayBigtext(flow);
      break;
  }
}

void setPhase() {
  //SET PHASE ____________________________ SET PHASE __________________________ SET PHASE ___________________ SET PHASE ___________________
  //in the set phase area I need to have each phase specify what range of tips to be used
  // which action and post action screens to be used.
  Today = 50;
  byte q = Today >= PhaseDay[2]; //a bunch of weird but fast math that calculates the current phase
  q += Today >= PhaseDay[1 + 2*q];
  PhaseNow = (PhaseType)(2*q + (Today >= PhaseDay[2*q]));
  PhaseNow = 2;
  switch (PhaseNow) {
    case BaselinePhase:
      whichTip = 0;
      JustOffAction = 0;
      whichAction = BaselineAction;
      break;
    case Precontemplation:
      if ((thisHour == 0 && !tipSwitch) || (thisHour == 14 && tipSwitch)) { //change tip at 12AM and at 2PM
        whichTip++;
        tipSwitch = !tipSwitch;
      }
      if (whichTip == 11) {
        whichTip = 1;
      }
      JustOffAction = 1;
      whichAction = Hello;
      break;
    case Contemplation:
      if ((thisHour == 0 && !tipSwitch) || (thisHour == 14 && tipSwitch)) { //change tip at 12AM and at 2PM
        whichTip++;
        tipSwitch = !tipSwitch;
      }
      if (whichTip == 18) {
        whichTip = 9;
      }
      JustOffAction = 1;
      whichAction = HighFlowGoodFor;
      break;
    case Preparation:
      if (millis() - actionTip > 14400000) { //change tip every 4 hours
        whichTip++;
        actionTip = millis();
      }
      if (whichTip == 31) {
        whichTip = 15;
      }
      JustOffAction = 2;
      whichAction = FlowBar;
      break;
    case Action:
      if (millis() - actionTip > 3600000) { //change tip each hour
        whichTip++;
        actionTip = millis();
      }
      if (whichTip == 53) {
        whichTip = 25;
      }
      JustOffAction = random(2, 4);
      whichAction = random(FlowBar, BigText + 1);
      break;
    case Maintenance:
      if (millis() - actionTip > 3600000) { //change tip each hour
        whichTip = random(1, 53);
        actionTip = millis();
      }
      JustOffAction = random(2, 4);
      whichAction = Hello;
      break;
  }
}

double calculateCurrentFlow(double rawData) {
  return (rawData/100.0) * (MaxFlow * 1000.0);
	double retVal;
	if (rawData <= thresh[0]) {
		retVal = ((ceA  * rawData + ceB ) * rawData + ceC);
	} else if (rawData <= thresh[1]) {
		retVal = ((ceA1 * rawData + ceB1) * rawData + ceC1);
	} else {
		retVal = ((ceA2 * rawData + ceB2) * rawData + ceC2);
	}
	return retVal;
}

//Classifies flow into off, vlow, low, med, high, vhigh dependign on the thresholds set
//in the FlowEWMA array
FlowType classifyFlow(double EWMA){
  byte q = EWMA >= FlowEWMA[2]; //a bunch of weird but fast math that calculates the flow classification
  q += EWMA >= FlowEWMA[1 + 2*q];
  return (FlowType)(2*q + (EWMA >= FlowEWMA[2*q]));
}

float standard_deviation(float data[], byte n) //Calculates the standard deviation of an array
{
  float mean = 0.0;
  float sum_deviation = 0.0;
  for (byte i = 0; i < n; ++i){
    mean += data[i];
  }
  mean /= n;
  for (byte i = 0; i < n; ++i){
    sum_deviation += (data[i] - mean) * (data[i] - mean);
  }
  return sqrt(sum_deviation / n);
}

