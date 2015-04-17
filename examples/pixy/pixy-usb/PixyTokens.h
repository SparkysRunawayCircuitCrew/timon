#include <string>

#include <stdio.h>
#include <pixy.h>

/**
 * Class to work with a CMUcam5 Pixy camera to access the objects
 * detected by the camera (a light C++ wrapper around the pixy C
 * methods).
 */

class PixyTokens {

private:
  /** Current status of the Pixy (0 is good). */
  int status;

  /** The major version number of the Pixy firmware. */
  int versionMaj;

  /** The minor version number of the Pixy firmware. */
  int versionMin;

  /** The build version number of the Pixy firmware. */
  int versionBuild;

  /** The number of frames that we have read objects from the Pixy. */
  int frames;

  /** The maximum number of objects we care about per frame. */
  int maxBlocks;

  /** The number of objects found in the last frame. */
  int numBlocks;

  /** Storage for the maximum number of objects per frame. */
  Block* blocks;

public:
  /**
   * Construct a new instance with a maximum number of objects per block.
   *
   * @param maxBlocks The maximum number of objects you care about
   * finding each time (defaults to 25).
   */
  PixyTokens(int maxBlocks = 25);

  /**
   * Releases allocated memory.
   */
  ~PixyTokens();

  /**
   * Opens a connection to the Pixy camera.
   *
   * @return true If successfully opened, false if not.
   */
  bool open();

  /**
   * Closes the connection to the Pixy camera (do this when you are done).
   */
  void close();

  /**
   * Indicates whether or not you have the camera open.
   *
   * @return true If camera is open and you can start reading frames.
   */
  bool isOpen() const { return (status == 0); }

  /**
   * Returns true if the Pixy has new data since the last time you read data.
   *
   * @return true if new data is available.
   */
  bool hasNewData();

  /**
   * Reads in all of the objects (up to your maximum count) detected
   * by the Pixy in the last frame.
   *
   * @return A value greater than 0 indicates the number of objects
   * found, a value of 0 indicates no objects were found. A value less
   * than 0 indicates an error has occurred (you will probably need to
   * close/open the Pixy camera again).
   */
  int readData();

  /**
   * Gets the total number of frames you have read from the Pixy so far.
   *
   * @return The number of times readData() was called and did not
   * report an error.
   */
  int getFrames() const { return frames; }

  /**
   * Returns the number of detected objects that you have access to
   * from the last readData() invocation.
   *
   * @return The number of detected objects from the last readData().
   */
  int blocksAvailable() const { return numBlocks; }

  /**
   * Get access to one of the detected objects.
   *
   * @param idx The index of the block in the range of [0,blocksAvailable()-1]
   * @return Reference to the object detected.
   */
  const Block& getBlock(int idx) const { return blocks[idx]; }

  /**
   * Formats information about a detected block as a string for
   * diagnostic output.
   *
   * @param idx The index of the block in the range of [0,blocksAvailable()-1]
   * @return Reference to the object detected.
   */
  std::string formatBlock(int idx) const {
    char buf[128]; blocks[idx].print(buf); return buf;
  }

  /**
   * Get the major version number of the Pixy firmware.
   *
   * @return Firmware major version number.
   */
  int getVersionMaj() const { return versionMaj; }

  /**
   * Get the minor version number of the Pixy firmware.
   *
   * @return Firmware minor version number.
   */
  int getVersionMin() const { return versionMin; }

  /**
   * Get the build version number of the Pixy firmware.
   *
   * @return Firmware build version number.
   */
  int getVersionBuild() const { return versionBuild; }

  /**
   * Get the firmware version identifier as a string.
   *
   * @return Firmware version like: "1.0.2" ("MAJ.MIN.BLD").
   */
  std::string getVersion() const;

  /**
   * Returns true if an error has occurred while trying to access the Pixy.
   *
   * @return true if the is a problem with the connection (you can try
   * to close/open to resolve it).
   */
  bool hasError() const { return (status != 0); }

  /**
   * Prints the Pixy error to stdout (for diagnostic purposes).
   */
  void printErrorMessage() const;
};
