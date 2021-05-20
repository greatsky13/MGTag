/**
 *
 *  BitmapWAH.h
 *  Created on : 2011-4-7
 *      Author : liupu
 *
 */

#ifndef _BITMAP_H_
#define _BITMAP_H_

#include "TripleBit.h"
#include "BitVectorWAH.h"
#include <map>

using namespace std;

class BitmapWAH
{

public:
	BitmapWAH();
	void insert(ID id, unsigned int pos);
	void print();
	size_t get_size();
	void completeInsert();
	//BitVector* getBitVector(ID id);
	virtual ~BitmapWAH();
private:
	bool isIdInBitmap(ID id);
	void expandBitmap();
	//BitVector* getBitVector(ID id);
private:
	typedef BitVectorWAH* BitMapType;
	map<ID,BitMapType> bitMap;
	size_t bitMapSize;
	unsigned int capacity;
};

#endif // !defined _BITMAP_H_
