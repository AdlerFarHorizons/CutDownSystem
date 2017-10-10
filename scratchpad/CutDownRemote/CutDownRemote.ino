#include <EEPROM.h>
#include <MsTimer2.h>

// ADD HYSTERESIS: charge capacitors until certain percent above 4.5 volts,
// then allow to drain until same percent below, then resume charging.

// Analog Pin Assignments
int vCutCapPin = A0; // Reads voltage from capacitor
int vBattPin = A2;
int vTempPin = A3;

// Digital Pin Assignments
int cutPin = 2;
int setSwitchPin = 3;
int startSwitchPin = 4;
int upSwitchPin = 5;
int downSwitchPin = 6;
int chVarSwitchPin = 7;
int chargeCap = 8; // Set to LOW to charge capacitor

// Charge Constants
float vRefStop = 4.4;
float vRefStart = 4.1;
float vCC = 3.3;
// define variable that keep track where the threshold is
float vThreshold = 4.1;
//float vBattRange = vRef * 4.092;
//float vCutCapRange = vRef * 2.0;
//float vBackupCapRange = vRef * 2.0;

// EEPROM Constants
float timeOhFactor = 0.1213; // 0.0333; // Empirical with 3 second waitTime
int maxEepromAddr = 1023; // ATMega328
String dataValues = "2*(T+75)(C), vB*20(V), vC*20(V), Cut";

// Cutdown variables
int cutDelayMins; // Time until drop
boolean isCut; // Line has been cut

// Wait time variables
int waitTime = 30000;
int ledFlashTime;

// Sample variables
int sampleCount;
int sampleNum;
int sampleTime;

// User defined variables
int varnum = 1;

// Charge variables
float vCharged;
float vCutCap;
float vBatt;
boolean isCharged;
boolean chgEnable;

// Sensor variables
int dataSampleInterval;
int sensType;

// Activity variables
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
  pinMode(chargeCap, OUTPUT);
  
  /*
  // LED Setup
  pinMode(LEDPinTime, OUTPUT);
  pinMode(LEDPinBatt, OUTPUT);
  pinMode(LEDPinCap, OUTPUT);
  pinMode(LEDPinMode, OUTPUT);
  pinMode(LEDPinTimer, OUTPUT);
  pinMode(LEDPinCharge, OUTPUT);
  */
  
  chgEnable = true;
  activateCut( false );
  isCut = false;
  setTimerLED( false );
  sensType = 0; // LM60
  sampleTime = 60; // ms
  float temp = ( 1000.0 * sampleTime ) / ( 1.0 * waitTime );
  sampleCount = (int)( 0.5 + temp);
  ledFlashTime = 10; // ms
  vCharged = 4.4; // When used for testing purposes set this to 0.0

  active = false;
  cutDelayMins = 0;
  sampleNum = 1;
  digitalWrite(chargeCap,HIGH); // default is not charging
  
  Serial.begin(9600);
  Serial.flush();
  
  MsTimer2::set( waitTime, timesUp );
}

void loop()
{ 
  // Every 3 seconds as defined by MSTime2
  while( active );
  //detachInterrupt( 1 );
  
  vBatt = analogRead( vBattPin ) * ((39.0+51.0)/39.0)* vCC / 1024.0;
  vCutCap = (analogRead( vCutCapPin ) * 2.0 * vCC ) / 1024.0;
  Serial.print("vCutCap:");Serial.println( vCutCap );
  Serial.print("vBatt:");Serial.println( vBatt );
  Serial.print("vThreshold:");Serial.println( vThreshold );
  // If discharging and below voltage reference, start charging
  if(vCutCap > vThreshold)
  {
    vThreshold = vRefStart;
    digitalWrite(chargeCap,HIGH);
  }  
  if (vCutCap < vThreshold)
  {
    vThreshold = vRefStop;
    digitalWrite(chargeCap,LOW);
  }
  delay(1000);
  /*
  if (isCharged & (vCutCap <= vRefStart))
  {
    //digitalWrite(LEDPinCharge, LOW);
    waitForCharge();
  }
  
  // If switch is pressed (assuming active HIGH), allow time to be set
  while (getModeSwitch())
  {
    //digitalWrite(LEDPinMode, HIGH);
    
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
    //digitalWrite(LEDPinTime, HIGH);
    //digitalWrite(LEDPinBatt, LOW);
    //digitalWrite(LEDPinCap, LOW);
    
    // Display Time
    Serial.print(cutDelayMins);
  }
  
  if (varnum == 2)
  {
    //digitalWrite(LEDPinTime, LOW);
    //digitalWrite(LEDPinBatt, HIGH);
    //digitalWrite(LEDPinCap, LOW);
    
    // Display Percent Charge on Battery
    Serial.print("Batt Charge");
  }
  
  if (varnum == 3)
  {
    //digitalWrite(LEDPinTime, LOW);
    //digitalWrite(LEDPinBatt, LOW);
    //digitalWrite(LEDPinCap, HIGH);
    
    // Display Percent Charge on Capacitors
    Serial.print("Percent Cap Charge");
  }
  
  //digitalWrite(LEDPinMode, LOW);
  
  if ( digitalRead(startSwitchPin) == HIGH )
  {
    active = true;
    //digitalWrite(LEDPinTimer, HIGH);
    
    if ( cutDelayMins > 0 )
    {
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
    
    else
    {
      active = false;
    }
  }
  
  if ( digitalRead(startSwitchPin) == LOW )
  {
    active = false;
  }
  
  else
  {
    flashTimerLED( ledFlashTime, 1 );
    boolean tmp = cutdownReceived();
    
    if ( tmp && chgEnable )
    {
      activateCut( tmp );
      chgEnable = false; // This branch only once
      isCut = true;
      delay(100);
    }
    
    if (sampleNum >= sampleCount)
    {
      float temp = readTemp( vTempPin, 0 );
      vBatt = analogRead( vBattPin ) / 1024.0;       
      float vCutCap = analogRead( vCutCapPin ) / 1024.0;
      
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
      
      // EEPROM TIMER CODE
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
  */ 
} // END OF VOID LOOP

// ACTIVATE CUTDOWN
void activateCut( boolean state )
{
  digitalWrite( cutPin, state );
}


// STOP TIMER
// Required for EEPROM timer
void timesUp()
{
  active = false;
}


// CUTDOWN CHECK
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


// SET TIMER LED
void setTimerLED( boolean state )
{
  if ( state )
  {
    //digitalWrite( LEDPinTimer, HIGH );
  }
  
  else
  {
    //digitalWrite( LEDPinTimer, LOW );
  }
}


// TIMER LED FLASH
void flashTimerLED( int flashTime, int numFlashes )
{
  for ( int i = 0 ; i < numFlashes ; i++ )
  {
    //digitalWrite( LEDPinTimer, HIGH );
    delay( flashTime );
    //digitalWrite( LEDPinTimer, LOW );
    delay(100);
  }
}


// GET TIMER ACTIVITY
boolean isActive()
{
  return( active );
}


// ACTIVATE TIMER
void setActive()
{
  active = true;
}


// SET MODE
// Is "set" switch pressed?
boolean getModeSwitch()
{
  // Assuming set switch is active low
  return( digitalRead( setSwitchPin ) == LOW );
}


// TEMPERATURE
// Reading temperature sensor
float readTemp( int pin, int sensType )
{
  // Temperature Sensor constants:
  //   0  LM60
  //   1  MAX6605
  //   2  TMP36
  int mVoltsAtRefTemp[] = { 424, 744, 750 };
  int refTempC[] = { 0, 0, 25 };
  float mVperDegC[] = { 6.25, 11.9, 10.0 };

  float mVolts = analogRead(vTempPin);

  return(mVolts);
}


// HYSTERESIS
// If not charged, call this method to wait until charged
void waitForCharge()
{
  isCharged = false;
  digitalWrite(chargeCap, HIGH);
  vCutCap = analogRead( vCutCapPin ) / 1024.0;
  vBatt = analogRead( vBattPin ) / 1024.0;
 
  while ( vCutCap < vRefStop )
  {
    delay(40);
    Serial.print("Battery Charge = ");
    Serial.print(vBatt);
    Serial.print("V, ");
    Serial.print("Capacitor Charge = ");
    Serial.print(vCutCap);
    Serial.println("V. Waiting for full capacitor charge...");
    Serial.flush();
    
    delay(5000);
    flashTimerLED( ledFlashTime, 1 );
    
    vCutCap = analogRead( vCutCapPin ) / 1024.0;
    vBatt = analogRead( vBattPin ) / 1024.0;
  }
  
  isCharged = true;
  digitalWrite(chargeCap, LOW);
  //digitalWrite(LEDPinCharge, HIGH);
}


// TROUBLESHOOTING
// If code is not working properly, run this to see if too much ram is being used
int freeRam() 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

