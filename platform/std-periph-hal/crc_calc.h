#include <FreeRTOS.h>
#ifndef DEF_CRC
#define DEF_CRC

/// \brief Computes the n-CRC of the data.
/// \param data     : data to CRC, size from 0 to (32-n) bits.
/// \param poly     : polynomial to use (order n, size n+1).
/// \param seed     : seed of the CRC, size n.
/// \param datasize : size of the data, in bits.
/// \param n        : size of the seed (order of the polynomial).
///
/// \return n-CRC of the data in the n least significant bits.
uint32_t nCRC(uint32_t data, uint32_t poly, uint32_t seed, uint32_t datasize, uint32_t n);
/// \brief Prepares a message to be sent to the CMS300 sensor by adding the 4-bit CRC at the end.
/// \param data : 28-bit data with 4 leading zeros. Data between bit 27 and bit 0.
/// \return 32-bit message with data on bits 31 to 4 and CRC on bits 3 to 0.
uint32_t prepareMessageCMS300(uint32_t data);
/// \brief Checks that the recieved message is intact by computing the CRC and comparing with the stored CRC.
/// \return True if the message is intact. False if the message has been altered.
uint32_t checkCRCmessageCMS300(uint32_t msg);
/// \brief Returns a 32-bit unsigned integer with the nth bit active and all zeros elsewhere.
/// \param n : bit number between 0 and 31 included.
/// \return 32-bit unsigned integer with the nth bit active and all zeros elsewhere.
uint32_t bitN(uint32_t n);
#endif
