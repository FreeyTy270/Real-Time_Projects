/*
 * time_calc.c
 *
 *  Created on: Oct 19, 2022
 *      Author: Ty Freeman
 */

#include "time_calc.h"

void format_time(int time_since, int *hr, int *min)
{
	int hr_since = 0;
	int min_since = 0;

	min_since = time_since / MIN;
	hr_since = min_since / HR;

	*hr = (hr_since > 3) ? hr_since - 3 : START + hr_since;
	*min = (min_since % HR);

}
