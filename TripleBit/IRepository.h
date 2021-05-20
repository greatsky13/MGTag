/*
 * IRepository.h
 *
 *  Created on: 2010-4-9
 *      Author: wdl
 */

#ifndef IREPOSITORY_H_
#define IREPOSITORY_H_

#include "TripleBit.h"

class IRepository {

public:
	IRepository(){}
	virtual ~IRepository(){}

	//virtual Status  open() = 0;
	///virtual Status  load() = 0;
	//virtual Status  create() = 0;
	//virtual void    close() = 0;

	//SO(id,string)transform
	virtual bool find_soid_by_string(SOID& soid, const std::string& str) = 0;
	virtual bool find_string_by_soid(std::string& str, SOID& soid) = 0;

	//P(id,string)transform
	virtual bool find_pid_by_string(PID& pid, const std::string& str) = 0;
	virtual bool find_string_by_pid(std::string& str, ID& pid) = 0;

	//create a Repository specific in the path .
	//static IRepository * create(const char * path);

	//Get some statistics information
	virtual int	get_predicate_count(PID pid) = 0;
	virtual int get_subject_count(ID subjectID) = 0;
	virtual int get_object_count(ID objectID) = 0;
	virtual int get_subject_predicate_count(ID subjectID, ID predicateID) = 0;
	virtual int get_object_predicate_count(ID objectID, ID predicateID) = 0;
	virtual int get_subject_object_count(ID subjectID, ID objectID) = 0;

	//scan the database;
	virtual Status getSubjectByObjectPredicate(ID oid, ID pod) = 0;
	virtual ID next() = 0;

	//Get the id by string;
	virtual bool lookup(const string& str, ID& id) = 0;
};

#endif /* IREPOSITORY_H_ */
