/*
 * PlanGenerator.h
 *
 *  Created on: 2010-5-12
 *      Author: liupu
 */

#ifndef PLANGENERATOR_H_
#define PLANGENERATOR_H_

class IRepository;
class TripleBitQueryGraph;

#include "IRepository.h"
#include "TripleBitQueryGraph.h"
#include "TripleBit.h"

class PlanGenerator {
private:
	IRepository& repo;
	TripleBitQueryGraph::SubQuery* query;
	TripleBitQueryGraph* graph;
	static PlanGenerator* self;
public:
	PlanGenerator(IRepository& _repo);
	Status generatePlan(TripleBitQueryGraph& _graph);
	virtual ~PlanGenerator();
	static PlanGenerator* getInstance();
	int	getSelectivity(TripleBitQueryGraph::TripleNodeID& tripleID);
private:
	/// Generate the scan operator for the query pattern.
	Status generateScanOperator(TripleBitQueryGraph::TripleNode& node, TripleBitQueryGraph::JoinVariableNodeID varID);
	Status generateSelectivity(TripleBitQueryGraph::JoinVariableNode& node, map<TripleBitQueryGraph::JoinVariableNodeID,int>& selectivityMap);
	TripleBitQueryGraph::JoinVariableNode::JoinType getJoinType(TripleBitQueryGraph::JoinVariableNode& node);
	Status bfsTraverseVariableNode();
	Status getAdjVariableByID(TripleBitQueryGraph::JoinVariableNodeID id, vector<TripleBitQueryGraph::JoinVariableNodeID>& nodes);
};

#endif /* PLANGENERATOR_H_ */
