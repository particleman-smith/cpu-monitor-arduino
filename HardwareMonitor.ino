#include <math.h>
#include <Adafruit_NeoPixel.h>

#define CPU_LED_PIN    2
#define CPU_LED_COUNT  12

#define READ_CPU_VAL_CHAR   'C'
#define READ_CPU_TEMP_CHAR  'c'
#define READ_GPU_VAL_CHAR   'G'
#define READ_GPU_TEMP_CHAR  'g'
#define READ_TERM_CHAR      '$'

#define BUFFER_SIZE  4

enum read_state {
  NONE,
  READ_CPU_VAL,
  READ_CPU_TEMP,
  READ_GPU_VAL,
  READ_GPU_TEMP
};

Adafruit_NeoPixel cpuLeds = Adafruit_NeoPixel(CPU_LED_COUNT, CPU_LED_PIN, NEO_GRB + NEO_KHZ800);

uint32_t cpuValue = 0;
uint32_t cpuTemp = 0;
uint32_t gpuValue = 0;
uint32_t gpuTemp = 0;

uint32_t colorBlack = cpuLeds.Color(0, 0, 0);
uint32_t colorBlue = cpuLeds.Color(0, 0, 255);
uint32_t colorRed = cpuLeds.Color(255, 0, 0);


char currentRead; // Current character being read from serial
char receivedBuffer[BUFFER_SIZE]; // Temporary concatenated string of characters
uint8_t currentBufferPos = 0;
read_state currentReadState; // Determines which variable the buffer will save the data

void setup() {
  Serial.begin(9600);
  clearBuffer();
  cpuLeds.begin();
  setAllLeds(colorBlack);
  cpuLeds.show();
}

void loop() {

  while (Serial.available() > 0) {
    char received = Serial.read();
    
    // Handle signal characters
    if (received == READ_CPU_VAL_CHAR) { // Begin CPU %
      saveDataFromBuffer();
      currentReadState = READ_CPU_VAL;
    }
    else if (received == READ_CPU_TEMP_CHAR) { // Begin CPU Temp
      saveDataFromBuffer();
      currentReadState = READ_CPU_TEMP;
    }
    else if (received == READ_GPU_VAL_CHAR) { // Begin GPU %
      saveDataFromBuffer();
      currentReadState = READ_GPU_VAL;
    }
    else if (received == READ_GPU_TEMP_CHAR) { // Begin GPU Temp
      saveDataFromBuffer();
      currentReadState = READ_GPU_TEMP;
    }
    else if (received == READ_TERM_CHAR) { // End of seq
      saveDataFromBuffer();
      currentReadState = NONE;
      break; // Stop reading from serial
    }
    else { // Any other character: save to buffer
      receivedBuffer[currentBufferPos] = received;
      currentBufferPos++;
    }
  }

  displayCpuUsage(cpuValue);
}

void saveDataFromBuffer() {
  String receivedStr = String(receivedBuffer);
  
  if (currentReadState == READ_CPU_VAL) {
    Serial.print("CPU %: ");
    cpuValue = receivedStr.toInt();
  }
  else if (currentReadState == READ_CPU_TEMP) {
    Serial.print("CPU Temp: ");
    cpuTemp = receivedStr.toInt();
  }
  else if (currentReadState == READ_GPU_VAL) {
    Serial.print("GPU %: ");
    gpuValue = receivedStr.toInt();
  }
  else if (currentReadState == READ_GPU_TEMP) {
    Serial.print("GPU Temp: ");
    gpuTemp = receivedStr.toInt();
  }

  Serial.println(receivedStr);
  
  clearBuffer();
}

void clearBuffer() {
  for (uint8_t i = 0; i < BUFFER_SIZE; i++) {
    receivedBuffer[i] = '\0';
  }
  currentBufferPos = 0;
}

void displayCpuUsage(uint32_t cpu) {
  float ledMultiplier = cpu * 0.01;
  uint32_t maxLed = (uint32_t)(cpuLeds.numPixels() * ledMultiplier - 0.5f);
  for (uint16_t i = 0; i < cpuLeds.numPixels(); i++) {
    if (i <= maxLed) {
      cpuLeds.setPixelColor(i, cpuLeds.Color(30, 30, 30));
    }
    else {
      cpuLeds.setPixelColor(i, colorBlack);
    }
  }
  cpuLeds.show();
}

void setAllLeds(uint32_t color) {
  for(uint16_t i = 0; i < cpuLeds.numPixels(); i++) {
      cpuLeds.setPixelColor(i, color);
  }
  cpuLeds.show();
}
