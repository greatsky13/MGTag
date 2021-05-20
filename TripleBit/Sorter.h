/*
 * Sorter.h
 * Create on: 2011-3-15
 * 	  Author: liupu
 */
#ifndef SORTER_H
#define SORTER_H

#include <vector>
#include <queue>
#include <fstream>
#include <sstream>
#include <sys/mman.h>
#include "MMapBuffer.h"
#include "MutexLock.h"
#include "MemoryBuffer.h"

class TempFile;

/// Sort a temporary file

struct Range {
    const char* from, *to;

    /// Constructor
    Range(const char* from, const char* to) :
        from(from), to(to) {
    }

    /// Some content?
    bool equals(const Range& o) {
        return ((to - from) == (o.to - o.from)) && (memcmp(from, o.from, to - from) == 0);
    }
/*    Range* operator=(Range& _range) {
		this->from = _range.from;
		this->to = _range.to;
		return this;
	}*/
};

class Sorter {
public:
	static MutexLock flushLock;
    /// Sort a file
    static void sort(TempFile& in,TempFile& out,const char* (*skip)(const char*),int (*compare)(const char*,const char*));

    static void mergeRun(MMapBuffer* in, vector<Range>& inRange, MMapBuffer* out, Range& outRange  );

    static void mergeRun1(MMapBuffer* in, vector<Range>& inRange, MMapBuffer* out, Range& outRange  );

    static void flush(vector<Range>& runs, TempFile* in, TempFile* out);

    static bool checkRight(const char* reader,const char *limit);

    static bool printCouple(const char* reader,const char *limit);
};

struct MergeRunWrapperArgs {
	pthread_t m_thread;
	MMapBuffer* m_in;
	vector<Range> m_inRange;
	MMapBuffer* m_out;
	Range& m_outRange;
	static MutexLock flushLock;

	MergeRunWrapperArgs(MMapBuffer* in, vector<Range>& inRange, MMapBuffer* out, Range& outRange) :
		m_thread(-1), m_in(in), m_inRange(inRange),m_out(out),m_outRange(outRange) {
		pthread_create(&m_thread, NULL, (void*(*)(void*)) MergeRun, this);
	}
	~MergeRunWrapperArgs() {
		pthread_join(m_thread, NULL);
/*		cout <<"delete:" <<(m_outRange.from - m_out->get_address())
						<< "    " << (m_outRange.to - m_out->get_address())
						<<"  len:" <<(m_outRange.to -m_outRange.from) << endl;*/
		m_thread = -1;
		m_inRange = vector<Range>();
	}

	static inline void MergeRun(MergeRunWrapperArgs* args) {
		Sorter::mergeRun1(args->m_in,args->m_inRange, args->m_out,
				args->m_outRange);
	}
};
#endif /*SOTER_H*/
