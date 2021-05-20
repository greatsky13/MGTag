local_src := $(wildcard $(subdirectory)/*.cpp)

$(eval $(call make-program,test, libmatrix.a libtriplebit.a,$(local_src)))

$(eval $(call compile-rules))