/*
 * RDFQuery.h
 *
 *  Created on: May 26, 2010
 *      Author: root
 */

#ifndef RDFQUERY_H_
#define RDFQUERY_H_

class SPARQLLexer;
class SPARQLParser;
class QuerySemanticAnalysis;
class PlanGenerator;
class TripleBitQuery;
class TripleBitQueryGraph;
class TripleBitRepository;
class TripleBitBuilder;

#include "TripleBit.h"
#include <string>
using namespace std;

class RDFQuery {
private:
	SPARQLLexer* lexer;
	SPARQLParser* parser;
	QuerySemanticAnalysis* semAnalysis;
	PlanGenerator* planGen;
	TripleBitQuery* bitmapQuery;
	TripleBitQueryGraph* queryGraph;
	TripleBitRepository* repo;
public:
	RDFQuery(TripleBitQuery* _bitmapQuery, TripleBitRepository* _repo);
	Status Execute(string& queryString, vector<string>& resultSet);
	void Print();
	virtual ~RDFQuery();
};

#endif /* RDFQUERY_H_ */
