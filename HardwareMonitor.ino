#include <math.h>
#include <Adafruit_NeoPixel.h>

#define CPU_LED_PIN    2
#define CPU_LED_COUNT  12

// Determines the rotation of the neopixel ring (the starting led)
#define CPU_LEDS_OFFSET   0

// Minimum and maximum CPU temperature range used for determining LED color
#define CPU_TEMP_MIN    40
#define CPU_TEMP_MAX    90

// Flags to determine what information is being sent by the driver software via serial
#define READ_CPU_VAL_CHAR   'C'
#define READ_CPU_TEMP_CHAR  'c'
#define READ_GPU_VAL_CHAR   'G'
#define READ_GPU_TEMP_CHAR  'g'
#define READ_TERM_CHAR      '$'

#define BUFFER_SIZE  4 // Buffer used for receiving strings via serial

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
uint32_t colorBlue = cpuLeds.Color(0, 0, 100);
uint32_t colorRed = cpuLeds.Color(100, 0, 0);

char currentRead; // Current character being read from serial
char receivedBuffer[BUFFER_SIZE]; // Temporary concatenated string of characters received via serial
uint8_t currentBufferPos = 0; // Tracks the index of the receivedBuffer
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

  displayCpuUsage();
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

void displayCpuUsage() {
  // Calculate number of LEDs to turn on
  float ledMultiplier = cpuValue * 0.01;
  uint32_t maxLed = (uint32_t)(cpuLeds.numPixels() * ledMultiplier - 0.5f);

  // Calculate the LED color
  // (linear interpolation [lerp] of the min color to the max color based on the current cpu
  // temperature between the minimum and maximum temperature range)
  
  // t is the percentage of blue to red used for lerp
  float t = 0; // Default 0: Cpu temp is at or below the minimum (100% blue, 0% red)
  if (cpuTemp >= CPU_TEMP_MAX) { // Cpu temp is at or above the maximum
    t = 1; // 0% blue, 100% red
  }
  else if (cpuTemp > CPU_TEMP_MIN) { // Cpu temp is within the minimum and maximum range
    // Calculate t for the temperature between the minimum and the maximum
    t = (float)(cpuTemp - CPU_TEMP_MIN) / (float)(CPU_TEMP_MAX - CPU_TEMP_MIN);
  }
  
  for (uint16_t i = 0; i < cpuLeds.numPixels(); i++) {
    if (i <= maxLed) {
      cpuLeds.setPixelColor(i, lerpColor(colorBlue, colorRed, t));
    }
    else {
      cpuLeds.setPixelColor(i, colorBlack);
    }
  }
  cpuLeds.show();
}

uint32_t lerpColor(uint32_t colorA, uint32_t colorB, float t) {
  uint8_t aRed, aGreen, aBlue;
  uint8_t bRed, bGreen, bBlue;
  uint8_t cRed, cGreen, cBlue;
  
  aRed = red(colorA);
  aGreen = green(colorA);
  aBlue = blue(colorA);

  bRed = red(colorB);
  bGreen = green(colorB);
  bBlue = blue(colorB);

  cRed = (t * bRed + 0.5) + ((1 - t) * aRed + 0.5);
  cGreen = (t * bGreen + 0.5) + ((1 - t) * aGreen + 0.5);
  cBlue = (t * bBlue + 0.5) + ((1 - t) * aBlue + 0.5);
  
  return cpuLeds.Color(cRed, cGreen, cBlue);
}

// Returns the Red component of a 32-bit color
uint8_t red(uint32_t color) {
    return (color >> 16) & 0xFF;
}

// Returns the Green component of a 32-bit color
uint8_t green(uint32_t color) {
    return (color >> 8) & 0xFF;
}
 
// Returns the Blue component of a 32-bit color
uint8_t blue(uint32_t color) {
    return color & 0xFF;
}

void setAllLeds(uint32_t color) {
  for(uint16_t i = 0; i < cpuLeds.numPixels(); i++) {
      cpuLeds.setPixelColor(i, color);
  }
  cpuLeds.show();
}
