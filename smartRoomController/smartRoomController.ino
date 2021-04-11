/*
 *  Project:      HueHeader
 *  Description:  Header File for Hue Lights in IoT Classroom using Hue IoT LIbrary
 *  Authors:      Gabriel Fosse
 *  Date:         01-April-2021
 */
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

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define NUMFLAKES     10 // Number of snowflakes in the animation example
#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

// create display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Wemo my_wemo_dev; // create wemo object
Adafruit_BME280 bme; //create bme object --
const int kettle = 3;
const int fan = 2;
byte bmeAddress = 0x76;
bool status; // returns true or false --
float tempC, pressPA, humidRH;
char percent = 0x25;
char degree = 0xF8;
float tempF;
const int echoPin = 2; // attach pin D2 Arduino to Echo pin of HC-SR04
const int trigPin = 3; //attach pin D3 Arduino to Trig pin of HC-SR04
unsigned long duration; // variable for the duration of sound wave travel
int ultraSonicVal; // ultrasonic returned value 0 - 255
int lastUltraSonicVal;
int distance; // variable for the distance measurement
int dist_cm; // distance in cm
float inches; // distance in inches
const int buttonPin = 16;//encoder button
const int GREENLED = 15;//encoder built-in LEDs
const int REDLED = 17;
bool buttonState;
const int encodeA =  7;//encoder dial pins
const int encodeB =  8;
int position = 0; //encoder position
int dialHue;
int lastPosition;
const int GREENBUTTONPIN = 23;
const int REDBUTTONPIN = 22;
bool greenState;
bool redState;
int hueColor;
int i;
bool lastGreenState;
int touchlessColor;
Encoder myEncoder(encodeA,encodeB); //create encoder object!
OneButton button1 (buttonPin , false ); // create button objects!
OneButton greenButton (GREENBUTTONPIN, false );
OneButton redButton (REDBUTTONPIN, false );
#include <Adafruit_NeoPixel.h>
const int LED_PIN = 0;
const int LED_COUNT = 1;
int p;
int time;
Adafruit_NeoPixel pixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
int mode;
int tenSeconds;

void setup()
{
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
  status = bme.begin(bmeAddress);
  if(status==false) {
  Serial.print("initializattion failed\n"); // initialization failed
}
  else {
    Serial.print("BME280 intitalized successfully\n");
  }
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT
  pinMode (buttonPin,INPUT_PULLUP);
  pinMode (GREENLED,OUTPUT);
  pinMode (REDLED,OUTPUT);
  button1.attachClick(click); // initialize objects
  greenButton.attachClick(greenClick);
  redButton.attachClick(redClick);
  bool buttonState = false ; // set variables
  hueColor = 0;
  mode = 1;
  Serial.begin(9600);

  // ensure all SPI devices start off
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  Ethernet.begin(mac);
  delay(200);          //ensure Serial Monitor is up and running           
  printIP();
  Serial.printf("LinkStatus: %i  \n",Ethernet.linkStatus());
  buttonState = false;
  redState = false;
  pixel.begin();
  pixel.show();
  pixel.setBrightness(20);
  lastGreenState = false;
}

void loop() {
  time = millis();
  button1.tick () ;  
  greenButton.tick();
  redButton.tick();
  setEncoderPosition(); //binds encoder from 0-255, if there is a change it uses setHueLghts
  getUltraSonicVal();
  touchlessColor = map(ultraSonicVal,0,255,0,6);
//  autoFan();
//  autoHumidity();
  bmeDisplay();

  switch(mode) {
    case 1:
    //display brightness adjustment
      if (ultraSonicVal%5==0 && (ultraSonicVal != lastUltraSonicVal)) {
        displayText(ultraSonicVal,"select brightness value with touchless sensor (0-255)");
      }
      break;
      
    case 2:
    //select brightness, display value 
//      position = ultraSonicVal;    
      displayText(position,"Selected brightness value");
      setHueLights();
      break;
      
    case 3:
    //display color adjustment 
    if (ultraSonicVal%5==0 && (ultraSonicVal != lastUltraSonicVal)) {
        displayText(touchlessColor,"select color value with touchless sensor (0-6)");
    }
    break;
    
    case 4:
    //select color, display value  
    setHueLights();
    displayText(hueColor,"Selected color value");
    break;
    
    case 5:
    displayText(buttonState, "Motion Sensor Lights in position");
    if (ultraSonicVal<255) {
      click();
//    buttonState = true;
    setHueLights();
    delay(500);
    }
     if ((time-tenSeconds)>10000) {
         tenSeconds = millis();
         buttonState = false;
//           click();
         digitalWrite(GREENLED,LOW);
         digitalWrite(REDLED,HIGH);
         setHue(3,false,0,0,0);
      }
    break;
  };
  
  }
//}
void click() {
  buttonState = !buttonState;
  Serial.printf("buttonState is %i\n",buttonState);
  setHueLights();
  
}
void greenClick() { //color select

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
      delay(3000);

}
void redClick() { //touchless select
  hueColor = touchlessColor;
  myEncoder.write(ultraSonicVal);
  mode = mode+1;
  Serial.print(mode);
  if (mode > 5) {
    mode = 1;
  }
 
//  if (redState) {
//  position = ultraSonicVal;
//  setHueLights();
//  myEncoder.write(ultraSonicVal);
//  displayText(position,"Selected brightness value");
//  if (ultraSonicVal%5==0 && (ultraSonicVal != lastUltraSonicVal)) {
//    Serial.printf("Brightness select value is %i\n",ultraSonicVal);
//    lastUltraSonicVal = ultraSonicVal;
//    displayText(ultraSonicVal,"select brightness value with touchless sensor (0-255)");
//}
//  }
//  else {
////  myEncoder.write(ultraSonicVal);
//  touchlessColor = map(ultraSonicVal,0,255,0,7);
//  hueColor = touchlessColor;
//  setHueLights();
//  displayText(hueColor,"Selected color value");   
//  }
  redState  = !redState;
  //delay(4000);
}
void printIP() {
  Serial.printf("My IP address: ");
  for (byte thisByte = 0; thisByte < 3; thisByte++) {
    Serial.printf("%i.",Ethernet.localIP()[thisByte]);
  }
  Serial.printf("%i\n",Ethernet.localIP()[3]);
}
void setHueLights() {
  if (buttonState) {
      digitalWrite(GREENLED,HIGH);
      digitalWrite(REDLED,LOW);
      setHue(3,true,HueRainbow[hueColor],position,255); // set brightness w/encoder
      
    }
    else {
      digitalWrite(GREENLED,LOW);
      digitalWrite(REDLED,HIGH);
      setHue(3,false,0,0,0);
      
  /*  setHue function needs 5 parameters
   *  int bulb - this is the bulb number
   *  bool activated - true for bulb on, false for off
   *  int color - Hue color from hue.h
   *  int - brightness - from 0 to 255
   *  int - saturation - from 0 to 255
   */
}
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
  // Calculating the distance
  distance = duration * 0.34 / 2; // time passed * speed of sound divided by 2 (go and back)
  dist_cm = distance / 10;
  inches = dist_cm * 0.3937;
  if (distance > 295) {
    distance = 295;
  }
  if (distance < 40) {
    distance = 40;
  }
  ultraSonicVal = map(distance,40,295,0,255);
  return ultraSonicVal;
}
void setEncoderPosition() {
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
//  displayText(position,"Encoder brightness setting is");
// 
}
}
void displayText(int desiredOutput,char desiredString[]) {
  //  display.clearDisplay();

  display.setCursor(0,0);             // Start at top-left corner
//  display.println(F("Hello, world!"));
//  delay(500);
  display.clearDisplay();
  display.setRotation(0) ;
  display.setTextSize(1); 
  display.setTextColor(SSD1306_WHITE); // Draw 'inverse' text
  display.printf("%s %i\n",desiredString,desiredOutput);
//  Serial.printf("Temp is %0.1f%c Farenheit,\nPressure is %0.1f mmHG,\nand there is %0.1f%c relative humidity\n",tempF, degree, mmHG, humidRH, percent);
  display.display();
//  delay(2000);
}
void turnOnNeoPixel() {
//  for (p = 1; p <=16; p++) {
  pixel.clear();
  pixel.show();
  pixel.setPixelColor(0,rainbow[hueColor]);
  pixel.setBrightness(20); //lastUltraSonicVal for brightness
  pixel.show();
//  delay(50);
//  pixel.clear();
//  pixel.show();
}
void autoFan() {
  tempC = bme.readTemperature(); //deg C
  pressPA = bme.readPressure(); //pascals
  humidRH = bme.readHumidity(); //%RH
  float tempF = map(tempC,0,100,32,212);
  float mmHG = pressPA/3386.389;

  if(tempF > 73.0) {
    my_wemo_dev.switchON(fan);
  }
  else {
    my_wemo_dev.switchOFF(fan);
  }
  }
void autoHumidity() {
  tempC = bme.readTemperature(); //deg C
  pressPA = bme.readPressure(); //pascals
  humidRH = bme.readHumidity(); //%RH
  float tempF = map(tempC,0,100,32,212);
  float mmHG = pressPA/3386.389;

  if(humidRH < 20.0) {
    my_wemo_dev.switchON(kettle);
  }
  else {
    my_wemo_dev.switchOFF(kettle);
  }
  }
void bmeDisplay() {
  tempC = bme.readTemperature(); //deg C
  pressPA = bme.readPressure(); //pascals
  humidRH = bme.readHumidity(); //%RH
  float tempF = map(tempC,0,100,32,212);
  float mmHG = pressPA/3386.389;
//  display.clearDisplay();
//  display.setRotation(1) ;
//  display.setTextSize(1);             // Normal 1:1 pixel scale
//  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,24);             // Start at top-left corner
//  display.println(F("Hello, world!"));
//  delay(500);
  display.clearDisplay();
  display.setRotation(0) ;
  display.setTextSize(1); 
  display.setTextColor(SSD1306_WHITE); // Draw 'inverse' text
  display.printf("%0.1f%c F/Humidity %0.1f%c", tempF, degree, humidRH, percent);
//display.printf("t = %f\n", tempF);
  Serial.printf("Temp is %0.1f%c Farenheit,\nPressure is %0.1f mmHG,\nand there is %0.1f%c relative humidity\n",tempF, degree, mmHG, humidRH, percent);
  display.display();
 
  }
