<h3>CutDownRemote</h3>
<h4>Code for Remote Cutdown Module</h4>
Placed between parachute and ballooon tether, the Remote Cutdown Module cuts connection to the tether when certain conditions are met as determined by the Base Cutdown Module and triggered by wireless sigal. The remote module has a timer pre-set at launch that will trigger the cutdown if other criteria are not met or fail to get a trigger to the remote.

<h4>Revision History</h4>

Placed under CVS at V5.00

V5.00 Adds support for the BaseModule's GPS functions. It will ask
the user about the maximum altitude of the flight and supply this to
the BaseModule.

V3.11 Now No longer Sends the Query 'Q'

V3.01 Now avoids character conflict with base module, to allow the use of a third Xbee to program both the base module and Cutdown arduino

V3.00 Some fixes for high XBee current and workarounds for lack of single point setup control in alpha base station.
- Put XBee to sleep when not needed. Required hardware change.
- Use variable LED flash count to signal ready for input from base allows setup without terminal connection.
- Programming mode until cutter cap is nearly charged.

<h4>Credits</h4>

Lou Nigra
Adler Planetarium - Far Horizons

Brendan Batliner and Milan Shah
Illinois Mathematics and Science Academy - SIR Program

<h4>Controls</h4>

- Raw voltage tied to charger ON switch
- Programming mode when serial port is connected via 3.3V FTDI and XBee removed
- Timer activate/de-activate momentary contact switch (no longer used)

<h4>Indicators</h4>

- LED no blink: Charging => Programming mode inhibited.
- LED fast blink: Active => Timer is counting down.
- LED long Off: Standby => Waiting for programming window to open.
- LED single three flashes: Programming mode => Window open for "non-D"
                            input to enter programming mode.

The following sequence follows if a non-D character is entered while
the window is open:

- LED single six flashes: Parameter prompt: => Ready for parameter entry.
- LED three flashes: Confirmation: => Ready for y or n

<h4>State Machine</h4>
- standby -> WAITFORCHARGE
- WAITFORCHARGE
-   -> GETTTY
- GETTTY
-   (timeout) -> SLEEP
-   (entries) -> active -> SLEEP
- SLEEP
-   standby? > TTY
-   active?  -> timerUpdate
