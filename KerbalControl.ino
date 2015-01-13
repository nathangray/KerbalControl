/* Kerbal Space Program controller

   You must select Serial+Keyboard+Mouse+Joystick from the "Tools > USB Type" menu

   Adapted from Teensy example code
*/


#include <Bounce.h>


// Structs for keyboard buttons
typedef struct {
  byte pin;
  byte key;
  boolean momentary;
  Bounce _button;
} KB_Button;


// Create Bounce objects for each button.  The Bounce object
// automatically deals with contact chatter or "bounce", and
// it makes detecting changes very simple.
#define BOUNCE_TIME 30
#define BUTTON_COUNT 4

KB_Button buttons[BUTTON_COUNT] = {
  {0,'l', true,  Bounce(0, BOUNCE_TIME)},  // Lights
  {1,'b', false, Bounce(1, BOUNCE_TIME)},  // Brakes - can hold down
  {2,' ', true,  Bounce(2, BOUNCE_TIME)},  // Stage
  {3,'v', true,  Bounce(3, BOUNCE_TIME)},  // Camera view
};

void setup() {
  
  for(byte i = 0; i < BUTTON_COUNT; i++)
  {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }
  Keyboard.begin();
}

void loop() {
  
  // Read buttons that are handled as keyboard keys
  
  for(byte i = 0; i < BUTTON_COUNT; i++)
  {
    buttons[i]._button.update();
    if(buttons[i]._button.fallingEdge())
    {
      if(buttons[i].momentary)
      {
        Keyboard.write(buttons[i].key);
      }
      else
      {
        Keyboard.press(buttons[i].key);
      }
    }
    else if (buttons[i]._button.risingEdge())
    {
      Keyboard.release(buttons[i].key);
    }
  }
  
  // read analog inputs and set X-Y position
  /*
  Joystick.X(analogRead(0));
  Joystick.Y(analogRead(1));
  Joystick.sliderLeft(analogRead(2));
*/
  // a brief delay, so this runs 50 times per second
  //delay(10);
}

