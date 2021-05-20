/*
 * HashID.h
 *
 *  Created on: 2013-6-4
 *      Author: yaya
 */

#ifndef HASHID_H_
#define HASHID_H_

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

struct Degree{
	unsigned indeg;
	unsigned outdeg;
};

struct startid_offset{
	ID startid;
	size_t offset;
};

struct startid_offset_index{
	ID startid;
	unsigned len;
};

//////啊
struct Child{
	ID id;
	struct Child *next;
};

struct Abnormal{
		ID parent;
		struct Child *first_child;//最大出度
		unsigned child_num;
		struct Abnormal *next;
};
struct sccID{
	ID* scc_id;
	unsigned num;
};
//////////
class HashID {
private:
	ID *oldID_mapto_newID;
	ID *newID_mapto_oldID;

	//周双
	ID *idtag;
	//struct Abnormal * abnormal;
	struct Abnormal * first_abnormal;
	//struct new_tree_adj *new_tree;
	unsigned abnormal_count;
	unsigned ID_count;
	unsigned scc_num;
	struct sccID *scc_ID;

	vector<char *> reverse_adj;
	unsigned *reverse_global_adj_index;

	vector<MMapBuffer *> reverse_adjMap;
	MMapBuffer *reverse_global_adj_indexMap;

	vector<char *> scc;
	unsigned *global_scc_index;

	vector<MMapBuffer *> sccMap;
	MMapBuffer *global_scc_indexMap;

	vector<char *> DAG;
	unsigned *global_DAG_index;

	vector<MMapBuffer *> DAGMap;
	MMapBuffer *global_DAG_indexMap;

	vector<char *> DAG_reverse_adj;
	unsigned *DAG_reverse_global_adj_index;

	vector<MMapBuffer *> DAG_reverse_adjMap;
	MMapBuffer *DAG_reverse_global_adj_indexMap;

	vector<char *> exceptions;
	unsigned *global_exceptions_index;

	vector<MMapBuffer *> exceptionsMap;
	MMapBuffer *global_exceptions_indexMap;

	vector<char *> newGrpah;
	unsigned *global_newGrpah_index;

	vector<MMapBuffer *> newGrpahMap;
	MMapBuffer *global_newGrpah_indexMap;

	vector<char *> collect_excp;
	unsigned *global_collect_excp_index;

	vector<MMapBuffer *> collect_excpMap;
	MMapBuffer *global_collect_excp_indexMap;

	ID reverse_start_ID[100];
	ID *scc_start_ID;
	ID DAG_start_ID[100];
	ID excp_start_ID[100];
	ID newGraph_start_ID[100];
	unsigned newGraph_idcount;
	ID *newGraph_id;
	unsigned *id_belong_tree;

	unsigned reverse_adjNum;

	ID *scc_located;

	unsigned DAG_idcount;
	ID DAG_maxID;
	unsigned excp_count;
	unsigned excp_idcount;
	unsigned *excp_located;

	//unsigned scc_num;
	//周双

	ID maxID;
	ID maxFromID;
	unsigned maxCount;
	size_t totalCouple;
	unsigned adjNum;

	vector<char *> adj;
	unsigned *global_adj_index;

	vector<MMapBuffer *> adjMap;
	MMapBuffer *global_adj_indexMap;

	ID start_ID[100];
	Degree* degree;

	stack<ID> s;
	ID idcount;
	static string Dir;

public:
	HashID(ID maxID);
	HashID(string dir);
	virtual ~HashID();
	void init(bool forward_or_backward);

	status convert_edge(string inputfile, unsigned lineCount);
	status convert_edge_to_reverse_adj(string inputfile, unsigned lineCount);
	status convert_adj(string inputfile, unsigned lineCount);
	status convert_adj_nonum(string inputfile, unsigned lineCount);
	status convert_adj_to_reverse_adj(string inputfile, unsigned lineCount);

	void sort_degree(unsigned fileindex);
	void DFS(bool forward_or_backward = true);
	ID convertToRaw(TempFile *rawFile, bool forward_or_backward = true);

	unsigned getAdjNum(){ return adjNum; }
	unsigned getMaxID(){ return maxID; }
	static void FIXLINE(char * s);

	static size_t loadFileinMemory(const char* filePath, char*& buf);
	static void parallel_load_task(char *&buf, MemoryMappedFile *&temp, unsigned long cur_pos, size_t copy_size);
	static void parallel_load_inmemory(const char* filePath, char*& buf);

	void encode(bool forward_or_backward);
	
	//------------------------------------------------------------------------------------
	void topology_encode(string path);
	static bool intersect_or_not(ID vertex1, ID vertex2, startid_offset_index *offset_index_arr, unsigned len, startid_offset *offset_arr, const char *adj_ptr);

	/////////////////双双
	ID get_idcount(){return idcount;}
	void pre_code(string path, unsigned is_first, struct timeval &start_time, struct timeval &end_time, struct timeval &end_time1);
	void vertices_reachable(ID x, ID y, bool &reachable);
//	void DFS_Fst(string path);
///	void DFS_Sec(string path, bool *&is_scc_vertices_arr, bool *&visited_arr);
	////////////////双双
private:
	inline bool isNewIDExist(ID id, ID& newoff);
	inline bool setNew(ID pos, ID oldID);
	inline bool setOld(ID pos, ID newID);
	char* getOffset(ID);

	inline bool zeroDegree(ID id);
	inline bool both_in_and_out_vertex(ID id);
	void get_root_vertices(ID maxID);

	unsigned DFS_V(ID);
	unsigned DFS_V(ID v, bool* &neighbors_visited, ID &newRootID);

	unsigned bfs_tree_builder(ID rootID, bool *&visited, hash_map<ID, size_t> &id_offset);
	void dfs_forward_encode(ID rootID, bool *&visited, hash_map<ID, size_t> id_offset, ID &newRootID);
	void dfs_backward_encode(ID rootID, bool *&visited, hash_map<ID, size_t> id_offset, ID &newRootID, unsigned unique_vertices_num);
	void save_encoded_ids(bool forward_or_backward);
	
	//-------------------------------------------------------------------------------------
	void get_forward_root_vertices(ID maxID);
	ID *mark_cross_vertices(string path, ID *root_arr, unsigned rootNum, bool *&visited_arr, bool *&cross_vertices_arr, unsigned &cross_idcount);
	void deal_with_cross_vertex(bool *&visited_arr, bool *cross_vertices_arr, ID crossid, TempFile &excp_id);
	void cross_collector(ID *cross_id, unsigned cross_idcount, bool *cross_vertices_arr, bool *&visited_arr, TempFile &excp_id, TempFile &new_graph_root);
	void construct_range_graph(TempFile &sorted_range, TempFile &range_graph, size_t &adjsize);
	void convert_range_graph_into_adjlist(string path, TempFile &sorted_range_graph, size_t adjsize);
	void subtree_collector(string path, ID *root_arr, unsigned rootNum, bool *&visited_arr, bool *cross_vertices_arr);

	//////啊
	//struct Abnormal *find_abnormal(ID id);
	void get_backward_root_vertices(ID maxID);
	void get_old_root_vertices(ID maxID);
	ID posttranv_tree(ID id, unsigned count, bool *cross_vertices_arr);
	bool find_equal(ID root_id, ID child_id, bool *cross_vertices_arr);
	bool is_exceptions(ID id1, ID id2);
	void mark_vertices_tag(TempFile &new_graph_root, bool *cross_vertices_arr);
	void deal_with_excp(TempFile &new_graph_root, bool *&cross_vertices_arr);
	void abnormal_mapto_newID(struct Abnormal *abn);

	void DFS_Fst(string path, ID *&scc_tag, bool *&visited_arr);
	void DFS_Sec(string path, ID *&scc_tag, bool *&visited_arr);
	ID *get_unvisited_neighbors(ID curID, bool *visited_arr, ID *&tmpvalue_arr, ID component);
	ID *get_unvisited_neighbors(ID curID, bool *visited_arr);
	void DFS_V(ID* root_arr, unsigned rootNum, ID *&scc_tag, bool *&visited_arr);
	void DFS_V(ID id, ID *&scc_tag, bool *&visited_arr);
	ID* find_max_scc_tag(ID *scc_tag, bool *visited_arr);
	deque<ID>* reverse_DFS_V(ID id, bool *&visited_arr);
	char* reverse_getOffset(ID);
	char* scc_getOffset(ID);
	char* DAG_getOffset(ID);
	char* DAG_reverse_getOffset(ID);
	char* excp_getOffset(unsigned);
	char* newGraph_getOffset(ID);
	unsigned find_idcount(ID id);
	void print_adj();
	void print_reverse_adj();
	void print_scc();
	void print_DAG();
	void print_DAG_degree();
	void print_excp();
	void print_newGraph();
	void print_newID_newGraph();
	void init_scc();
	void init_DAG();
	void init_reverse_DAG();
	void init_excp();
	void init_newGraph();
	bool is_equal(deque<ID>* que, ID curID);
	void deal_with_scc(ID i, unsigned &ct, ID *&scc_visite_firstid, EntityIDBuffer *&tempEnt, TempFile &DAGfile);
	bool find_cycles(ID id, bool *&visited_arr, bool *cross_vertices_arr);
	bool find_visited(ID id, ID *addr, unsigned k);
	void generate_DAG(string path);
	status generate_DAG_reverse_adj(TempFile &DAGfile);
	bool is_new_root(ID root_id, ID *oldroot_arr, unsigned oldrootNum);
	void dfs_code(string path, bool *cross_vertices_arr, bool *&visited_arr);
	void construct_excp_id(TempFile &excp_id);
	void excp_collect(ID *root_arr, unsigned rootNum, bool *&visited_arr);
	void construct_newGraph(ID *root_arr, unsigned rootNum, TempFile &new_graph, TempFile &new_graph_root, bool *cross_vertices_arr, bool *visited_arr);
	void construct_newGraph_idtag(bool *cross_vertices_arr);
	ID posttranv_tree(ID id, bool *cross_vertices_arr);
	void construct_newGraph_adj(TempFile &new_graph);
	void vertices_reachability(struct timeval &start_time, struct timeval &end_time1);
	unsigned belong_tree(ID id);
	bool tree_reach(ID x, ID y);
	bool new_tree_reach(ID x, ID y, unsigned x_belong);
	bool newGraph_reachable(ID id1, ID id2);
	bool find_reach_path(ID id1, ID id2);
	void convert_DAG_adj();
};
#endif /* HASHID_H_ */
