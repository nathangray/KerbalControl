/* Kerbal Space Program controller

   You must select Serial+Keyboard+Mouse+Joystick from the "Tools > USB Type" menu

   Adapted from Teensy example code
*/

#define DEBUG(message) Serial.println(message)

// For software debouncing button presses
#include <Bounce.h>

// Structs for keyboard buttons
typedef struct {
  byte pin;
  byte key;
  byte joy_button;     // Joystick buttons are 1 based
  boolean momentary;
  Bounce _button;
} KB_Button;


// Create Bounce objects for each button.  The Bounce object
// automatically deals with contact chatter or "bounce", and
// it makes detecting changes very simple.
#define BOUNCE_TIME 30
#define BUTTON_COUNT 5
KB_Button buttons[] = {
//PIN  kbrd or joy momentary  bounce(pin,BOUNCE_TIME)
  {0,/*'u'*/0, 1,   true,  Bounce(0, BOUNCE_TIME)},
  {1,/*'b'*/0, 2,   false, Bounce(1, BOUNCE_TIME)},
  {2,/*' '*/0, 3,   true,  Bounce(2, BOUNCE_TIME)},
  {3,/*'v'*/0, 4,   true,  Bounce(3, BOUNCE_TIME)},
  {4,/*'v'*/0, 5,   true,  Bounce(4, BOUNCE_TIME)},
};

// Structs for lights
typedef struct {
  byte pin;
} KB_LED;

// These should be tied to something logically...
KB_LED lights[] = {
  {11}, // 
  {12}
};

void setup() {
  // Initialize pins - lights
  for(byte i = 0; i < sizeof(lights); i++)
  {
    pinMode(lights[i].pin, OUTPUT);
    // Turn on lights so we know it's on
    digitalWrite(lights[i].pin, LOW);
  }
  // Initialize pins - buttons
  for(byte i = 0; i < sizeof(buttons); i++)
  {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }
    
  // These need to start, I guess
  Keyboard.begin();
  Joystick.begin();
  #ifdef DEBUG
  Serial.begin(9600);
  #endif
  
  // Turn off lights
  delay(500);
  for(byte i = 0; i < sizeof(lights); i++)
  {
    digitalWrite(lights[i].pin, HIGH);
  }
  
  // No hat, make sure it's disabled
  Joystick.hat(-1);
  
  DEBUG("Good morning, Dave");
}

void loop() {
  // Read buttons that are handled as keyboard keys
  for(byte i = 0; i < BUTTON_COUNT; i++)
  {
    buttons[i]._button.update();
    if(buttons[i]._button.fallingEdge())
    {
      if(i < 2) digitalWrite(lights[i].pin, LOW);
      
      // Can't fake a momentary joystick button press without timing
      // but that's OK because joysticks don't have key repeat
      if(buttons[i].momentary && buttons[i].key)
      {
        if(buttons[i].key)
        {
          Keyboard.write(buttons[i].key);
        }
      }
      else
      {
        if(buttons[i].key)
        {
          Keyboard.press(buttons[i].key);
        }
        else
        {
          Joystick.button(buttons[i].joy_button,1);
        }
      }
    }
    else if (buttons[i]._button.risingEdge())
    {
      if(i < 2) digitalWrite(lights[i].pin, HIGH);
      if(buttons[i].key)
      {
        Keyboard.release(buttons[i].key);
      }
      else
      {
        Joystick.button(buttons[i].joy_button,0);
      }
    }
  }
  
  // read analog inputs and set X-Y position
  
  Joystick.X(analogRead(0));
  Joystick.Y(analogRead(1));
  // Throttle
  Joystick.sliderLeft(analogRead(2));

  // Check touch
  getTouch();
  // a brief delay, so this runs 50 times per second
  //delay(20);
}

/**
 * Check and handle any touch controls
 */
void getTouch()
{
  for(byte i = 23; i < 24; i++)
  {
    DEBUG(touchRead(i));
    Joystick.button(i, touchRead(i) > 3000);
  }
}
