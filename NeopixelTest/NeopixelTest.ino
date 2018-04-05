#include <WS2812FX.h>

#define LED_COUNT 76
#define LED_PIN 36

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
int s = 700; //speed, realized as delay time in ms

const int nSegs = 10;
int segLengths[nSegs] = {7, 7, 9, 8, 7, 7, 8, 9, 7, 7};

void setup() {
  Serial.begin(9600);
  ws2812fx.init();
  ws2812fx.setBrightness(10); //brightness can be up to 255, but low values work better
  ws2812fx.setColor(00, 128, 255); 
  int curIdx = 0;
  bool a = true;
  bool b = false;
  for (int segIdx = 0; segIdx < nSegs; segIdx++) {
    ws2812fx.setSegment(segIdx, curIdx, curIdx + segLengths[segIdx] - 1, FX_MODE_COMET,  0x0080FF, s + random(80) - 40, a);
    //arguments: (index of segment, first pixel in segment, last pixel in segment, effect, color, speed (delay time), is reversed?)
    //FX_MODE_COMET is the effect that looked like the rain we needed, but we can change it slightly if we need to without needing to start from scratch
    curIdx += segLengths[segIdx];
    
    /*
     * do this stuff so that every other pair of segments 
     * is reversed, so that the strands dont need to all 
     * be going the same way (the wires ended up being too short to all go the same way)
     */
    a = a ^ b; 
    b = !b;
  }
  ws2812fx.start();
}

void loop() {
  ws2812fx.service(); //call service on every iteration, timing is taken care of internally
}

