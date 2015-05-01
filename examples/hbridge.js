/**
 * Test code for playing with controlling a single side of the drive train on
 * the timon robot. We need to:
 * 
 * 1. Regulate motor output power using a PWM output.
 * 2. Set direction of the motor using two digital outputs
 * 
 * We will use an analog input to allow dynamic range adjustments.
 */

var b = require("bonescript");

/**
 * Constructs and initializes a new tank tread object (shifts into forward gear
 * at zero power initially).
 * 
 * name - Name to associate with object (anything you want).
 * pwm - The BBB pin to use for the PWM signal out.
 * fwd - The BBB pin used to specify forward power.
 * rev - The BBB pin used to specify reverse power.
 */
function Tread(name, pwm, fwd, rev) {
    this.name = name;
    this.pwm = pwm;
    this.fwd = fwd;
    this.rev = rev;
    this.pwmHz = 1000.0;
    this.inFwdGear = false;

    // Set GPIO pins to digital output mode
    b.pinMode(pwm, b.OUTPUT);
    b.pinMode(fwd, b.OUTPUT);
    b.pinMode(rev, b.OUTPUT);

    // Initially set drive power to 0 (stopped) and shift drive into forward
    this.setPower(0.0);
    this.shiftIntoForward();
}

/**
 * Verifies value passed is a number (if not returns the defVal) and then trims
 * number to range limits.
 */
Tread.prototype.rangeCheck = function(val, min, max, defVal) {
    val = parseFloat(val);
    if (isNaN(val)) {
        return defVal;
    }
    else if (val < min) {
        return min;
    }
    else if (val > max) {
        return max;
    }
    return val;
}

/**
 * Dumps state of Tread object to string for logging to console.
 */
Tread.prototype.toString = function() {
    var s = "Tread(name=" + this.name + ", forward=" + this.inFwdGear 
            + ", freq=" + this.pwmHz + ", duty=" + this.duty;
    return s;
}

/**
 * Set power output in range of [0.0, 100.0] where 0.0 is off.
 */
Tread.prototype.setPower = function(power) {
    if (power > 0) {
        this.shiftIntoForward();
    }
    else if (power < 0) {
        this.shiftIntoReverse();
        power = -power;
    }
    this.duty = this.rangeCheck(power / 100.0, 0.0, 1.0, 0.0);
    b.analogWrite(this.pwm, this.duty, this.pwmHz);
}

Tread.prototype.shiftIntoForward = function() {
    if (this.inFwdGear != true) {
        b.digitalWrite(this.fwd, true);
        b.digitalWrite(this.rev, false);
        this.inFwdGear = true;
    }
}

Tread.prototype.shiftIntoReverse = function() {
    if (this.inFwdGear == true) {
        b.digitalWrite(this.fwd, false);
        b.digitalWrite(this.rev, true);
        this.inFwdGear = false;
    }
}

/**
 * OK, now lets create two tread instances (for our robot) and control them
 * using 2 analog inputs.
 */

//
// Robot Map
//
var gpioStart = "P9_23";  // Not used currently

var gpioLeftPwm = "P9_14";
var gpioLeftFwd = "P9_11";
var gpioLeftRev = "P9_12";

var gpioRightPwm = "P9_21";
var gpioRightFwd = "P9_13";
var gpioRightRev = "P9_15";

var ainXAxis = "P9_36";
var ainYAxis = "P9_38";

//
// Two "tank treads" to drive the vehicle
//
var rightTread = new Tread("RightTread", gpioRightPwm, gpioRightFwd, gpioRightRev);
var leftTread = new Tread("LeftTread", gpioLeftPwm, gpioLeftFwd, gpioLeftRev);

// global variable to hold x and y values read from Joystick
var pos = {};

// Milliseconds between reading inputs, values above 200 will log info to console
var inputDelay = 50;

// How long to let the program run (in milliseconds)
var driveTimeMillis = 60 * 1000;

// After we get x-axis value from Joystick, go read y-axis value
function onX(x) {
    pos.x = inputToPower(x.value);
    b.analogRead(ainYAxis, onY);
}

// Call back after reading y-axis. At this point we have
// both joystick values, go update tread power
function onY(x) {
    pos.y = inputToPower(x.value);

    // We have reading from both axis, go update drive (if not disabled)
    if (driveInterval != null) {
        leftTread.setPower(pos.y);
        rightTread.setPower(pos.x);
    }

    if (inputDelay > 200) {
        console.log(JSON.stringify(pos));
        console.log(leftTread.toString());
        console.log(rightTread.toString());
    }
}

//
// Helper method to convert a joystick axis value to a number
// in the range of [-100.0, +100.0]
//
function inputToPower(val) {
    // Get power roughly to range of [-.5, +.5]
    var power = parseFloat(val) - 0.5;

    if (power < 0) {
        // If close to zero, just call it zero
        if (power > -0.025) {
            power = 0;
        }
        else {
            // Stretch value to range of [-1.0, 0.0] (allow it to go a little
            // over in case analog input can't quite reach upper range)
            power = (power + 0.025) * (1.05 / (0.5 - 0.025));
            if (power < -1.0) {
                // if it went over, force to limit
                power = -1.0;
            }
        }
    }
    else {
        if (power < 0.025) {
            power = 0;
        }
        else {
            // Stretch value to range of [0.0, +1.0] (allow it to go a little
            // over in case analog input can't quite reach upper range)
            power = (power - 0.025) * (1.05 / (0.5 - 0.025));
            if (power > 1.0) {
                // if it went over, force to limit
                power = 1.0;
            }
        }
    }
    // Convert to range of [-100.0, +100.0]
    power *= 100.0;
    return power.toFixed(0);
}

//
// OK, here is where we actually start things
//
// 1. Start a periodic task to read/apply joystick values
// 2. Add a timeout to shutdown the program down after a bit

// Periodic function to read joystick values and apply to drive motors
function drive() {
    b.analogRead(ainXAxis, onX);
}

var driveInterval = setInterval(drive, inputDelay);

// Shuts down the driving program
function stopDrive() {
    clearInterval(driveInterval);
    driveInterval = null;
    leftTread.setPower(0);
    rightTread.setPower(0);
}

// Set a timer to stop the program after letting it run a bit
setTimeout(stopDrive, driveTimeMillis);
