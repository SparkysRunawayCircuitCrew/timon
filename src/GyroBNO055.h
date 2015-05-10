/**
 * GyroBNO055 definition.
 */
#ifndef __avc_GyroBNO055_h
#define __avc_GyroBNO055_h

#include <BlackI2C.h>

namespace avc {

  /**
   * The GyroBNO055 class is used to configure and read information
   * from the Adafruit BNO055 absolute 9DOF sensor
   * (https://www.adafruit.com/product/2472).
   *
   * <p>This sensor provides directions about which way you are facing
   * and is one of the easiest and most stable gyros I have used.</p>
   * 
   * <p>This C++ implementation uses the BlackLib I2C classes for
   * communications and a lot of information from Adafruit's sampe
   * Arduino code at https://github.com/adafruit/Adafruit_BNO055. For
   * full details on what the sensor is capable of, refer to the Bosh
   * datasheet
   * (http://www.adafruit.com/datasheets/BST_BNO055_DS000_12.pdf).</p>
   *
   * <p>Example usage:</p>
   *
   * <pre><code>
   * GyroBNO055 gyro;
   * 
   * gyro.reset();
   *
   * float ang;
   * if (gyro.getHeading(ang)) {
   *   cout << "Heading: " << ang << "\n";
   * } else {
   *   cout << "Gyro not responding\n";
   * }
   * </code></pre>
   */
  class GyroBNO055 {

  public:
    // Default I2C address of the sensor
    static const int PRIMARY_I2C_ADDR = 0x28;
    // Alternate address for the sensor if ADR line is tied to 3V
    static const int SECONDARY_I2C_ADDR = 0x29;
    
    /**
     * Construct a new GyroBNO055 instance.
     *
     * @param i2cDev The I2C device to communicate with. From what I
     * can tell, is that it should typically be BlackLib::I2C_1
     * (corresponding to /dev/i2c-1 on Debian). This is the default
     * value if omitted.
     *
     * @param i2cAddr The I2C address of the sensor. This is typically
     * PRIMARY_I2C_ADDR (0x28) unless you have tied the ADR line high,
     * then it is SECONDARY_I2C_ADDR (0x29). This parameter is
     * optional and defaults to PRIMARY_I2C_ADDR if omitted.
     */
    GyroBNO055(BlackLib::i2cName i2cDev = BlackLib::I2C_1,
	       int i2cAddr = PRIMARY_I2C_ADDR);

    /**
     * Destructor won't change the sensor, just cleans up any internal
     * info.
     */
    ~GyroBNO055();

    /**
     * This method initializes the sensor to NDOF (nine degrees of
     * freedom mode).
     *
     * <p>This method should be called once at the start of your
     * program. This method takes a long time (think at least 20
     * milliseconds) to complete. The gyro should be stationary during
     * this time.</p>
     *
     * <p>DO NOT USE for relative turns. Instead, read the current
     * angle and try to rotate to the new angle.</p>
     *
     * @return true If the gyro was successfully reset, false if there
     * was a problem (like gyro not found on the I2C bus).
     */
    bool reset();

    /**
     * Get the current heading of the gyro (which way you are facing).
     *
     * @param angDeg Where to store the results if information is available.
     * Value set will be in the range of [0, 360).
     *
     * @return true If value returned, false if error getting value.
     */
    bool getHeading(float& angDeg);

    /**
     * Dumps debug information about the gyto to the output stream provided.
     */
    std::ostream& dumpInfo(std::ostream& out) const;

  private:
    BlackLib::BlackI2C i2cGyro;
  };

}

#endif
