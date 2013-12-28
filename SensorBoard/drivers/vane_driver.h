/*
 * vane_driver.h
 *
 * Created: 2013-12-28 10:56:40
 *  Author: mikael
 */ 


#ifndef VANE_DRIVER_H_
#define VANE_DRIVER_H_

/************************************************************************/
/* Vane                                                                 */
/************************************************************************/


class WindVane 
{
	private:
		uint8_t parseReading (uint16_t reading);
	
	public:
		WindVane();	
};

#endif /* VANE_DRIVER_H_ */