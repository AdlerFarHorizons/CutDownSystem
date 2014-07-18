BaseModule

v3.00 Updated to work with a Copernicus II's GPS readings. This version uses altitude
for its cutdown check, but the position (coordinates) is also available.
SD card logging capability added. Note that GPS connections changed for new software
serial pin assignments for XBee shield SD card compatibility.

v1.10 Updated to avoid conflicts with other Serial messages and added support for
putting the XBee to sleep until near cutdown time.

Brendan Batliner and Milan Shah
Illinois Mathematics and Science Academy - SIR Program


Notes on SD card integration - LN
REF: http://arduino.cc/en/Main/ArduinoWirelessShield
  
  The Wireless shield hardwires Arduino pin 4 to the on-board
  SD card slot's SPI chip select (CS), so it can't be used as
  a Software Serial pin as in the TinyGPS example. Also
  The Arduino's MOSI, MISI and CLK pins (11,12 and 13) are used
  by the SD library and even if not used for SD CS, the default
  CS pin 10 must be programmed as an output.
  
  So, software serial can be assigned to 5 and 6 safely.
  
  TESTING:
  Can be tested without remote cutdown and remote programmer units:
  1. Remove XBee from XBee shield so Arduino USB I/O will work
  2. Provide input via a single monitor line entry emulating the
     remote inputs, for example:
     X90,100000,300,41.1000322,-87.9167100
     This yields: Remote cutdown time (deadman): 90 minutes
                  Cutdown altitude: 100,000 feet
                  Cutdown range: 300 miles
                  Launch Lat/Lon: Koerner Aviation