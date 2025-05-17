/*
 * Sys.h
 *
 *  Created on: 2024年10月27日
 *      Author: sun
 */

#ifndef CODE_SYS_SYS_H_
#define CODE_SYS_SYS_H_

#include "MyHeadFile.h"

#define LIMIT_VAL(a,min,max) ((a)<(min)?(min):((a)>(max)?(max):(a)))
//#include "zf_common_typedef.h"

float absolute(float z);
int16 Compare_Num(int16 a, int16 b, uint8 compare_value);
void Triangular_Filter(uint8 *line, uint8 start, uint8 end);
float Q_rsqrt( float number );
void my_pit_init (pit_index_enum pit_index, float time);

#endif /* CODE_SYS_SYS_H_ */
