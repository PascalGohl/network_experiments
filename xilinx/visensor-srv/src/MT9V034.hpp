/*
 * MT9V034.hpp
 *
 *  Created on: Jul 15, 2013
 *      Author: pascal
 */

#ifndef MT9V034_HPP_
#define MT9V034_HPP_

#include <string>

#include "I2CBusRaw.h"
#include "Sensor.hpp"

class MT9V034: public Sensor {
public:
	MT9V034(int register_address);
	virtual ~MT9V034();

	void power_on();
	void power_off();

private:
	I2CBusRaw i2c_bus;
};

#endif /* MT9V034_HPP_ */
