/**
 * Example of reading distance from the analog output of a MaxBotix MB1361
 * ultrasonic sensor. 
 *
 * See BeagleBone example page for wiring diagram (GND, 5V, then a
 * 1200 + 3000 ohm resistor to divide voltage into a safe level for
 * connecting to an analog input).
 *
 * WARNING/NOTE: The BeagleBone diagram has the sensors BACKWARDS (at
 * least I think so)!  You want to scale the voltage from [0, 5] volts
 * down to [0, 1.8] volts.  you need to read the voltage drop across
 * the SMALLER resistor. Also, the 1200 and 3000 ohm resistors are way
 * to small and impact the voltage output from the sensor (hint: read
 * the voltage output pin with and without the two sensors connected
 * to it). I found that if I increased the resistors to 240K and 430K,
 * I could achieve the same scaling factor with less impact on the
 * circuit.
 */

var b = require('bonescript');

var anPin = "P9_40";

// Source voltage
var vcc = 5.02;

// Two resitors in series that we use to divide voltage to keep
// output voltage below 1.8, choose resistors such that
// (5 * r1 / (r1 + r2)) is less than 1.8, but as big as you can make it.
var r1 = 237300;
var r2 = 426000;
var r1Max = vcc * r1 / (r1 + r2);
var r2Max = vcc * r2 / (r1 + r2);

console.log("Vcc: " + vcc.toFixed(2) + " Volts");
console.log("R1:  " + (r1 / 1000.0).toFixed(1) + " K Ohms");
console.log("R2:  " + (r2 / 1000.0).toFixed(1) + " K Ohms");
console.log("R1 Drop: [0.000, " + r1Max.toFixed(3) + "] volts (tied to pin " + anPin + ")");
console.log("R2 Drop: [0.000, " + r2Max.toFixed(3) + "] volts");

var r1RangePercent = 100.0 * r1Max / 1.8;

if (r1RangePercent <= 100.0) {
  console.log("R1 voltage range OK for pin " + anPin + " (" + r1RangePercent.toFixed(1) + "%)");
} else {
  var minR2 = (vcc * r1 / 1.8) - r1;
  var maxR1 = (vcc * r2 / 3.2) - r2;
  console.log("***WARNING*** R1 voltage range TOO HIGH for pin " + anPin
	      + " (" + r1RangePercent.toFixed(1) + "%)");
  console.log("Increase R2 to at least " + minR2 + " K Ohms, OR");
  console.log("Reduce R1 to no more than " + maxR1 + " K Ohms");
}

// From MaxBotix MB1361 data sheet (sensor puts out about 4.9 millivolts per 2 cm);
var maxbotixVoltsToCm = 2.0 / (vcc / 1024);

/* Check the sensor values every 2 seconds*/
setInterval(read, 2000);

function read(){
  b.analogRead(anPin, printStatus);
}

function printStatus(x) {
  var distanceInches;
  var anVolts = x.value*1.8; // ADC Value (0, 1.0) converted to voltage
  var anRaw = x.value * 1024;
  var sensorVolts = anVolts * (r1 + r2) / r1;
  var distanceCm = sensorVolts * maxbotixVoltsToCm;
//  var distanceCm = anRaw / 1024.0 * 1068.0 * (34.0 / 4.0);
  var distanceInches = distanceCm / 2.54;

  console.log(anPin + " volts: " + anVolts.toFixed(3)
	      + "  Sensor raw: " + anRaw.toFixed(1)
	      + "  Sensor volts: " + sensorVolts.toFixed(3)
	      + "  Dist (cm): " + distanceCm.toFixed(1)
	      + "  Dist (in): " + distanceInches.toFixed(1));
}
