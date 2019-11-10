AC_DEFUN([AC_HEADER_REQUIRE],[
  AC_CHECK_HEADER($1, [], AC_MSG_ERROR(missing required header $1))
])
