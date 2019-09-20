#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(silc-common, 0.1)

AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIR([m4])

if test "x" = "x$CFLAGS"
then
CFLAGS=" -O0 -g -Wall -Werror -D_GNU_SOURCE -D_XOPEN_SOURCE=500"
else
CFLAGS+=" -g -Wall -Werror -D_GNU_SOURCE -D_XOPEN_SOURCE=500"
fi

if test "x" = "x$CXXFLAGS" 
then
CXXFLAGS=" -O0 -g -Wall -Werror "
fi

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL


# Check for with-pcap
AC_MSG_CHECKING([Checking if we can use PCAP])
AC_ARG_WITH(pcap,
    AC_HELP_STRING([--with-pcap=DIR],
        [use pcap files from custom user DIR (default is system)]),
    [PCAPDIR="$with_pcap"],
    [PCAPDIR=""])

#if other PCAP folder specified then standart will assign only those folder for proper checking
#and after the verification the originals will be restored and will be added the custome folder if found PCAP
if test x$with_pcap != x
then    
    CFLAGS+=" -I$PCAPDIR"
    CXXFLAGS+=" -I$PCAPDIR"
    LDFLAGS+=" -L$PCAPDIR -L$PCAPDIR/lib -L$PCAPDIR/lib64"
fi

if test x$host_alias == xmips64-nlm-linux
then
CFLAGS+=" -DMIPS64_NLM_LINUX"
fi
if test x$host_alias == xpowerpc-fsl-linux
then
CFLAGS+=" -DPOWERPC_FSL_LINUX"
fi

AC_MSG_NOTICE(using CFLAGS as "$CFLAGS")
AC_MSG_NOTICE(using CXXFLAGS as "$CXXFLAGS")


# Checks for libraries.

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h netinet/in.h stdint.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
#AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([dup2 gettimeofday inet_ntoa memset select socket strerror strstr strtoul])

AC_SUBST([CFLAG_SILC_COMMON], ['-I$(top_srcdir)/include -I$(top_srcdir)/include/silc_common'])


AC_CONFIG_FILES([Makefile
                 lib/Makefile
                 include/Makefile
                 test/Makefile
                 net_test/Makefile

])

AC_OUTPUT

AC_MSG_NOTICE([  NetPerform Common Include = ${CFLAG_SILC_COMMON}])
AC_MSG_NOTICE([  build for platform $host])

