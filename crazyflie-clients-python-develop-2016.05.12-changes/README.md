# Crazyflie 1.0/2.0 Firmware

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
magnetic inclination, the direction of the crazyflie canbe referenced to the geographic
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
branch master 2016.05.12.  For the host pc cfclient, the starting point source code was
crazy-client-python branch develop 2016.05.12 along with crazyflie-lib-python branch master 2016.04.27. 

Todate, the call to the gtgps.c deck driver has been implemented in sensors_stock.c.  The compass
controller is located in the module section and the first attempt to call this controller within
estimated_complementary.c failed due to timing problems.  Currently, the compass controller is
called from within stabilizer.c.  A few detail comments on each of these code sections can be found
withing the individual firmware source code. 

Collaboration with Bitcraze is solicited in order to to advance forward and to achieve this project's
objective.  Any preliminary feedback, comments and suggestions on this proposed software effort would
be greatly appreciated.  In addition, if more background or clarification information is of interest,
please let this be known.

