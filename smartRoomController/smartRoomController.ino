/*
 *  Project:      Smart Room Controller
 *  Description:  Control lights and switches using manual and auto modes
 *  Author:      Gabriel Fosse
 *  Date:         11-April-2021
 */
 
// include libraries
#include <wemo.h>
#include <Adafruit_BME280.h>
#include <Encoder.h>
#include <SPI.h>
#include <Ethernet.h>
#include <mac.h>
#include <hue.h>
#include <OneButton.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <colors.h>
#include <Adafruit_NeoPixel.h>

// OLED screen perameters
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define NUMFLAKES     10 // Number of snowflakes in the animation example
#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

int displayTimeout;
const int kettle = 3;
const int fan = 2;
byte bmeAddress = 0x76; //i2c address
bool status; // returns true or false for BME sensor--
float tempC, pressPA, humidRH;
char percent = 0x25;
char degree = 0xF8;
float tempF, mmHG;
const int echoPin = 2; // attach pin D2 Arduino to Echo pin of HC-SR04
const int trigPin = 3; //attach pin D3 Arduino to Trig pin of HC-SR04
unsigned long duration; // variable for the duration of sound wave travel
int ultraSonicVal; // ultrasonic returned value 0 - 255
int lastUltraSonicVal;
int distance; // variable for the distance measurement
int dist_cm; // distance in cm
float inches; // distance in inches
const int buttonPin = 16;//encoder button
const int GREENLED = 15;//encoder built-in green LED
const int REDLED = 17;//encoder built-in red LED
bool buttonState; //encoder press down button
const int encodeA =  7;//encoder dial pin A
const int encodeB =  8;//encoder dial pin B
int position = 0; //encoder position
int dialHue;
int lastPosition;
const int GREENBUTTONPIN = 23;
const int REDBUTTONPIN = 22;
const int BLUEBUTTONPIN = 21;
const int YELLOWBUTTONPIN = 20;
bool greenState;
bool redState;
int hueColor;
int i;
bool yellowState;
bool blueState;
bool lastGreenState;
int touchlessColor;
const int NEOPIXEL_PIN = 0; // pin to neopixel.neopixel indicates what color the hue lights are.
const int NEOPIXEL_COUNT = 1; // must tell neopixel class how many neopixel there are connected
int p;
int time;
int mode;
int tenSeconds; // timer for 'motion sensor'. 


// create objects
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Wemo my_wemo_dev; 
Adafruit_BME280 bme; 
Encoder myEncoder(encodeA,encodeB); //args are encoder dial pins
Adafruit_NeoPixel pixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
OneButton button1 (buttonPin , false ); // encoder button
OneButton greenButton (GREENBUTTONPIN, false );
OneButton redButton (REDBUTTONPIN, false );
OneButton yellowButton (YELLOWBUTTONPIN, false );
OneButton blueButton (BLUEBUTTONPIN, false );

void setup(){ 
  
//OLED display setup
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds
  // Clear the buffer
  display.clearDisplay();
  // Draw a single pixel in white
  display.drawPixel(10, 10, SSD1306_WHITE);
  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();

//BME sensor setup
  status = bme.begin(bmeAddress);
  if(status==false) {
  Serial.print("initializattion failed\n"); // initialization failed
  }
  else {
    Serial.print("BME280 intitalized successfully\n");
  }

//activate pins for LEDs and devices
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT
  pinMode (GREENLED,OUTPUT);
  pinMode (REDLED,OUTPUT);

// initialize button clicks
  button1.attachClick(click); 
  greenButton.attachClick(greenClick);
  redButton.attachClick(redClick);
  yellowButton.attachClick(yellowClick);
  blueButton.attachClick(blueClick);
  
  hueColor = 0;
  mode = 1;
  Serial.begin(9600);

// ensure all SPI devices start off
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  Ethernet.begin(mac);
//  delay(200);          //ensure Serial Monitor is up and running           
//  printIP();
//  Serial.printf("LinkStatus: %i  \n",Ethernet.linkStatus());
 
// initialize conditions and variables
  buttonState = false;
  redState = false;
  yellowState = false;
  blueState = false;
  lastGreenState = false;

//kick off neopixel
  pixel.begin();
  pixel.show();
  pixel.setBrightness(20);
  
  digitalWrite(GREENLED,LOW);
  digitalWrite(REDLED,HIGH);
  turnOnNeoPixel();

  displayLargeText("MAJESTIC \nI/O");
  delay(1500);
} // end setup

void loop() {
  
  time = millis(); //start time in milliseconds
  
  button1.tick(); // was button1 clicked? 
  greenButton.tick();
  redButton.tick();
  yellowButton.tick();
  blueButton.tick();
  setEncoderPosition(); //function binds encoder from 0-255,if there is a change it uses setHueLghts()
  getUltraSonicVal(); // returns value from the ultrasonic sensor
  
  touchlessColor = map(ultraSonicVal,0,255,0,6); // changes value from 0-255 to 0-6 (colors of rainbow)
  bmeReadTempF_HumidRH(); // displays temp in F and % humidity to the OLED screen

  switch(mode) { //modes for the 'touchless select' button. brightness, color, and motion sensor
    
    case 1: //display brightness adjustment
      if (ultraSonicVal%5==0 && (ultraSonicVal != lastUltraSonicVal)) {
        displayText(ultraSonicVal,"Select bright value\nwith touchless sensor (0-255) value =");
      }
      break;
      
    case 2: //select brightness, display value 
      displayText(position,"Selected brightness\nvalue was");
      turnOnNeoPixel();
      if ((time-displayTimeout)>1000) {
        displayTimeout = millis();
        setHueLights();
      }
      break;
      
    case 3: //display color adjustment 
    if (ultraSonicVal%5==0 && (ultraSonicVal != lastUltraSonicVal)) {
        displayText(touchlessColor,"Select color value\nwith touchless sensor (0-6) value =");
    }
    break;
    
    case 4: //select color, display value
    turnOnNeoPixel();
    if ((time-displayTimeout)>1000) {
        displayTimeout = millis();
        setHueLights();
    }
    displayText(hueColor,"Selected color value\nwas");
    break;
    
    case 5: //turns on motion sensor for the hue lights and auto wemo controls
    displayText(buttonState, "Auto selected.\nTemp/RH wemo switch\nMotionLights on/off");
    autoFan(); // turns on wemo switch for the fan
    autoHumidity(); //same for humidity
    if (ultraSonicVal<255) {
      digitalWrite(GREENLED,HIGH);
      digitalWrite(REDLED,LOW);
      buttonState = true; 
      turnOnNeoPixel();  
      setHue(3,true,HueRainbow[hueColor],position,255);  
      delay(200); //lil delay so the sensor doesn't get tripped again
    }
    if ((time-tenSeconds)>10000) { //setting timer for 10 seconds. change if needed
      tenSeconds = millis();     
      digitalWrite(GREENLED,LOW);
      digitalWrite(REDLED,HIGH);
      buttonState = false;
      turnOffHueLights(); // turn off hue lights
//      pixel.clear(); // turn off neopixel too
//      pixel.show();
    }
    break;
    };
  
} //end void loop
  
void click() { //encoder button click action
  buttonState = !buttonState;
//  Serial.printf("buttonState is %i\n",buttonState);
  setHueLights();
  turnOnNeoPixel();
}
void yellowClick() {
  yellowState = !yellowState;
  if (yellowState) {
    Serial.print("Fan is ON!\n");
    displayLargeText("Fan \nis ON!!!");
    my_wemo_dev.switchON(fan);
  }
  else {
    Serial.print("Fan is OFF!\n");
    displayLargeText("Fan \nis OFF!!!");
    my_wemo_dev.switchOFF(fan);
  }
}
void blueClick() {
  blueState =! blueState;
  if (blueState) {
    Serial.print("kettle is ON!\n");
    displayLargeText("Kettle \nis ON!!!");
    my_wemo_dev.switchON(kettle);
  }
  else {
    Serial.print("kettle is OFF!\n");
    displayLargeText("Kettle \nis OFF!!!");
    my_wemo_dev.switchOFF(kettle);
  }
}
void greenClick() { //green button, color select
  
  if (hueColor <6 ) {
    hueColor += 1;
  }
  else {
    hueColor = 0;
  }
  Serial.printf("hueColor value is %i\n",hueColor);
  setHueLights();
  displayText(hueColor,"Selected hue color");
  turnOnNeoPixel();
  delay(1500); //pause display for 2 seconds
}

void redClick() { //red button, touchless select. changes 'switch' case in void loop
  hueColor = touchlessColor;
  myEncoder.write(ultraSonicVal);
  mode = mode+1;
  Serial.print(mode);
  if (mode > 5) {
    mode = 1;
  }
}

void printIP() {
  Serial.printf("My IP address: ");
  for (byte thisByte = 0; thisByte < 3; thisByte++) {
    Serial.printf("%i.",Ethernet.localIP()[thisByte]);
  }
  Serial.printf("%i\n",Ethernet.localIP()[3]);
}

void setHueLights() { // controls hue lights. indicates on/off with encoder's LEDs
  
  if (buttonState) {
      digitalWrite(GREENLED,HIGH);
      digitalWrite(REDLED,LOW);
      setHue(1,true,HueRainbow[hueColor],position,255); // set brightness w/encoder
      setHue(2,true,HueRainbow[hueColor],position,255);
      setHue(3,true,HueRainbow[hueColor],position,255);
      setHue(4,true,HueRainbow[hueColor],position,255);
      setHue(5,true,HueRainbow[hueColor],position,255);
    }
    else {
      digitalWrite(GREENLED,LOW);
      digitalWrite(REDLED,HIGH);
      turnOffHueLights();
      
  /*  setHue function needs 5 parameters
   *  int bulb - this is the bulb number
   *  bool activated - true for bulb on, false for off
   *  int color - Hue color from hue.h
   *  int - brightness - from 0 to 255
   *  int - saturation - from 0 to 255
   */
}
}

void turnOffHueLights() {
  setHue(1,false,0,0,0);
  setHue(2,false,0,0,0);
  setHue(3,false,0,0,0);
  setHue(4,false,0,0,0);
  setHue(5,false,0,0,0);
}

int getUltraSonicVal() {
  
  //this section sends out an ultrasonic pulse-
  digitalWrite(trigPin, LOW); // Clears the trigPin condition
  delayMicroseconds(2);
  
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculating the time and distance
  distance = duration * 0.34 / 2; // time passed * speed of sound divided by 2 (go and back)
  dist_cm = distance / 10;
  inches = dist_cm * 0.3937;
  if (distance > 295) { // only keeping the values I want
    distance = 295;
  }
  if (distance < 40) { // compensation for irreglar readings when too close to sensor
    distance = 40;
  }
  ultraSonicVal = map(distance,40,295,0,255); // maps distance in mm to range 0-255 with added margin
  return ultraSonicVal;
}
void setEncoderPosition() { //reads position and binds it to range of 0-255 for our needs
  position = myEncoder.read(); //96 positions on encoder, can count higher
  if (position > 255) {
    position = 255; 
    myEncoder.write(255);
  }
  if (position < 0) {
    position = 0;
    myEncoder.write(0);
  }
  
  if (position != lastPosition) {
//  mode = 5;
    Serial.printf("encoder position is %i\n",position);
    lastPosition = position;
    setHueLights();
}
}
void displayText(int desiredOutput,char desiredString[]) { //takes args and prints them to OLED

  display.clearDisplay();
  display.setCursor(0,0);             // Start cursor at top-left corner
  display.setRotation(0) ;
  display.setTextSize(1); 
  display.setTextColor(SSD1306_WHITE); // Draw 'inverse' text
  display.printf("%s %i\n%0.1f%c F / Humid %0.1f%c", desiredString, desiredOutput, tempF, degree, humidRH, percent);
  display.display();
}

void displayLargeText(char desiredString[]) { //takes args and prints them to OLED

  display.clearDisplay();
  display.setCursor(0,0);             // Start cursor at top-left corner
  display.setRotation(0) ;
  display.setTextSize(2); 
  display.setTextColor(SSD1306_WHITE); // Draw 'inverse' text
  display.printf("%s", desiredString);
  display.display();
  display.startscrollright(0x00, 0x07);
  delay(1500);
  display.stopscroll();
}

void turnOnNeoPixel() {

    pixel.clear();
    pixel.show();
    pixel.setPixelColor(0,rainbow[hueColor]);
    pixel.setBrightness(20); //lastUltraSonicVal for brightness?
    pixel.show();
//  }
//  else {
//    pixel.clear();
//    pixel.show();
//  }
}

void autoFan() { // wemo switch control, turns on fan when above 73 degrees F.
  
  if(tempF > 73.0) {
    my_wemo_dev.switchON(fan);
  }
  else {
    my_wemo_dev.switchOFF(fan);
  }
}
  
void autoHumidity() { // wemo switch control, turns on humidifier when below 20% Humidity.
  
  if(humidRH < 12.0) {
    my_wemo_dev.switchON(kettle);
  }
  else {
    my_wemo_dev.switchOFF(kettle);
  }
}
  
void bmeReadTempF_HumidRH() { // sets BME variables for displayText
  
  tempC = bme.readTemperature(); //deg C
  pressPA = bme.readPressure(); //pascals
  humidRH = bme.readHumidity(); //%RH
  tempF = map(tempC,0,100,32,212);
  mmHG = pressPA/3386.389;
  if (time%300==0) {
    Serial.printf("Temp is %0.1f%c Farenheit,\nPressure is %0.1f mmHG,\nand there is %0.1f%c relative humidity\n",tempF, degree, mmHG, humidRH, percent);
  }
}
