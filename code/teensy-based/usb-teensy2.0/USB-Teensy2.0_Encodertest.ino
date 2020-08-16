#include <MIDI.h>
#include <ResponsiveAnalogRead.h>
#include <Bounce.h> 
#include <Encoder.h>





// Change these pin numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
Encoder knobRight(3, 2);  //Pins without Interrupt
Encoder knobLeft(8, 7); //Pins with Interrupt
//   avoid using pins with LEDs attached





MIDI_CREATE_DEFAULT_INSTANCE();
// Dont forget the Interrupts

const int midiChannel = 3;
const int amountOfPotiControllerInputs = 10;
const int amountOfDigitalButtonController = 4; 
const int cable = 0;
const int ON_VELOCITY = 99;


const int potiControllerPin[amountOfPotiControllerInputs] = {A9, A0, A1, A2, A3, A4, A5, A6, A7, A8};
const int CCID[amountOfPotiControllerInputs] = {21, 22, 23, 24, 25, 26, 27, 28, 29, 30};

const int buttonControllerPin[amountOfDigitalButtonController] = {0, 1, 4, 9}; 
const int note[amountOfDigitalButtonController] = {60, 61, 62, 63}; 
const int BOUNCE_TIME = 7; // 5 ms is usually sufficient
const boolean toggled = true;

int positionLeft, positionRight;

byte potiData[amountOfPotiControllerInputs];
byte potiDataLag[amountOfPotiControllerInputs];

ResponsiveAnalogRead analogPotiController[]{
  {potiControllerPin[0],true},
  {potiControllerPin[1],true},
  {potiControllerPin[2],true},
  {potiControllerPin[3],true},
  {potiControllerPin[4],true},
  {potiControllerPin[5],true},
  {potiControllerPin[6],true},
  {potiControllerPin[7],true},
  {potiControllerPin[8],true},
  {potiControllerPin[9],true}
};

Bounce digitalButtonController[] =   {
  Bounce(buttonControllerPin[0], BOUNCE_TIME), 
  Bounce(buttonControllerPin[1], BOUNCE_TIME),
  Bounce(buttonControllerPin[2], BOUNCE_TIME),
  Bounce(buttonControllerPin[3], BOUNCE_TIME)
}; 


void setup()
{
    MIDI.begin(midiChannel);
    /*while( potiControllerPin.length != amountOfPotiControllerInputs ){ // || potiControllerValues.length != amountOfPotiControllerInputs){
      if( !Serial ) {
        Serial.begin(9600);
      }
      Serial.println("Error Controller Pin Values Amount mismatch");
      delay(2000);
    }*/
    for (int digitalControllerID = 0; digitalControllerID < amountOfDigitalButtonController; digitalControllerID++) {
      pinMode(buttonControllerPin[digitalControllerID], INPUT_PULLUP);
    }
}

void getPotiData(){  
 for ( int potiControllerID = 0; potiControllerID  < amountOfPotiControllerInputs; potiControllerID++ ) {
    analogPotiController[potiControllerID].update(); 
    if(analogPotiController[potiControllerID].hasChanged()) {
      potiData[potiControllerID] = analogPotiController[potiControllerID].getValue()>>3;
      // map(analogRead(controllerPin[ controllerID ]),0,1023,0,127);
      if (potiData[potiControllerID] != potiDataLag[potiControllerID]){
        potiDataLag[potiControllerID] = potiData[potiControllerID];
        usbMIDI.sendControlChange(CCID[potiControllerID], potiData[potiControllerID], midiChannel);
      }
    }
  }
}

void getButtonData(){
  for ( int buttonControllerId = 0; buttonControllerId < amountOfDigitalButtonController; buttonControllerId++ ) {
    digitalButtonController[buttonControllerId].update();
    if (digitalButtonController[buttonControllerId].fallingEdge()) {
      Serial.println(buttonControllerId);
      usbMIDI.sendNoteOn(note[buttonControllerId], ON_VELOCITY, midiChannel);  
    }
    // Note Off messages when each button is released
    if (digitalButtonController[buttonControllerId].risingEdge()) {
      usbMIDI.sendNoteOff(note[buttonControllerId], 0, midiChannel);  
    }
  }
}

void getEncoderData(){
  int newLeft, newRight;
  newLeft = map(knobLeft.read(),-255,256,0,127);         // The Encoder-Library seems to change the value by 4 with each click                              
  newRight = map(knobRight.read(),-255,256,0,127);
  newLeft = constrain(newLeft,0,127);
  newRight = constrain(newRight,0,127);
  if (newLeft != positionLeft) {
    /*
    Serial.print("Left = ");
    Serial.print(newLeft);
    Serial.println();
    */
    positionLeft = newLeft;
    usbMIDI.sendControlChange(40, positionLeft, midiChannel);     
  }  
  if (newRight != positionRight) {
    /*
    Serial.print(", Right = ");
    Serial.print(newRight);
    Serial.println();
    */
    positionRight = newRight;
    usbMIDI.sendControlChange(41, positionRight, midiChannel);
  }
}

void loop()
{
  //if(!digitalRead(4)) Serial.print("4");
  //if(!digitalRead(9)) Serial.print("9");
  getPotiData();
  getButtonData();   
  getEncoderData();    
}