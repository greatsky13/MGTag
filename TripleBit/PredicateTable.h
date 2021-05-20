/*
 * PredicateTable.h
 *
 *  Created on: Mar 11, 2010
 *      Author: root
 */

#ifndef PREDICATETABLE_H_
#define PREDICATETABLE_H_

#include "StringIDSegment.h"
#include "TripleBit.h"

class PredicateTable {
	StringIDSegment* prefix_segment;
	StringIDSegment* suffix_segment;
	LengthString prefix, suffix;
	LengthString searchLen;

	string SINGLE;
	string searchStr;

private:
	Status getPrefix(const char* URI);
public:
	PredicateTable() : SINGLE("single") { }
	PredicateTable(const string dir);
	virtual ~PredicateTable();
	Status insertTable(const char* str, ID& id);
	string getPrediacateByID(ID id);
	Status getIDByPredicate(const char* str, ID& id);

	size_t getSize() {
		return prefix_segment->getSize() + suffix_segment->getSize();
	}

	size_t getPredicateNo();
	void dump();
public:
	static PredicateTable* load(const string dir);
};

#endif /* PREDICATETABLE_H_ */
