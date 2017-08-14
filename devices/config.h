#ifndef DEF_config
#define DEF_config

#include <iostream>
#include <bitset>
#include <cstdint>

#define PRINT_VAR(x) std::cout << #x << "\t= " << (x) << "\n"
#define PRINT_VAR_BINARY(x) std::cout << #x << "\t= " << std::bitset<32>((x)) << "\n"
#define PRINT_VAR_BINARY_N(x, n) std::cout << #x << "\t= " << std::bitset<(n)>((x)) << "\n"

typedef double real_T;//!< Real number typedef

#define USE_DUMMY_SPI_CODE                  // define to use dummy SPI code. Otherwise, the real STM32-f4 code is used.
constexpr uint8_t SPI_MAX_TRIAL_COUNT = 10; // maximum number of trials when the CRC of the received message is wrong.

#endif
