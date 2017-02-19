#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Mikaboo], [0.0.1], [noaddress@nowhere.org])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/p1test.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdint.h features.h gnu/stubs-32.h gnu/stubs.h],[HAVE_GNU_STUBS_32_H=1],[HAVE_GNU_STUBS_32_H=0])
AM_CONDITIONAL([USE_GNU_STUBS_32_H], [test "$HAVE_GNU_STUBS_32_H" -eq 1])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.

AC_OUTPUT(Makefile src/Makefile)
