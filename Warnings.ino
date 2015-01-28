
// Handle warnings

// Constants for type and timing
const byte WARNING_PIN = 23;
const byte WARNING_CLEAR_PIN = 22;
Bounce clear_button(WARNING_CLEAR_PIN, BOUNCE_TIME);

const float LOW_RESOURCE = 0.05;
const float NO_RESOURCE = 0.01;

// Severity levels
const byte WARN_OFF = 0;  // Nothing
const byte WARN_WARN = 1; // Hey.
const byte WARN_ALARM = 2;// HEY
const byte WARN_IN_PROGRESS = 10; // Don't go again

// Check for warnings every 1/2 second
Metro warningTimer = Metro(500);

volatile byte warn_level;

// Check function
typedef boolean (*check_func)();

// Struct for warnings
typedef struct {
  char message[16];
  byte severity;
  boolean triggered;
  boolean silence;
  check_func check;
} KP_Warning;

// 

// Define warnings and their severity
KP_Warning warnings[] = {
  {"Low fuel     ", WARN_WARN,  false, false, &low_fuel},
  {"Low power    ", WARN_WARN, false, false, &low_power},
  {"NO POWER     ", WARN_ALARM, false, false, &no_power}
};
const int warning_count = sizeof(warnings) / sizeof(warnings[0]);

// Initialize warnings
void warning_init()
{
  pinMode(WARNING_CLEAR_PIN, INPUT_PULLUP);
  pinMode(WARNING_PIN, OUTPUT);
  digitalWrite(WARNING_PIN, HIGH);
}

// Reset silenced warnings
void warning_reset()
{
  Serial1.print(0xFE, BYTE);
  //Serial1.print(0x01, BYTE);
  Serial1.write(0x80);         // new line
  Serial1.print("Alarm reset");
  for(byte i = 0; i < warning_count; i++)
  {
    warnings[i].silence = false;
    warnings[i].triggered = false;
  }
}

// Silence silenced warnings
void warning_silence()
{
  Serial1.print(0xFE, BYTE);
  //Serial1.print(0x01, BYTE);
  Serial1.write(0x80);         // new line
  Serial1.print("Alarm silence");
  for(byte i = 0; i < warning_count; i++)
  {
    warnings[i].silence = true;
  }
}
  
  
/**
 * Check various stuff like if we're still connected to KSP, fuel in the tank, etc.
 */
void warning_check()
{
  // This is for bounce
  clear_button.update();
;
  // SILENCE
  if(clear_button.fallingEdge())
  {
    warn_level == WARN_ALARM? warning_silence() : warning_reset();
    warn_level = WARN_OFF;
    digitalWrite(WARNING_PIN, HIGH);
  }
  
  // If it's not time yet, return
  if(!warningTimer.check()) return;
  
  // Don't bother if we're alreading doing one
  if(warn_level == WARN_IN_PROGRESS)
  {
    warning();
    return;
  }
  else if (warn_level == WARN_OFF)
  {
    Serial1.write(0x80);         // new line
    Serial1.print("                ");
  }
  
  // Check to see if we're connected to KSP
  if(KSP.connected())
  {
    digitalWrite(KSP_CONNECTED_LED, HIGH);
  }
  else
  {
    // Turn on connection light
    digitalWrite(KSP_CONNECTED_LED, LOW);
    
    // Turn off all status lights
    for(byte i = 0; i < light_count; i++)
    {
      digitalWrite(lights[i].pin, HIGH);
    }
    
    // Reset warnings, might be ship changing
    warning_reset();
    return;
  }
  for(byte i = 0; i < warning_count; i++)
  {
    if(warnings[i].silence) continue;
    
    if(!warnings[i].triggered && warnings[i].check())
    {
      warnings[i].triggered = true;
      warn_level = warnings[i].severity;
      
      
      // Warnings auto-silence
      if(warn_level == WARN_WARN)
      {
        warnings[i].silence = true;
      }
    }
    if(warnings[i].triggered)
    {
      
      Serial1.print(0xFE, BYTE);
      Serial1.print(0x01, BYTE);
      Serial1.print(warnings[i].message);
    }
  }
  if(warn_level || digitalRead(WARNING_PIN) == LOW)
  {
    warning();
  }
}

/**
 * Sound a warning according to current level
 */
void warning()
{
  noInterrupts();
  switch (warn_level) {
    case WARN_IN_PROGRESS:
      // Should be done now
      warn_level = WARN_OFF;
      
      warningTimer.interval(500);
      // Fall through
    case WARN_OFF:
      noTone(WARNING_PIN);
      digitalWrite(WARNING_PIN, HIGH);
      break;
    case WARN_WARN:
      tone(WARNING_PIN, 150, 1000);
      warn_level = WARN_IN_PROGRESS;
      // Come back after 1 second
      warningTimer.interval(1000);
      break;
    case WARN_ALARM:
      // Constant
      tone(WARNING_PIN, 2);
      break;
  }
  interrupts();
}

// Check for low fuel
boolean low_fuel()
{
  return KSP.vessel.LiquidFuelTot > 0 && (KSP.vessel.LiquidFuel / KSP.vessel.LiquidFuelTot <= LOW_RESOURCE);
}

// Check for low power
boolean low_power()
{
  return KSP.vessel.EChargeTot > 0 && (KSP.vessel.ECharge / KSP.vessel.EChargeTot <= LOW_RESOURCE);
}
// Check for no power
boolean no_power()
{
  return KSP.vessel.EChargeTot > 0 && (KSP.vessel.ECharge / KSP.vessel.EChargeTot <= NO_RESOURCE);
}
