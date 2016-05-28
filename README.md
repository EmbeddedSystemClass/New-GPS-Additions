# New-GPS-Additions
crazyflie 1.0/2.0
Date: 2016.05.28

This is the 2nd phase of this project and is an update on the work-in-progress effort to
modify the crazyflie firmware's hover-in-place feature and add an on-board gps receiver
for horizontal position control.  The 1st phase changed the gps deck driver to support
a GlobalTop Titan-2 GNNS module and incorporated a new compass controller that uses the
onboard magnetometer to coordinate applying gps positions to the pointing direction
of the crazyflie.

This 2nd phase alters the current approach for altitude hold from a thrust-based method
to a vertical position sensor-based method.  This takes advantage of the barometric
altimeter to set the vertical position hold point.  The implementation also is capable
of accepting throttle changes from the controlling device to raise or lower the set point.

Only a limited amount of testing has been performed on these changes.  Qualitatively speaking,
the vertical swings while in altitude hold when using either the altimeter or the thrust-base
method appear to be on the same order of variations.  Neither approach looks to be as stable
as might be possible with a different approach or a better type of sensor that is both
responsive and accurate.     

The thrust-based method requires firmware changes in order to find the optimum setting for
different crazyflie payloads.  Also, it is dependent upon the discharge of the battery and
a drop in voltage level.  The heavier the payload the more this issue can become a factor.

In this tab of the repository there are four folders, one set of two supports the firmware
and the other set of two supports the client-python source code. The folders marked “changes”
contain the modified modules in their respective directory tree holding folders. Only the
modules actually changed are included. The folders marked “originals” contain copies of
all those modules that were actually modified, but are the untouched originals. Modules
that have not been modified in any way have all been excluded from the uploaded folders.


Date: 2016.05.26

This project contains the feasibility source code modifications to the crazyflie firmware
that supports a gps receiver attached to the crazyflie, with specific attention to the
GlobalTop Titan-2 GNSS module and source code modification for the crazyflie pc client-python
host that is compatible with the firmware changes.  The overall project objective is to
implement a hover-in-place capability into the crazyflie with vertical altitude hold and
horizontal position hold using an onboard gps sensor.

The current work-in-progress source code acquires latitude and longitude at a 5Hz update
rate parsed from either NMEA or binary messages output by the receiver.  In addition, the
onboard magnetometer is utilized to provide direction orientation, with tilt-compensation
to the earth's horizontal magnetic field, and combined with a manual input for the local
magnetic inclination, so that the direction of the crazyflie can be referenced to the geographic
north pole and the latitude/longitude reference system.

The current software code commit is prior to resolving integration compatibility with what
Bitcraze has been developing for the position hold function and the positioning controller function.

The magnetometer sensor requires calibration for the local magnetic disturbances and correction
to the geographic north pole.  Calibration involves a manual procedure to 360 degree rotate the
crazyflie horizontally and 360 degree rotate the crazyflie vertically.  Synchronizing the manual
procedure with the crazyflie firmware can be performed using the host pc client or by using the
flight controller device.

The latter implementation has been tested using the xbox360 game controller.

The baseline firmware starting point for the source code came from Github crazyflie-firmware
branch master 2016.05.19.  For the host pc cfclient, the starting point source code was
crazy-client-python branch develop 2016.05.11 along with crazyflie-lib-python branch master 2016.04.27. 

Todate, the call to the gtgps.c deck driver has been implemented in sensors_stock.c.  The compass
controller is located in the module section and the first attempt to call this controller within
estimated_complementary.c failed due to timing problems.  Currently, the compass controller is
called from within stabilizer.c.  A few detail comments on each of these code sections can be found
withing the individual firmware source code. 

Collaboration with Bitcraze is solicited in order to advance forward and to achieve this project's
objective.  Any preliminary feedback, comments and suggestions on this proposed software effort would
be greatly appreciated.  In addition, if more background or clarification information is of interest,
please let this be known.

