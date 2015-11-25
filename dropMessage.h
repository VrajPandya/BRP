/*
Licensed under creative commons 4.0.
License: Please see the "CREATIVE COMMONS LICENSE" file.
*/
#ifndef DROPMESSAGE_H
#define DROPMESSAGE_H

/*
 * This function returns 1 if generated random number is less than parameter value p.
 * else it returns 0.
 * p is probabilty value which is given by user program.
 */
int dropMessage(double p);


/*
 * This function returns random number generated between 0 and 1.
 * */
double random_generator(double min,double max);
#endif
