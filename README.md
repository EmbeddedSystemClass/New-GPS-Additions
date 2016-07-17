# New-GPS-Additions
crazyflie 1.0/2.0

Date 2016.07.16

>>>>Branch gtgps-9 - This update and new commit contains the latest updates involving changes to sensfusion6.c The code change
in function sensfusion6GetEulerRPY() corrects a minor bug.  In addition map.js has been updated in cfclient that replaces the
url from MapQuest to OpenSourceMap.  Map.js is used to generate the display map in GpsTab.py. 

Date 2016.07.15

>>>>Branch gtgps-9 - This new branch and new commit contains the latest updates involving changes to sensfusion6.c The code change
in function sensfusion6GetEulerRPY() corrects roll, pitch and yaw when roll or pitch nears +/- 90 degrees and when the crazyflie
tumbles or flies upside-down.

It also facilitates performing compass calibration when the crazyflie is vertically held with the nose up or nose down.  Before
this code change, only starboard side down or port side up could actually be used.  This commit does not change this limitation,
but a future commit will take advantage of the modified Euler angles and not have such orientation restrictions.

In function sensfusion6GetEulerRPY(), the new code handles the conversion from quaternion to Euler angles when yaw hits the
singularities near a +/- 90 degree pitch angle.  This situation also known as gimbal lock, is handled specially.  The pitch angle
is now +/- 180 degrees instead of +/- 90 degrees.  In addition, some numerical instabilities have been corrected when certain
quaternion values are used within the arctan2(n,d) trigonometric function.

Not tested, but the new Euler angles should work better in the special cases the crazyflie tumbles or is flown upside down. 

Date 2016.07.12

>>>>Branch gtgps-8 - This new branch and new commit contains the latest updates involving changes to compass.c and small changes to
commander.h and commander.c. The code change simplifies compass calibration using the device controller.

The prior implementation required three buttons or equivalent actuation's on the device controller to communicate when to perform
a calibration, when to take readings of a full circle rotation of the horizontally held crazyflie in the horizontal plane, when to
take full circle vertically orientated crazyflie readings in the horizontal plane, and finally when to calculate a new set of
calibration parameters using these readings.

This new approach eliminates the extra buttons by detecting the switching of the position hold mode switch (toggle on/off) 5 times
in quick succession (within a 5 second period).  The M1 LED becomes a solid green indicating when to begin rotating the horiontally
held crazyflie. After 350 degrees is detected. the the M1 KED turns a solid red indicating when to begin rotating a vertically held
crazyflie.  When internally recognizing the 360 degrees circle is completed, the calibration parameters are automatically updated.
Then the M1 LED returns to its normal flashing red condition.  Once the calibration procedure has begun and the M1 LED is a solid
color, if necessary the procedure can be aborted by repeating the 5 times toggle of the position hold mode switch.

When the M1 LED is green, readings begin when the crazyflie roll and pitch angles are both less than +/- 25 degrees.  When the M1 LED
is a solid red, reading begin when the roll angle is more than +/- 65 degrees and the pitch angle remains less than +/- 25 degrees.

The original plan was to put the nose down (pitch -90 degrees), but a quirk in the current baseline firmware prevented this. Currently,
the port side must be down or the starboard side must be down for the vertically held crazyflie horizontal rotation segment.

Date 2016.07.10

>>>>Branch gtgps-7 - This commit contains the latest updates involving changes to function compassGyroBias() in compass.c.
Gyro->yaw drift compensation has been disabled in this commit. It can be enabled by setting CFLAGS += -DGYRO_MAG_FUSION_ENABLE in
config.mk.  The noise level on yawFusion (yawGyro & yawMag) has been lowered to the point that when enabled (gyro->yaw is set equal
to yawFusion), it no longer disturbs the crazyflie yaw stability when flying.  As mentioned earlier, gyro->yaw appears to lose a small
amount of absolute value when the crazyflie is subjected to attitude dynamics, but the yawMag drift compensation appears to nicely
compensate for this.

Date 2016.07.08

>>>>Branch gtgps-7 - This commit contains the latest updates involving changes to function compassGyroBias() in compass.c.  Gyro->yaw
drift compensation has been disabled.  The code computing drift correction in function compassGyroBias() has been simplified and
appears more robust than the previous implementation.  However, it appears that the couple of degrees of noise applied to gyro->yaw
is too much and can cause yaw instability.  

Date 2016.07.07

>>>>Branch gtgps-7 - This commit contains the latest updates involving changes to function compassGyroBias() in compass.c.  Gyro bias
updates in imu_cf2.c and thrust tilt compensation in controller_pid.c are included in this build.  In order to enable gyro->yaw drift
compensation using the calibrated compass, CFLAGS += -DGYRO_MAG_FUSION_ENABLE must be added to config.mk.

Perhaps another qualification regarding eeprom retention of compass calibration values is that bootloading firmware into the
crazyflie that is something other than a gtgps-6 or newer gtgps commit, will effectively erase the calibrations values.  In this case,
after downloading a gtgps-6 or newer commit, the compass will need to be calibrated once again.  

Date 2016.07.02

>>>>Branch gtgps-6 - This commit contains the latest updates involving changes to function sensorsAcquire() in sensors_stock.c,
to function stateEstimator() in estimator_complementary.c, and the more aggressive pid values used in the attitude roll/pitch controller
are hard coded in pid.h.  Gyro bias updates in imu_cf.c and thrust tilt compensation in controller_pid.c are not included in this build.

A previous statement that the eeprom is cleared of compass calibration values when the bootloader downloads a new flash program into
the crazyflie was incorrect. The last compass cal values remain there until a new cal is performed.  The reported problem that the
"sensor->position.timestamp" in the point_t structure was clobbering baro.asl was incorrect.  A bettter understanding of the baseline
code in stateEstimator() helped resolve what was actually hapenning to baro.asl and new code was added to properly handle gps fix
position updates.  Gps position x & y are passed through, and filtered baro.asl continues to be used for position z updates.  Gps hMSL
is not used to control position z updates.  A future study might be to investigate whether velocity information derived from gps 3D
positions would be of benefit or not.

Date 2016.06.30

>>>>Branch gtgps-6 - This commit contains a fix to the gyro.yaw drift compensation to assure the angle stays within +/- 180 degrees.
In addition, uart1.c was modified to increase the size of the interrupt buffer.

Date 2016.06.27

>>>>Branch gtgps-6 - This commit contains the latest updates involving changes to files gtgps.c, compass.h, compass.c, and
sensors_stock.c.  If the compass (magnetometer) has not been calibrated, then (a)the gtgps driver does not pass on lat/lon positions,
(b)yawgeo (yawangle) is (are) set to gyro.yaw, and (c)drift compensation is not applied to gyro.yaw.  As a result, if position hold
mode is activated when uncalibrated, position x, y and z remain zero values.  A bug fix was applied to the function sensorsAcquire()
that if position.timestamp is non-zero, the faulty statement "sensor->position = position;" was replaced by individual statements
"sensor->position.x = position.x;" and correspondingly for y and z, without timestamp.  For an unknown reason setting
"sensor->position.timestamp" in the point_t structure was clobbering the sensorData_t structure (specifically baro.asl).    

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
