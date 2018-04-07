const int pirPin = 34;
const int lcdPin = 29;
void setup() {
  Serial.begin(9600);
  pinMode(pirPin, INPUT);
  digitalWrite(pirPin, LOW); // Turn it on
  Serial.println("Calibrating...");
  pinMode(lcdPin, OUTPUT);
  delay(1000);
}

void loop() {
  /*
   * PRESS CTRL+SHIFT+M TO SEE THE SERIAL MONITOR
   */
   digitalWrite(lcdPin, digitalRead(pirPin));
   Serial.println(digitalRead(pirPin));
}

