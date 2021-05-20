/*
 * QuerySemanticAnalysis.h
 *
 *  Created on: 2010-4-8
 *      Author: wdl
 */

#ifndef QUERYSEMANTICANALYSIS_H_
#define QUERYSEMANTICANALYSIS_H_

#include "IRepository.h"

class IRepository;
class SPARQLParser;
class TripleBitQueryGraph;

///Semantic Analysis for SPARQL query. Transform the parse result into a query Graph.
class QuerySemanticAnalysis {

private:
	///Repository use for String and URI lookup.
	IRepository& repo;
public:
	QuerySemanticAnalysis(IRepository &repo);
	virtual ~QuerySemanticAnalysis();

	/// Perform the transformation
	bool transform(const SPARQLParser& input, TripleBitQueryGraph& output);
};

#endif /* QUERYSEMANTICANALYSIS_H_ */
