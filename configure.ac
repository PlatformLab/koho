#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([koho], [0.01a], [greghill@cs.stanford.edu])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([src/frontend/koho-client.cc])
AC_CONFIG_HEADERS([config.h])

LT_INIT
AC_CONFIG_MACRO_DIR([m4])

# Add picky CXXFLAGS
CXX11_FLAGS="-std=c++11 -pthread"
PICKY_CXXFLAGS="-pedantic -Wall -Wextra -Weffc++"
AC_SUBST([CXX11_FLAGS])
AC_SUBST([PICKY_CXXFLAGS])

# Checks for programs.
AC_PROG_CXX

AC_ARG_VAR([IPTABLES], [path to iptables])
AC_PATH_PROG([IPTABLES], [iptables], [no], [$PATH$PATH_SEPARATOR/sbin$PATH_SEPARATOR/usr/sbin$PATH_SEPARATOR/bin$PATH_SEPARATOR/usr/bin])
if test "$IPTABLES" = "no"; then
  AC_MSG_ERROR([iptables is required])
fi
AC_DEFINE_UNQUOTED([IPTABLES], ["$IPTABLES"], [path to iptables])

AC_ARG_VAR([IP], [path to ip])
AC_PATH_PROG([IP], [ip], [no], [$PATH$PATH_SEPARATOR/sbin$PATH_SEPARATOR/usr/sbin$PATH_SEPARATOR/bin$PATH_SEPARATOR/usr/bin])
if test "$IP" = "no"; then
  AC_MSG_ERROR([ip is required])
fi
AC_DEFINE_UNQUOTED([IP], ["$IP"], [path to ip])

AC_ARG_VAR([DNSMASQ], [path to dnsmasq])
AC_PATH_PROG([DNSMASQ], [dnsmasq], [no], [$PATH$PATH_SEPARATOR/sbin$PATH_SEPARATOR/usr/sbin$PATH_SEPARATOR/bin$PATH_SEPARATOR/usr/bin])
if test "$DNSMASQ" = "no"; then
  AC_MSG_ERROR([dnsmasq is required])
fi
AC_DEFINE_UNQUOTED([DNSMASQ], ["$DNSMASQ"], [path to dnsmasq])

AC_PATH_PROG([PROTOC], [protoc], [])
AS_IF([test x"$PROTOC" = x],
  [AC_MSG_ERROR([cannot find protoc, the Protocol Buffers compiler])])


# Checks for SSL certificate file
AC_CHECK_FILE([/etc/ssl/certs/ssl-cert-snakeoil.pem], [],
              [AC_MSG_ERROR([ssl certificate file is required])])

AC_DEFINE_UNQUOTED([MOD_SSL_CERTIFICATE_FILE], ["/etc/ssl/certs/ssl-cert-snakeoil.pem"], [path to ssl certificate file])

AC_DEFINE_UNQUOTED([MOD_SSL_KEY], ["/etc/ssl/private/ssl-cert-snakeoil.key"], [path to ssl key])

# Checks for libraries.
PKG_CHECK_MODULES([libssl], [libcrypto libssl])

# Checks for header files.
AC_HEADER_RESOLV
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h paths.h sys/ioctl.h sys/socket.h unistd.h], [], [AC_MSG_ERROR([Missing required header file.])])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([clock_gettime inet_ntoa memset mkdir setenv socket strerror strtol])

AC_CONFIG_FILES([Makefile
		 tests/Makefile
		 src/Makefile
		 src/util/Makefile
		 src/tcp_splitter/Makefile
		 src/packet/Makefile
		 src/frontend/Makefile])
AC_OUTPUT
