/*
 * ds1820_tests.h
 *
 * Created: 2013-12-28 13:59:09
 *  Author: mikael
 */ 


#ifndef DS1820_TESTS_H_
#define DS1820_TESTS_H_

#if DS1820_ENABLE==1

#ifdef __cplusplus
extern "C" {
#endif

extern void ds1820_tests_setup();
extern bool ds1820_tests();

#ifdef __cplusplus
}
#endif

#endif

#endif /* DS1820_TESTS_H_ */