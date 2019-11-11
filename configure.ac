AC_INIT([hdaps-gl], [0.6.0], [michael@orlitzky.com])
AM_INIT_AUTOMAKE([-Wall foreign no-dist-gzip dist-xz tar-ustar])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_CONFIG_SRCDIR([src/hdaps-gl.c])
AC_CONFIG_MACRO_DIRS([m4])

# Checks for programs.
AC_PROG_CC
AM_PROG_AR

# Predefined header checks.
AC_HEADER_STAT    # sys/stat.h
AC_HEADER_STDC    # stdlib.h string.h (implied: errno.h limits.h stdio.h)

# Check for header files not covered by the predefined macros above.
AC_HEADER_REQUIRE(sys/types.h)
AC_HEADER_REQUIRE(fcntl.h)
AC_HEADER_REQUIRE(unistd.h)

# OpenGL headers
AC_HEADER_REQUIRE(GL/glut.h) # freeglut
AC_HEADER_REQUIRE(GL/gl.h)   # mesa
AC_HEADER_REQUIRE(GL/glu.h)  # glu

# Choose three functions more or less at random to look for in our GL
# libraries. This ensures that these libraries get linked in automatically.
AC_SEARCH_LIBS([gluPerspective], [GLU])
AC_SEARCH_LIBS([glMatrixMode], [GL])
AC_SEARCH_LIBS([glutMainLoop], [glut])

LT_INIT
AC_OUTPUT
