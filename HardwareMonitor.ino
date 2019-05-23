#include <math.h>
#include <Adafruit_NeoPixel.h>

#define CPU_LED_PIN    2
#define CPU_LED_COUNT  12

#define READ_CPU_VAL   'C'
#define READ_CPU_TEMP  'c'
#define READ_GPU_VAL   'G'
#define READ_GPU_TEMP  'g'
#define READ_TERM      '$'

Adafruit_NeoPixel cpuLeds = Adafruit_NeoPixel(CPU_LED_COUNT, CPU_LED_PIN, NEO_GRB + NEO_KHZ800);

// Used for receiving serial data
char currentRead;
String receivedData;
uint32_t cpuValue = 0;
uint32_t cpuTemp = 0;

void setup() {
  Serial.begin(9600);
  cpuValue = '0';
  cpuLeds.begin();
  cpuLeds.show();
}

void loop() {
  while (Serial.available() > 0) {
    char received = Serial.read();

    /*if (received == READ_CPU_VAL) {
      currentRead = READ_CPU_VAL;
      receivedData = "";
    }
    else if (received == READ_CPU_TEMP) {
      currentRead == READ_CPU_TEMP;
      receivedData = "";
    }
    else if (received == READ_TERM) {
      receivedData = "";
    }
    else {
      switch (currentRead) {
        case READ_CPU_VAL:
          cpuValue += received;
          break;
        case READ_CPU_TEMP:
          cpuTemp += received;
          break;
      }
    }*/
    
    if (received == READ_TERM)
    {
      cpuValue = receivedData.toInt();
      Serial.println(receivedData);
      receivedData = "";
    }
    else {
      receivedData += received;
    }
  }

  /*if (cpuValue == 0) {
    setAllLeds(cpuLeds.Color(0, 0, 0));
  }
  else if (cpuValue == 1) {
    setAllLeds(cpuLeds.Color(0, 255, 0));
  }
  else if (cpuValue == 25) {
    setAllLeds(cpuLeds.Color(255, 255, 255));
  }*/

  displayCpuUsage(cpuValue);
}

void displayCpuUsage(uint32_t cpu) {
  float ledMultiplier = cpu * 0.01;
  uint32_t maxLed = (uint32_t)(cpuLeds.numPixels() * ledMultiplier - 0.5f);
  for (uint16_t i = 0; i < cpuLeds.numPixels(); i++) {
    if (i <= maxLed) {
      cpuLeds.setPixelColor(i, cpuLeds.Color(30, 30, 30));
    }
    else {
      cpuLeds.setPixelColor(i, cpuLeds.Color(0, 0, 0));
    }
  }
  cpuLeds.show();
}

void setAllLeds(uint32_t color) {
  for(uint16_t i=0; i<cpuLeds.numPixels(); i++) {
      cpuLeds.setPixelColor(i, color);
  }
  cpuLeds.show();
}
