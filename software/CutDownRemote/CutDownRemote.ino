#include <EEPROM.h>

#include <Narcoleptic.h>

/*
 * CutDownRemote
 *
 * Lou Nigra
 * Adler Planetarium - Far Horizons
 *
 * Brendan Batliner and Milan Shah
 * Illinois Mathematics and Science Academy - SIR Program
 * 
 * Controls:
 *  Raw voltage tied to charger ON switch
 *  Programming mode when serial port is connected
 *  Timer activate/de-activate momentary contact switch (no longer used)
 *
 * Indicators:
 *  LED no blink: Charging => Programming mode inhibited.
 *  LED fast blink: Active => Timer is counting down.
 *  LED long Off: Standby => Waiting for programming window to open.
 *  LED single three flashes: Programming mode => Window open for "non-D"
 *                          input to enter programming mode.
 *
 *  The following sequence follows if a non-D character is entered while
 *  the window is open:
 *
 *  LED single six flashes: Parameter prompt: => Ready for parameter entry.
 *  LED three flashes: Confirmation: => Ready for y or n
 *
 * State Machine:
 *  standby -> WAITFORCHARGE
 *  WAITFORCHARGE
 *    -> GETTTY
 *  GETTTY
 *    (timeout) -> SLEEP
 *    (entries) -> active -> SLEEP
 *  SLEEP
 *      standby? => TTY
 *      active?  -> timerUpdate
 */
// General constants 
float vRef = 3.3;
int cutPin = 10;
int vTempPin = 5;
int vBattPin = 4;
int pwrDnPin = 7;
int cutChgDisablePin = 8;
float vBattRange = vRef * 4.092;
int vCutCapPin = 3;
float vCutCapRange = vRef * 2.0;
int vBackupCapPin = 2;
float vBackupCapRange = vRef * 2.0;
int ledPin = 13;
int modePin = 9;
float timeOhFactor = 0.1213; //0.0333; //Empirical with 3 second activeSleepTime
int maxEepromAddr = 1023; //ATMega328
String dataValues = "2*(T+75)(C), vB*20(V), vC*20(V), Cut";
float vBatt;

// Variable declarations;
int cut;
int cutDelayMins;
int ttyPollTime;
int ttyWindowTimeSecs;
int standbySleepTime;
int activeSleepTime;
int sleepTime;
int sampleCount;
int sampleNum;
int sampleTime;
float cutTimerMins;
float maxAlt;
float maxRadius;
float center_lat;
float center_lon;
float vCharged;
boolean isCharged;
boolean ledState;
int ledFlashTime;
int dataSampleInterval;
int sensType;
boolean standby;
boolean active;
boolean chgEnable;
boolean isCut;
int eepromAddr;

void setup()
{
  pinMode(cutPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(modePin, INPUT_PULLUP );
  pinMode(pwrDnPin, OUTPUT);
  pinMode(cutChgDisablePin, OUTPUT);
  chgEnable = true;
  setCutChg( chgEnable );
  setCut( false );
  isCut = false;
  setLED( false );
  setPwrDown( true );
  sensType = 0; //LM60
  ttyPollTime = 6000; //ms
  ttyWindowTimeSecs = 10; //secs
  standbySleepTime = 30000; //ms
  activeSleepTime = 3000; //ms
  sampleTime = 60; //ms
  float temp = ( 1000.0 * sampleTime ) / ( 1.0 * activeSleepTime );
  sampleCount = (int)( 0.5 + temp);
  ledFlashTime = 10; //ms
  vCharged = 4.1; //When used for testing purposes set this to 0.0

  active = false;
  standby = true;
  sleepTime = standbySleepTime;
  cutDelayMins = 0;
  cutTimerMins = 0;
  sampleNum = 1;
  Serial.begin(9600);
  Serial.flush();

}

void loop() // run over and over again
{ 
  vBatt = vBattRange * analogRead( vBattPin ) / 1024.0;  
  if (!isCharged ) waitForCharge();
  
  setPwrDown(false);
  //Serial.println(freeRam()); // For testing
  delay(100);
  if (standby) {
    flashLED( ledFlashTime, 3 );
    cutDelayMins = getTTY( ttyPollTime, ttyWindowTimeSecs );
    if ( cutDelayMins > 0 ) {
      standby = false;
      active = true;
      sleepTime = activeSleepTime;
      Serial.println("Timer is now active.");
      Serial.println("");
      Serial.flush();
      Serial.write('X');
      Serial.print(cutDelayMins);
      Serial.print(",");
      Serial.print(maxAlt);
      Serial.print(",");
      Serial.print(maxRadius);
      Serial.print(",");
      Serial.print(center_lat);
      Serial.print(",");
      Serial.println(center_lon);
      Serial.println( "Min, T(C), Vbat(V), Vcut(V), Cut");
      Serial.flush();
      eepromAddr = 1;
      EEPROM.write( 0, eepromAddr ); //Initial eeprom address
    }
  } else {
    flashLED( ledFlashTime, 1 );
    
    sleepTime = activeSleepTime;
    boolean tmp = updateTimer() || cutdownReceived();
    if ( tmp && chgEnable ) {
      chgEnable = false; // This branch only once
      setCutChg( chgEnable ); // Disable cut cap charging if cut is imminent.
      isCut = true;
      delay(100);
    }
    setCut( tmp );
    if (sampleNum >= sampleCount) {
      float temp = readTemp( vTempPin, 0 );
      vBatt = vBattRange * analogRead( vBattPin ) / 1024.0;       
      float vCutCap = vCutCapRange * analogRead( vCutCapPin ) / 1024.0;       
      float vBackupCap = vBackupCapRange * analogRead( vBackupCapPin ) / 1024.0;
      Serial.print( cutTimerMins );Serial.print( ", ");       
      Serial.print( temp );Serial.print( ", ");
      Serial.print( vBatt );Serial.print( ", ");
      Serial.print( vCutCap );Serial.print( ", ");
      Serial.println( tmp );
      Serial.flush();
      temp = 2.0 * ( temp + 75.0 ); // Shift temperature range
      // Constrain readings to byte values
      if ( temp > 255 ) temp = 255; if ( temp < 0 ) temp = 0;
      vBatt /= 0.05; if ( vBatt > 255 ) vBatt = 255; if ( vBatt < 0  ) vBatt = 0;
      vCutCap /= 0.05;if ( vCutCap > 255 ) vCutCap = 255; if ( vCutCap < 0  ) vCutCap = 0;
      
      // If out of eeprom, keep overwriting the last set of samples
      if ( eepromAddr > maxEepromAddr ) eepromAddr = ( maxEepromAddr - 3 );
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
  setPwrDown( true );
  Narcoleptic.delay( sleepTime );
}

void setPwrDown( boolean state ) {
  digitalWrite( pwrDnPin, state );
}

void setCut( boolean state ) {
  digitalWrite( cutPin, state );
}

void setCutChg( boolean state ) {
  digitalWrite( cutChgDisablePin, !state );
}

boolean updateTimer() {
  cutTimerMins += ( ( 1 + timeOhFactor ) * activeSleepTime / 60000.0 );
  return( int(cutTimerMins) == cutDelayMins );
}

boolean cutdownReceived() {
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

void setLED( boolean state ) {
  if ( state ) {
    digitalWrite( ledPin, HIGH );
  } else {
    digitalWrite( ledPin, LOW );
  }
}

void flashLED( int flashTime, int numFlashes ) {
  for ( int i = 0 ; i < numFlashes ; i++ ) {
    digitalWrite( ledPin, HIGH );
    delay( flashTime );
    digitalWrite( ledPin, LOW );
    delay(100);
  }
  
}

boolean isActive() {
  return( active );
}

void setActive() {
  active = true;
}

boolean isStandby() {
  return( standby );
}

boolean getModeSwitch() {
  // Mode switch is active low
  return( digitalRead( modePin ) == LOW );
}

float readTemp( int pin, int sensType ) {
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

// Starts serial interface and waits for tty activity for a while to start
//  a dialog to get a new value for the global cutDelayMins cutdown time.
//  If no input, returns with value unmodified.
int getTTY( int pollTimeMs, int windowTimeSecs ) {
  int rcvdBytes[4];
  int rcvdBytesCnt;
  int timeDelay;
  float deadTime;
  boolean done;
  int inputByte;
  int timeOutCnt = 0;

  // Clear the input buffer
  while ( Serial.available() > 0 ) {
    Serial.read();
  }
  Serial.println(""); 
  Serial.print("Vbat = ");Serial.print(vBatt);Serial.println("V");
  Serial.println("Enter D to dump EEProm data, ");
  Serial.println("any other key to set up the timer...");
  Serial.flush();
  
  // Wait for an input, but only for windowTimeSecs
  deadTime = 0;
  done = false;
  while( Serial.available() <= 0 && !done ) {
    delay(pollTimeMs);
    flashLED(ledFlashTime, 3);
    delay(100);
    deadTime += pollTimeMs/ 1000.0;
    if ( deadTime > windowTimeSecs ) {
      Serial.println( "" );
      Serial.print( "Going back to sleep. Back in " );
      Serial.print( standbySleepTime / 1000 );Serial.println( " sec" );
      timeDelay = 0;
      done = true;
   }
  }
  
  if ( ( inputByte = Serial.read() ) == 68 ) {
    dumpData();
    return(0);
  } 
  while ( !done ) {
    // Clear the input buffer
    while ( Serial.available() > 0 ) {
      Serial.read();
    }
    Serial.println("");
    Serial.println("Ready for parameter settings.");
    Serial.println("");
    
    promptUserForData(&maxAlt, "max altitude", "feet");
    
    promptUserForData(&maxRadius, "max radius", "miles");
    
    promptUserForData(&center_lat, "launch latitude", "degrees");
    
    promptUserForData(&center_lon, "launch longitude", "degrees");
     
    float timer;    
    Serial.println( "NOTE: Timer entry is next.");
    Serial.println( "Once timer is confirmed cutdown system will be activated.");
    Serial.println("");
    promptUserForData(&timer, "timer duration", "minutes");
    timeDelay = int( timer + 0.5);
    flashLED( ledFlashTime, 6 );
    Serial.flush();
    done = true; 
  }
  return(timeDelay);  
}

void dumpData() {
  Serial.flush();
  Serial.println( dataValues );
  int lastAddr = int(EEPROM.read(0)) * 4;
  int addr = 1;
  while ( addr <= lastAddr ) {
    Serial.print(EEPROM.read(addr));Serial.print(", ");
    Serial.print(EEPROM.read(addr+1));Serial.print(", ");
    Serial.print(EEPROM.read(addr+2));Serial.print(", ");
    Serial.println(EEPROM.read(addr+3));
    addr += 4;
  }
  Serial.println("End");
  Serial.flush();
}

void waitForCharge() {
  float vCutCap = vCutCapRange * analogRead( vCutCapPin ) / 1024.0;
  vBatt = vBattRange * analogRead( vBattPin ) / 1024.0;       
  while ( vCutCap <= vCharged ) {
    setPwrDown( false );delay(40);
    Serial.print("Vbat = ");Serial.print(vBatt);Serial.print("V, ");
    Serial.print("Vcut = ");Serial.print(vCutCap);
    Serial.println("V. Waiting for full charge...");
    Serial.flush();
    setPwrDown( true );
    delay(10000);
    flashLED( ledFlashTime, 1 );
    vCutCap = vCutCap = vCutCapRange * analogRead( vCutCapPin ) / 1024.0; 
    vBatt = vBattRange * analogRead( vBattPin ) / 1024.0;       
  }
  isCharged = true; 
}

void promptUserForData(float * data, String dataName, String unit)
{
  Serial.print("Enter " + dataName + " of flight, in " + unit + ":");
    flashLED( ledFlashTime, 6 );
    *data = 0;
    boolean typing = true;
    while (typing) {
      while (Serial.available() > 0) {
        *data = Serial.parseFloat();
        Serial.println(*data); Serial.flush();
        if (*data != 0) {
          Serial.print(*data); Serial.println(" " + unit + " entered as the " + dataName + " ."); Serial.print("Is this correct? (y/n) ");
          Serial.flush();
          flashLED( ledFlashTime, 3 );
          int response = 0;
          while (response != 'y' && response != 'n') {
            response = Serial.read();
          }
          Serial.println((char)response);
          if (response == 'y') {
            Serial.println("Confirmed.");
            Serial.print(*data); Serial.println(" " + unit + "."); Serial.println("");
            Serial.flush();
            typing = false;
            while (Serial.available() > 0)
              Serial.read();
          }
          else {
            Serial.print("Enter " + dataName + " of flight, in " + unit);
            Serial.flush();
            while (Serial.available() > 0)
              Serial.read();
            *data = 0;
          }
        }
        else {
          Serial.println("Please enter a valid number.");
          Serial.flush();
        }
      }
    }
}

int freeRam() 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

  

