/*
>> Pulse Sensor Amped 1.2 <<
This code is for Pulse Sensor Amped by Joel Murphy and Yury Gitman
    www.pulsesensor.com 
    >>> Pulse Sensor purple wire goes to Analog Pin 0 <<<
Pulse Sensor sample aquisition and processing happens in the background via Timer 2 interrupt. 2mS sample rate.
PWM on pins 3 and 11 will not work when using this code, because we are using Timer 2!
The following variables are automatically updated:
Signal :    int that holds the analog signal data straight from the sensor. updated every 2mS.
IBI  :      int that holds the time interval between beats. 2mS resolution.
BPM  :      int that holds the heart rate value, derived every beat, from averaging previous 10 IBI values.
QS  :       boolean that is made true whenever Pulse is found and BPM is updated. User must reset.
Pulse :     boolean that is true when a heartbeat is sensed then false in time with pin13 LED going out.

This code is designed with output serial data to Processing sketch "PulseSensorAmped_Processing-xx"
The Processing sketch is a simple data visualizer. 
All the work to find the heartbeat and determine the heartrate happens in the code below.
Pin 13 LED will blink with heartbeat.
If you want to use pin 13 for something else, adjust the interrupt handler
It will also fade an LED on pin fadePin with every beat. Put an LED and series resistor from fadePin to GND.
Check here for detailed code walkthrough:
http://pulsesensor.myshopify.com/pages/pulse-sensor-amped-arduino-v1dot1

Code Version 1.2 by Joel Murphy & Yury Gitman  Spring 2013
This update fixes the firstBeat and secondBeat flag usage so that realistic BPM is reported.

*/

#include <Wire.h>  // Include Wire Set Pins.....
#include <SFE_MicroOLED.h>  // Include the SFE_MicroOLED library

//////////////////////////
// MicroOLED Definition //
//////////////////////////
#define PIN_RESET 33  // (dummy my oled only has 4 pins) (req. for SPI and I2C)
#define DC_JUMPER 0   // (dummy my oled only has 4 pins)

//////////////////////////////////
// MicroOLED Object Declaration //
//////////////////////////////////
// Declare a MicroOLED object. The parameters include:
// 1 - Reset pin: Any digital pin
MicroOLED oled(PIN_RESET, DC_JUMPER); // Example I2C declaration

const int WIDTH=64;
const int HEIGHT=48;
const int LENGTH=WIDTH;


//  VARIABLES
int fadePin = 12;                 // pin to do fancy classy fading blink at each beat
int fadeRate = 0;                 // used to fade LED on with PWM on fadePin


// these variables are volatile because they are used during the interrupt service routine!
volatile int BPM;                   // used to hold the pulse rate
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // holds the time between beats, must be seeded! 
volatile boolean Pulse = false;     // true when pulse wave is high, false when it's low
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

// For the display

int x;
int y[LENGTH];

void clearY(){
  for(int i=0; i<LENGTH; i++){
    y[i] = -1;
  }
}

void drawY(){
  oled.pixel(0, y[0]);
  for(int i=1; i<LENGTH; i++){
    if(y[i]!=-1){
      //u8g.drawPixel(i, y[i]);
      oled.line(i-1, y[i-1], i, y[i]);
    }else{
      break;
    }
  }
}


void setup(){
  Wire.begin(21,22);
  oled.begin();
  oled.clear(PAGE);                 // Clear the screen
  x = 0;
  clearY();
  pinMode(LED_BUILTIN,OUTPUT);      // pin that will blink to your heartbeat!
  pinMode(fadePin,OUTPUT);          // pin that will fade to your heartbeat!
  Serial.begin(115200);             // we agree to talk fast!
  interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS 
}



void loop(){
  y[x] = map(Signal, 0, 1023, HEIGHT-1, 0);
    drawY();
  x++;
  if(x >= WIDTH){
        oled.clear(PAGE);
    x = 0;
    clearY();
  }

  sendDataToProcessing('S', Signal);     // send Processing the raw Pulse Sensor data
  if (QS == true){                       // Quantified Self flag is true when arduino finds a heartbeat
        fadeRate = 255;                  // Set 'fadeRate' Variable to 255 to fade LED with pulse
        sendDataToProcessing('B',BPM);   // send heart rate with a 'B' prefix
        sendDataToProcessing('Q',IBI);   // send time between beats with a 'Q' prefix
        QS = false;                      // reset the Quantified Self flag for next time    
        oled.setFontType(0);             // Set font to type 0
        oled.setCursor(0, 0);           // Set cursor to bottom line
        oled.print("BPM = ");           
        oled.print(BPM);
        oled.print("  ");
     }

  //ledFadeToBeat();
  oled.display();   
  //delay(20);                             //  take a break
  yield();
}


/*void ledFadeToBeat(){
    fadeRate -= 15;                         //  set LED fade value
    fadeRate = constrain(fadeRate,0,255);   //  keep LED fade value from going into negative numbers!
    analogWrite(fadePin,fadeRate);          //  fade LED
  }*/


void sendDataToProcessing(char symbol, int data ){
    Serial.print(symbol);                // symbol prefix tells Processing what type of data is coming
    Serial.println(data);                // the data to send culminating in a carriage return
  }


