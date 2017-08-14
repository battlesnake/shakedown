#ifndef DEF_Sensor
#define DEF_Sensor

#include "config.h"

//! \brief This class represents a sensor connected to the processor via a data link such as IC or SPI.
//! It is an abstract class and must be specialized for each of the sensor types used on the board.
class Sensor
{
	public:
        Sensor();
        virtual ~Sensor();
};

#endif
