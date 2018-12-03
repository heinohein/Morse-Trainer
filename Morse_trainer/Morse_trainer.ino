/*
  Morse Code Project

  This code will loop through a string of characters and convert these to morse code.  
  It will blink a LED light and show current letter ob oled display
  
  Oled Based on OLED_ssd1306_128*64_i2c scetch
  Written by Limor Fried/Ladyada  for Adafruit Industries.  

  Morse Code Player Code (Revised By Curt Rowlett)
  21-11-2018 HV start with sos, then show a-z 0-9 slowly, then train leetrs randomly
                speed is adjustable, loops are interuptable with pushbutton switch
*/

#include <SPI.h>
#include <Wire.h>
//#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

char stringToMorseCode[] = "sos";

// Create variable to define the input-output pins
#define led1 13               // blink led on output 13
#define switchPin 2           // connect pushbutton to D2
#define speedPin A0           // connect variable resistor to A0 to vary speed
#define randomPinNumber A1    // open pin for seeding random routine

/*
  Set the speed of your morse code
  Adjust the 'dotlen' length to speed up or slow down your morse code
    (all of the other lengths are based on the dotlen)

  Here are the ratios code elements:
    Dash length = Dot length x 3
    Pause between elements = Dot length
      (pause between dots and dashes within the character)
    Pause between characters = Dot length x 3
    Pause between words = Dot length x 7
*/

#define DOT 300
int dotLen = DOT;             // default length of the morse code 'dot'
int dashLen;                  // length of the morse code 'dash'
int elemPause;                // length of the pause between elements of a character
int charPause;                // length of the spaces between characters
int wordPause;                // length of the pause between words
int wordPause2;               // length of the pause between words used here
int testPause=3000;           // length of the pause during testing of knowledge of Morse codes

#define LOOKPAUSE 800         // length of pause to look at the oled display
#define PHASEPAUSE 1000       // length of pause between the phases of the programm

#define blink 200             // delay when blinking led

int xpos0=88;                 // x,y positions on the display
int ypos0=0;
int xpos1=20;                 // letter                 
int ypos1=16; 
int xpos2=0;                  // morse code
int ypos2=38;

int stepNumber=0;            // state of the programm
long randNumber;              // number to compute a random letter/digit 
long lastFactor=9999;         // save previous computed delay time factor
          
// the setup routine runs once when you press reset:
void setup() 
{
  Serial.begin(115200);                       // standard serial
  Serial.println();
  Serial.println("Start Morse trainer");                

  // initialize the digital pin as an output for LED lights.
  pinMode(led1, OUTPUT);
  pinMode(switchPin, INPUT_PULLUP);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.clearDisplay();                     // set up empty display
  display.display();                          // show changes on oled LCD
}

//***********************************************************************
void loop()
{
  display.clearDisplay();                     // set up empty display
//  display.display();                          // show changes on oled LCD
  calculateDotLen();                           // recalculate delay times
  stepNumber=1;
  initPhase(stepNumber);                      // show phase with blink of led end start display on oled 

  // Loop through the string and get each character one at a time
  for (int i = 0; i < sizeof(stringToMorseCode) - 1; i++)
  {
    // Get the character in the current position
    char tmpChar = stringToMorseCode[i];
    showMorseSignals(tmpChar);                // and show the morse code
  }
  // At the end of the string long pause before starting again
  display.clearDisplay();                     // make the display empty
  display.display();                          // show changes on oled LCD
  LightsOff(PHASEPAUSE);
//*********************
  stepNumber=2;
  initPhase(stepNumber);                     // show phase with blink of led

  for (int i=0; i<36; i++)                  // rotate A to Z, 0 to 9
  {
    if (digitalRead(switchPin)==LOW)        // switch pressed
    {
      delay(50);
      if (digitalRead(switchPin)==LOW)
      {
//        while (digitalRead(switchPin)==LOW)
//        {
//        }
        i=36;                               //stop morse A-Z, 0-9
      }
    }
    
    if (i<26)
    {
      calculateDotLen();                    // recaculate delay times
      char tmpChar='A'+i;
      Serial.println(tmpChar);
      showMorseSignals(tmpChar);
      LightsOff(wordPause2);                // wait for the next 1 letter "word"
    }
    else
    {
      calculateDotLen();                    // recalculate delay times
      char tmpChar='0'+i-26;
      Serial.println(tmpChar);
      showMorseSignals(tmpChar);       
      LightsOff(wordPause2);                // extra wait for the next 1 letter "word"
    }
  }
  
  display.clearDisplay();                     // make the display empty
  display.display();                          // show changes on oled LCD
  LightsOff(PHASEPAUSE);
//******************
  while (digitalRead(switchPin)==LOW)
  {
  }
  stepNumber=3;
  initPhase(stepNumber);                     // show phase with blink of led
  randomSeed(analogRead(randomPinNumber)); 

  for (int j=0; j<9; j++)
  {
    calculateDotLen();                        // recalculate delay times
    int k=(j+1)*4;
    for (int i=0; i<k; i++)                    // get a random letter/digit
    {
      if (digitalRead(switchPin)==LOW)        // switch pressed
      {
        delay(50);
        if (digitalRead(switchPin)==LOW)
        {
          i=36;                               //stop morse A-Z, 0-9
          j=9;
        }
      }
      randNumber=random(0, k);                // generate next letter
      int l=randNumber;
      char tmpChar='A'+l;
      Serial.println(tmpChar);
      testMorseSignals(tmpChar);               // show led signal, wait, then display letter and morsecode
      LightsOff(testPause);                    // wait for the next 1 letter "word"
    }
  }

  LightsOff(PHASEPAUSE);
} // end loop


//********************************************************************************
//********************************************************************************
// indicate the start of phase of the programm by 1 to n short blinks of the LED
void initPhase(int j)
{
  Serial.print(F("Start step "));
  Serial.println(j);
  
  display.setTextSize(1);
  display.setTextColor(WHITE);           // display is in yellow
  display.setCursor(0,0);                // x,y pixels 0->127,0->64
  display.print(F("Morse-"));            // fill in headerline
  display.print(j);
  display.print(" wait =");
  display.setCursor(xpos0,ypos0);
  display.print(dotLen);
  
  display.setCursor(0,ypos1);
  display.setTextColor(BLACK, WHITE);    // Show letter as 'inverted' text
  display.setTextSize(3);
  display.print(F("       "));
  
  display.setCursor(0,ypos2);
  display.print(F("       "));
  display.display();                     // show changes on oled LCD
  
  for (int i=0;i<j;i++)
  {
    digitalWrite(led1, HIGH);            // turn the LED on for 3 seconds
    delay(blink);
    digitalWrite(led1, LOW);             // turn the LED off
    delay(blink);   
  }
  LightsOff(PHASEPAUSE);                 // dark for x seconds, Morse starts
} // end initPhase


//***************************************************************************
// read variable resistor and caculate all delays
void calculateDotLen()
{
  long factor=analogRead(speedPin);      // read variable resistor
  factor=factor*800/1024;                // convert R value to amount
  dotLen=100+factor; 
  if (factor<lastFactor-50||factor>lastFactor+50)
  {
  }
  else
  {
//    Serial.print(factor);
//    Serial.print(F(" "));
//    Serial.println(lastFactor);
    return;                              // limit time delay by statements hereafter
  }
  lastFactor=factor;                     // save for next time
                    // vary from 100 to 900
  // now calculate standard waiting times
  dashLen = dotLen * 3;                  // length of the morse code 'dash'
  elemPause = dotLen;                    // length of the pause between elements of a character
  charPause = dotLen * 3;                // length of the spaces between characters
  wordPause = dotLen * 7;                // length of the pause between words
  wordPause2= dotLen * 4;                // length of the pause between words used here
  display.setTextSize(1);
  display.setTextColor(WHITE,BLACK);     // display is in yellow
  display.setCursor(xpos0,ypos0);        // print speed
  display.print(dotLen);
  display.display();
} //end calculateDotLen


//**************************************************************************
// DOT
void MorseDot()
{
  digitalWrite(led1, HIGH);              // turn the LED on 
  delay(dotLen);                        // hold in this position
}

//**************************************************************************
// DASH
void MorseDash()
{
  digitalWrite(led1, HIGH);             // turn the LED on 
  delay(dashLen);                       // hold in this position
}


//**************************************************************************
// Turn Off led and wait
void LightsOff(int delayTime)
{
  digitalWrite(led1, LOW);              // turn the LED off   
  delay(delayTime);                     // hold in this position
}


//**************************************************************************
// *** Show letter, Morse Code en blinking led in Morse code *** 
void showMorseSignals(char inputChar)
{
  display.setCursor(xpos1,ypos1);         // line withe the leer
  display.setTextColor(BLACK, WHITE);   
  display.setTextSize(3);
  display.print(inputChar);               // Show letter large size as 'inverted' text
  
  display.setCursor(xpos2,ypos2);         // set to line with morse code
  
char tmpChar = toLowerCase(inputChar);  // Set the case to lower case
  morseSignals(tmpChar);                // and show blinking morse code and text on LCD
}


//***************************************************************************************
// *** Test knowledge of Morse Code with blinking led, after waiting show the letter *** 
void testMorseSignals(char inputChar)
{
  display.setTextColor(BLACK, WHITE);   // Show morse code as 'inverted' text
  display.setCursor(xpos1,ypos1);       // line with the letter
  display.setTextSize(3);
  display.print(" ");                   // do not show letter yet

  display.setCursor(xpos2,ypos2);
char tmpChar = toLowerCase(inputChar);  // Set the case to lower case
  morseSignals(tmpChar);
  display.setCursor(xpos1,ypos1);
  display.setTextSize(3);
  display.print(inputChar);
  display.display();
}


//**************************************************************************
// show morse signal text on oled lcd and blink led in Morse patron
void morseSignals(char tmpChar)
{  
  // Take the passed character and use a switch case to find the morse code for that character
  switch (tmpChar) 
  {
    case 'a':
      display.print(F("._     "));
      displayAndLook();
      MorseDot();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(charPause);
      break;
    case 'b':
      display.print(F("_...   "));
      displayAndLook();
      MorseDash();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(charPause);
      break;
    case 'c':
      display.print(F("_._.   "));
      displayAndLook();
      MorseDash();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(charPause);
      break;
    case 'd':
      display.print(F("_..    "));
      displayAndLook();
      MorseDash();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(charPause);
      break;
    case 'e':
      display.print(F(".      "));
      displayAndLook();
      MorseDot();
      LightsOff(charPause);
      break;
    case 'f':
	    display.print(F(".._.   "));
	    displayAndLook();
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(charPause);
      break;
    case 'g':
      display.print(F("__.    "));
      displayAndLook();
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(charPause);
      break;
    case 'h':
      display.print(F("....   "));
      displayAndLook();
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(charPause);
      break;
    case 'i':
      display.print(F("..     "));
      displayAndLook();
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(charPause);
      break;
    case 'j':
      display.print(F(".___   "));
      displayAndLook();
      MorseDot();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(charPause);
      break;
    case 'k':
      display.print(F("_._    "));
      displayAndLook();
      MorseDash();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(charPause);
      break;
    case 'l':
      display.print(F("._..   "));
      displayAndLook();
      MorseDot();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(charPause);
    break;
      case 'm':
      display.print(F("__     "));
      displayAndLook();
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(charPause);
      break;
    case 'n':
      display.print(F("_.     "));
      displayAndLook();
      MorseDash();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(charPause);
      break;
    case 'o':
      display.print(F("___    "));
      displayAndLook();
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(charPause);
      break;
    case 'p':
      display.print(F(".__.   "));
      displayAndLook();
      MorseDot();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(charPause);
      break;
    case 'q':
      display.print(F("__._   "));
      displayAndLook();
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(charPause);
      break;
    case 'r':
      display.print(F("._.    "));
      displayAndLook();
      MorseDot();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(charPause);
      break;
    case 's':
      display.print(F("...    "));
      displayAndLook();
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(charPause);
      break;
    case 't':
      display.print(F("_      "));
      displayAndLook();
      MorseDash();
      LightsOff(charPause);
      break;
    case 'u':
      display.print(F(".._    "));
      displayAndLook();
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(charPause);
      break;
    case 'v':
      display.print(F("..._   "));
      displayAndLook();
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(charPause);
      break;
    case 'w':
      display.print(F(".__    "));
      displayAndLook();
      MorseDot();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(charPause);
      break;
    case 'x':
      display.print(F("_.._   "));
      displayAndLook();
      MorseDash();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(charPause);
      break;
    case 'y':
      display.print(F("_.__   "));
      displayAndLook();
      MorseDash();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(charPause);
      break;
    case 'z':
      display.print(F("__..   "));
      displayAndLook();
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(charPause);
      break;
    case '0':           //digits
      display.print(F("_____  "));
      displayAndLook();
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(charPause);
      break;
    case '1':
      display.print(F(".____  "));
      displayAndLook();
      MorseDot();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(charPause);
      break;
    case '2':
      display.print(F("..___  "));
      displayAndLook();
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(charPause);
      break;
    case '3':
      display.print(F("...__  "));
      displayAndLook();
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(charPause);
      break;
    case '4':
      display.print(F("...._  "));
      displayAndLook();
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(charPause);
      break;
    case '5':
      display.print(F(".....  "));
      displayAndLook();
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(charPause);
      break;
    case '6':
      display.print(F("_....  "));
      displayAndLook();
      MorseDash();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(charPause);
      break;
    case '7':
      display.print(F("__...  "));
      displayAndLook();
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(charPause);
      break;
    case '8':
      display.print(F("___..  "));
      displayAndLook();
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(charPause);
      break;
    case '9':
      display.print(F("____.  "));
      displayAndLook();
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDash();
      LightsOff(elemPause);
      MorseDot();
      LightsOff(charPause);
      break;
    default: 
    // If a matching character was not found it will default to a blank space
    LightsOff(wordPause);      
  }
}

void displayAndLook()
{
  display.display();                      // show changes on oled lcd
  delay(LOOKPAUSE);                       // and give time to look at it
}

