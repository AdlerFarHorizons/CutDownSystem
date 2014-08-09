#include <EEPROM.h>
#include <MsTimer2.h>

// ADD HYSTERESIS: charge capacitors until certain percent above 4.5 volts,
// then allow to drain until same percent below, then resume charging.

// Analog Pin Assignments
int vCutCapPin = 3;
int vBattPin = 4;
int vTempPin = 5;

// Digital Pin Assignments
int cutPin = 2;
int setSwitchPin = 3;
int startSwitchPin = 4;
int upSwitchPin = 5;
int downSwitchPin = 6;
int chVarSwitchPin = 7;

int LEDPinTime = 8;
int LEDPinBatt = 9;
int LEDPinCap = 10;
int LEDPinMode = 11;
int LEDPinCharge = 12;
int LEDPinTimer = 13;

// Charge Constants
float vRef = 3.3;
float vBattRange = vRef * 4.092;
float vCutCapRange = vRef * 2.0;
float vBackupCapRange = vRef * 2.0;
float vBatt;

// EEPROM Constants
float timeOhFactor = 0.1213; // 0.0333; // Empirical with 3 second activeWaitTime
int maxEepromAddr = 1023; // ATMega328
String dataValues = "2*(T+75)(C), vB*20(V), vC*20(V), Cut";

// Cutdown variables
int cutDelayMins; // Time until drop
boolean isCut; // Line has been cut

// Wait time variables
int standbyWaitTime;
int activeWaitTime;
int waitTime;
int ledFlashTime;

// Sample variables
int sampleCount;
int sampleNum;
int sampleTime;

// User defined variables
int varnum = 1;

// Charge variables
float vCharged;
boolean isCharged;
boolean chgEnable;

// Sensor variables
int dataSampleInterval;
int sensType;

// Activity variables
boolean standby;
boolean active;

// Necessary for EEPROM
int eepromAddr;

void setup()
{
  // Switches & Cut Pin
  pinMode(cutPin, OUTPUT);
  pinMode(setSwitchPin, INPUT_PULLUP );
  pinMode(startSwitchPin, INPUT_PULLUP);
  pinMode(upSwitchPin, INPUT_PULLUP);
  pinMode(downSwitchPin, INPUT_PULLUP);
  pinMode(chVarSwitchPin, INPUT_PULLUP);
  
  // LED Setup
  pinMode(LEDPinTime, OUTPUT);
  pinMode(LEDPinBatt, OUTPUT);
  pinMode(LEDPinCap, OUTPUT);
  pinMode(LEDPinMode, OUTPUT);
  pinMode(LEDPinTimer, OUTPUT);
  pinMode(LEDPinCharge, OUTPUT);
  
  chgEnable = true;
  activateCut( false );
  isCut = false;
  setTimerLED( false );
  sensType = 0; // LM60
  standbyWaitTime = 30000; // ms
  activeWaitTime = 3000; // ms
  sampleTime = 60; // ms
  float temp = ( 1000.0 * sampleTime ) / ( 1.0 * activeWaitTime );
  sampleCount = (int)( 0.5 + temp);
  ledFlashTime = 10; // ms
  vCharged = 4.1; // When used for testing purposes set this to 0.0

  active = false;
  standby = true;
  waitTime = standbyWaitTime;
  cutDelayMins = 0;
  sampleNum = 1;
  
  Serial.begin(9600);
  Serial.flush();
  
  MsTimer2::set( waitTime, timesUp );
}

void loop()
{ 
  // Every 3 seconds as defined by MSTime2
  while( active );
  detachInterrupt( 1 );
  
  vBatt = vBattRange * analogRead( vBattPin ) / 1024.0;
  
  // If not charged, turn off LED and wait till charged
  if (!isCharged )
  {
    digitalWrite(LEDPinCharge, LOW);
    waitForCharge();
  }
  
  // If switch is pressed (assuming active HIGH), allow time to be set
  while (getModeSwitch())
  {
    digitalWrite(LEDPinMode, HIGH);
    
    // Display Current Timer Value
    Serial.print(cutDelayMins);
    
    if (digitalRead(upSwitchPin) == HIGH)
    {
      cutDelayMins += 1;
    }
    if (digitalRead(downSwitchPin) == HIGH)
    {
      cutDelayMins -= 1;
    }
  }
  
  if (digitalRead(chVarSwitchPin) == HIGH)
  {
    if(varnum == 3)
    {
      varnum = 1;
    }
    else
    {
      varnum += 1;
    }
  }
  
  if (varnum == 1)
  {
    digitalWrite(LEDPinTime, HIGH);
    digitalWrite(LEDPinBatt, LOW);
    digitalWrite(LEDPinCap, LOW);
    
    // Display Time
    Serial.print(cutDelayMins);
  }
  
  if (varnum == 2)
  {
    digitalWrite(LEDPinTime, LOW);
    digitalWrite(LEDPinBatt, HIGH);
    digitalWrite(LEDPinCap, LOW);
    
    // Display Percent Charge on Battery
    Serial.print("Batt Charge");
  }
  
  if (varnum == 3)
  {
    digitalWrite(LEDPinTime, LOW);
    digitalWrite(LEDPinBatt, LOW);
    digitalWrite(LEDPinCap, HIGH);
    
    // Display Percent Charge on Capacitors
    Serial.print("Cap Charge");
  }
  
  digitalWrite(LEDPinMode, LOW);
  
  if ( digitalRead(startSwitchPin) == HIGH )
  {
    standby = false;
    active = true;
    eepromAddr = 1;
    EEPROM.write( 0, eepromAddr ); // Initial eeprom address
  }
  
  if ( digitalRead(startSwitchPin) == LOW )
  {
    standby = true;
    active = false;
  }
  
  delay(100);
  
  if (standby)
  {
    digitalWrite(LEDPinTimer, LOW);
    
    if ( cutDelayMins > 0 )
    {
      standby = false;
      active = true;
      digitalWrite(LEDPinTimer, HIGH);
      waitTime = activeWaitTime;
      
      Serial.println("Timer is now active.");
      Serial.println("");
      Serial.flush();
      Serial.write('X');
      Serial.print(cutDelayMins);
      Serial.print(",");
      Serial.println( "Min, T(C), Vbat(V), Vcut(V), Cut");
      Serial.flush();
      eepromAddr = 1;
      EEPROM.write( 0, eepromAddr ); // Initial eeprom address
    }
  } 
  
  else
  {
    flashTimerLED( ledFlashTime, 1 );
    
    waitTime = activeWaitTime;
    boolean tmp = cutdownReceived();
    
    if ( tmp && chgEnable )
    {
      chgEnable = false; // This branch only once
      isCut = true;
      delay(100);
    }
    
    activateCut( tmp );
    if (sampleNum >= sampleCount)
    {
      float temp = readTemp( vTempPin, 0 );
      vBatt = vBattRange * analogRead( vBattPin ) / 1024.0;       
      float vCutCap = vCutCapRange * analogRead( vCutCapPin ) / 1024.0;
      
      Serial.print( cutDelayMins );
      Serial.print( ", ");       
      Serial.print( temp );
      Serial.print( ", ");
      Serial.print( vBatt );
      Serial.print( ", ");
      Serial.print( vCutCap );
      Serial.print( ", ");
      Serial.println( tmp );
      Serial.flush();
      
      temp = 2.0 * ( temp + 75.0 ); // Shift temperature range
      // Constrain readings to byte values
      if ( temp > 255 )
      {
        temp = 255;
      }
      
      if ( temp < 0 ) temp = 0;
      {
        vBatt /= 0.05;
      }
      
      if ( vBatt > 255 )
      {
        vBatt = 255;
      }
      
      if ( vBatt < 0  )
      {
        vBatt = 0;
      }
      
      vCutCap /= 0.05;
      
      if ( vCutCap > 255 )
      {
        vCutCap = 255;
      }
      
      if ( vCutCap < 0  )
      {
        vCutCap = 0;
      }
      
      // If out of eeprom, keep overwriting the last set of samples
      if ( eepromAddr > maxEepromAddr )
      {
        eepromAddr = ( maxEepromAddr - 3 );
      }
      EEPROM.write( eepromAddr, byte( temp ) );
      eepromAddr +=1;
      EEPROM.write( eepromAddr, byte( vBatt ) );
      eepromAddr +=1;
      EEPROM.write( eepromAddr, byte( vCutCap ) );
      eepromAddr +=1;
      EEPROM.write( eepromAddr, byte( isCut ) );
      EEPROM.write( 0, byte(eepromAddr) );
      eepromAddr += 1;
      sampleNum = 0;
    }
    
    sampleNum += 1;
  }
  
  Serial.flush();
}

// Activate
void activateCut( boolean state )
{
  digitalWrite( cutPin, state );
}

// Required for MSTimer2
void timesUp()
{
  active = false;
}

// Has the remote cutdown order ('C' byte) been recieved?
boolean cutdownReceived()
{
  boolean isReceived = false;
  for (int i = 0; i < 10; i++) 
  {
    if (Serial.available() > 0)
    {
      int incomingByte = Serial.read();
      if (incomingByte == 'C')
      {
        isReceived = true;
        Serial.println("Cutdown command received.");
        Serial.flush();
      }
    }
    delay(5);
  }
  return isReceived;
}

// Method to set Timer LED
void setTimerLED( boolean state )
{
  if ( state )
  {
    digitalWrite( LEDPinTimer, HIGH );
  }
  
  else
  {
    digitalWrite( LEDPinTimer, LOW );
  }
}

// Makes Timer LED flash when Timer is active
void flashTimerLED( int flashTime, int numFlashes )
{
  for ( int i = 0 ; i < numFlashes ; i++ )
  {
    digitalWrite( LEDPinTimer, HIGH );
    delay( flashTime );
    digitalWrite( LEDPinTimer, LOW );
    delay(100);
  }
}

// Is timer active?
boolean isActive()
{
  return( active );
}

// Timer is now active.
void setActive()
{
  active = true;
}

// Is timer in standby?
boolean isStandby()
{
  return( standby );
}

// Is "Set" switch pressed?
boolean getModeSwitch()
{
  // Assuming set switch is active high
  return( digitalRead( setSwitchPin ) == HIGH );
}

// Read internal temperature
float readTemp( int pin, int sensType )
{
  // Temperature Sensor constants:
  //   0  LM60
  //   1  MAX6605
  //   2  TMP36
  int mVoltsAtRefTemp[] = { 424, 744, 750 };
  int refTempC[] = { 0, 0, 25 };
  float mVperDegC[] = { 6.25, 11.9, 10.0 };

  int reading = analogRead(vTempPin);
  float mVolts = reading * vRef / 1.024;

  return( ( mVolts - mVoltsAtRefTemp[sensType] ) / 
            ( mVperDegC[sensType] ) + 
            refTempC[sensType]);
}

// If not charged, call this method to wait until fully charged
void waitForCharge()
{
  float vCutCap = vCutCapRange * analogRead( vCutCapPin ) / 1024.0;
  vBatt = vBattRange * analogRead( vBattPin ) / 1024.0;
 
  while ( vCutCap <= vCharged )
  {
    delay(40);
    Serial.print("Vbat = ");
    Serial.print(vBatt);
    Serial.print("V, ");
    Serial.print("Vcut = ");
    Serial.print(vCutCap);
    Serial.println("V. Waiting for full charge...");
    Serial.flush();
    
    delay(10000);
    flashTimerLED( ledFlashTime, 1 );
    vCutCap = vCutCap = vCutCapRange * analogRead( vCutCapPin ) / 1024.0; 
    vBatt = vBattRange * analogRead( vBattPin ) / 1024.0;       
  }
  
  isCharged = true;
  digitalWrite(LEDPinCharge, HIGH);
}

// If code is not working properly, run this to see if too much ram is being used
int freeRam() 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

