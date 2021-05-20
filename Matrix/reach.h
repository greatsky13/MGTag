/*
 * reach.h
 *
 *  Created on: 2015-6-26
 *      Author: shuangzhou
 */

#ifndef REACH_H_
#define REACH_H_

#include "MemoryBuffer.h"
#include <vector>
#include "MMapBuffer.h"
#include "TempFile.h"
#include "TripleBitBuilder.h"
#include "MutexLock.h"
#include "EntityIDBuffer.h"
#include <Matrixmap.h>

#define MEMCPY_SIZE (16 * 1024 * 1024)
#define RANGE_BINARY_SIZE 1024
#define TMP_BUF_SIZE (16 * 1024)

struct idDegree{
	unsigned indeg;
	unsigned outdeg;
};

class reach {
private:

//	ID* rev_oldID_mapto_newID;
//	ID* rev_rl;
//	ID* rev_rlt;
//
//	ID* left_to_right;
	//ID* nor_interval;

	ID* oldID_mapto_newID;
	ID* root_layer;
	ID *rl;
	unsigned *Lup;
	unsigned *Ldown;
	ID *root_located;
	ID *rlt;
	ID* interval;
	vector < vector <ID> > excpID;
	vector < vector <ID> > id_cross_arr;

	vector<ID> TopoOrder;
	vector<ID> TopoOrder_cross;
	vector <ID> cross_id;

	vector<char *> DAG;
	unsigned *global_DAG_index;

	vector<MMapBuffer *> DAGMap;
	MMapBuffer *global_DAG_indexMap;

	vector<char *> DAG_reverse_adj;
	unsigned *DAG_reverse_global_adj_index;

	vector<MMapBuffer *> DAG_reverse_adjMap;
	MMapBuffer *DAG_reverse_global_adj_indexMap;

	ID DAG_start_ID[100];
	ID maxID;
	ID maxFromID;
	unsigned maxCount;
	size_t totalCouple;
	unsigned adjNum;
	unsigned* degree;

	stack<ID> s;
	ID idcount;
	static string Dir;

public:
	reach(ID maxID);
	reach(string dir);
	reach(istream &in, string dir);
	reach(istream &in, string dir, unsigned f);
	//reach(istream &in, string dir, int num);
	reach(istream &in, string dataset, string querypath);
	reach(istream &in, string dataset, string sccpath, string tfpath);
	reach(istream &in, string dataset, string daggraph, int num);
	virtual ~reach();

	status readGraph(istream& in);
	void strTrimRight(string& str);

	unsigned getAdjNum(){ return adjNum; }
	unsigned getMaxID(){ return maxID; }
	static void FIXLINE(char * s);
	static size_t loadFileinMemory(const char* filePath, char*& buf);
	static void parallel_load_task(char *&buf, MemoryMappedFile *&temp, unsigned long cur_pos, size_t copy_size);
	static void parallel_load_inmemory(const char* filePath, char*& buf);
	ID get_idcount(){return idcount;}

	void process(char *testfilename);
	ID tranverse(ID x, ID y);

	void generate_querys(string path);

private:
	void init_degree();
	inline bool zeroDegree(ID id);
	inline bool both_in_and_out_vertex(ID id);
	void get_forward_root_vertices(ID maxID);
	void get_backward_root_vertices(ID maxID);

	char* DAG_getOffset(ID);
	char* DAG_reverse_getOffset(ID);
	void init_DAG();
	void init_reverse_DAG();

	//ID cal_nor_interval(ID id, bool *&visited_arr);

	//construct index
	void construct_index();
	//void code(ID *root_arr, unsigned rootNum, unsigned *&curInDegree);

	void pre_process(bool *&cross_vertices_arr, unsigned *&nextInDegree);
	void decode(unsigned *&curInDegree);
	//void cal_cross_located(bool *&visited_arr ,bool *&cross_vertices_arr);
	void pre_process_cross(bool *&cross_vertices_arr, unsigned *&curInDegree, unsigned *&nextInDegree);
	//void pre_process_cross_from_root(unsigned rootNum, bool *&visited_arr, bool *&cross_vertices_arr, unsigned *&curInDegree);
	bool is_excp(ID id1, ID id2, int left, int right);
	void cal_index();
	void cal_excp_and_cross_arr();
	bool is_reachable(ID id1, ID id2, int len);
	//bool down_layer_to_up_layer_without_cross_located_mark(ID id1, ID id2);

	void cal_cross_degree(bool *&visited_arr, unsigned *&curInDegree);//, unsigned *&nextInDegree);

//	void rev_pre_process(bool *&cross_vertices_arr, unsigned *&curInDegree, unsigned *&nextInDegree);
//	void cal_rev_cross_degree(bool *&visited_arr, unsigned *&curInDegree);
//	void rev_decode(unsigned *&curInDegree);

	//query
	bool query(ID id1, ID id2);
	bool same_layer_tree_reach(ID id1, ID id2);
	bool same_tree_reach(ID id1, ID id2);
	bool down_layer_to_up_layer_without_cross_located(ID id1, ID id2);
};

#endif /* REACH_H_ */
