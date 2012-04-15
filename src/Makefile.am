SUBDIRS = include

AM_CPPFLAGS = @PCFLAGS@ -I$(srcdir)/include -I$(top_srcdir)/libmissing -I$(top_builddir)/libmissing -DSQL_PLUGIN_DIR=\"@sql_plugin_dir@\" -DFORMAT_PLUGIN_DIR=\"@format_plugin_dir@\" @LIBREQUIEM_CFLAGS@

lib_LTLIBRARIES = librequiemdb.la

if HAVE_MYSQL
 MYSQL_OBJS=-dlopen $(top_builddir)/plugins/sql/mysql/mysql.la
endif

if HAVE_PGSQL
 PGSQL_OBJS=-dlopen $(top_builddir)/plugins/sql/pgsql/pgsql.la
endif

if HAVE_SQLITE3
 SQLITE3_OBJS=-dlopen $(top_builddir)/plugins/sql/sqlite3/sqlite3.la
endif

DLOPENED_OBJS=$(MYSQL_OBJS) $(PGSQL_OBJS) $(SQLITE3_OBJS) -dlopen $(top_builddir)/plugins/format/classic/classic.la

librequiemdb_la_LDFLAGS =  \
			  -no-undefined \
			  -version-info @LIBREQUIEMDB_SONAME@ \
			  -export-symbols-regex "^requiemdb_" \
			  $(LTLIBTHREAD) \
			  #$(DLOPENED_OBJS)

librequiemdb_la_LIBADD = @LIBREQUIEM_LIBS@ $(LTLIBTHREAD) ../libmissing/libmissing.la

librequiemdb_la_SOURCES =		\
	requiemdb.c			\
	requiemdb-path-selection.c	\
	requiemdb-plugin-format.c	\
	requiemdb-plugin-sql.c		\
	requiemdb-sql.c			\
	requiemdb-sql-settings.c	\
	requiemdb-version.c		\
	requiemdb-error.c		

-include $(top_srcdir)/git.mk
