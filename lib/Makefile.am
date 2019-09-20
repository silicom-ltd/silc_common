# What to build: static and shared versions of the library.
lib_LTLIBRARIES = libsilc.la


# note a special case for --Wno-redundant-decls added because of dagopts and unistd are generating redundatn declaration 
# --Wno-redundant-decls
libsilc_la_CPPFLAGS = $(CFLAG_SILC_COMMON)
libsilc_la_CFLAGS = $(CFLAG_SILC_COMMON)

libsilc_la_LDFLAGS = 
libsilc_la_SOURCES = \
                    silc_socket.c \
                    silc_logging.c \
                    silc_time.c \
                    silc_htbl_str.c \
                    silc_file.c \
                    silc_init.c \
                    silc_stats.c \
                    silc_gpio.c
#                    silc_htbl_cxxstr.cc \
#                    silc_id_gen.cc 


SUBDIRS =


