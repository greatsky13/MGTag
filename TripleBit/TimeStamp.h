/*
 * TimeStamp.h
 *
 *  Created on: Jan 24, 2011
 *      Author: root
 */

#ifndef TIMESTAMP_H_
#define TIMESTAMP_H_

#include <sys/time.h>
#include <stdio.h>

class TimeStamp {
	struct timeval start, end;
	double time;
public:
	TimeStamp(){
		time = 0;
	}

	void startTimer() {
		gettimeofday(&start,NULL);
	}

	void endTimer() {
		gettimeofday(&end, NULL);
		time = time + ((end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec) / 1000.0;
	}

	void printTime(const char* timername) {
		fprintf(stderr, "%s time used %f ms.\n", timername,time);
	}

	void resetTimer() {
		time = 0;
	}

	virtual ~TimeStamp(){};
};

#endif /* TIMESTAMP_H_ */
