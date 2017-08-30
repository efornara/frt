#! /bin/sh

for filename in $* ; do
	echo -n "Processing $filename... "
	libname=`basename "$filename" .dl`
	grep '^#include' <"$libname.dl" >"$libname.gen.h"
	echo >>"$libname.gen.h" \
		"static inline bool frt_load_$libname(const char *) { return true; }"
	rm -f "$libname.gen.cpp"
	echo "done."
done
