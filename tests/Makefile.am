check-local: checkpatch.pl
	find $(top_srcdir)/src -name \*.c -exec $(srcdir)/checkpatch.pl --ignore SPLIT_STRING --show-types -q --no-tree  -emacs -f {} \;
	find $(top_srcdir)/src -name \*.h -exec $(srcdir)/checkpatch.pl --ignore SPLIT_STRING --show-types -q --no-tree  -emacs -f {} \;

EXTRA_DIST = checkpatch.pl \
	test-cppcheck.sh

TESTS=

if CPPCHECK
TESTS += test-cppcheck.sh
endif