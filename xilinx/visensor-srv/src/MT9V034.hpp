/*
 * MT9V034.hpp
 *
 *  Created on: Jul 15, 2013
 *      Author: pascal
 */

#ifndef MT9V034_HPP_
#define MT9V034_HPP_

#include "I2CBus.h"

class MT9V034 {
public:
	MT9V034();
	virtual ~MT9V034();

	void power_on();
	void power_off();

private:
	I2CBus i2c_bus;
};

#endif /* MT9V034_HPP_ */
