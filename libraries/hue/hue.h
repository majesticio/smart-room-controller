/*
 *  Project: Hue IoT Library
 *  Description: Library for controlling Hue Lights at IOT Classroom
 *  Authors:  Brian Rashap
 *  Date:     1-MAY-2020
 */

/* Usage:
 * setHue(int lightNum, bool HueOn, int HueColor, int HueBright, int HueSat);
 * where:
 *    lightNum is the number of the light to be controlled
 *    HueOn is true to turn the light on, false to turn it off
 *    HueColor is a number between 0 and 65353 (see constants below)
 *    HueBright is the brightness between 0 and 255
 *    HueSat is the saturation between 0 and 255
 *
 * NOTE: In your main code, Ethernet.begin(mac) needs to be called
 */


// Hue Configuration
const char hueHubIP[] = "192.168.1.2";       // Hue hub IP
const char hueUsername[] = "MQlZziRO0Wai5MsMHll8xAUAQqw85Qrr8tM37F3T";
const int hueHubPort = 80;   // HTTP: 80, HTTPS: 443, HTTP-PROXY: 8080

//  Hue variables
boolean hueOn;  // on/off
int hueBri;  // brightness value
long hueHue;  // hue value
String hueCmd;  // Hue command

// Hue colors
int HueRed = 0;
int HueOrange = 5000;
int HueYellow = 10000;
int HueGreen = 22500;
int HueBlue = 45000;
int HueIndigo = 47500;
int HueViolet = 50000;
int HueRainbow[] = {HueRed, HueOrange, HueYellow, HueGreen, HueBlue, HueIndigo, HueViolet};

EthernetClient HueClient;

boolean setHue(int lightNum, bool HueOn, int HueColor, int HueBright, int HueSat) {

  String command = "";  

  if(HueOn == true) {
    /*
    command = "{\"on\":true,\"sat\":255,\"bri\":255,\"hue\":";
    command = command + String(HueColor) + "}";
    */
    
    command = "{\"on\":true,\"sat\":";
    command = command + String(HueSat) + ",\"bri\":";
    command = command + String(HueBright) + ",\"hue\":";
    command = command + String(HueColor) + "}";
  }
  else {
    command = "{\"on\":false}";
  }

  if (HueClient.connect(hueHubIP, hueHubPort))
  {
    //while (HueClient.connected())
    //{
      Serial.println("Sending Command to Hue");
      Serial.println(command);
      HueClient.print("PUT /api/");
      HueClient.print(hueUsername);
      HueClient.print("/lights/");
      HueClient.print(lightNum);  // hueLight zero based, add 1
      //  HueClient.println("/state");
      HueClient.println("/state HTTP/1.1");
      HueClient.println("keep-alive");
      HueClient.print("Host: ");
      HueClient.println(hueHubIP);
      HueClient.print("Content-Length: ");
      HueClient.println(command.length());
      HueClient.println("Content-Type: text/plain;charset=UTF-8");
      HueClient.println();  // blank line before body
      HueClient.println(command);  // Hue command
      Serial.println("From Hue");
      Serial.println(HueClient.readString()); // To close connection
    HueClient.stop();
    return true;  // command executed
  }
  else
    return false;  // command failed
}


boolean getHue(int lightNum)
{
  if (HueClient.connect(hueHubIP, hueHubPort))
  {
    HueClient.print("GET /api/");
    HueClient.print(hueUsername);
    HueClient.print("/lights/");
    HueClient.print(lightNum);  
    HueClient.println(" HTTP/1.1");
    HueClient.print("Host: ");
    HueClient.println(hueHubIP);
    HueClient.println("Content-type: application/json");
    HueClient.println("keep-alive");
    HueClient.println();
    while (HueClient.connected())
    {
      if (HueClient.available())
      {
        Serial.println();
        Serial.println(HueClient.readString());
        Serial.println();
        HueClient.findUntil("\"on\":", "\0");
        hueOn = (HueClient.readStringUntil(',') == "true");  // if light is on, set variable to true
        Serial.print("Hue Status: ");
        Serial.println(hueOn);
 
        HueClient.findUntil("\"bri\":", "\0");
        hueBri = HueClient.readStringUntil(',').toInt();  // set variable to brightness value
        //Serial.println(hueBri);
        
        HueClient.findUntil("\"hue\":", "\0");
        hueHue = HueClient.readStringUntil(',').toInt();  // set variable to hue value
        //Serial.println(hueHue);
        
        break;  // not capturing other light attributes yet
      }
    }
    HueClient.stop();
    return true;  // captured on,bri,hue
  }
  else
    return false;  // error reading on,bri,hue
}
