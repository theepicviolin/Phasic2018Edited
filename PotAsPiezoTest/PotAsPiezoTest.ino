#define MIN_VAL 0xFFFF
#define MAX_VAL 0x7FFF
const byte nReadings = 20;
int readings[nReadings];
float dev[nReadings];
byte readIndex = 0;              // the index of the current reading
long total = 0;                  // the running total
double average = 0;              // the average
double stdD = 0;
int count = 0;
const int ptpTime = 15;
int mxm = MIN_VAL; //initialize mxm to minimum int value so that any measured value will be greater
int mnm = MAX_VAL; //initialize mnm to maximum int value so that any measured value will be lesser
double ptp = 0;
double EWMA = 0;
double stdDEWMA =  0;
int inputPin = A0;

const double lam = 0.5;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int value = analogRead(inputPin);
  int reading = 512 + (random(value) - (value/2))/3;

  total -= readings[readIndex]; // subtract the last reading:
  readings[readIndex] = reading; // read from the sensor:
  
  total += readings[readIndex]; // add the reading to the total:
  
  average = (double)total / nReadings; // calculate the average
  
  dev[readIndex] = readings[readIndex]; //add reading to running stdDev
  stdD = standard_deviation(dev, nReadings); //calc stdev

  readIndex = (readIndex + 1) % nReadings; 
  
  //Serial.println(average);
  //return;
  if (count <= ptpTime) {
    //check if average is max or min, cut super high
    if (average >= mxm) {
      mxm = average;
    }
    if (average <= mnm) {
      mnm = average;
    }
    count++; //add to count for PTP window
    return;
  }

  ptp = mxm - mnm; //set ptp
  EWMA = lam * ptp + (1 - lam) * EWMA ;//EWMA Calculation
  stdDEWMA = lam * stdD + (1 - lam) * stdDEWMA;
  //if (!digitalRead(5)) {
    Serial.println("{\"EWMA\": " + String(stdDEWMA) + ", \"time\": " + String(millis()) + "},");
  //}
  mxm = MIN_VAL;
  mnm = MAX_VAL;
  count = 0;
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

