const int piezoPin = A0;
void setup() {
  Serial.begin(9600);
  pinMode(piezoPin, INPUT);
}

void loop() {
  /*
   * PRESS CTRL+SHIFT+M TO SEE THE SERIAL MONITOR
   */
   Serial.println(analogRead(piezoPin));
   /*
    * Default value should be around 512, higher flow rates will show high amplitude oscilations around that value
    */
}

