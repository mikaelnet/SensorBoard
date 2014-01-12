/*
 * bmp085_tests.h
 *
 * Created: 2013-12-29 10:13:03
 *  Author: mikael
 */ 


#ifndef BMP085_TESTS_H_
#define BMP085_TESTS_H_

#if BMP085_ENABLE==1

#ifdef __cplusplus
extern "C" {
#endif

extern void bmp085_tests_setup();
extern bool bmp085_tests();

#ifdef __cplusplus
}
#endif

#endif

#endif /* BMP085_TESTS_H_ */