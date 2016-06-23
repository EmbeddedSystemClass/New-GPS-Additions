# New-GPS-Additions
crazyflie 1.0/2.0

Date 2016.06.24

>>>>Branch gtgps-5 - This commit contains the latest updates as well as the whole python3.4 source code, the whole cfclient source
code, and the whole cf2 firmware containing the gtgps deck driver. These have been tested in a windows 7 & windows 10 pc environment.
In addition, the earlier branch gtgps-6 that incorporated gyro.yaw drift corrections using the magnetometer sensor and converting the
gyro.yaw to geographic north reference, this new commit applies gyro drift correction but leaves the gyro.yaw referenced to the cf2
power-on reference heading at zero degrees.  The code changes in compass.c attempt to minimize passing on sensor data noise from the
magnetometer onto the gyro.yaw estimates while at the same time striving to remain responsive to instantaneous changes in the crazyflie
heading.  The sensor fusion aspects are still work-in-progress to find a good tradeoff of limiting noise entering the drift correction
without compromising gyro heading sensitivity.  

Date 2016.06.20

>>>>Branch gtgps-6 - Has since been deleted. 
This commit contains the latest updates that supports the gtgps, compass, compass calibration, altitude hold and
position hold features.  The github crazyflie-firmware dated 2016.05.19 is still the baseline code for these updates.  This is still
in the work-in-process state and has yet to be validated by testing the actual flight performance.  In summary, (a) a bug fix was
applied to position_controller_pid.c, (b) gtgps.c was updated to better define when a position fix can be used for position hold
purposes, (c) compass calibration values are saved in eeprom when calibration is performed in conjunction with the Xbox350 controller
and read back whenever there is a subsequent powering up of the crazyflie, (d) added gyro yaw bias using the compass to eliminate gyro
drift, (e) the watchdog timer is disabled and not used, (f) added experimental PID settings in pid.h that result in a faster and
tighter control of the CF2 attitude, (g) the gyro bias calculation has been improved in imu_cf2.c as updated in github 2016.06.17,
and (h) control->thrust now has tilt compensation in controller_pid.c as updated in github 2016.06.17.  It has been found that recent
updates of the github firmware (after 2016.05.19) are currently not compatible with the gtgps using uart1 serial port communication.    

Date 2016.06.04

>>>>Branch gtgps-5 - Has since been deleted.
This commit completes the planned modifications to function commanderGetSetpoint() in commander.c for setting up
position hold mode when gps is present and positions are available out of the deck gtgps driver.  This change is in the work-in-process
state and has yet to be validated by testing.  The pid tuning values in position_controller_pid.c (.pidX & .pidY) are untouched, and
may require changes in the future. 

Date 2016.05.31

>>>>Branch gtgps-4 - This commit corrects a few configuration control errors that crept into Branches gtgps-2 and gtgps-3.  Module
sensors_stock.c was corrected (5 Hz loop), module pid.h (aggressive values) was previously missing, and module compass.c was corrected
such that yawgeo in the log group is counterclockwise positive. 

Date 2019=6.05.29

>>>>Branch gtgpsTab - The 1st commit contains the GlobalTop Titan-2 GNSS deck driver and magnetometer (compass) controller that provides
state->attitude/yawgeo = +/- 180 degrees positive is counterclockwise. This has been converted to geographic north. The
"#define GPS_Present" is active in stabilizer_types.h. Also, the altitude hold mode code is untouched and is the original thrust-based
method for setting the desired altitude. This commit contains the original pid.h values.
>>>>Branch gtgps-2 - The 2nd commit contains the new altitude hold mode code which uses the altimeter sensor position to set the desired
altitude. The #define GPS_Present is inactive in stabilizer_types.h and removes the call to the gps deck drive in sensors_stock.c and
the call to the compassController() in stabilizer.c. The Magnetometer sensor data is not processed. This commit contains the more
aggressive pid.h values.
>>>>Branch gtgps-3 - This 3rd commit contains the same code as in the 2nd commit except the #define GPS_Present is active in
stabilizer_types.h. This enables the sensors->position to contains the gps latitude, longitude and height above mean sea level when
there is a 3D Fix or 3D/DGPS Fix. In addition, magnetometer-based yaw is placed in state->attitude/yawgeo. As in the 2nd commit the
desired altitude using the altimeter is used in the altitude hold mode. This commit contains the more aggressive pid.h values used in
the attitude roll/pitch controller. 

Date: 2016.05.28

This is the 2nd phase of this project and is an update on the work-in-progress effort to modify the crazyflie firmware's hover-in-place
feature and add an on-board gps receiver for horizontal position control. The 1st phase changed the gps deck driver to support a
GlobalTop Titan-2 GNNS module and incorporated a new compass controller that uses the onboard magnetometer to coordinate applying gps
positions to the pointing direction of the crazyflie.  This 2nd phase alters the current approach for altitude hold from a thrust-based
method to a vertical position sensor-based method. This takes advantage of the barometric altimeter to set the vertical position hold
point. The implementation also is capable of accepting throttle changes from the controlling device to raise or lower the set point.
Only a limited amount of testing has been performed on these changes. Qualitatively speaking, the vertical swings while in altitude
hold when using either the altimeter or the thrust-base method appear to be on the same order of variations. Neither approach looks to
be as stable as might be possible with a different approach or a better type of sensor that is both responsive and accurate. 
The thrust-based method requires firmware changes in order to find the optimum setting for different crazyflie payloads. Also, it is
dependent upon the discharge of the battery and a drop in voltage level. The heavier the payload the more this issue can become a
factor.
In this tab of the repository there are four folders, one set of two supports the firmware and the other set of two supports the
client-python source code. The folders marked “changes” contain the modified modules in their respective directory tree holding
folders. Only the modules actually changed are included. The folders marked “originals” contain copies of all those modules that were
actually modified, but are the untouched originals. Modules that have not been modified in any way have all been excluded from the
uploaded folders.

Date: 2016.05.26

This project contains the feasibility source code modifications to the crazyflie firmware that supports a gps receiver attached to the
crazyflie, with specific attention to the GlobalTop Titan-2 GNSS module and source code modification for the crazyflie pc client-python
host that is compatible with the firmware changes. The overall project objective is to implement a hover-in-place capability into the
crazyflie with vertical altitude hold and horizontal position hold using an onboard gps sensor. The current work-in-progress source code
acquires latitude and longitude at a 5Hz update rate parsed from either NMEA or binary messages output by the receiver. In addition, the
onboard magnetometer is utilized to provide direction orientation, with tilt-compensation to the earth's horizontal magnetic field, and
combined with a manual input for the local magnetic inclination, so that the direction of the crazyflie can be referenced to the
geographic north pole and the latitude/longitude reference system.
The current software code commit is prior to resolving integration compatibility with what Bitcraze has been developing for the
position hold function and the positioning controller function.
The magnetometer sensor requires calibration for the local magnetic disturbances and correction to the geographic north pole.
Calibration involves a manual procedure to 360 degree rotate the crazyflie horizontally and 360 degree rotate the crazyflie vertically.
Synchronizing the manual procedure with the crazyflie firmware can be performed using the host pc client or by using the flight
controller device.
The latter implementation has been tested using the xbox360 game controller.
The baseline firmware starting point for the source code came from Github crazyflie-firmware branch master 2016.05.19. For the host
pc cfclient, the starting point source code was crazy-client-python branch develop 2016.05.11 along with crazyflie-lib-python branch
master 2016.04.27. 
To-date, the call to the gtgps.c deck driver has been implemented in sensors_stock.c. The compass controller is located in the module
section and the first attempt to call this controller within estimated_complementary.c failed due to timing problems. Currently, the
compass controller is called from within stabilizer.c. A few detail comments on each of these code sections can be found withing the
individual firmware source code. 
Collaboration with Bitcraze is solicited in order to advance forward and to achieve this project's objective. Any preliminary feedback,
comments and suggestions on this proposed software effort would be greatly appreciated. In addition, if more background or
clarification information is of interest, please let this be known.
