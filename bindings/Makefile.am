BINDING_DEPENDS=librequiemdb.i librequiemdb_perl.i librequiemdb_python.i \
		$(top_srcdir)/src/include/requiemdb-error.h \
		$(top_srcdir)/src/include/requiemdb.h \
		$(top_srcdir)/src/include/requiemdb-path-selection.h \
		$(top_srcdir)/src/include/requiemdb-plugin-format.h \
		$(top_srcdir)/src/include/requiemdb-plugin-sql.h \
		$(top_srcdir)/src/include/requiemdb-sql.h \
		$(top_srcdir)/src/include/requiemdb-sql-settings.h

EXTRA_DIST = librequiemdb.i librequiemdb_perl.i librequiemdb_python.i perl/Makefile.PL.in perl/RequiemDB.pm python/setup.py.in python/requiemdb.py perl/RequiemDB.c python/_requiemdb.c

CLEANFILES = perl/pm_to_blib perl/RequiemDB.o perl/blib python/build
MAINTAINERCLEANFILES =  perl/Makefile		\
			perl/Makefile.old	\
			perl/RequiemDB.bs	\
			perl/RequiemDB.c	\
			python/_requiemdb.c

all-am: perl python


if HAVE_PERL
perl: perl-build
else
perl:
endif


if HAVE_PYTHON
python: python-build
else
python:
endif



if HAVE_PERL
#
# Workaround invalid Makefile generated by MakeMaker when UTF-8 is enabled.
#
perl-makefile:
	cd perl && LANG="" DESTDIR=$(DESTDIR) LIBDIR=$(libdir) $(PERL) Makefile.PL @PERL_EXTRA_PARAMS@ CC="$(BINDINGS_CC)" 
else
perl-makefile:
endif


# proxy classes are an option for swig <= 1.3.19 and are activated by default with swig > 1.3.19
# as we don't want proxy classes, we must explicitly tell swig > 1.3.19 (that support the -noproxy option)
# to not generate them
perl/RequiemDB.c: $(BINDING_DEPENDS)
if HAVE_SWIG
	$(SWIG) -I$(top_srcdir)/src/include @LIBREQUIEM_CFLAGS@ -o $@ -perl5 -nopm \
	 `$(SWIG) -perl5 -help 2>&1 | grep noproxy > /dev/null && echo -noproxy` $(srcdir)/librequiemdb.i
endif

python/_requiemdb.c: $(BINDING_DEPENDS)
if HAVE_SWIG
	$(SWIG) -I$(top_srcdir)/src/include @LIBREQUIEM_CFLAGS@ -o $@ -python -noproxy -interface _requiemdb -module _requiemdb $(srcdir)/librequiemdb.i
endif


perl-build: perl/RequiemDB.c perl-makefile
	cd perl && $(MAKE) LD_RUN_PATH=""

python-build: python/_requiemdb.c
	cd python && CC="$(BINDINGS_CC)" $(PYTHON) setup.py build



install-exec-hook: perl-makefile
if HAVE_PERL
	cd perl && $(MAKE) LD_RUN_PATH="" install
endif

if HAVE_PYTHON
	cd python && $(PYTHON) setup.py install `test -n "$(DESTDIR)" && echo --root $(DESTDIR)`
endif

uninstall-hook: perl-makefile
if HAVE_PERL
	cd perl && $(MAKE) uninstall
endif

if HAVE_PYTHON
	cd python && $(PYTHON) setup.py uninstall `test -n "$(DESTDIR)" && echo --root $(DESTDIR)`
endif


clean-local:
if HAVE_PERL
	cd perl && if test -f Makefile; then $(MAKE) clean; fi
endif

if HAVE_PYTHON
	cd python && $(PYTHON) setup.py clean -a
endif

-include $(top_srcdir)/git.mk
