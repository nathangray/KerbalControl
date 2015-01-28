
/* Kerbal Space Program controller

   You must select Serial+Keyboard+Mouse+Joystick from the "Tools > USB Type" menu

   Adapted from Teensy example code
*/

//#define DEBUG(message) Serial1.println(message)
#ifndef DEBUG(message)
#define DEBUG(message)
#endif

// Serial communications direct to KSP
#include "KSPSerialIO.h"

// For software debouncing button presses
#include <Bounce.h>

// For scheduling things
#include <Metro.h>

// Graphic LCD
#include <ST7565.h>
  
// KPS connection indicator
const byte KSP_CONNECTED_LED = 2;

// Structs for keyboard buttons
typedef struct {
  byte pin;
  boolean toggle;      // TRUE if pressing it changes state, false for momentary
  byte joy_button;     // Joystick buttons are 1 based
  byte control_code;   // KSPSerial control code
  Bounce _button;
} KB_Button;


// Create Bounce objects for each button.  The Bounce object
// automatically deals with contact chatter or "bounce", and
// it makes detecting changes very simple.
#define BOUNCE_TIME 50
#define BUTTON_COUNT 5
KB_Button buttons[] = {
//PIN toggle joy ctrl   bounce(pin,BOUNCE_TIME)
  {3, false, 1,  0xFF,   Bounce(3, BOUNCE_TIME)},
  {4, false, 2,  STAGE,  Bounce(4, BOUNCE_TIME)},
  {5, true,  3,  0xFF,   Bounce(5, BOUNCE_TIME)},
  {6, true,  4,  GEAR,   Bounce(6, BOUNCE_TIME)},
  {7, false, 5,  BRAKES, Bounce(7, BOUNCE_TIME)},
  {19,false, 5,  LIGHTS, Bounce(19, BOUNCE_TIME)},
  {20,false, 5,  SAS,    Bounce(20, BOUNCE_TIME)},
  {21,false, 5,  RCS,    Bounce(21, BOUNCE_TIME)},
};

// Structs for lights
typedef struct {
  byte pin;
  byte status_flag;
} KB_LED;

// Define pins and their corresponding control flag
KB_LED lights[] = {
  { 8, AGRCS},
  { 9, AGSAS},
  {10, AGLight},
  {11, AGGear},
  {12, AGBrakes},
};
const int light_count = sizeof(lights) / sizeof(lights[0]);


KSPSerialIO KSP(115200);

void setup() {
  
  // These need to start, I guess
  Keyboard.begin();
  Joystick.begin();
  
  // Using a Sparkfun serial backpack on Serial1
  Serial1.begin(9600);
  Serial1.print(0xFE, BYTE);
  Serial1.print(0x01, BYTE);
  
  // Init system status
  pinMode(KSP_CONNECTED_LED, OUTPUT);
  digitalWrite(KSP_CONNECTED_LED, HIGH);
  warning_init();
  
  // Initialize pins - control lights
  for(byte i = 0; i < light_count; i++)
  {
    pinMode(lights[i].pin, OUTPUT);
    // Turn on lights so we know it's on
    digitalWrite(lights[i].pin, LOW);
  }
  
  // Initialize pins - buttons
  for(byte i = 0; i < BUTTON_COUNT; i++)
  {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }
  
  // Turn off lights
  delay(500);
  for(byte i = 0; i < light_count; i++)
  {
    digitalWrite(lights[i].pin, HIGH);
  }
    
  // No hat, make sure it's disabled
  Joystick.hat(-1);
  
  Serial1.print("Good morning,   Dave");
  delay(1000);
}

void loop() {
  // Check for serial info, send direct commands
  KSP.update();
  
  // Check for warnings
  warning_check();
  
  // Status LEDs, etc.
  update_status();
  
  update_buttons();
  
  set_axies();
  
  // Check touch
  getTouch();
  
}

/**
 * Update LEDs and displays with the latest information
 */
void update_status() {
  // If there's no data, not much we can do
  if(!KSP.connected()) return;
  
  // Update status lights
  for(byte i = 0; i < light_count; i++)
  {
    digitalWrite(lights[i].pin, !KSP.controlStatus(lights[i].status_flag));
  }
  
  // Display AP (& PE)
  Serial1.print(0xFE, BYTE);
  //Serial1.print(0x01, BYTE);
  Serial1.write(192);         // new line
  Serial1.print("AP: ");Serial1.print(KSP.vessel.AP);
  /*
  Serial1.print(0xFE,BYTE);   //command flag
  Serial1.write(192);         // new line
  Serial1.print("PE: ");Serial1.print(KSP.vessel.PE);
  
  // Display Electric
  /*
  Serial1.print(0xFE, BYTE);
  Serial1.print(0x01, BYTE);
  Serial1.print(KSP.vessel.ECharge); Serial1.print("/"); Serial1.print(KSP.vessel.EChargeTot);
  */
}

/**
 * Send commands & stuff
 */
void update_buttons()
{
  
  // Read buttons that are handled as keyboard keys
  for(byte i = 0; i < BUTTON_COUNT; i++)
  {
    // This is for bounce
    buttons[i]._button.update();
    
    if(buttons[i]._button.fallingEdge())
    {
      
      // Can't fake a momentary joystick button press without timing
      // but that's OK because joysticks don't have key repeat
      if(buttons[i].joy_button)
      {
        Joystick.button(buttons[i].joy_button,1);
      }
      if(buttons[i].control_code != 0xFF)
      {
        if(!buttons[i].toggle)
        {
          KSP.setControl(buttons[i].control_code, true);
        }
        else
        {
          // Things get a little more complicated here because the command & status are different
          byte value = true;
          switch(buttons[i].control_code)
          {
            case SAS: value=KSP.controlStatus(AGSAS); break;
            case RCS: value=KSP.controlStatus(AGRCS); break;
            case LIGHTS: value=KSP.controlStatus(AGLight); break;
            case GEAR: value=KSP.controlStatus(AGGear); break;
            case BRAKES: value=KSP.controlStatus(AGBrakes); break;   
            case ABORT: value=KSP.controlStatus(AGAbort); break;
          }
          KSP.setControl(buttons[i].control_code, !value);
        }
      }
    }
    else if (buttons[i]._button.risingEdge())
    {
      if(buttons[i].joy_button)
      {
        Joystick.button(buttons[i].joy_button,0);
      }
      if(buttons[i].control_code != 0xFF && !buttons[i].toggle)
      {
        KSP.setControl(buttons[i].control_code, false);
      }
    }
  }
}

/**
 * Read & set axis
 */
void set_axies()
{
  // read analog inputs and set X-Y position
  Joystick.X(analogRead(0));
  Joystick.Y(analogRead(1));
  // Throttle
  Joystick.sliderLeft(analogRead(2));
}


/**
 * Check and handle any touch controls
 */
void getTouch()
{
  for(byte i = 17; i < 19; i++)
  {
    Joystick.button(i, touchRead(i) > 3000);
  }
}
