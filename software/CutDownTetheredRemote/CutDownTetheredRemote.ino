#include <EEPROM.h>

#include <MsTimer2.h>

/*
 * CutDownRemoteTemp
 *
 * Lou Nigra
 * Adler Planetarium - Far Horizons
 *
 * Brendan Batliner and Milan Shah
 * Illinois Mathematics and Science Academy - SIR Program
 * 
 * An interim design Upgraded for digital charging control, but
 * without the display and manual programming interface.
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
int cutChgPin = 8;
float vChgLow = 4.45; //Hysteresis low
float vChgHigh = 4.55; //Hysteresis high
int vTempPin = 3;
int vBattPin = 2;
int pwrDnPin = 7;
//int cutChgDisablePin = 8;
float vBattRange = vRef * ( 39 + 82 ) / 39.0 ;
int vCutCapPin = 0;
float vCutCapRange = vRef * (51 + 51 ) / 51.0; //2.0;
//int vBackupCapPin = 2;
//float vBackupCapRange = vRef * 2.0;
int ledPin = 13;
int modePin = 3;
float timeOhFactor = 0.0996; //0.1213; //0.0333; //Empirical with 3 second activeSleepTime
int maxEepromAddr = 1023; //ATMega328
String dataValues = "2*(T+75)(C), vB*20(V), vC*20(V), Cut";
float vBatt;
float vCutCap;

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
boolean sleep;
boolean chgEnable;
boolean cutActive;
boolean isCut;
int eepromAddr;

void setup()
{
  pinMode(cutPin, OUTPUT);
  pinMode(cutChgPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(modePin, INPUT_PULLUP );
  pinMode(pwrDnPin, OUTPUT);
  //pinMode(cutChgDisablePin, OUTPUT);
  setCut( false );
  setCutChg( false );
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
  active = false;
  standby = true;
  sleepTime = activeSleepTime;
  cutDelayMins = 0;
  cutTimerMins = 0;
  sampleNum = 1;
  Serial.begin(9600);
  Serial.flush();
  isCharged = false;
  MsTimer2::set( 3000, timesUp );
  cutActive = false; // Flag to limit active cutdown to one interrupt cycle.
  isCut = false; // Flag to make sure only one cutdown command is recognized
  vCutCap = vCutCapRange * analogRead( vCutCapPin ) / 1024.0;
  vBatt = vBattRange * analogRead( vBattPin ) / 1024.0;
  vCharged = vChgLow;
  //waitForCharge();
  MsTimer2::start();
}

void loop() // run over and over again
{ 
  if ( !sleep ) {    
    setPwrDown( false );
    if ( isCharged ) {
      Serial.write('R');
    } else {
      Serial.write('W');
    }
    Serial.write('B');Serial.print(vBatt);;
    Serial.write('C');Serial.print(vCutCap);
    sleep = true;
    if ( !isCut ) cutActive = cutdownReceived();
    setCut( cutActive );
    if ( cutActive ) {
      cutActive = false;
      isCut = true;
    }
    Serial.flush();
    setPwrDown( true );
  }
}

void setPwrDown( boolean state ) {
  digitalWrite( pwrDnPin, state );
  if ( !state) delay(100); // Allow 100 ms for xBee to turn on.
}

void setCut( boolean state ) {
  digitalWrite( cutPin, state );
}

void setCutChg( boolean state ) {
  //digitalWrite( cutChgDisablePin, !state );
}

void timesUp() {
  charge();
  sleep = false;
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
        Serial.write('D');
      }
    }
    delay(30);
  }
  Serial.flush();
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

void charge() {
  vCutCap = vCutCapRange * analogRead( vCutCapPin ) / 1024.0;
  vBatt = vBattRange * analogRead( vBattPin ) / 1024.0;
  if ( vCutCap < vChgLow ) {
    setCharge( HIGH );
  }
  if ( vCutCap > vChgHigh )  {
    setCharge( LOW );
  }
  isCharged = vCutCap > vCharged;
}

boolean setCharge( boolean charge ) {
  digitalWrite( cutChgPin, charge );
}

int freeRam() 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

  

