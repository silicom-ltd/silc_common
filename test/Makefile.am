# What to build: static and shared versions of the library.
AUTOMAKE_OPTIONS = foreign 1.9
bin_PROGRAMS = testlib

ACLOCAL_AMFLAGS = -I m4

# note a special case for --Wno-redundant-decls added because of dagopts and unistd are generating redundatn declaration 
# --Wno-redundant-decls
common_include =  -I../include -O2 $(CFLAG_SILC_COMMON)
testlib_CPPFLAGS = ${common_include} 
testlib_CFLAGS = ${common_include}


testlib_LDFLAGS = -lpopt \
		${top_builddir}/lib/libsilc.la \
		-lpthread
			   

testlib_SOURCES = \
			silc_test_lib.c \
            silc_test_array_lib.c \
            silc_test_file_lib.c  \
            silc_test_mem_lib.c  \
            silc_test_ringbuf_lib.c \
            silc_test_socket_lib.c


SUBDIRS =
