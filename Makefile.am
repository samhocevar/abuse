
include $(top_srcdir)/lol/build/autotools/common.am

ACLOCAL_AMFLAGS = -I lol/build/autotools/m4
EXTRA_DIST += COPYING.GPL COPYING.WTFPL PACKAGERS bootstrap

SUBDIRS = lol src
DIST_SUBDIRS = $(SUBDIRS) data doc

test: check

dist-hook:
	@echo ""
	@echo "          ===================================================="
if HAVE_NONFREE
	@echo "            WARNING: This tarball will contain non-free data"
else
	@echo "            WARNING: This tarball will exclude non-free data"
endif
	@echo "          ===================================================="
	@echo ""

