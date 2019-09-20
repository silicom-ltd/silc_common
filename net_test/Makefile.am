# What to build: static and shared versions of the library.
AUTOMAKE_OPTIONS = foreign 1.9
bin_PROGRAMS = silc_net_test

ACLOCAL_AMFLAGS = -I m4

# note a special case for --Wno-redundant-decls added because of dagopts and unistd are generating redundatn declaration 
# --Wno-redundant-decls
common_include =  -I../include -O2 $(CFLAG_SILC_COMMON)
silc_net_test_CPPFLAGS = ${common_include} 
silc_net_test_CFLAGS = ${common_include}


silc_net_test_LDADD = ${top_builddir}/lib/.libs/libsilc.a \
				-lpthread  -lpopt
			   

silc_net_test_SOURCES = \
			silc_udp_test.c silc_tcp_test.c silc_net_test.c


SUBDIRS =
