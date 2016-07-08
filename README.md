# New-GPS-Additions
crazyflie 1.0/2.0

Date 2016.07.08

>>>>Branch dkgps-7 - This commit contains the latest updates involving changes
to function compassGyroBias() in compass.c.  Gyro->yaw drift compensation has been
disabled.  The code computing drift correction in function compassGyroBias() has
been simplified and appears more robust than the previous implementation.  However,
it appears that the couple of degrees of noise applied to gyro->yaw is too much and
can cause yaw instability. Gyro bias updates in imu_cf2.c and thrust tilt compensation
in controller_pid.c are included in this build.

Perhaps another qualification regarding eeprom retention of compass calibration
values is that bootloading firmware into the crazyflie that is something other than
a dkgps-6 or newer dkgps commit, will effectively erase the calibrations values.
In this case, after downloading a dkgps-6 or newer commit, the compass will need to
be calibrated once again. 

Date 2016.07.02

>>>>Branch dkgps-6 - This commit contains the latest firmware updates for the
uBlox Max M8c Pico Breakout with Chip Antenna gps receiver.  The firmware is
identical to that of Branch gtgps-6 for the GlobalTop Titan-2 gps receiver,
except for the deck driver, the config.mk, the makefile, and the corresponding
changes in sensors_stock.c.

Date 2016.06.24

>>>>Branch dkgps-5 - This commit contains the latest firmware updates for the
uBlox Max M8c Pico Breakout with Chip Antenna gps receiver.  The firmware is
identical to that of Branch gtgps-5 for the GlobalTop Titan-2 gps receiver,
except for the deck driver (which is now dkgps.h & dkgps.c), the config.mk
(which is now CFLAGS += -DDECK_FORCE=bcDKGPS), the makefile (which is now
dkgps.o), and the corresponding changes in sensors_stock.c (which is now #include
"dkgps.h" and function call (which is now dkgpsGetFrameData() ).  In all
other aspects the altitude hold mode using the altimeter sensor, the position hold
mode using gps, the compass using the magnetometer sensor with tilt compensation &
compass calibration in concert with using the controller device (currently Xbox360),
and gyro.yaw drift mitigation using the compass is at the same code level as that
of the gtgps.

The gtgps receiver with its integrated patch antenna is superior to that of the
dkgps receiver with its chip antenna as far as gps satellite signal reception.  It
is less susceptible to crazyflie electrical interference, can be mounted closer
to the crazyflie control board, and may yield a faster time to first 3D fix solution.
The primary advantage of the dkgps gps receiver is less weight (a savings of ~4.5 g)
which yields a higher crazyflie lift to weight ratio, and longer flight times.                                                               
                                                            
