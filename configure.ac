#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])

########################################
# Generate releaes version
########################################

m4_include([maint/version.m4])
dnl 2nd arg is intentionally underquoted
AC_INIT([CASPER],
        CASPER_VERSION_m4,
        [casper-users@lists.mpich.org],
        [casper],
        [http://www.mcs.anl.gov/project/casper])

# build info
CONFIGURE_ARGS_CLEAN=`echo $* | tr '"' ' '`

# these values come from the m4_include above
CASPER_VERSION=CASPER_VERSION_m4
AC_SUBST(CASPER_VERSION)
CASPER_RELEASE_DATE="CASPER_RELEASE_DATE_m4"
AC_SUBST(CASPER_RELEASE_DATE)

if test -z "$CASPER_VERSION" ; then
    AC_MSG_ERROR([CASPER_VERSION is empty, check maint/version.m4 for errors])
fi

# Produce a numeric version assuming the following format:
# Version: [MAJ].[MIN].[REV][EXT][EXT_NUMBER]
# Example: 1.0.7rc1 has
#          MAJ = 1
#          MIN = 0
#          REV = 7
#          EXT = rc
#          EXT_NUMBER = 1
#
# Converting to numeric version will convert EXT to a format number:
#          ALPHA (a) = 0
#          BETA (b)  = 1
#          RC (rc)   = 2
#          PATCH (p) = 3
# Regular releases are treated as patch 0
#
# Numeric version will have 1 digit for MAJ, 2 digits for MIN,
# 2 digits for REV, 1 digit for EXT and 2 digits for EXT_NUMBER.
changequote(<<,>>)
V1=`expr $CASPER_VERSION : '\([0-9]*\)\.[0-9]*\.*[0-9]*[a-zA-Z]*[0-9]*'`
V2=`expr $CASPER_VERSION : '[0-9]*\.\([0-9]*\)\.*[0-9]*[a-zA-Z]*[0-9]*'`
V3=`expr $CASPER_VERSION : '[0-9]*\.[0-9]*\.*\([0-9]*\)[a-zA-Z]*[0-9]*'`
V4=`expr $CASPER_VERSION : '[0-9]*\.[0-9]*\.*[0-9]*\([a-zA-Z]*\)[0-9]*'`
V5=`expr $CASPER_VERSION : '[0-9]*\.[0-9]*\.*[0-9]*[a-zA-Z]*\([0-9]*\)'`
changequote([,])

if test "$V2" -le 9 ; then V2=0$V2 ; fi
if test "$V3" = "" ; then V3=0; fi
if test "$V3" -le 9 ; then V3=0$V3 ; fi
if test "$V4" = "a" ; then
    V4=0
elif test "$V4" = "b" ; then
    V4=1
elif test "$V4" = "rc" ; then
    V4=2
elif test "$V4" = "" ; then
    V4=3
    V5=0
elif test "$V4" = "p" ; then
    V4=3
fi
if test "$V5" -le 9 ; then V5=0$V5 ; fi

CASPER_NUMVERSION=`expr $V1$V2$V3$V4$V5 + 0`
AC_SUBST(CASPER_NUMVERSION)


########################################
# Check and configure setup
########################################

AC_CONFIG_AUX_DIR(confdb)
AC_CONFIG_MACRO_DIR(confdb)

# needed by hwloc in embedded mode.  Must come very early to avoid
# bizarre expansion ordering warnings
AC_CANONICAL_TARGET
AC_ARG_PROGRAM

# also needed by hwloc in embedded mode, must also come early for expansion
# ordering reasons
AC_USE_SYSTEM_EXTENSIONS

AM_INIT_AUTOMAKE([-Wall -Werror -Wno-portability-recursive silent-rules foreign 1.12.3 subdir-objects])

# Bug in libtool adds -O2 and -g by default
PAC_PUSH_FLAG([CFLAGS])
AC_PROG_CC(mpicc)
PAC_POP_FLAG([CFLAGS])

AM_PROG_AR

LT_PREREQ([2.2.6])

AC_CONFIG_HEADER([include/casperconf.h])

# Bug in libtool adds -O2 and -g by default
PAC_PUSH_FLAG([CFLAGS])
LT_INIT()
PAC_POP_FLAG([CFLAGS])
 
CONFIGURE_ARGS_CLEAN="$CONFIGURE_ARGS_CLEAN CC=$CC CFLAGS=$CFLAGS "
CONFIGURE_ARGS_CLEAN="$CONFIGURE_ARGS_CLEAN LDFLAGS=$LDFLAGS LIBS=$LIBS CPPFLAGS=$CPPFLAGS  "
CONFIGURE_ARGS_CLEAN="$CONFIGURE_ARGS_CLEAN $BUILD_INFO LT_SYS_LIBRARY_PATH=$LT_SYS_LIBRARY_PATH CPP=$CPP"
AC_SUBST(CONFIGURE_ARGS_CLEAN)

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h])

# Non-verbose make
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_RESTRICT
AC_C_INLINE

# Check for enabling strict
PAC_ARG_STRICT

# This test is complicated by the fact that top_srcdir is not set until
# the very end of configure.  Instead, we get it ourselves
if test -z "$top_srcdir" ; then
    csp_top_srcdir=$srcdir   
else
    csp_top_srcdir=$top_srcdir
fi
if test -z "$csp_top_srcdir" ; then 
    # This needs to be an absolute pathname
    case "$csp_top_srcdir" in
    /*) ;;
    *)
        csp_top_srcdir=`(cd $csp_top_srcdir && pwd)`
    ;;  
    esac
fi

# Get the directory that we're running in...
if test -z "$csp_top_builddir" ; then
   csp_top_builddir="`pwd`"
fi

# Runtime load balancing optimization
AC_ARG_ENABLE(runtime-load, AC_HELP_STRING([--enable-runtime-load],
                 [Enable Runtime Load Balancing (no by default)]),
                 [ enable_runtime_load=$enableval ],
                 [ enable_runtime_load=no ])
AC_MSG_CHECKING(runtime Load balancing support)
AC_MSG_RESULT($enable_runtime_load)
if test "$enable_runtime_load" = "yes"; then
   AC_DEFINE(CSP_ENABLE_RUNTIME_LOAD_OPT,1,[Define if enable runtime load balancing])
fi

# RMA error check
AC_ARG_ENABLE(rmaerr-check, AC_HELP_STRING([--disable-rmaerr-check],
                 [Disable RMA error check for better performance (no by default).
                 If the RMA error check is disabled in Casper, the behavior of 
                 Error Handling becomes undefined.]),
                 [ enable_rmaerr_check=$enableval ],
                 [ enable_rmaerr_check=yes ])
AC_MSG_CHECKING(RMA error checking support)
AC_MSG_RESULT($enable_rmaerr_check)
if test "$enable_rmaerr_check" = "yes"; then
   AC_DEFINE(CSP_ENABLE_RMA_ERR_CHECK,1,[Define if enable RMA error check])
fi

# Thread-safety
AC_ARG_ENABLE(thread-safety, AC_HELP_STRING([--enable-thread-safety],
                 [Enable Casper thread safety (no by default).
                 Only MPI_THREAD_MULTIPLE program requires the thread safety
                 to be enabled in Casper. For MPI SINGLE/FUNNELED/SERIALIZED thread
                 levels, it is recommanded to disable the thread-safety in Casper
                 for better performance. Also see option --with-thread-package.]),
                 [ enable_thread_safety=$enableval ],
                 [ enable_thread_safety=no ])
AC_MSG_CHECKING(Thread safety support)
AC_MSG_RESULT($enable_thread_safety)
if test "$enable_thread_safety" = "yes"; then
   AC_DEFINE(CSP_ENABLE_THREAD_SAFE,1,[Define if enable thread safety])
fi

# MPI IO wrapper check
AC_ARG_ENABLE(mpio-wrapper, AC_HELP_STRING([--enable-mpio-wrapper],
                 [Enable MPI IO wrapper. (yes by default).
                 Casper sets MPI IO wrapper based on the detection at autogen.sh.
                 However, the wrapper is statically set in release tarball.
                 This option allows user to disable it at configure time.]),
                 [ enable_mpio_wrapper=$enableval ],
                 [ enable_mpio_wrapper=yes ])
AC_MSG_CHECKING(MPI IO wrapper support)
AC_MSG_RESULT($enable_mpio_wrapper)
if test "$enable_mpio_wrapper" = "yes"; then
   AC_DEFINE(CSP_ENABLE_MPIO_WRAPPER,1,[Define if enable MPI IO wrapper.])
fi

# Profile output
AC_ARG_ENABLE(profile, AC_HELP_STRING([--enable-profile],
                 [Enable internal profiling routine (no by default).
                  Enable it may degrade performance.]),
                 [ enable_profile=$enableval ],
                 [ enable_profile=no ])
AC_MSG_CHECKING(Profiling routine support)
AC_MSG_RESULT($enable_profile)
if test "$enable_profile" = "yes"; then
   AC_DEFINE(CSP_ENABLE_PROFILE,1,[Define if enable profiling routine])
fi

# External IZEM
AC_ARG_WITH(izem, [AC_HELP_STRING([--with-izem[=DIR]],
                [Use the selected IZEM; Header file lock/zm_mcs.h should be in 
                dir/include, and libzm should be in dir/lib. Also see option
                --with-thread-package])],,with_izem=no)

# Thread package used in critical section
AC_ARG_WITH([thread-package],
[  --with-thread-package=posix|pthreads|izem
                          Thread package to implement internal critical section 
                          when multiple threads are present. Supported thread 
                          packages include:
                          posix or pthreads - POSIX threads (default)
                          izem              - IZEM mcs lock (user-space spinlock. 
                                              User should provide the directory of 
                                              external IZEM library, see option --with-izem).
],,with_thread_package=default)
AC_MSG_CHECKING(Thread package to implement critical section)
AC_MSG_RESULT($with_thread_package)

# Set thread package only when thread safety is enabled
if test "$enable_thread_safety" = "yes"; then
    THREAD_PACKAGE_NAME=NONE
    case $with_thread_package in
        default|posix|pthreads)
            # check if pthread is supported (e.g., invalid on windows or solaris)
            have_pthreads=no
            AC_CHECK_HEADERS(pthread.h)
            AC_CHECK_LIB([pthread],[pthread_key_create],have_lpthread=yes)
            if test "$have_lpthread" = "yes" ; then
                PAC_PREPEND_FLAG([-lpthread],[LIBS])
                # this check should come after the AC_CHECK_LIB for -lpthread
                AC_CHECK_FUNCS([pthread_mutex_lock],have_pthreads=yes,AC_MSG_ERROR([unable to find pthreads library.]))
            fi
            THREAD_PACKAGE_NAME=CSP_THREAD_CS_LOCK__PTHREAD_MUTEX
        ;;
        izem)
            # izem provides platform independent user-space lock.
            # check if user provided izem exists.
            if test "$with_izem" = "no" ; then
                AC_MSG_ERROR([unable to find izem library.])
            fi
    
            CFLAGS="$CFLAGS -I$with_izem/include"
            LDFLAGS="$LDFLAGS -L$with_izem/lib"
            AC_CHECK_HEADERS(lock/zm_mcs.h)
            AC_CHECK_LIB([zm],[zm_mcs_init],have_izem=yes,AC_MSG_ERROR([unable to find izem library.]))
    
            THREAD_PACKAGE_NAME=CSP_THREAD_CS_LOCK__IZEM_MCS_LOCK
        ;;
        *)
            AC_MSG_ERROR([The specified thread package, $with_thread_package, is not supported.])
        ;;
    esac

    AC_DEFINE_UNQUOTED(CSP_THREAD_CS_LOCK,[$THREAD_PACKAGE_NAME],
                       [Thread package to implement critical section])
fi

# HWLOC library (optional)

# Unconditionally include the hwloc macros, even if we are using an
# external hwloc (or hwloc is disabled). This is needed for the
# AM_CONDITIONALS that we will set later.
m4_include([src/hwloc/config/hwloc.m4])
m4_include([src/hwloc/config/hwloc_check_attributes.m4])
m4_include([src/hwloc/config/hwloc_check_visibility.m4])
m4_include([src/hwloc/config/hwloc_check_vendor.m4])
m4_include([src/hwloc/config/hwloc_components.m4])
m4_include([src/hwloc/config/hwloc_pkg.m4])

hwlocsrcdir=""
AC_SUBST([hwlocsrcdir])
hwloclibdir=""
AC_SUBST([hwloclibdir])
hwloclib=""
AC_SUBST([hwloclib])
 
have_hwloc=no
AC_ARG_WITH(hwloc, [AC_HELP_STRING([--with-hwloc[=DIR|embedded|no]],
                [Use the specified HWLOC installation; Header file hwloc.h 
                should be in dir/include, and libhwloc should be in dir/lib. 
                Use embedded hwloc by default])],,with_hwloc=embedded)

# This needs to be an absolute pathname.
# We do not support embedded mode because linking its static library to
# casper dynamic library is not portable.
case "$with_hwloc" in
    embedded)
        HWLOC_SETUP_CORE([src/hwloc],[have_hwloc=yes],[have_hwloc=no],[1])
        # Only build hwloc in embedded mode
        if test "$have_hwloc" = "yes" ; then
            use_embedded_hwloc=yes
            CFLAGS="$HWLOC_EMBEDDED_CFLAGS $CFLAGS"
            CPPFLAGS="$HWLOC_EMBEDDED_CPPFLAGS $CPPFLAGS"
            LIBS="$HWLOC_EMBEDDED_LIBS $LIBS"
            hwlocsrcdir="src/hwloc"
            hwloclib="src/hwloc/src/libhwloc_embedded.la"
        fi
    ;;
    no)
        AC_MSG_WARN([No hwloc library.])
    ;;
    *)
        CFLAGS="$CFLAGS -I$with_hwloc/include"
        LDFLAGS="$LDFLAGS -L$with_hwloc/lib"
        LIBS="$LIBS -lhwloc"
        AC_CHECK_HEADERS(hwloc.h)
        AC_CHECK_LIB([hwloc],[hwloc_topology_init],have_hwloc=yes,AC_MSG_ERROR([unable to find hwloc_topology_init.]))
        
        hwloclib="-lhwloc"
        if test -d ${with_hwloc}/lib64 ; then
            hwloclibdir="-L${with_hwloc}/lib64"
        else
            hwloclibdir="-L${with_hwloc}/lib"
        fi
    ;;
esac

HWLOC_DO_AM_CONDITIONALS
if test "$have_hwloc" = "yes" ; then
    AC_DEFINE(HAVE_HWLOC,1,[Define if hwloc is available])
    AC_DEFINE(CSP_ENABLE_TOPO_OPT,1,[Define if topology optimization is enabled. Require hwloc.])
fi

AM_CONDITIONAL([csp_have_topo_opt], [test "$have_hwloc" = "yes"])

# OPA library (required)
opasrcdir=""
AC_SUBST([opasrcdir])
opalibdir=""
AC_SUBST([opalibdir])
opalib=""
AC_SUBST([opalib])
AC_ARG_WITH(opa, [AC_HELP_STRING([--with-opa[=DIR|embedded|no]],
                [Use the specified OPA installation; Header file opa_primitives.h should be in 
                dir/include, and libopa should be in dir/lib.Use embedded openpa by default])]
                ,,with_opa=embedded)
case "$with_opa" in
    embedded)
    if test -e "${csp_top_srcdir}/src/openpa" ; then
        opasrcdir="src/openpa"
        # only need header files
        PAC_APPEND_FLAG([-I${csp_top_srcdir}/src/openpa/src],[CPPFLAGS])
        PAC_APPEND_FLAG([-I${csp_top_builddir}/src/openpa/src],[CPPFLAGS])

        # no need for libtool versioning when embedding OPA
        opa_subdir_args="--disable-versioning --enable-embedded"
        PAC_CONFIG_SUBDIR_ARGS([src/openpa],[$opa_subdir_args],[],[AC_MSG_ERROR([OpenPA configure failed])])
    else
        AC_MSG_WARN([Attempted to use the embedded OpenPA source tree in "src/openpa", but it is missing.  Configuration or compilation may fail later.])
    fi
    ;;
    no)
        AC_MSG_ERROR([unable to find opa library.])
    ;;
    *)
    CFLAGS="$CFLAGS -I$with_opa/include"
    AC_CHECK_HEADERS(opa_primitives.h)
    # do not check library because all needed functions are inlined. 
    # LDFLAGS="$LDFLAGS -L$with_opa/lib"
    # AC_CHECK_LIB([opa],[OPA_swap_ptr],have_opa=yes,AC_MSG_ERROR([unable to find OPA_swap_ptr.]))
    ;;
esac

# check for attribute support
PAC_C_GNU_ATTRIBUTE

# check for compiler support for the __typeof() extension
AC_CACHE_CHECK([whether the compiler supports __typeof(variable)],
               [pac_cv_have___typeof],
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]],[[double foo = 0.0; __typeof(foo) bar = 1.0;]])],
                  [pac_cv_have___typeof=yes],
                  [pac_cv_have___typeof=no])]
)
if test "$pac_cv_have___typeof" = "yes" ; then
    AC_DEFINE([HAVE___TYPEOF],[1],[defined if the C compiler supports __typeof(variable)])
fi

# Check optional datatype support
AC_DEFUN([UD_CHECK_MPI_DTYPE], [
  AC_MSG_CHECKING(if $1 defined)
  AC_COMPILE_IFELSE(
  [AC_LANG_SOURCE([
  #include "mpi.h"
  #include <stdio.h>
  int main (int argc, char **argv) {
  MPI_Datatype dtype;
  MPI_Type_contiguous(1, $1, &dtype);
  }])],
  AC_MSG_RESULT(yes)
  have_$1=yes,
  AC_MSG_RESULT(no)
  have_$1=no)
  if test "$have_$1" = "yes" ; then
    AC_DEFINE([HAVE_$1],[1],[defined if the MPI library defines $1])
  fi
])

UD_CHECK_MPI_DTYPE(MPI_DOUBLE_COMPLEX)
UD_CHECK_MPI_DTYPE(MPI_INTEGER1)
UD_CHECK_MPI_DTYPE(MPI_INTEGER2)
UD_CHECK_MPI_DTYPE(MPI_INTEGER4)
UD_CHECK_MPI_DTYPE(MPI_INTEGER8)
UD_CHECK_MPI_DTYPE(MPI_INTEGER16)
UD_CHECK_MPI_DTYPE(MPI_REAL2)
UD_CHECK_MPI_DTYPE(MPI_REAL4)
UD_CHECK_MPI_DTYPE(MPI_REAL8)
UD_CHECK_MPI_DTYPE(MPI_REAL16)
UD_CHECK_MPI_DTYPE(MPI_COMPLEX4)
UD_CHECK_MPI_DTYPE(MPI_COMPLEX8)
UD_CHECK_MPI_DTYPE(MPI_COMPLEX16)
UD_CHECK_MPI_DTYPE(MPI_COMPLEX32)

AC_CONFIG_FILES([include/casper.h])
AC_CONFIG_SUBDIRS([test])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
