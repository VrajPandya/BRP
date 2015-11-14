#include"dropMessage.h"
/*
 * This function returns 1 if generated random number is less than parameter value p.
 * else it returns 0.
 * p is probabilty value which is given by user program.
 */
int dropMessage(double p){
	if(random_generator(0,1) < p)
		return 1;
	else
		return 0;
}

/*
 * This function returns random number generated between 0 and 1.
 * */
double random_generator(double min,double max){
	return min + rand() / ((double)RAND_MAX / (max - min));
}
