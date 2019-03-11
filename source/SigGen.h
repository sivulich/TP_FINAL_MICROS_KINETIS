/*
 * SigGen.h
 *
 *  Created on: Feb 9, 2019
 *      Author: Santi
 */

#ifndef SIGGEN_H_
#define SIGGEN_H_
#if defined(__cplusplus)
extern "C" {
#endif

typedef struct SigGen_ SigGen_;

extern SigGen_ SigGen;

struct SigGen_{
	int (*init)();
	int (*setupSignal)(unsigned short** buffsL,unsigned short** buffsR, int qnt,unsigned len,unsigned freq);
	int (*start)();
	/*If returns 1, you should fill buff1, if it returns 2 you should fill buf2, if it returns 0 everthing is OK!*/
	int (*status)();
	int (*pause)();
	int (*stop)();
};

#if defined(__cplusplus)
}
#endif




#endif /* SIGGEN_H_ */
