local_src := $(wildcard $(subdirectory)/*.cpp)

$(eval $(call make-program-cilk,matrixProduct,libmatrix.a libtriplebit.a,Algorithm/matrix_product.cpp))

$(eval $(call make-program-cilk,pageRank,libmatrix.a libtriplebit.a,Algorithm/pagerank.cpp))

$(eval $(call make-program-cilk,matrix_builder,libmatrix.a libtriplebit.a,Algorithm/matrix_builder.cpp))

$(eval $(call make-program-cilk,reach_builder,libmatrix.a libtriplebit.a,Algorithm/reach_builder.cpp))

$(eval $(call make-program-cilk,graph_builder,libmatrix.a libtriplebit.a,Algorithm/graph_builder.cpp))

$(eval $(call make-program-cilk,generate_querys,libmatrix.a libtriplebit.a,Algorithm/generate_querys.cpp))

$(eval $(call make-program-cilk,range_graph_builder,libmatrix.a libtriplebit.a,Algorithm/range_graph_builder.cpp))

$(eval $(call make-program-cilk,spmv,libmatrix.a libtriplebit.a,Algorithm/spmv.cpp))

$(eval $(call make-program-cilk,bfs,libmatrix.a libtriplebit.a,Algorithm/bfs.cpp))

$(eval $(call make-program-cilk,bfs_forest,libmatrix.a libtriplebit.a,Algorithm/bfs_forest.cpp))

$(eval $(call make-program-cilk,par_bfs_forest,libmatrix.a libtriplebit.a,Algorithm/parallel_bfs_forest.cpp))

$(eval $(call make-program-cilk,ser_wcc,libmatrix.a libtriplebit.a,Algorithm/serial_wcc.cpp))

$(eval $(call make-program-cilk,par_wcc,libmatrix.a libtriplebit.a,Algorithm/parallel_wcc.cpp))

$(eval $(call make-program-cilk,pageRank_ws,libmatrix.a libtriplebit.a,Algorithm/pagerank_workstealing.cpp))

$(eval $(call make-program-cilk,spmv_ws,libmatrix.a libtriplebit.a,Algorithm/spmv_workstealing.cpp))

$(eval $(call make-program-cilk,par_bfs_forest_ws,libmatrix.a libtriplebit.a,Algorithm/parallel_bfs_forest_workstealing.cpp))

$(eval $(call make-program-cilk,par_wcc_ws,libmatrix.a libtriplebit.a,Algorithm/parallel_wcc_workstealing.cpp))


$(eval $(call make-program-cilk,generate_judgeDAG_and_tf_graph,libmatrix.a libtriplebit.a,Algorithm/generage_judgeDAG_and_tf_graph.cpp))

$(eval $(call make-program-cilk,generate_dag_and_tf,libmatrix.a libtriplebit.a,Algorithm/generate_dag_and_tf.cpp))

$(eval $(call make-program-cilk,trans_to_graph_format,libmatrix.a libtriplebit.a,Algorithm/trans_to_graph_format.cpp))

$(eval $(call make-program-cilk,generate_tol_graph,libmatrix.a libtriplebit.a,Algorithm/generate_tol_graph.cpp))

$(eval $(call compile-rules))
