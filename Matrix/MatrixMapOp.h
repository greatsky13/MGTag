/*
 * MatrixMapOp.h
 *
 *  Created on: 2012-11-23
 *      Author: yaya
 */

#ifndef MATRIXMAPOP_H_
#define MATRIXMAPOP_H_

#include "Matrixmap.h"
#include "map"
#include "MutexLock.h"
struct Node {
	char left;
	char right;
};
class MatrixMapOp {
	map<char,Matrixmap*> matrixMap;
	MutexLock lock;
public:
	virtual ~MatrixMapOp();
	MatrixMapOp();
	static Status matrixFactorization(char n,map<char,Node>&);
	Status matrixMultiply(Matrixmap* matrix1, Matrixmap* matrix2,char* upperDir,ID preID);
};

#endif /* MATRIXMAPOP_H_ */
