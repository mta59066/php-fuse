dnl
dnl $ Id: $
dnl

export FUSE_LIBRARY=fuse

PHP_ARG_WITH(osxfuse, whether osxfuse is available,[  --with-osxfuse[=DIR]    With osxfuse support],[no],[no])
PHP_ARG_WITH(fuse, whether fuse is available,[  --with-fuse[=DIR]    With fuse support])

if test "$PHP_OSXFUSE" != "no"; then
  if test "$PHP_OSXFUSE" = "yes"; then
	PHP_OSXFUSE="/usr/local/include/osxfuse"
  fi
  AC_DEFINE(FUSE_USE_VERSION, 25, [ ])
  export FUSE_LIBRARY=osxfuse
  if test -r "$PHP_OSXFUSE/fuse/fuse.h"; then
	PHP_FUSE_DIR="$PHP_OSXFUSE/fuse"
        PHP_ADD_INCLUDE($PHP_FUSE_DIR)
  fi
  PHP_SUBST(OSXFUSE_SHARED_LIBADD)
fi

if test "$PHP_FUSE" != "no"; then

  if test -r "$PHP_FUSE/include/fuse.h"; then
	PHP_FUSE_DIR="$PHP_FUSE"
  else
	AC_MSG_CHECKING(for fuse in default path)
	for i in /usr /usr/local /usr/local/fuse; do
	  if test -r "$i/include/fuse.h"; then
		PHP_FUSE_DIR=$i
		AC_MSG_RESULT(found in $i)
		break
	  fi
	done
	if test "x" = "x$PHP_FUSE_DIR"; then
	  AC_MSG_ERROR(not found)
	fi
  fi

  export OLD_CPPFLAGS="$CPPFLAGS"
  export CPPFLAGS="$CPPFLAGS $INCLUDES -DHAVE_FUSE"
  export CPPFLAGS="$OLD_CPPFLAGS"

  PHP_ADD_INCLUDE($PHP_FUSE_DIR/include)
  export OLD_CPPFLAGS="$CPPFLAGS"
  export CPPFLAGS="$CPPFLAGS $INCLUDES -DHAVE_FUSE"
  export CPPFLAGS="$CPPFLAGS -D_FILE_OFFSET_BITS=64"
  AC_CHECK_HEADER([fuse.h], [], AC_MSG_ERROR('fuse.h' header not found))
  export CPPFLAGS="$OLD_CPPFLAGS"
  PHP_SUBST(FUSE_SHARED_LIBADD)

  PHP_CHECK_LIBRARY($FUSE_LIBRARY, fuse_get_context,
  [
	PHP_ADD_LIBRARY_WITH_PATH($FUSE_LIBRARY, $PHP_FUSE_DIR/lib, FUSE_SHARED_LIBADD)
  ],[
	AC_MSG_ERROR([wrong fuse lib version or lib not found])
  ],[
	-L$PHP_FUSE_DIR/lib
  ])

  PHP_SUBST(FUSE_SHARED_LIBADD)
  AC_DEFINE(HAVE_FUSE, 1, [ ])

  PHP_NEW_EXTENSION(fuse, fuse.c , $ext_shared)
fi
