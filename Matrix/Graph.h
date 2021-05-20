#ifndef Graph_H_
#define Graph_H_

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

struct IDmap{
	ID oldID;
	ID newID;
};

class Graph {
private:
	ID *oldID_mapto_newID;
	ID *newID_mapto_oldID;
	ID *newID_mapto_newIDass;

	vector < vector <ID> > excpID;
	ID *root_located;
	ID *cross_located;
	vector <ID> cross_id;
	vector < vector <ID> > cross_located_arr;
	vector < vector <ID> > crossid_mapto_newID;
	vector < vector <IDmap> > idmap;
	vector < vector <ID> > interval;
	vector < vector <ID> > crossid_adj;

	//ÖÜË«
	ID *newGraph_id;
	unsigned *id_belong_tree;

	unsigned reverse_adjNum;

	ID *scc_located;
	ID *scc_num_located;

	unsigned DAG_idcount;
	ID DAG_maxID;
	unsigned excp_count;
	unsigned excp_idcount;
	unsigned *excp_located;

	vector< vector<ID> > hv;
	unsigned *Lup;
	unsigned *Ldown;
	unsigned *gLup;
	unsigned *gLdown;
	ID *idtag;
	//struct Abnormal * abnormal;
	struct Abnormal * first_abnormal;
	//struct new_tree_adj *new_tree;
	unsigned abnormal_count;
	int ID_count;
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

	vector<char *> reverse_newGrpah;
	unsigned *global_reverse_newGrpah_index;

	vector<MMapBuffer *> reverse_newGrpahMap;
	MMapBuffer *global_reverse_newGrpah_indexMap;

	vector<char *> hugeVertices;
	unsigned *global_hugeVertices_index;

	vector<MMapBuffer *> hugeVerticesMap;
	MMapBuffer *global_hugeVertices_indexMap;

	ID reverse_start_ID[100];
	ID *scc_start_ID;
	ID DAG_start_ID[100];
	ID excp_start_ID[100];
	ID newGraph_start_ID[100];
	unsigned newGraph_idcount;

	//Degree* newGraph_degree;
	//unsigned scc_num;
	//ÖÜË«

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
	Graph(ID maxID);
	Graph(string dir);
	Graph(istream &in, string dir, unsigned f);
	Graph(istream &in, istream &inScc, string dir1, string dir2, string dir3, string dir4);
	virtual ~Graph();
	void init(bool forward_or_backward);

	status readScc(istream& in);
	status readGraph(istream& in);
	status readGraph2(istream& in, istream& inScc, string path, string path2, string path3, string path4);
	void construct_adj(TempFile *tempfile);
	void construct_reverse_adj(TempFile *tempfile);
	void construct_adj_DAG(string path, TempFile *tempfile);
	void construct_reverse_adj_DAG(TempFile *tempfile);
	status convert_edge(string inputfile, unsigned lineCount);
	status convert_edge_to_reverse_adj(string inputfile, unsigned lineCount);
	status convert_adj(string inputfile, unsigned lineCount);
	status convert_adj_nonum(string inputfile, unsigned lineCount);
	status convert_adj_to_reverse_adj(string inputfile, unsigned lineCount);

	void sort_degree(unsigned fileindex);
	void DFS(bool forward_or_backward = true);

	unsigned getAdjNum(){ return adjNum; }
	unsigned getMaxID(){ return maxID; }
	static void FIXLINE(char * s);

	static size_t loadFileinMemory(const char* filePath, char*& buf);
	static void parallel_load_task(char *&buf, MemoryMappedFile *&temp, unsigned long cur_pos, size_t copy_size);
	static void parallel_load_inmemory(const char* filePath, char*& buf);

	ID get_idcount(){return idcount;}
	void pre_code(string path, unsigned is_first, struct timeval &start_time, struct timeval &end_time, struct timeval &end_time1);
	void construct_index(string path);
	bool topo_reach(ID x, ID y, bool *cross_vertices_arr, unsigned h);
	void strTrimRight(string& str);
	void check(ID x, ID y);
	ID tranverse(ID x, ID y);
	void prepare_query(string path, char* testfilename);
	void process(string path, char *testfilename);

private:
	void recode(TempFile *&tempfile, bool *&visited_arr);
	void generate_querys(string path);

	inline bool isNewIDExist(ID id, ID& newoff);
	inline bool setNew(ID pos, ID oldID);
	inline bool setOld(ID pos, ID newID);
	char* getOffset(ID);

	inline bool zeroDegree(ID id);
	inline bool both_in_and_out_vertex(ID id);
	void get_root_vertices(ID maxID);

	unsigned DFS_V(ID);
	unsigned DFS_V(ID v, bool* &neighbors_visited, ID &newRootID);

	//-------------------------------------------------------------------------------------
	void get_forward_root_vertices(ID maxID);
	void mark_cross_vertices(string path, ID *root_arr, unsigned rootNum, bool *&visited_arr, bool *&cross_vertices_arr);
	void cross_collector(ID *cross_id, unsigned cross_idcount, bool *cross_vertices_arr, bool *&visited_arr, TempFile &excp_id, TempFile &new_graph_root);
	void deal_with_cross_vertex(bool *&visited_arr, bool *cross_vertices_arr, ID crossid, TempFile &excp_id);

	void get_backward_root_vertices(ID maxID);
	void get_old_root_vertices(ID maxID);
	ID posttranv_tree(ID id, unsigned count, bool *cross_vertices_arr);
	bool is_exceptions(ID id1, ID id2);
	void mark_vertices_tag(TempFile &new_graph_root, bool *cross_vertices_arr);

	void DFS_Fst(string path, ID *&scc_tag, bool *&visited_arr);
	void DFS_Sec(string path, ID *&scc_tag, bool *&visited_arr);
	ID *get_unvisited_neighbors(ID curID, bool *visited_arr, ID *&tmpvalue_arr, ID component);
	ID *get_unvisited_neighbors(ID curID, bool *visited_arr);
	void DFS_V(ID* root_arr, unsigned rootNum, ID *&scc_tag, bool *&visited_arr);
	void DFS_V(ID id, ID *&scc_tag, bool *&visited_arr);
	void find_max_scc_tag(ID *scc_tag, bool *visited_arr);
	bool reverse_DFS_V(ID id, bool *&visited_arr, EntityIDBuffer*& tempEnt);
	char* reverse_getOffset(ID);
	char* scc_getOffset(ID);
	char* DAG_getOffset(ID);
	char* DAG_reverse_getOffset(ID);
	char* excp_getOffset(unsigned);
	char* newGraph_getOffset(ID);
	char* reverse_newGraph_getOffset(ID);
	char* hugeVertices_getOffset(ID);
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
	void init_reverse_newGraph();
	void init_hugeVertices();
	bool is_equal(deque<ID>* que, ID curID);
	void deal_with_scc(ID i, unsigned &ct, ID *&scc_visite_firstid, EntityIDBuffer *&tempEnt, TempFile &DAGfile);
	bool find_cycles(ID id, bool *&visited_arr, bool *cross_vertices_arr);
	bool find_visited(ID id, ID *addr, unsigned k);
	void generate_DAG(string path);
	status generate_DAG_reverse_adj(TempFile &DAGfile);
	bool is_new_root(ID root_id, ID *oldroot_arr, unsigned oldrootNum);
	void dfs_code(string path, bool *&cross_vertices_arr, bool *&visited_arr, unsigned h);
	void construct_excp_id(TempFile &excp_id);
	void deal_with_excp(TempFile &new_graph_root, bool *&cross_vertices_arr, unsigned &cross_idcount);
	void construct_newGraph(ID *root_arr, unsigned rootNum, TempFile &new_graph, TempFile &new_graph_root, bool *cross_vertices_arr, bool *visited_arr, unsigned cross_idcount);
	void construct_newGraph_ass(ID *root_arr, unsigned rootNum, TempFile &new_graph_root, bool *cross_vertices_arr, bool *visited_arr, unsigned cross_idcount);
	void construct_newGraph_idtag(bool *cross_vertices_arr);
	ID posttranv_tree(ID id, bool *cross_vertices_arr);
	void construct_newGraph_adj(TempFile &new_graph, TempFile &new_graph_path, bool *cross_vertices_arr);
	void construct_newGraph_reverse_adj(TempFile &new_graph);
	void newGraph_Optimization(ID *root_arr, unsigned rootNum, unsigned h);
	void optimization(ID *root_arr, unsigned rootNum, bool *&visited_arr);
	unsigned cal_Lup(ID id, bool *&visited_arr);
	unsigned cal_Ldown(ID id, bool *&visited_arr);
	unsigned cal_gLup(ID id, bool *&visited_arr);
	unsigned cal_gLdown(ID id, bool *&visited_arr);
	void cal_hv(ID id, unsigned h, bool *&visited_arr);
	void construct_newGraph_levelLabel(bool *cross_vertices_arr);
	void construct_hugeVertices();
	void tranverse(ID id, EntityIDBuffer*& tempEnt);
	void vertices_reachability(struct timeval &start_time, struct timeval &end_time1);
	unsigned belong_tree(ID id);
	bool tree_reach(ID x, ID y);
	bool graph_reach(ID x, ID y, unsigned h);
	bool is_existed(ID x, ID y);
	bool is_reachable(ID x, ID y, unsigned h, bool *&visit);
	bool new_tree_reach(ID x, ID y, unsigned x_belong);
	bool newGraph_reachable(ID id1, ID id2);
	bool find_reach_path(ID id1, ID id2);
	void convert_DAG_adj();

	void construct(string path, bool *&cross_vertices_arr, bool *&visited_arr);
	void cal_cross_located_arr(unsigned clt, bool *&visited_arr, TempFile &crosslocatedarr);
	void is_excp(bool *&cross_vertices_arr, bool *&visited_arr);
	void cal_tree_degree(ID root, unsigned *&tree_degree, bool *&visit);
	void cal_tree_interval(ID *root_arr, unsigned rootNum, bool *&visited_arr);
	void deal_with_crossid_interval();
	ID pt_tranv_tree(ID id, bool *&visited_arr);
	ID pt_tranv_tree(ID id, bool *&visited_arr, unsigned *&cross_tag);
	ID pt_tran_cross_id(ID id, bool *&visited_arr, unsigned *&cross_tag);
	bool is_excp(ID id1, ID id2);
	void cal_tree_excp(ID id, bool *&visited_arr);
	void write_interval();
	void write_excp();
	void init_degree();
	void is_same_root(unsigned l, bool *&visit);
};
#endif /* Graph_H_ */
