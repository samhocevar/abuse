dnl
dnl  Configure script for a Lol Engine project
dnl

AC_INIT(abuse, 0.0)

dnl
dnl  Standard autoconf setup and tools requirements
dnl

AC_PREREQ(2.50)
AC_CONFIG_AUX_DIR(.auto)
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([subdir-objects no-define tar-ustar silent-rules])
AM_DEFAULT_VERBOSITY=0

AC_PROG_CXX
AM_PROG_LIBTOOL
AC_LIBTOOL_CXX

AC_ARG_ENABLE(debug,
  [  --enable-debug          build debug versions of the game (default disabled)])
AC_ARG_ENABLE(release,
  [  --enable-release        build final release of the game (default disabled)])
AC_ARG_ENABLE(nonfree,
  [  --disable-nonfree       disable non-free data (default enabled)])
AC_ARG_ENABLE(network,
  [  --disable-network       disable networking (default enabled)])

AC_ARG_WITH(assetdir,
  [  --with-assetdir=DIR     override default game data directory])

AC_CONFIG_HEADER(config.h)

AC_CONFIG_FILES(
 [Makefile
  src/Makefile
  data/Makefile
  doc/Makefile
  doc/abuse.6
  doc/abuse-tool.6
])

dnl
dnl  Inherit all Lol Engine checks
dnl

LOL_AC_SUBPROJECT()

dnl
dnl  Project-specific checks
dnl

AC_CHECK_HEADERS(bstring.h netinet/in.h sys/ioctl.h unistd.h)
AC_CHECK_FUNCS(getlogin)

dnl  Figure out where the datafiles will be
if test "x${with_assetdir}" = x; then
  with_assetdir='$(datadir)/games/abuse'
fi
AC_SUBST([assetdir], [$with_assetdir])

dnl  Is our package stripped from its non-free data? Or did the user
dnl  maybe disable non-free data?
ac_cv_have_nonfree="no"
if test -f "${srcdir}/data/sfx/ambcave1.wav"; then
  ac_cv_have_nonfree="yes"
fi
if test "x${enable_nonfree}" = xno; then
  ac_cv_have_nonfree="no"
fi
if test "x${ac_cv_have_nonfree}" = xno; then
  distdir="\$(PACKAGE)-free-\$(VERSION)"
else
  distdir="\$(PACKAGE)-\$(VERSION)"
fi
AC_SUBST(distdir)
AM_CONDITIONAL(HAVE_NONFREE, test "x${ac_cv_have_nonfree}" = xyes)

dnl  Is networking enabled?
if test "x${enable_network}" != xno; then
  AC_DEFINE(HAVE_NETWORK, 1, Define to 1 to enable networking)
fi
AM_CONDITIONAL(HAVE_NETWORK, test "x${enable_network}" != xno)

dnl
dnl  Perform the actual commands
dnl

AC_OUTPUT

