Parameters that can be updated while running:
SPEED_FACTOR
YAW_PID_KP
YAW_PID_KD
YAW_PID_KI
YAW_PID_INTEGRAL_LIMIT

Differential control implementation:
Diff = (dTheta*Tau*OldDiff)/(Tau+dT)
Differential portion of rudder output = Kd*Diff

Integral control limit:
max rudder command = Ki*IntegralLimit

SPEED_CONTROLLER config parameter can be:
factor
pid
fit_pid

New config for FIT+PID control method for speed:
SPEED_SLOPE
SPEED_OFFSET
ANGLE_LIMIT
TIME_DELAY

Theory of operation:

SPEED_SLOPE and SPEED_OFFSET should define a linear relation from speed
(m/s) to thrust (percent): Thrust = SPEED_SLOPE*speed + SPEED_OFFSET

The linear fit determines the initial thrust output for a set desired 
speed.  The speed PID is intended to run with a slow integral gain on
top of the linear fit to account for external disturbances or errors in
the fit.  The PID will only be run when the vehicle's heading is within
ANGLE_LIMIT of the desired heading for at least TIME_DELAY seconds.  
The output of the PID will always be applied, even if it is not being 
run.  