/*
 * globals.h
 *
 *  Created on: Dec 1, 2022
 *      Author: Ty Freeman
 */

#ifndef INC_GLOBALS_H_
#define INC_GLOBALS_H_

#include <inttypes.h>

/* Signal Characteristics */
typedef enum sigtype {
	ARB = 65,
	RECT = 82,
	SIN,
	TRI,
}sigType_t;

typedef struct signal {
	_Bool channel;
	sigType_t type;
	int freq;
	int min;
	int max;
	int width;
	uint16_t *ROM;
}sig_t;

/* RTOS Priority Enum */
enum  Priority{
  PriorityIdle,          ///< priority: idle (lowest)
  PriorityLow,                    ///< priority: low
  PriorityBelowNormal,          ///< priority: below normal
  PriorityNormal,          ///< priority: normal (default)
  PriorityAboveNormal,          ///< priority: above normal
  PriorityHigh,          ///< priority: high
  PriorityRealtime,          ///< priority: realtime (highest)
  PriorityError =  0x84        ///< system cannot determine priority or thread has illegal priority
};

/* Status Flags */
extern _Bool full;
extern _Bool cr_flg;
extern _Bool cmd_flg;
extern _Bool rd_flg;
extern _Bool out_flg;
extern _Bool adc_done;

extern double sum;
extern double todig;
extern double toreal;

#endif /* INC_GLOBALS_H_ */
