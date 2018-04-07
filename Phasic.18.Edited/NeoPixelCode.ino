void setupNeoPixels() {
  pixels.init();
  pixels.setBrightness(0);
  setPixelSpeed(maxPixelSpeed);
  pixels.start();
}

void setPixelSpeed(int s) {
  //Serial.println(s);
  int curIdx = 0;
  bool a = true;
  bool b = false;
  for (int segIdx = 0; segIdx < nSegs; segIdx++) {
    pixels.setSegment(segIdx, curIdx, curIdx + segLengths[segIdx] - 1, FX_MODE_COMET,  color, s + random(80) - 40, a);
    curIdx += segLengths[segIdx];
    a = a ^ b; 
    b = !b;
  }
}

void updatePixelParams() {
  if (millis() - lastPixelUpdate < fadeDelay) {
    return;
  }
  setPixelBrightness();
  Serial.println((CurrentFlow / (1000 * MaxFlow)));
  setPixelSpeed(maxPixelSpeed / (CurrentFlow / (1000 * MaxFlow)));
  int curBright = pixels.getBrightness();
  pixels.setBrightness(curBright + sign(endBright - curBright));
  lastPixelUpdate = millis();
}

void setPixelBrightness() {
  if (whichAction == Hello) {
    endBright = .5*fadeFactor;
  } else {
    //flow can be casted from the enum FlowType to an int
    endBright = ((int)flow) * fadeFactor;
  }
}

int sign(int num) {
  return (num > 0) - (num < 0);
}

