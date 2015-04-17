#include <SoftwareSerial.h>

SoftwareSerial ss(12,13);
int redPin = 5;
int grnPin = 6;
int armPin = 8;
int cutPin = 9;
boolean isArmed, isReady, isComm, isCut, cutConfirm;
boolean grnLed, redLed;
int remoteCount;
void setup() {

  pinMode( redPin, OUTPUT );
  pinMode( grnPin, OUTPUT );
  pinMode( armPin, INPUT_PULLUP );
  pinMode( cutPin, INPUT_PULLUP );
  
  isArmed = false;
  isReady = false;
  isComm = false;
  cutConfirm = false;
  remoteCount = 0;
  grnLed = false;
  redLed = false;
  ss.begin(9600);
  Serial.begin(9600);
  while ( Serial.available() ) {
    Serial.read();
  }
  
}

void loop() {

  //Green LED: G
  //Red LED: R
  //
  //if (ready == F && communication == T) 
  //   //sender and remote are communicating
  //   G solid
  //
  //if (ready == T && communication == T){
  //   G flashing
  //   if(armButton == T)
  //       R solid
  //   else if(armbutton == T && cutButton == T)
  //       R flashing
  //   
  //   if(D is received)
  //       R off
  //} else 
  //   G off

  char in;
  if ( ss.available() ) {
    remoteCount = 4;
  } else {
    remoteCount--;
    if ( remoteCount < 0 ) remoteCount = 0 ;
  }
  //Serial.println( remoteCount );
  isComm = remoteCount > 0;
  while ( ss.available() ) {
    in = ss.read();
    if ( in == 'B' ) {
      Serial.println("");Serial.print(" Batt:");
    } else if ( in == 'C' ) {
      Serial.println("");Serial.print("  Cap:");
    } else if ( in == 'R' ) {
      Serial.println("");Serial.print("Ready...");
      isReady = true;
    } else if ( in == 'W' ) {
      Serial.println("");Serial.print("Wait...");
      isReady = false;
    } else if ( in == 'D' ) {
      Serial.println("");Serial.print("Cut!!!!");
      cutConfirm = true;
      isCut = false;
      isArmed = false;
    } else {
      Serial.write( in );
    }
  }
  if ( !isReady && isComm ) {
    //sender and remote are communicating
    grnLed = true; //G solid
  }
  
  if ( isReady && isComm ){
    grnLed = !grnLed; //G flashing
    if( isArmed ) {
      redLed = true; // R solid
    } else if ( isArmed && isCut ) {
      redLed = !redLed; //  R flashing
    }
    if( isCut ) {
      redLed = false; //  R off
      grnLed = false; // G off (added 4/17/15)
    }
  }
  if ( cutConfirm ) {
    redLed = !redLed;
    Serial.println( "CUT CONFIRMED!" );
  }  
     
  if ( !isArmed && !cutConfirm ) {
    isArmed = !digitalRead( armPin );
  }
  if ( isArmed ) {
    isCut = !digitalRead( cutPin );
  }
  digitalWrite( grnPin, grnLed );
  digitalWrite( redPin, redLed );
  if ( isCut ) { // Transmit 'C' every 20ms for 4 seconds
    for ( int i = 0 ; i < 400 ; i++ ) {
      ss.write( 'C' );
      delay( 10 );
    }
    isCut = false;
  } else {
    delay(1000);
  }
  
  if ( !isComm ) {
    grnLed = false; // G off (added 4/17/15)
  }
  


