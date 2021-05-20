local_src := $(wildcard $(subdirectory)/*.cpp)

$(eval $(call make-program,triplebitServer,libtriplebit.a,$(local_src)))

$(eval $(call compile-rules))
