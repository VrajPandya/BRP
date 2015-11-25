/*
Licensed under creative commons 4.0.
License: Please see the "CREATIVE COMMONS LICENSE" file.
*/
#include"dropMessage.h"
#include<stdlib.h>
/*
 * This function returns 1 if generated random number is less than parameter value p.
 * else it returns 0.
 * p is probability value which is given by user program.
 */
int dropMessage(double p) {
	return (random_generator(0, 1) < p) ? 1 : 0;
}

/*
 * This function returns random number generated between min and max values.
 * */
double random_generator(double min, double max) {
	return min + rand() / ((double) RAND_MAX / (max - min));
}
