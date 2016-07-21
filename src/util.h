/**
 * Utility functions that don't belong to a specific class
 * @file util.h
 */

#include <Arduino.h>

/**
 * Convert an ASCII hex character to its integer value
 *
 * @param c The character (0-9, a-f or A-F)
 * @return The integer value or -1 if it is invalid
 */
int8_t asciiToHex(char c);

/**
 * Check if a string contains one or more colons
 *
 * @param str The string to check
 * @return True if the string contains at least one colon
 */
boolean containsColon(const char *str);

/**
 * Print a 2-byte human readable hex value for an 8-bit integer
 *
 * @param byte The value to print (in range 0x00 to 0xFF)
 * @param p The stream to print to (default Serial)
 */
void printPaddedHex(uint8_t byte, Print &p=Serial);

/**
 * Calculate a IP type 16-bit checksum for a buffer
 *
 * @param sum The current sum accumulator (or 0 for first call)
 * @param data A pointer to the data buffer to calculate checksum for
 * @param len The length of the data (in bytes) to perform checksum on
 * @return The calculated checksum
 */
uint16_t chksum(uint16_t sum, const uint8_t *data, uint16_t len);
