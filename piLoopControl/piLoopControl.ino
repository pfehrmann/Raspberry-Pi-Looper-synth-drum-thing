#include <Bounce.h>
#include "SPI.h"
#include <Adafruit_NeoPixel.h>

#define PIN            22
#define NUMPIXELS      24
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

//drumPads
#define NUM_BTN_COLUMNS (4)
#define NUM_BTN_ROWS (4)
#define MAX_DEBOUNCE (1)
static const uint8_t btncolumnpins[NUM_BTN_COLUMNS] = {38, 39, 40, 41};
static const uint8_t btnrowpins[NUM_BTN_ROWS]       = {42, 43, 44, 45};
static int8_t debounce_count[NUM_BTN_COLUMNS][NUM_BTN_ROWS];

//Encoder
int encBtnPin = 11;
Bounce encBtn = Bounce(encBtnPin,5);
int encoderPin1 = 24;
int encoderPin2 = 12;
volatile int lastEncoded = 0;
volatile long encoderValue = 0;
long lastencoderValue = 0;

int instNumber;
bool instSelectMode = false;
String instName[16] = {
  "Kit 1", "Kit 2","Kit 3","Kit 4",
  "Lead 1", "Pad 1", "Bass 1", "Keys 1",
  "Lead 2", "Pad 2", "Bass 2", "Keys 2",
  "Lead 3", "Pad 3", "Bass 3", "Keys 3",
};

//LP Buttons
int lpBtnPins[8] = {37, 30, 29, 28, 36, 33, 35, 34};
int lpBtnState[8];
int lpPrevBtnState[8] = {HIGH};
Bounce lpBounce[] = {
  Bounce(lpBtnPins[0],10),
  Bounce(lpBtnPins[1],10),
  Bounce(lpBtnPins[2],10),
  Bounce(lpBtnPins[3],10),
  Bounce(lpBtnPins[4],10),
  Bounce(lpBtnPins[5],10),
  Bounce(lpBtnPins[6],10),
  Bounce(lpBtnPins[7],10),
};
unsigned long clearTimstamp;

// Remove Change D11 to D1
String digitalStrings[8] = {
  "D11: ","D2: ","D3: ", "D4: ","D5: ","D6: ","D7: ","D8: "
};
String drumStrings[16] = {
  "Dr1: ","Dr2: ","Dr3: ", "Dr4: ","Dr5: ","Dr6: ","Dr7: ","Dr8: ",
  "Dr9: ","Dr10: ","Dr11: ", "Dr12: ","Dr13: ","Dr14: ","Dr15: ","Dr16: "
};
uint32_t neoColorArray[24] = {
  pixels.Color(0, 255, 255),
  pixels.Color(0, 255, 0),
  pixels.Color(255, 255, 0),
  pixels.Color(255,0,50),
  pixels.Color(0, 255, 255),
  pixels.Color(0, 255, 0),
  pixels.Color(255, 255, 0),
  pixels.Color(255,0,50),

  pixels.Color(0, 255, 255),
  pixels.Color(0, 255, 0),
  pixels.Color(255, 255, 0),
  pixels.Color(255,0,50),
  pixels.Color(0, 255, 255),
  pixels.Color(0, 255, 0),
  pixels.Color(255, 255, 0),
  pixels.Color(255,0,50),

  pixels.Color(0, 255, 255),
  pixels.Color(0, 255, 0),
  pixels.Color(255, 255, 0),
  pixels.Color(255,0,50),
  pixels.Color(0, 255, 255),
  pixels.Color(0, 255, 0),
  pixels.Color(255, 255, 0),
  pixels.Color(255,0,50),
};
uint32_t neoColorArrayDim[8] = {
  pixels.Color(0, 15.9375, 15.9375),
  pixels.Color(0, 15.9375, 0),
  pixels.Color(15.9375, 15.9375, 0),
  pixels.Color(15.9375,0,50),
  pixels.Color(0, 15.9375, 15.9375),
  pixels.Color(0, 15.9375, 0),
  pixels.Color(15.9375, 15.9375, 0),
  pixels.Color(15.9375,0,3.125),
};
uint32_t neoColorOff = pixels.Color(0,0,0);


//Mux control pins
int s0 = 3;
int s1 = 4;
int s2 = 5;
int s3 = 6;

//Mux in "SIG" pin
int SIG_pin = 14;

#define NUM_ANALOG (20)
int analogValues[NUM_ANALOG];
int analogValuesLag[NUM_ANALOG];
String analogStrings[NUM_ANALOG] = {
  "A22: ","A4: ","A3: ", "A2: ","A1: ","A17: ","A19: ","A18: ","A9: ","A10: ","A11: ",
  "A12: ","A13: ","A14: ","A15: ","A16: ","A5: ","A6: ","A7: ","A8: "
};

unsigned long readTime;
unsigned long prevReadTime;
unsigned long readTimeTwo;
unsigned long prevReadTimeTwo;

int readMux(int channel){
  int controlPin[] = {s0, s1, s2, s3};

  int muxChannel[16][4]={
    {0,0,0,0}, //channel 0
    {1,0,0,0}, //channel 1
    {0,1,0,0}, //channel 2
    {1,1,0,0}, //channel 3
    {0,0,1,0}, //channel 4
    {1,0,1,0}, //channel 5
    {0,1,1,0}, //channel 6
    {1,1,1,0}, //channel 7
    {0,0,0,1}, //channel 8
    {1,0,0,1}, //channel 9
    {0,1,0,1}, //channel 10
    {1,1,0,1}, //channel 11
    {0,0,1,1}, //channel 12
    {1,0,1,1}, //channel 13
    {0,1,1,1}, //channel 14
    {1,1,1,1}  //channel 15
  };

  //loop through the 4 sig
  for(int i = 0; i < 4; i ++){
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }

  int val;
  val = analogRead(SIG_pin);

  return val;
}

void setup() {

  //Btns
  pinMode(11, INPUT_PULLUP);
  //LPBTNS
  pinMode(28,INPUT_PULLUP);
  pinMode(29,INPUT_PULLUP);
  pinMode(30,INPUT_PULLUP);
  pinMode(33,INPUT_PULLUP);
  pinMode(34,INPUT_PULLUP);
  pinMode(35,INPUT_PULLUP);
  pinMode(36,INPUT_PULLUP);
  pinMode(37,INPUT_PULLUP);

  Serial.begin(9600);
  //Pixels
  pixels.begin();
  //Waiting for Serial Animation
  while (!Serial){
    rainbowCycle(2);
  }
  //Initialize stuff
  //ENCODER STUFF
  pinMode(encoderPin1, INPUT); 
  pinMode(encoderPin2, INPUT);
  digitalWrite(encoderPin1, HIGH); //turn pullup resistor on
  digitalWrite(encoderPin2, HIGH); //turn pullup resistor on
  //call updateEncoder() when any high/low changed seen
  //on interrupt 5 (pin 5), or interrupt 6 (pin 6) 
  attachInterrupt(1, updateEncoder, CHANGE); 
  attachInterrupt(2, updateEncoder, CHANGE);

  //MUX
  pinMode(s0, OUTPUT); 
  pinMode(s1, OUTPUT); 
  pinMode(s2, OUTPUT); 
  pinMode(s3, OUTPUT); 
  pinMode(SIG_pin, INPUT);

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);

  //digitalBtns
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(24, INPUT_PULLUP);
  pinMode(25, INPUT_PULLUP);
  pinMode(26, INPUT_PULLUP);
  pinMode(27, INPUT_PULLUP);

  //drumPads
  setuppins();
}

//Serial Read Vars
bool serialStarted = false;
bool serialEnded = false;
char inData[80];
int serIndex;


//Serial Write Vars
const int num_of_digital_pins = 1;
int digital_values[num_of_digital_pins];


///SCREEN VARS for recall
int loopPosVal;
String Recstatus;
int leftInputAmp;
int rightInputAmp;
int lpNumber;
int lpStatus;
int loopRec[8];
int loopMute[8];
bool ledBlinking[8] = {false};
int blinkBrightness;
bool lpCurrentlyRec[8] = {false};

int songSelIndex;
int menuIndex;
int menuSelPos[4] = {118,148,178,208};
uint32_t lcdColorArray[16] = {
  0x033f,0x07f0,0xffa4,0xf8ea,0x033f,0x07f0,0xffa4,0xf8ea,
  0x033f,0x07f0,0xffa4,0xf8ea,0x033f,0x07f0,0xffa4,0xf8ea
};

//InstPads
static void setuppins()
{
  uint8_t i;
  // button columns
  for (i = 0; i < NUM_BTN_COLUMNS; i++)
  {
    pinMode(btncolumnpins[i], OUTPUT);
    // with nothing selected by default
    digitalWrite(btncolumnpins[i], HIGH);
  }
  // button row input lines
  for (i = 0; i < NUM_BTN_ROWS; i++)
  {
    pinMode(btnrowpins[i], INPUT_PULLUP);
  }
  // Initialize the debounce counter array
  for (uint8_t i = 0; i < NUM_BTN_COLUMNS; i++)
  {
    for (uint8_t j = 0; j < NUM_BTN_ROWS; j++)
    {
      debounce_count[i][j] = 0;
    }
  }
}
//InstPads
static void scan()
{
  static uint8_t current = 0;
  uint8_t val;
  uint8_t i, j;

  // Select current columns
  digitalWrite(btncolumnpins[current], LOW);

  // Read the button inputs
  for ( j = 0; j < NUM_BTN_ROWS; j++)
  {
    val = digitalRead(btnrowpins[j]);

    if (val == LOW)
    {
      // active low: val is low when btn is pressed
      if ( debounce_count[current][j] < MAX_DEBOUNCE)
      {
        debounce_count[current][j]++;
        if ( debounce_count[current][j] == MAX_DEBOUNCE )
        {
          if(instSelectMode){
            Serial.print("Inst: ");
            Serial.print((current * NUM_BTN_ROWS) + j);
            Serial.print(" ");
            Serial.println(0);
            instNumber = (current * NUM_BTN_ROWS) + j;
          }else{
            Serial.print(drumStrings[(current * NUM_BTN_ROWS) + j]);
            Serial.print(1);
            Serial.print(" ");
            Serial.println("");
          }
          // int btnNum = (13 - (current * NUM_BTN_ROWS) + j);
          // pixels.setPixelColor(btnNum+9,pixels.Color(255,0,50));
        }
      }
    }
    else
    {
      // otherwise, button is released
      if ( debounce_count[current][j] > 0)
      {
        debounce_count[current][j]--;
        if ( debounce_count[current][j] == 0 )
        {
          Serial.print(drumStrings[(current * NUM_BTN_ROWS) + j]);
          Serial.print(0);
          Serial.print(" ");
          Serial.println("");
          // int btnNum = (13 - (current * NUM_BTN_ROWS) + j);
          // pixels.setPixelColor(btnNum+9,pixels.Color(0,0,0));
        }
      }
    }
  }// for j = 0 to 3;

  digitalWrite(btncolumnpins[current], HIGH);

  current++;
  if (current >= NUM_BTN_COLUMNS)
  {
    current = 0;
  }

}
// ====================================================================================================================================// 
// ! Start Loop                                                                  
// ====================================================================================================================================// 

void loop(void) { 
  while (!Serial){
    rainbowCycle(2);
  }
  pixels.show();
  //DrumBtns
  scan();
  //Loop Btns
  for(int i=0; i<8; i++){
    lpBtnState[i] = digitalRead(lpBtnPins[i]);
    if(lpBounce[i].update()){
      if (lpBtnState[i] == LOW && lpPrevBtnState[i] == HIGH){
        clearTimstamp = millis();
        Serial.print(digitalStrings[i]);
        Serial.print(1);
        Serial.print(" ");
        Serial.println(0);
      }
    }
    if (lpBtnState[i] == LOW && lpPrevBtnState[i] == LOW){
      if((millis() - clearTimstamp) > 1000 && (millis() - clearTimstamp) < 1200){
        Serial.print("clearSingle: ");
        Serial.print(i+1);
        Serial.print(" ");
        Serial.println("");
        clearTimstamp = millis();
      }
    }
    lpPrevBtnState[i] = lpBtnState[i];
  }
  readTimeTwo = millis();
  //FX and Loop Volume 
  for(int i = 0; i < NUM_ANALOG; i ++){
    if(i<16){
      analogValues[i] = readMux(i);
    }else{
      analogValues[i] = analogRead(i-15);
    }
    if (abs(analogValues[i] - analogValuesLag[i]) > 8){
      if(readTimeTwo - prevReadTimeTwo > 5){
        Serial.print(analogStrings[i]);
        Serial.print(analogValues[i]);
        Serial.print(" ");
        Serial.println(0);
        analogValuesLag[i] = analogValues[i];
        prevReadTimeTwo = readTimeTwo;
      }
    }
  }

  //NEOPIXELS
  //Drum Pad Pixels
  if(!instSelectMode){

    if(instNumber>3){
      if (instNumber == 4 || instNumber == 8 || instNumber == 12){
        for(int i=0; i<16; i++){
          pixels.setPixelColor(i, neoColorArray[0]);
        }
      }else if (instNumber == 5 || instNumber == 9 || instNumber == 13){
        for(int i=0; i<16; i++){
          pixels.setPixelColor(i, neoColorArray[1]);
        }
      }else if (instNumber == 6 || instNumber == 10 || instNumber == 14){
        for(int i=0; i<16; i++){
          pixels.setPixelColor(i, neoColorArray[2]);
        }
      }else{
        for(int i=0; i<16; i++){
          pixels.setPixelColor(i, neoColorArray[3]);
        }
      }
    }else{
      pixels.setPixelColor(0, pixels.Color(0, 255, 0));
      pixels.setPixelColor(1, pixels.Color(0, 255, 0));
      pixels.setPixelColor(2, pixels.Color(0, 255, 0));
      pixels.setPixelColor(8, pixels.Color(0, 255, 0));
      pixels.setPixelColor(9, pixels.Color(0, 255, 0));
      pixels.setPixelColor(10, pixels.Color(0, 255, 0));

      pixels.setPixelColor(4, pixels.Color(0, 255, 255));
      pixels.setPixelColor(5, pixels.Color(0, 255, 255));
      pixels.setPixelColor(6, pixels.Color(0, 255, 255));
      pixels.setPixelColor(12, pixels.Color(0, 255, 255));
      pixels.setPixelColor(13, pixels.Color(0, 255, 255));
      pixels.setPixelColor(14, pixels.Color(0, 255, 255));

      pixels.setPixelColor(3, pixels.Color(255,0,50));
      pixels.setPixelColor(7, pixels.Color(255,0,50));
      pixels.setPixelColor(11, pixels.Color(255,0,50));
      pixels.setPixelColor(15, pixels.Color(255,0,50));

      //pixels.Color(255,0,50),

    }
  }else{
    for(int i=0; i<16; i++){
      pixels.setPixelColor(i, neoColorArray[i]);
    }
    pixels.setPixelColor(instNumber, pixels.Color(blinkBrightness, blinkBrightness, blinkBrightness));
  }
  //Loop Pad Pixes
  blinkBrightness = 128+127*cos(2*PI/500*readTime);

  for(int i=0; i<8; i++){
    if(loopMute[i] == 1){
      if(!ledBlinking[i]){
        pixels.setPixelColor(i+16, neoColorArray[i]);
      }else{
        if(lpCurrentlyRec[i]){
          pixels.setPixelColor(i+16, pixels.Color(blinkBrightness, 0, 0));
        }else{
          pixels.setPixelColor(i+16, pixels.Color(blinkBrightness, blinkBrightness, 0));
        }
      }
    }else{
      if(!ledBlinking[i]){
        if(loopRec[i]){
          pixels.setPixelColor(i+16, neoColorArrayDim[i]);
        }else{
          pixels.setPixelColor(i+16, neoColorOff);
        }
      }else{
        if(lpCurrentlyRec[i]){
          pixels.setPixelColor(i+16, pixels.Color(blinkBrightness, 0, 0));
        }else{
          pixels.setPixelColor(i+16, pixels.Color(blinkBrightness, blinkBrightness, 0));
        }
      }
    }
  }
  
  //Read Serial
  while(Serial.available() > 0)
  {
    char c = Serial.read();
    if(c=='-'){
      serIndex = 0;
      inData[serIndex] = '\0';
      serialStarted = true;
    }else if(c=='/'){
      serialEnded = true;
      break;
    }else{
      if(serIndex < 79){
        inData[serIndex] = c;
        serIndex++;
        inData[serIndex] = '\0';
      }
    }
  }
  //Process Serial Package
  if(serialStarted && serialEnded){

    String PrintString = String(inData);

    int commaIndex = PrintString.indexOf('_');
    int secondCommaIndex = PrintString.indexOf('_', commaIndex+1);
    int thirdCommaIndex = PrintString.indexOf('_', secondCommaIndex+1);


    String part1 = PrintString.substring(0, commaIndex);
    String part2 = PrintString.substring(commaIndex+1, secondCommaIndex);
    String part3 = PrintString.substring(secondCommaIndex+1, thirdCommaIndex);

    if (part1.equals("pos")){
      loopPosVal = part2.toInt();
    }else if (part1.equals("sta")){
      Recstatus = part2;
      if(Recstatus.equals("r")){
        ledBlinking[part3.toInt()-1] = true;
        lpCurrentlyRec[part3.toInt()-1] = true;
      }else if(Recstatus.equals("p")){
        ledBlinking[part3.toInt()-1] = true;
        lpCurrentlyRec[part3.toInt()-1] = false;
      }else{
        ledBlinking[part3.toInt()-1] = false;
      }
    }else if (part1.equals("mut")){
      lpNumber = part2.toInt();
      lpStatus = part3.toInt();
      loopMute[lpNumber-1] = lpStatus;
    }else if(part1.equals("wav")){
      lpNumber = part2.toInt();
      lpStatus = part3.toInt();
      loopRec[lpNumber-1] = lpStatus;
    }else if (part1.equals("amp")){
      if(part2.equals("left")){
        leftInputAmp = part3.toInt()*1.6;
      }else if (part2.equals("right")){
        rightInputAmp = part3.toInt()*1.6;
      }else{
        extraNeoPixels(part3.toInt());
      }
    }else if(part1.equals("sel")){
      // Selection of instruments

      int mode = part2.toInt();
      
      if(mode == 1){
        for(int i=0; i<16; i++){
          pixels.setPixelColor(i, pixels.Color(0, 0, 0));
        }
        instSelectMode = true;
      } else {
        instSelectMode = false;
      }
    }

    serialStarted = false;
    serialEnded = false;
    serIndex = 0;
    for(int i=0; i<80; i++){
       inData[i] = '\0'; 
    }  
  }
}

void updateEncoder(){
  int MSB = digitalRead(encoderPin1); //MSB = most significant bit
  int LSB = digitalRead(encoderPin2); //LSB = least significant bit

  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;
  if(encoderValue > 208){
    encoderValue = 208;
  }else if (encoderValue < 0){
    encoderValue = 0;
  }

  lastEncoded = encoded; //store this value for next time
}
void extraNeoPixels(int amp){
  int ampOne = map(amp, 0, 100, 0, 30);
  for (int i = 0; i < 30; ++i){
    if (i<ampOne){
      pixels.setPixelColor(i+26, Wheel((i * 256 / pixels.numPixels()-26*ampOne)));
    }else{
      pixels.setPixelColor(i+26, pixels.Color(0,0,0));
    }
  }
  int ampTwo = map(amp, 0, 100, 30, 40);
  for (int i = 30; i < 40; ++i){
    if (i<ampTwo){
      pixels.setPixelColor(i+26, Wheel((i * 256 / pixels.numPixels()-26*ampTwo)));
    }else{
      pixels.setPixelColor(i+26, pixels.Color(0,0,0));
    }
  }  
  int ampThree = map(amp, 0, 100, 40, 60);
  for (int i = 40; i < 60; ++i){
    if (i<ampThree){
      pixels.setPixelColor(i+26, Wheel((i * 256 / pixels.numPixels()-26*ampThree)));
    }else{
      pixels.setPixelColor(i+26, pixels.Color(0,0,0));
    }
  }  
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + j) & 255));
    }
    pixels.show();
    delay(wait);
  }
}
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
