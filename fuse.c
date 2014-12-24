/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2007 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.0 of the PHP license,       |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_0.txt.                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Masaki Fujimoto <fujimoto@php.net>                           |
  +----------------------------------------------------------------------+
*/
#include "php_fuse.h"

#if HAVE_FUSE

#define MAKE_LONG_ZVAL(name, val)	MAKE_STD_ZVAL(name); ZVAL_LONG(name, val);

/* {{{ fuse_functions[] */
zend_function_entry fuse_functions[] = {
	{ NULL, NULL, NULL }
};
/* }}} */

ZEND_DECLARE_MODULE_GLOBALS(fuse)

/* {{{ fuse_module_entry
 */
zend_module_entry fuse_module_entry = {
	STANDARD_MODULE_HEADER,
	"fuse",
	fuse_functions,
	PHP_MINIT(fuse),     /* Replace with NULL if there is nothing to do at php startup   */ 
	NULL,
	PHP_RINIT(fuse),     /* Replace with NULL if there is nothing to do at request start */
	NULL,
	PHP_MINFO(fuse),
	"0.9.2", 
	PHP_MODULE_GLOBALS(fuse),
	NULL,
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_FUSE
ZEND_GET_MODULE(fuse)
#endif

/* {{{ php_fuse_call_method() (copy from zend_call_method - ze takes only 2 parameters...why?) */
#define php_fuse_call_method_with_0_params(obj, obj_ce, fn_proxy, function_name, retval) php_fuse_call_method(obj, obj_ce, fn_proxy, function_name, sizeof(function_name)-1, retval TSRMLS_CC, 0)
#define php_fuse_call_method_with_1_params(obj, obj_ce, fn_proxy, function_name, retval, arg1) php_fuse_call_method(obj, obj_ce, fn_proxy, function_name, sizeof(function_name)-1, retval TSRMLS_CC, 1 , &arg1)
#define php_fuse_call_method_with_2_params(obj, obj_ce, fn_proxy, function_name, retval, arg1, arg2) php_fuse_call_method(obj, obj_ce, fn_proxy, function_name, sizeof(function_name)-1, retval TSRMLS_CC, 2 , &arg1, &arg2)
#define php_fuse_call_method_with_3_params(obj, obj_ce, fn_proxy, function_name, retval, arg1, arg2, arg3) php_fuse_call_method(obj, obj_ce, fn_proxy, function_name, sizeof(function_name)-1, retval TSRMLS_CC, 3 , &arg1, &arg2, &arg3)
#define php_fuse_call_method_with_4_params(obj, obj_ce, fn_proxy, function_name, retval, arg1, arg2, arg3, arg4) php_fuse_call_method(obj, obj_ce, fn_proxy, function_name, sizeof(function_name)-1, retval TSRMLS_CC, 4 , &arg1, &arg2, &arg3, &arg4)
#define php_fuse_call_method_with_5_params(obj, obj_ce, fn_proxy, function_name, retval, arg1, arg2, arg3, arg4, arg5) php_fuse_call_method(obj, obj_ce, fn_proxy, function_name, sizeof(function_name)-1, retval TSRMLS_CC, 5 , &arg1, &arg2, &arg3, &arg4, &arg5)

static zval* php_fuse_call_method(zval **object_pp, zend_class_entry *obj_ce, zend_function **fn_proxy, char *function_name, int function_name_len, zval **retval_ptr_ptr TSRMLS_DC, int param_count, ...) {
	int result;
	zend_fcall_info fci;
	zval z_fname;
	zval *retval;
	HashTable *function_table;

	int i;
	va_list va_params;
	zval ***params = emalloc(sizeof(zval**) * param_count);

	va_start(va_params, param_count);
	for (i = 0; i < param_count; i++) {
		zval **tmp = va_arg(va_params, zval**);
		*(params+i) = tmp;
	}
	va_end(va_params);
//	printf("%s(...) = ?\r\n", function_name);

	fci.size = sizeof(fci);
	/*fci.function_table = NULL; will be read form zend_class_entry of object if needed */
#if PHP_VERSION_ID < 50300
	fci.object_pp = object_pp;
#else
	fci.object_ptr = *object_pp;
#endif
	fci.function_name = &z_fname;
	fci.retval_ptr_ptr = retval_ptr_ptr ? retval_ptr_ptr : &retval;
	fci.param_count = param_count;
	fci.params = params;
	fci.no_separation = 1;
	fci.symbol_table = NULL;

	if (!fn_proxy && !obj_ce) {
		/* no interest in caching and no information already present that is
		 * needed later inside zend_call_function. */
		ZVAL_STRINGL(&z_fname, function_name, function_name_len, 0);
		fci.function_table = !object_pp ? EG(function_table) : NULL;
		result = zend_call_function(&fci, NULL TSRMLS_CC);
	} else {
		zend_fcall_info_cache fcic;

		fcic.initialized = 1;
		if (!obj_ce) {
			obj_ce = object_pp ? Z_OBJCE_PP(object_pp) : NULL;
		}
		if (obj_ce) {
			function_table = &obj_ce->function_table;
		} else {
			function_table = EG(function_table);
		}
		if (!fn_proxy || !*fn_proxy) {
			if (zend_hash_find(function_table, function_name, function_name_len+1, (void **) &fcic.function_handler) == FAILURE) {
				/* error at c-level */
				zend_error(E_CORE_ERROR, "Couldn't find implementation for method %s%s%s", obj_ce ? obj_ce->name : "", obj_ce ? "::" : "", function_name);
			}
			if (fn_proxy) {
				*fn_proxy = fcic.function_handler;
			}
		} else {
			fcic.function_handler = *fn_proxy;
		}
		fcic.calling_scope = obj_ce;
#if PHP_VERSION_ID < 50300
		fcic.object_pp = object_pp;
#else
		fcic.object_ptr = *object_pp;
#endif
		result = zend_call_function(&fci, &fcic TSRMLS_CC);
	}
	if (result == FAILURE) {
		/* error at c-level */
		if (!obj_ce) {
			obj_ce = object_pp ? Z_OBJCE_PP(object_pp) : NULL;
		}
		if (!EG(exception)) {
			zend_error(E_CORE_ERROR, "Couldn't execute method %s%s%s", obj_ce ? obj_ce->name : "", obj_ce ? "::" : "", function_name);
		}
	}
	efree(params);
	if (!retval_ptr_ptr) {
		if (retval) {
			zval_ptr_dtor(&retval);
		}
//		printf("%s(...) = null\r\n", function_name);
		return NULL;
	}
	long r = Z_LVAL_P(*retval_ptr_ptr);
//	printf("%s(...) = %ld\r\n", function_name, r);
	return *retval_ptr_ptr;
}
/* }}} */

/* {{{ fuse call back functions */
PHP_FUSE_API int php_fuse_getattr(const char * path, struct stat * st) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -ENOENT;
	}

	memset(st, 0, sizeof(struct stat));

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_st;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_st);
	array_init(arg_st);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "getattr", &retval, arg_path, arg_st);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);

	if (r < 0) {
		zval_ptr_dtor(&arg_st);
		pthread_mutex_unlock(&FUSEG(m));
		return r;
	}

	/* reference retval handling */
	if (Z_TYPE_P(arg_st) != IS_ARRAY) {
		zval_ptr_dtor(&arg_st);
		pthread_mutex_unlock(&FUSEG(m));
		return -ENOENT;
	}

	HashPosition	pos;
	zval			**entry;
	char			*tmp_s_key;
	uint			tmp_s_key_len;
	ulong			tmp_n_key;

	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(arg_st), &pos);
	while (zend_hash_get_current_data_ex(Z_ARRVAL_P(arg_st), (void **)&entry, &pos) == SUCCESS) {
		convert_to_long_ex(entry);
		int value = Z_LVAL_PP(entry);

		switch (zend_hash_get_current_key_ex(Z_ARRVAL_P(arg_st), &tmp_s_key, &tmp_s_key_len, &tmp_n_key, 0, &pos)) {	// no duplication
		case HASH_KEY_IS_STRING:
			if (strncmp(tmp_s_key, "dev", tmp_s_key_len) == 0) {
				st->st_dev = (dev_t)value;
			} else if (strncmp(tmp_s_key, "ino", tmp_s_key_len) == 0) {
				st->st_ino = (ino_t)value;
			} else if (strncmp(tmp_s_key, "mode", tmp_s_key_len) == 0) {
				st->st_mode = (mode_t)value;
			} else if (strncmp(tmp_s_key, "nlink", tmp_s_key_len) == 0) {
				st->st_nlink = (nlink_t)value;
			} else if (strncmp(tmp_s_key, "uid", tmp_s_key_len) == 0) {
				st->st_uid = (uid_t)value;
			} else if (strncmp(tmp_s_key, "gid", tmp_s_key_len) == 0) {
				st->st_gid = (gid_t)value;
			} else if (strncmp(tmp_s_key, "rdev", tmp_s_key_len) == 0) {
				st->st_rdev = (dev_t)value;
			} else if (strncmp(tmp_s_key, "size", tmp_s_key_len) == 0) {
				st->st_size = (size_t)value;
			} else if (strncmp(tmp_s_key, "atime", tmp_s_key_len) == 0) {
				st->st_atime = (time_t)value;
			} else if (strncmp(tmp_s_key, "mtime", tmp_s_key_len) == 0) {
				st->st_mtime = (time_t)value;
			} else if (strncmp(tmp_s_key, "ctime", tmp_s_key_len) == 0) {
				st->st_ctime = (time_t)value;
			} else if (strncmp(tmp_s_key, "blksize", tmp_s_key_len) == 0) {
				st->st_blksize = value;			// TODO: why? blksize_t is not typedefed in some environment
			} else if (strncmp(tmp_s_key, "blocks", tmp_s_key_len) == 0) {
				st->st_blocks = (blkcnt_t)value;
			}
			break;
		case HASH_KEY_IS_LONG:
			switch (tmp_n_key) {
			case 0:
				st->st_dev = (dev_t)value;
				break;
			case 1:
				st->st_ino = (ino_t)value;
				break;
			case 2:
				st->st_mode = (mode_t)value;
				break;
			case 3:
				st->st_nlink = (nlink_t)value;
				break;
			case 4:
				st->st_uid = (uid_t)value;
				break;
			case 5:
				st->st_gid = (gid_t)value;
				break;
			case 6:
				st->st_rdev = (dev_t)value;
				break;
			case 7:
				st->st_size = (off_t)value;
				break;
			case 8:
				st->st_atime = (time_t)value;
				break;
			case 9:
				st->st_mtime = (time_t)value;
				break;
			case 10:
				st->st_ctime = (time_t)value;
				break;
			case 11:
				st->st_blksize = value;			// TODO: why? blksize_t is not typedefed in some environment
				break;
			case 12:
				st->st_blocks = (blkcnt_t)value;
				break;
			}
			break;
		}
		zend_hash_move_forward_ex(Z_ARRVAL_P(arg_st), &pos);
	}

	zval_ptr_dtor(&arg_st);

	pthread_mutex_unlock(&FUSEG(m));
	
	return r;
}

PHP_FUSE_API int php_fuse_readlink(const char * path, char * buf, size_t buf_len) {
	TSRMLS_FETCH();
	zval *active_object = NULL;

	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_buf;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_buf);
	ZVAL_STRING(arg_buf, "", 0);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "readlink", &retval, arg_path, arg_buf);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);

	if (r < 0) {
		zval_ptr_dtor(&arg_buf);
		pthread_mutex_unlock(&FUSEG(m));
		return r;
	}

	convert_to_string_ex(&arg_buf);
	if(Z_STRVAL_P(arg_buf) == NULL) {
		zval_ptr_dtor(&arg_buf);
		pthread_mutex_unlock(&FUSEG(m));
		return 0;
	}

	int mr = Z_STRLEN_P(arg_buf);
	memset(buf,'\0',buf_len);
	memcpy(buf, Z_STRVAL_P(arg_buf), mr > buf_len ? buf_len : mr);
	buf[buf_len-1] = '\0';
	zval_ptr_dtor(&arg_buf);

	pthread_mutex_unlock(&FUSEG(m));

	return r;
}

PHP_FUSE_API int php_fuse_getdir(const char * path, fuse_dirh_t dh, fuse_dirfil_t df) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
		
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_list;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_list);
	array_init(arg_list);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "getdir", &retval, arg_path, arg_list);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);

	if (r < 0) {
		zval_ptr_dtor(&arg_list);
		pthread_mutex_unlock(&FUSEG(m));
		return r;
	}

	/* reference retval handling */
	if (Z_TYPE_P(arg_list) != IS_ARRAY) {
		zval_ptr_dtor(&arg_list);
		pthread_mutex_unlock(&FUSEG(m));
		return -ENOENT;
	}

	HashPosition	pos;
	zval			**entry;
	char			*tmp_s_key;
	uint			tmp_s_key_len;
	ulong			tmp_n_key;

	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(arg_list), &pos);
	while (zend_hash_get_current_data_ex(Z_ARRVAL_P(arg_list), (void **)&entry, &pos) == SUCCESS) {
		int r = zend_hash_get_current_key_ex(Z_ARRVAL_P(arg_list), &tmp_s_key, &tmp_s_key_len, &tmp_n_key, 0, &pos);
		char buf[1024];
		if (r == HASH_KEY_IS_LONG) {
			snprintf(buf, sizeof(buf), "%d", tmp_n_key);
		}
		convert_to_array_ex(entry);

		zval **tmp_type;
		zval **tmp_ino;
		int type = DT_UNKNOWN;
		int ino = 0;
		if (zend_hash_find(Z_ARRVAL_PP(entry), "type", sizeof("type"), (void**)&tmp_type) == SUCCESS) {
			type = Z_LVAL_PP(tmp_type);
			zval_ptr_dtor(tmp_type);
		}
		if (zend_hash_find(Z_ARRVAL_PP(entry), "ino", sizeof("ino"), (void**)&tmp_ino) == SUCCESS) {
			ino = Z_LVAL_PP(tmp_ino);
			zval_ptr_dtor(tmp_ino);
		}

		df(dh, r == HASH_KEY_IS_LONG ? buf : tmp_s_key, type, ino);
		zend_hash_move_forward_ex(Z_ARRVAL_P(arg_list), &pos);
	}
	zval_ptr_dtor(&arg_list);

	pthread_mutex_unlock(&FUSEG(m));
	
	return r;
}

PHP_FUSE_API int php_fuse_mknod(const char * path, mode_t mode, dev_t dev) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -EINVAL;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_mode;
	zval *arg_dev;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_mode);
	ZVAL_LONG(arg_mode, mode);

	MAKE_STD_ZVAL(arg_dev);
	ZVAL_LONG(arg_dev, dev);

	php_fuse_call_method_with_3_params(&active_object, Z_OBJCE_P(active_object), NULL, "mknod", &retval, arg_path, arg_mode, arg_dev);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_mode);
	zval_ptr_dtor(&arg_dev);

	pthread_mutex_unlock(&FUSEG(m));
	return r;
}

PHP_FUSE_API int php_fuse_mkdir(const char * path, mode_t mode) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_mode;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_mode);
	ZVAL_LONG(arg_mode, mode);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "mkdir", &retval, arg_path, arg_mode);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_mode);

	pthread_mutex_unlock(&FUSEG(m));
	
	return r;
}

PHP_FUSE_API int php_fuse_unlink(const char * path) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	php_fuse_call_method_with_1_params(&active_object, Z_OBJCE_P(active_object), NULL, "unlink", &retval, arg_path);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);

	pthread_mutex_unlock(&FUSEG(m));
	
	return r;
}

PHP_FUSE_API int php_fuse_rmdir(const char * path) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	php_fuse_call_method_with_1_params(&active_object, Z_OBJCE_P(active_object), NULL, "rmdir", &retval, arg_path);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);

	pthread_mutex_unlock(&FUSEG(m));
	
	return r;
}

PHP_FUSE_API int php_fuse_symlink(const char * path_from, const char * path_to) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path_from;
	zval *arg_path_to;
	char *p = estrdup(path_from);
	char *q = estrdup(path_to);

	MAKE_STD_ZVAL(arg_path_from);
	ZVAL_STRING(arg_path_from, p, 0);

	MAKE_STD_ZVAL(arg_path_to);
	ZVAL_STRING(arg_path_to, q, 0);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "symlink", &retval, arg_path_from, arg_path_to);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path_from);
	zval_ptr_dtor(&arg_path_to);
	
	pthread_mutex_unlock(&FUSEG(m));
	
	return r;
}

PHP_FUSE_API int php_fuse_rename(const char * path_from, const char * path_to) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path_from;
	zval *arg_path_to;
	char *p = estrdup(path_from);
	char *q = estrdup(path_to);

	MAKE_STD_ZVAL(arg_path_from);
	ZVAL_STRING(arg_path_from, p, 0);

	MAKE_STD_ZVAL(arg_path_to);
	ZVAL_STRING(arg_path_to, q, 0);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "rename", &retval, arg_path_from, arg_path_to);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path_from);
	zval_ptr_dtor(&arg_path_to);

	pthread_mutex_unlock(&FUSEG(m));
	
	return r;
}

PHP_FUSE_API int php_fuse_link(const char * path_from, const char * path_to) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path_from;
	zval *arg_path_to;
	char *p = estrdup(path_from);
	char *q = estrdup(path_to);

	MAKE_STD_ZVAL(arg_path_from);
	ZVAL_STRING(arg_path_from, p, 0);

	MAKE_STD_ZVAL(arg_path_to);
	ZVAL_STRING(arg_path_to, q, 0);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "link", &retval, arg_path_from, arg_path_to);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path_from);
	zval_ptr_dtor(&arg_path_to);
	
	pthread_mutex_unlock(&FUSEG(m));
	
	return r;
}

PHP_FUSE_API int php_fuse_chmod(const char * path, mode_t mode) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_mode;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_mode);
	ZVAL_LONG(arg_mode, mode);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "chmod", &retval, arg_path, arg_mode);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_mode);

	pthread_mutex_unlock(&FUSEG(m));
	
	return r;
}

PHP_FUSE_API int php_fuse_chown(const char * path, uid_t uid, gid_t gid) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_uid;
	zval *arg_gid;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_uid);
	ZVAL_LONG(arg_uid, uid);

	MAKE_STD_ZVAL(arg_gid);
	ZVAL_LONG(arg_gid, gid);

	php_fuse_call_method_with_3_params(&active_object, Z_OBJCE_P(active_object), NULL, "chown", &retval, arg_path, arg_uid, arg_gid);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_uid);
	zval_ptr_dtor(&arg_gid);
	
	pthread_mutex_unlock(&FUSEG(m));
	
	return r;
}

PHP_FUSE_API int php_fuse_truncate(const char * path, off_t offset) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_offset;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_offset);
	ZVAL_LONG(arg_offset, offset);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "truncate", &retval, arg_path, arg_offset);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_offset);

	pthread_mutex_unlock(&FUSEG(m));
	
	return r;
}

PHP_FUSE_API int php_fuse_utime(const char * path, struct utimbuf * buf) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_atime;
	zval *arg_mtime;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_atime);
	ZVAL_LONG(arg_atime, buf->actime);

	MAKE_STD_ZVAL(arg_mtime);
	ZVAL_LONG(arg_mtime, buf->modtime);

	php_fuse_call_method_with_3_params(&active_object, Z_OBJCE_P(active_object), NULL, "utime", &retval, arg_path, arg_atime, arg_mtime);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_atime);
	zval_ptr_dtor(&arg_mtime);

	pthread_mutex_unlock(&FUSEG(m));
	
	return r;
}

PHP_FUSE_API int php_fuse_create(const char * path, mode_t mode, struct fuse_file_info *fi) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_mode;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_mode);
	ZVAL_LONG(arg_mode, mode);

	zend_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "create", &retval, arg_path, arg_mode);

	convert_to_long_ex(&retval);
	long r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);
	
	if (r >= 0) {
		fi->fh = (ulong)r;
	}

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_mode);

	pthread_mutex_unlock(&FUSEG(m));
	
	return r >= 0 ? 0 : r;
}

PHP_FUSE_API int php_fuse_open(const char * path, struct fuse_file_info * fi) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_mode;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_mode);
	ZVAL_LONG(arg_mode, fi->flags);

	zend_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "open", &retval, arg_path, arg_mode);

	convert_to_long_ex(&retval);
	long r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);
	
	if (r >= 0) {
		fi->fh = (ulong)r;
	}

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_mode);

	pthread_mutex_unlock(&FUSEG(m));
	
	return r >= 0 ? 0 : r;
}

PHP_FUSE_API int php_fuse_read(const char * path, char * buf, size_t buf_len, off_t offset, struct fuse_file_info * fi) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -EBADF;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_fh;
	zval *arg_offset;
	zval *arg_buf_len;
	zval *arg_buf;
	char *p = estrdup(path);
	char *mybuf = estrdup(buf);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_fh);
	ZVAL_LONG(arg_fh, fi ? fi->fh : 0);

	MAKE_STD_ZVAL(arg_offset);
	ZVAL_LONG(arg_offset, offset);

	MAKE_STD_ZVAL(arg_buf_len);
	ZVAL_LONG(arg_buf_len, buf_len);

	MAKE_STD_ZVAL(arg_buf);
	ZVAL_STRING(arg_buf, mybuf, 0);

	php_fuse_call_method_with_5_params(&active_object, Z_OBJCE_P(active_object), NULL, "read", &retval, arg_path, arg_fh, arg_offset, arg_buf_len, arg_buf);

	convert_to_long_ex(&retval);
	long r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);
	
	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_fh);
	zval_ptr_dtor(&arg_offset);
	zval_ptr_dtor(&arg_buf_len);

	if (r < 0) {
		zval_ptr_dtor(&arg_buf);
		pthread_mutex_unlock(&FUSEG(m));
		return r;
	}
	convert_to_string_ex(&arg_buf);
	if (Z_STRVAL_P(arg_buf) == NULL) {
		zval_ptr_dtor(&arg_buf);
		pthread_mutex_unlock(&FUSEG(m));
		return 0;
	}

	r = Z_STRLEN_P(arg_buf);
	memcpy(buf, Z_STRVAL_P(arg_buf), r > buf_len ? buf_len : r);
	zval_ptr_dtor(&arg_buf);

	pthread_mutex_unlock(&FUSEG(m));
	
	return r > buf_len ? buf_len : r;
}

PHP_FUSE_API int php_fuse_write(const char * path, const char * buf, size_t buf_len, off_t offset, struct fuse_file_info * fi) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -EBADF;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_fh;
	zval *arg_buf;
	zval *arg_offset;
	char *p = estrdup(path);
	//buf is not nullbyte-terminated! we have to add it by hand here...
	char *q = ecalloc(sizeof(char),buf_len+1);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	memcpy(q, buf, buf_len);
	MAKE_STD_ZVAL(arg_buf);
	ZVAL_STRINGL(arg_buf, q, buf_len, 0);

	MAKE_STD_ZVAL(arg_fh);
	if ( fi == 0 )
	{
		ZVAL_LONG(arg_fh, 0);
	}
	else
	{
		ZVAL_LONG(arg_fh, fi->fh);
	}

	MAKE_STD_ZVAL(arg_offset);
	ZVAL_LONG(arg_offset, offset);

	php_fuse_call_method_with_4_params(&active_object, Z_OBJCE_P(active_object), NULL, "write", &retval, arg_path, arg_fh, arg_offset, arg_buf);

	convert_to_long_ex(&retval);
	long r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);
	
	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_fh);
	zval_ptr_dtor(&arg_offset);
	zval_ptr_dtor(&arg_buf);

	pthread_mutex_unlock(&FUSEG(m));
	
	return r;
}

PHP_FUSE_API int php_fuse_statfs(const char * path, struct statvfs * st) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -EFAULT;
	}

	memset(st, 0, sizeof(struct statvfs));

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_st;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_st);
	array_init(arg_st);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "statfs", &retval, arg_path, arg_st);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);

	if (r < 0) {
		zval_ptr_dtor(&arg_st);
		pthread_mutex_unlock(&FUSEG(m));
		return r;
	}

	/* reference retval handling */
	if (Z_TYPE_P(arg_st) != IS_ARRAY) {
		zval_ptr_dtor(&arg_st);
		pthread_mutex_unlock(&FUSEG(m));
		return -EFAULT;
	}

	HashPosition	pos;
	zval			**entry;
	char			*tmp_s_key;
	uint			tmp_s_key_len;
	ulong			tmp_n_key;

	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(arg_st), &pos);
	while (zend_hash_get_current_data_ex(Z_ARRVAL_P(arg_st), (void **)&entry, &pos) == SUCCESS) {
		convert_to_long_ex(entry);
		int value = Z_LVAL_PP(entry);

		switch (zend_hash_get_current_key_ex(Z_ARRVAL_P(arg_st), &tmp_s_key, &tmp_s_key_len, &tmp_n_key, 0, &pos)) {	// no duplication
		case HASH_KEY_IS_STRING:
			if (strncmp(tmp_s_key, "bsize", tmp_s_key_len) == 0) {
				st->f_bsize = value;
			} else if (strncmp(tmp_s_key, "frsize", tmp_s_key_len) == 0) {
				st->f_frsize = value;
			} else if (strncmp(tmp_s_key, "blocks", tmp_s_key_len) == 0) {
				st->f_blocks = value;
			} else if (strncmp(tmp_s_key, "bfree", tmp_s_key_len) == 0) {
				st->f_bfree = value;
			} else if (strncmp(tmp_s_key, "bavail", tmp_s_key_len) == 0) {
				st->f_bavail = value;
			} else if (strncmp(tmp_s_key, "files", tmp_s_key_len) == 0) {
				st->f_files = value;
			} else if (strncmp(tmp_s_key, "ffree", tmp_s_key_len) == 0) {
				st->f_ffree = value;
			} else if (strncmp(tmp_s_key, "favail", tmp_s_key_len) == 0) {
				st->f_favail = value;
			} else if (strncmp(tmp_s_key, "fsid", tmp_s_key_len) == 0) {
				st->f_fsid = value;
			} else if (strncmp(tmp_s_key, "flag", tmp_s_key_len) == 0) {
				st->f_flag = value;
			} else if (strncmp(tmp_s_key, "namemax", tmp_s_key_len) == 0) {
				st->f_namemax = value;
			}
			break;
		case HASH_KEY_IS_LONG:
			break;
		}
		zend_hash_move_forward_ex(Z_ARRVAL_P(arg_st), &pos);
	}

	zval_ptr_dtor(&arg_st);

	pthread_mutex_unlock(&FUSEG(m));
	
	return r;
}

PHP_FUSE_API int php_fuse_flush(const char * path, struct fuse_file_info * fi) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -EBADF;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_fh;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_fh);
	ZVAL_LONG(arg_fh, fi->fh);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "flush", &retval, arg_path, arg_fh);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_fh);

	pthread_mutex_unlock(&FUSEG(m));
	
	return r;
}

PHP_FUSE_API int php_fuse_release(const char * path, struct fuse_file_info * fi) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_fh;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_fh);
	ZVAL_LONG(arg_fh, fi->fh);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "release", &retval, arg_path, arg_fh);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_fh);

	pthread_mutex_unlock(&FUSEG(m));
	
	return r;
}

PHP_FUSE_API int php_fuse_fsync(const char * path, int mode, struct fuse_file_info * fi) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -EBADF;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_fh;
	zval *arg_mode;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_fh);
	ZVAL_LONG(arg_fh, fi->fh);

	MAKE_STD_ZVAL(arg_mode);
	ZVAL_LONG(arg_mode, mode);

	php_fuse_call_method_with_3_params(&active_object, Z_OBJCE_P(active_object), NULL, "fsync", &retval, arg_path, arg_fh, arg_mode);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_fh);
	zval_ptr_dtor(&arg_mode);

	pthread_mutex_unlock(&FUSEG(m));
	
	return r;
}

PHP_FUSE_API int php_fuse_setxattr(const char * path, const char * name, const char * value, size_t value_len, int flag) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -ENOSPC;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_name;
	zval *arg_value;
	zval *arg_mode;
	char *p = estrdup(path);
	char *q = estrdup(name);
	char *v = emalloc(value_len);
	memcpy(v, value, value_len);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_name);
	ZVAL_STRING(arg_name, q, 0);

	MAKE_STD_ZVAL(arg_value);
	ZVAL_STRINGL(arg_value, v, value_len, 0);

	MAKE_STD_ZVAL(arg_mode);
	ZVAL_LONG(arg_mode, flag);

	php_fuse_call_method_with_4_params(&active_object, Z_OBJCE_P(active_object), NULL, "setxattr", &retval, arg_path, arg_name, arg_value, arg_mode);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_name);
	zval_ptr_dtor(&arg_value);
	zval_ptr_dtor(&arg_mode);

	pthread_mutex_unlock(&FUSEG(m));
	
	return r;
}

PHP_FUSE_API int php_fuse_getxattr(const char * path, const char * name, char * value, size_t value_len) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_name;
	zval *arg_value;
	char *p = estrdup(path);
	char *q = estrdup(name);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_name);
	ZVAL_STRING(arg_name, q, 0);

	MAKE_STD_ZVAL(arg_value);

	php_fuse_call_method_with_3_params(&active_object, Z_OBJCE_P(active_object), NULL, "getxattr", &retval, arg_path, arg_name, arg_value);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_name);

	if (r < 0) {
		zval_ptr_dtor(&arg_value);
		pthread_mutex_unlock(&FUSEG(m));
		return r;
	}

	convert_to_string_ex(&arg_value);
	memcpy(value, Z_STRVAL_P(arg_value), r > value_len ? value_len : r);
	zval_ptr_dtor(&arg_value);

	pthread_mutex_unlock(&FUSEG(m));
	
	return r > value_len ? value_len : r;
}

PHP_FUSE_API int php_fuse_listxattr(const char * path, char * list, size_t list_len) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_list;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_list);
	array_init(arg_list);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "listxattr", &retval, arg_path, arg_list);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);

	if (r < 0) {
		zval_ptr_dtor(&arg_list);
		pthread_mutex_unlock(&FUSEG(m));
		return r;
	}

	if (Z_TYPE_P(arg_list) != IS_ARRAY) {
		convert_to_string_ex(&arg_list);
		memcpy(list, Z_STRVAL_P(arg_list), r > list_len ? list_len : r);
		zval_ptr_dtor(&arg_list);
	} else {
		HashPosition	pos;
		zval			**entry;

		zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(arg_list), &pos);
		char *q = list;
		memset(list, 0, list_len);
		while (zend_hash_get_current_data_ex(Z_ARRVAL_P(arg_list), (void **)&entry, &pos) == SUCCESS) {
			convert_to_string_ex(entry);
			if ((q-list) + Z_STRLEN_PP(entry) >= list_len) {
				break;
			}
			memcpy(q, Z_STRVAL_PP(entry), Z_STRLEN_PP(entry));
			q += Z_STRLEN_PP(entry);
			if ((q-list) + 1 < list_len) {
				break;
			}
			*q++ = ' ';

			zend_hash_move_forward_ex(Z_ARRVAL_P(arg_list), &pos);
		}
	}

	pthread_mutex_unlock(&FUSEG(m));
	
	return r;
}

PHP_FUSE_API int php_fuse_removexattr(const char * path, const char * name) {
	TSRMLS_FETCH();
	zval *active_object = NULL;
	
	pthread_mutex_lock(&FUSEG(m));
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&FUSEG(m));
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_name;
	char *p = estrdup(path);
	char *q = estrdup(name);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_name);
	ZVAL_STRING(arg_name, q, 0);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "removexattr", &retval, arg_path, arg_name);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_name);

	pthread_mutex_unlock(&FUSEG(m));
	
	return r;
}

/*
PHP_FUSE_API int php_fuse_opendir(const char *, struct fuse_file_info *);
PHP_FUSE_API int php_fuse_readdir(const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *);
PHP_FUSE_API int php_fuse_releasedir(const char *, struct fuse_file_info *);
PHP_FUSE_API int php_fuse_fsyncdir(const char *, int, struct fuse_file_info *);
PHP_FUSE_API void* php_fuse_init(struct fuse_conn_info *conn);
PHP_FUSE_API void php_fuse_destroy(void *);
PHP_FUSE_API int php_fuse_access(const char *, int);
PHP_FUSE_API int php_fuse_ftruncate(const char *, off_t, struct fuse_file_info *);
PHP_FUSE_API int php_fuse_fgetattr(const char *, struct stat *, struct fuse_file_info *);
PHP_FUSE_API int php_fuse_lock(const char *, struct fuse_file_info *, int cmd, struct flock *);
PHP_FUSE_API int php_fuse_utimens(const char *, const struct timespec tv[2]);
PHP_FUSE_API int php_fuse_bmap(const char *, size_t blocksize, uint64_t *idx);
*/
/* }}} */

/* {{{ fuse static vars */
static char * php_fuse_method_list[] = {
	"getattr",
	"readlink",
	"getdir",
	"mknod",
	"mkdir",
	"unlink",
	"rmdir",
	"symlink",
	"rename",
	"link",
	"chmod",
	"chown",
	"truncate",
	"utime",
	"open",
	"create",
	"read",
	"write",
	"statfs",
	"flush",
	"release",
	"fsync",
	"setxattr",
	"getxattr",
	"listxattr",
	"removexattr",
};
static struct fuse_operations php_fuse_operations = {
	.getattr		= php_fuse_getattr,
	.readlink		= php_fuse_readlink,
	.getdir			= php_fuse_getdir,
	.mknod			= php_fuse_mknod,
	.mkdir			= php_fuse_mkdir,
	.unlink			= php_fuse_unlink,
	.rmdir			= php_fuse_rmdir,
	.symlink		= php_fuse_symlink,
	.rename			= php_fuse_rename,
	.link			= php_fuse_link,
	.chmod			= php_fuse_chmod,
	.chown			= php_fuse_chown,
	.truncate		= php_fuse_truncate,
	.utime			= php_fuse_utime,
	.open			= php_fuse_open,
	.create			= php_fuse_create,
	.read			= php_fuse_read,
	.write			= php_fuse_write,
	.statfs			= php_fuse_statfs,
	.flush			= php_fuse_flush,
	.release		= php_fuse_release,
	.fsync			= php_fuse_fsync,
	.setxattr		= php_fuse_setxattr,
	.getxattr		= php_fuse_getxattr,
	.listxattr		= php_fuse_listxattr,
	.removexattr		= php_fuse_removexattr,
};

static zend_class_entry *php_fuse_ce;
static zend_object_handlers php_fuse_object_handlers;
static zend_object_handlers php_fuse_object_handlers_ze1;
/* }}} */

/* {{{ fuse object handlers */
static void php_fuse_object_free_storage(void *object TSRMLS_DC) {
	php_fuse_object *intern = (php_fuse_object*)object;
	
	zend_object_std_dtor(&intern->zo TSRMLS_CC);
	efree(object);
}

static zend_object_value php_fuse_object_handler_new(zend_class_entry *ce TSRMLS_DC) {
	zend_object_value retval;
	php_fuse_object *intern;
	zval *tmp;

	intern = emalloc(sizeof(php_fuse_object));
	memset(intern, 0, sizeof(php_fuse_object));

	zend_object_std_init(&intern->zo, ce TSRMLS_CC);
#if PHP_VERSION_ID < 50399
	zend_hash_copy(intern->zo.properties, &ce->default_properties, (copy_ctor_func_t)zval_add_ref, (void*)&tmp, sizeof(zval*));
#else
	object_properties_init((zend_object*) intern, ce);
#endif
	retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t)php_fuse_object_free_storage, NULL TSRMLS_CC);
#if PHP_VERSION_ID < 50300
	retval.handlers = EG(ze1_compatibility_mode) ? &php_fuse_object_handlers_ze1 : &php_fuse_object_handlers;
#else
	retval.handlers = &php_fuse_object_handlers;
#endif

	return retval;
}
/* }}} */

/* {{{ fuse method */
static PHP_METHOD(Fuse, fuse_constructor) {
	zval *object = getThis();

	return;
}

static PHP_METHOD(Fuse, fuse_main) {
	zval *object = getThis();
	
	long ac;
	zval* av;
	HashTable* av_hash;
	HashPosition av_ptr;
	int av_size;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "la", &ac, &av) == FAILURE) {
		return;
	}

	FUSEG(active_object) = object;

	av_hash=Z_ARRVAL_P(av);
	av_size=zend_hash_num_elements(av_hash);
	
	if(ac!=av_size)
		php_error(E_ERROR,"Fuse.fuse_main: Size mismatch between argc=%d and sizeof(argv)=%d",ac,av_size);
	
//	php_printf("Fuse.fuse_main: launching with %d args",ac);
	
	char** av_c=safe_emalloc(sizeof(char*),ac,0);
	zval** d;
	int i=0;
	int have_singlethread=0; //did the user supply -s? if not, we must to avoid threading problems!
	for(zend_hash_internal_pointer_reset_ex(av_hash, &av_ptr); zend_hash_get_current_data_ex(av_hash, (void**) &d, &av_ptr) == SUCCESS; zend_hash_move_forward_ex(av_hash, &av_ptr)) {
//		php_printf("Element %d",i);
		if(Z_TYPE_PP(d)!=IS_STRING) {
			php_error(E_WARNING,"Fuse.fuse_main: argv[%d] is not a string, converting silently",i);
			convert_to_string_ex(d);
		}
		if(strcmp(Z_STRVAL_PP(d),"-s")==0)
			have_singlethread=1;
		av_c[i]=estrndup(Z_STRVAL_PP(d),Z_STRLEN_PP(d));
		i++;
        }
#ifndef PHP_FUSE_ALLOW_MT
        //enforce single threading
        if(!have_singlethread) {
//        	php_error(E_WARNING,"Fuse.fuse_main: User did not supply -s for single-thread operation");
        	av_c=erealloc(av_c,sizeof(char*)*(ac+1));
        	av_c[ac]=estrdup("-s");
        	add_next_index_string(av,"-s",1);
        	ac++;
        	av_size++;
	}
#endif
	
	struct fuse_operations op;
	memset(&op, 0, sizeof(struct fuse_operations));
	zend_class_entry *object_ce = Z_OBJCE_P(object);
	if (!object_ce) {
		RETURN_FALSE;
	}
	HashTable *object_ft = &object_ce->function_table;

	for (i = 0; i < sizeof(php_fuse_method_list) / sizeof(char*); i++) {
		void *tmp;
		if (zend_hash_find(object_ft, php_fuse_method_list[i], strlen(php_fuse_method_list[i])+1, &tmp) == FAILURE) {
			continue;
		}

		/* void** way could cause corruption...:( */
		switch (i) {
		case 0:
			op.getattr = php_fuse_operations.getattr; break;
		case 1:
			op.readlink = php_fuse_operations.readlink; break;
		case 2:
			op.getdir = php_fuse_operations.getdir; break;
		case 3:
			op.mknod = php_fuse_operations.mknod; break;
		case 4:
			op.mkdir = php_fuse_operations.mkdir; break;
		case 5:
			op.unlink = php_fuse_operations.unlink; break;
		case 6:
			op.rmdir = php_fuse_operations.rmdir; break;
		case 7:
			op.symlink = php_fuse_operations.symlink; break;
		case 8:
			op.rename = php_fuse_operations.rename; break;
		case 9:
			op.link = php_fuse_operations.link; break;
		case 10:
			op.chmod = php_fuse_operations.chmod; break;
		case 11:
			op.chown = php_fuse_operations.chown; break;
		case 12:
			op.truncate = php_fuse_operations.truncate; break;
		case 13:
			op.utime = php_fuse_operations.utime; break;
		case 14:
			op.open = php_fuse_operations.open; break;
		case 15:
			op.create = php_fuse_operations.create; break;
		case 16:
			op.read = php_fuse_operations.read; break;
		case 17:
			op.write = php_fuse_operations.write; break;
		case 18:
			op.statfs = php_fuse_operations.statfs; break;
		case 19:
			op.flush = php_fuse_operations.flush; break;
		case 20:
			op.release = php_fuse_operations.release; break;
		case 21:
			op.fsync = php_fuse_operations.fsync; break;
		case 22:
			op.setxattr = php_fuse_operations.setxattr; break;
		case 23:
			op.getxattr = php_fuse_operations.getxattr; break;
		case 24:
			op.listxattr = php_fuse_operations.listxattr; break;
		case 25:
			op.removexattr = php_fuse_operations.removexattr; break;
		}
	}

	fuse_main(ac, av_c, &op);
	
	for(i=0;i<ac;i++)
		efree(av_c[i]);
	efree(av_c);

	FUSEG(active_object) = NULL;
	return;
}

//Helper method: assemble a userdata struct out of a PHP ZVAL array following the format in the docs
zval* php_fuse_get_udata(void* udata) {
	zval* ret;
	ALLOC_INIT_ZVAL(ret);
	array_init(ret);

	zval* array=*(zval**)((char*)(udata)+0);
	
	HashTable* array_hash=Z_ARRVAL_P(array);
	HashPosition array_ptr;
	int array_size=zend_hash_num_elements(array_hash);
//	php_printf("php_fuse_get_udata: init with %d elements\n",array_size);
	
	int i=0;
	zval** d;
	unsigned long cur_offset=sizeof(void*);//for fuse_opt.offset
	
	for(zend_hash_internal_pointer_reset_ex(array_hash, &array_ptr); zend_hash_get_current_data_ex(array_hash, (void**) &d, &array_ptr) == SUCCESS; zend_hash_move_forward_ex(array_hash, &array_ptr)) {
		zval** initial; //initial value for the udata field
		zval** v;	//the value of the zval, temp
		
//		php_printf("Element %d, type %d/%s\n",i,Z_TYPE_PP(d),zend_get_type_by_const(Z_TYPE_PP(d)));
		if(Z_TYPE_PP(d)!=IS_ARRAY)
			php_error(E_ERROR,"Fuse.opt_parse: element %d is not an array",i);
		HashTable* d_hash=Z_ARRVAL_PP(d);
		
		if(zend_hash_find(d_hash,"value",sizeof("value"),(void**) &v)!=SUCCESS)
			php_error(E_ERROR,"Fuse.opt_parse: element %d doesn't contain a value key",i);
		initial=v;
		
//		php_printf("Element %d, current offset %ld\n",i,cur_offset);
		switch(Z_TYPE_PP(initial)) {
			case IS_LONG:
				add_next_index_long(ret,*(long*)((char*)(udata)+cur_offset));
				cur_offset+=sizeof(long);
			break;
			case IS_STRING:
				add_next_index_string(ret,*(char**)((char*)(udata)+cur_offset),1);
				cur_offset+=sizeof(char*)*2;
			break;
			default:
				php_error(E_ERROR,"Fuse.opt_parse: element %d initial value is neither int nor string",i);
		}
		i++;
	}

	return ret;
}
//Helper method: update the values in a PHP ZVAL array following the format in the docs
void php_fuse_set_udata(void* udata, zval* user_array) {
	zval* array=*(zval**)((char*)(udata)+0);
	
	HashTable* array_hash=Z_ARRVAL_P(array);
	HashPosition array_ptr;
	int array_size=zend_hash_num_elements(array_hash);
	
	if(Z_TYPE_P(user_array)!=IS_ARRAY)
		php_error(E_ERROR,"Fuse.opt_parse: didn't supply an array for writeback");
	HashTable* user_array_hash=Z_ARRVAL_P(user_array);
	int user_array_size=zend_hash_num_elements(user_array_hash);
	if(array_size!=user_array_size)
		php_error(E_ERROR,"Fuse.opt_parse: size mismatch in writeback from original %d to returned %d",array_size,user_array_size);
	
//	php_printf("php_fuse_set_udata: init with %d elements\n",array_size);
	
	int i=0;
	zval** d;
	unsigned long cur_offset=sizeof(void*);//for fuse_opt.offset
	
	for(zend_hash_internal_pointer_reset_ex(array_hash, &array_ptr); zend_hash_get_current_data_ex(array_hash, (void**) &d, &array_ptr) == SUCCESS; zend_hash_move_forward_ex(array_hash, &array_ptr)) {
		zval** initial; //initial value for the udata field
		zval** v;	//the value of the zval, temp
		
//		php_printf("Element %d, type %d/%s\n",i,Z_TYPE_PP(d),zend_get_type_by_const(Z_TYPE_PP(d)));
		
		if(Z_TYPE_PP(d)!=IS_ARRAY)
			php_error(E_ERROR,"Fuse.opt_parse: element %d is not an array",i);
		HashTable* d_hash=Z_ARRVAL_PP(d);
		
		if(zend_hash_find(d_hash,"value",sizeof("value"),(void**) &v)!=SUCCESS)
			php_error(E_ERROR,"Fuse.opt_parse: element %d doesn't contain a value key",i);
		initial=v;
		
		//search in user_array for the i'th element
		if(zend_hash_index_find(user_array_hash,i,(void**) &v)!=SUCCESS)
			php_error(E_ERROR,"Fuse.opt_parse: could not locate element %d in user-array",i);
		if(Z_TYPE_PP(initial)!=Z_TYPE_PP(v))
			php_error(E_ERROR,"Fuse.opt_parse: writeback type mismatch for element %d, original %d/%s, user-array: %d/%s",i,Z_TYPE_PP(initial),zend_get_type_by_const(Z_TYPE_PP(initial)),Z_TYPE_PP(v),zend_get_type_by_const(Z_TYPE_PP(v)));
//		php_printf("Element %d, current offset %ld\n",i,cur_offset);
		switch(Z_TYPE_PP(initial)) {
			case IS_LONG:
//				php_printf("updated lv from %ld to %ld\n",*(long*)((char*)(udata)+cur_offset),Z_LVAL_PP(v));
				*(long*)((char*)(udata)+cur_offset)=Z_LVAL_PP(v);
				cur_offset+=sizeof(long);
			break;
			case IS_STRING:
				udata=udata; //no declarations as first statement after cas
				char* orig=*(char**)((char*)(udata)+cur_offset);
				char* backup=*(char**)((char*)(udata)+cur_offset+sizeof(char*));
				char* newstr=Z_STRVAL_PP(v);
//				php_printf("updating str from '%s' to '%s'\n",orig,newstr);
				if(orig!=backup) {
					efree(backup);
					free(orig);
//					printf("freed %lx (orig) and %lx (backup)\n",orig,backup);
				} else {
					efree(backup);
//					printf("freed %lx (orig)\n",orig,backup);
				}
				*(char**)((char*)(udata)+cur_offset)=estrndup(Z_STRVAL_PP(v),Z_STRLEN_PP(v));
				*(char**)((char*)(udata)+cur_offset+sizeof(char*))=*(char**)((char*)(udata)+cur_offset);
				cur_offset+=sizeof(char*)*2;
			break;
			default:
				php_error(E_ERROR,"Fuse.opt_parse: element %d initial value is neither int nor string",i);
		}
		i++;
	}

}
//Helper method: allocate and initialize udata from PHP ZVAL array; modify fopts array to accomodate new options
void* php_fuse_init_udata(zval* array, struct fuse_opt** fopts, int* num_fopts) {
	HashTable* array_hash=Z_ARRVAL_P(array);
	HashPosition array_ptr;
	int array_size=zend_hash_num_elements(array_hash);
//	php_printf("php_fuse_init_udata: init with %d elements, fopts array has %d elements\n",array_size,*num_fopts);
	
	int i=0;
	zval** d;
	unsigned long cur_offset=sizeof(void*);//for fuse_opt.offset
	size_t size=sizeof(void*);
//	php_printf("Allocating %d bytes, sizeof is %d\n",size,cur_offset);
	void* udata=emalloc(size);
//	php_printf("udata located @ 0x%lx\n",udata);
	*(zval**)((char*)(udata)+0)=array;

	for(zend_hash_internal_pointer_reset_ex(array_hash, &array_ptr); zend_hash_get_current_data_ex(array_hash, (void**) &d, &array_ptr) == SUCCESS; zend_hash_move_forward_ex(array_hash, &array_ptr)) {
		zval** templ;
		zval** initial; //initial value for the udata field
		zval** key;
		
//		php_printf("Element %d, type %d/%s\n",i,Z_TYPE_PP(d),zend_get_type_by_const(Z_TYPE_PP(d)));
		if(Z_TYPE_PP(d)!=IS_ARRAY)
			php_error(E_ERROR,"Fuse.opt_parse: element %d is not an array",i);
		HashTable* d_hash=Z_ARRVAL_PP(d);
		
		if(zend_hash_find(d_hash,"templ",sizeof("templ"),(void**) &templ)!=SUCCESS)
			php_error(E_ERROR,"Fuse.opt_parse: element %d doesn't contain a templ key",i);
		if(Z_TYPE_PP(templ)!=IS_STRING)
			php_error(E_ERROR,"Fuse.opt_parse: element %d key templ is not a string",i);
		
		if(zend_hash_find(d_hash,"value",sizeof("value"),(void**) &initial)!=SUCCESS)
			php_error(E_ERROR,"Fuse.opt_parse: element %d doesn't contain a value key",i);
		
		if(zend_hash_find(d_hash,"key",sizeof("key"),(void**) &key)!=SUCCESS)
			php_error(E_ERROR,"Fuse.opt_parse: element %d doesn't contain a key key",i);
		if(Z_TYPE_PP(key)!=IS_LONG)
			php_error(E_ERROR,"Fuse.opt_parse: element %d key key is not a long",i);
		
//		php_printf("Element %d, current offset %ld, current size %ld\n",i,cur_offset,size);
		switch(Z_TYPE_PP(initial)) {
			case IS_LONG:
				size+=sizeof(long);
				udata=erealloc(udata,size);
//				php_printf("element %d is long, resized to %ld, udata now at %lx\n",i,size,udata);
				*(long*)((char*)(udata)+cur_offset)=Z_LVAL_PP(initial);
//				printf("set value of %lx to %lx\n",((char*)(udata)+cur_offset),Z_LVAL_PP(initial));
				(*fopts)[*num_fopts].offset=cur_offset; //set this here before modifying it and thus causing overflow!
				cur_offset+=sizeof(long);
			break;
			case IS_STRING:
				//Allocate two char*, because fuse will overwrite the first. By comparing with the second, free can later check
				//if fuse messed up and leaked memory.
				size+=sizeof(char*)*2;
				udata=erealloc(udata,size);
//				php_printf("element %d is string, resized to %ld, udata now at %lx\n",i,size,udata);
				*(char**)((char*)(udata)+cur_offset)=estrndup(Z_STRVAL_PP(initial),Z_STRLEN_PP(initial));
				*(char**)((char*)(udata)+cur_offset+sizeof(char*))=*(char**)((char*)(udata)+cur_offset);
//				printf("set value of %lx to %lx, content '%s'\n",((char*)(udata)+cur_offset),*(char**)((char*)(udata)+cur_offset),Z_STRVAL_PP(initial));
				(*fopts)[*num_fopts].offset=cur_offset;
				cur_offset+=sizeof(char*)*2;
			break;
			default:
				php_error(E_ERROR,"Fuse.opt_parse: element %d initial value is neither int nor string",i);
		}
		// add entry in fopts
		
		(*fopts)[*num_fopts].templ=estrndup(Z_STRVAL_PP(templ),Z_STRLEN_PP(templ));;		
		(*fopts)[*num_fopts].value=(int)Z_LVAL_PP(key);
		*num_fopts=*num_fopts+1;
//		php_printf("fopts now has %d opts, assigned %li bytes of RAM for %li bytes wide struct\n",*num_fopts,(sizeof(struct fuse_opt)*(*num_fopts+1)),sizeof(struct fuse_opt));
		*fopts=safe_erealloc(*fopts,sizeof(struct fuse_opt),(*num_fopts)+1,0);
		(*fopts)[*num_fopts].templ=NULL;
		(*fopts)[*num_fopts].offset=0;
		(*fopts)[*num_fopts].value=0;
		i++;
	}
//	int c=1/0;
	return udata;
}
//Helper method: write back to zval, destroy and free the udata
void php_fuse_free_udata(void* udata) {
	zval* array=*(zval**)((char*)(udata)+0);
	
	HashTable* array_hash=Z_ARRVAL_P(array);
	HashPosition array_ptr;
	int array_size=zend_hash_num_elements(array_hash);
//	php_printf("php_fuse_free_udata: init with %d elements\n",array_size);
	
	int i=0;
	zval** d;
	unsigned long cur_offset=sizeof(void*);//for fuse_opt.offset
	
	for(zend_hash_internal_pointer_reset_ex(array_hash, &array_ptr); zend_hash_get_current_data_ex(array_hash, (void**) &d, &array_ptr) == SUCCESS; zend_hash_move_forward_ex(array_hash, &array_ptr)) {
		zval** initial; //initial value for the udata field
		zval** v;	//the value of the zval, temp
		
//		php_printf("Element %d, type %d/%s\n",i,Z_TYPE_PP(d),zend_get_type_by_const(Z_TYPE_PP(d)));
		if(Z_TYPE_PP(d)!=IS_ARRAY)
			php_error(E_ERROR,"Fuse.opt_parse: element %d is not an array",i);
		HashTable* d_hash=Z_ARRVAL_PP(d);
		
		if(zend_hash_find(d_hash,"value",sizeof("value"),(void**) &v)!=SUCCESS)
			php_error(E_ERROR,"Fuse.opt_parse: element %d doesn't contain a value key",i);
		initial=v;
		
//		php_printf("Element %d, current offset %ld, udata at %lx\n",i,cur_offset,udata);
		switch(Z_TYPE_PP(initial)) {
			case IS_LONG:
//				php_printf("element %d is long, nothing to free here\n",i);
				ZVAL_LONG(*initial,*(long*)((char*)(udata)+cur_offset));
				cur_offset+=sizeof(long);
			break;
			case IS_STRING:
//				php_printf("element %d is string, offset %d, loc %lx, loc_backup %lx, content '%s'\n",i,cur_offset,*(char**)((char*)(udata)+cur_offset),*(char**)((char*)(udata)+cur_offset+sizeof(char*)),*(char**)((char*)(udata)+cur_offset));
				//Free the backup, because if fuse overwrote the "original" we have a problem...
				udata=udata; //pointless hack: a declaration can't be the first thing after case
				char* orig=*(char**)((char*)(udata)+cur_offset);
				char* backup=*(char**)((char*)(udata)+cur_offset+sizeof(char*));
#ifdef ZTS
				//free the old value of the ZVAL to avoid leaking it's old content
				if(Z_STRVAL_PP(initial))
					efree(Z_STRVAL_PP(initial));
#endif
				ZVAL_STRING(*initial,orig,1);
				if(orig!=backup) {
					efree(backup);
					free(orig);
//					printf("freed %lx (orig) and %lx (backup)\n",orig,backup);
				} else {
					efree(backup);
//					printf("freed %lx (orig)\n",orig,backup);
				}
				
				cur_offset+=sizeof(char*)*2;
			break;
			default:
				php_error(E_ERROR,"Fuse.opt_parse: element %d initial value is neither int nor string",i);
		}
		i++;
	}
	efree(udata);
//	php_printf("free done\n");
	return;
}

PHP_FUSE_API int php_fuse_opt_parse_proc(void* data, const char* arg, int key, struct fuse_args* outargs) {
	TSRMLS_FETCH();
//	php_printf("----\nopt_parse_proc called from external. Key is %d, arg is %s, outargs.argc is %d, outargs.argv are:\n",key,arg,outargs->argc);
	int i;
//	for(i=0;i<outargs->argc;i++)
//		php_printf("'%s'\n",outargs->argv[i]);
	
	int ret=0;
	
	//step 1: convert the parameters to zvals so they can be passed to userland
	zval* arg_data=php_fuse_get_udata(data);

	zval* arg_arg;
	ALLOC_INIT_ZVAL(arg_arg);
	ZVAL_STRING(arg_arg,arg,1);
	
	zval* arg_key;
	ALLOC_INIT_ZVAL(arg_key);
	ZVAL_LONG(arg_key,key);
	
	zval* arg_argc;
	ALLOC_INIT_ZVAL(arg_argc);
	ZVAL_LONG(arg_argc,outargs->argc);
	
	zval* arg_argv;
	zval** d;
	HashTable* arg_argv_hash;
	HashPosition arg_argv_ptr;
	int arg_argv_size;
	ALLOC_INIT_ZVAL(arg_argv);
	array_init(arg_argv);
	for(i=0;i<outargs->argc;i++)
		add_index_string(arg_argv,i,outargs->argv[i],1); //copy and duplicate

	fuse_opt_free_args(outargs); //free the old resource, we'll rebuild it later with the return from userland

/*	//Dump it
	arg_argv_hash=Z_ARRVAL_P(arg_argv);
	arg_argv_size=zend_hash_num_elements(arg_argv_hash);
	php_printf("arg_argv contents (size: %d):\n",arg_argv_size);
	i=0;
	for(zend_hash_internal_pointer_reset_ex(arg_argv_hash, &arg_argv_ptr); zend_hash_get_current_data_ex(arg_argv_hash, (void**) &d, &arg_argv_ptr) == SUCCESS; zend_hash_move_forward_ex(arg_argv_hash, &arg_argv_ptr)) {
		if(Z_TYPE_PP(d)!=IS_STRING) {
			php_error(E_WARNING,"php_fuse_opt_parse_proc: modified argv key %d is not a string (type %d/%s), converting silently",i,Z_TYPE_PP(d),zend_get_type_by_const(Z_TYPE_PP(d)));
			convert_to_string_ex(d);
		}
		php_printf("Element %d: '",i);
		php_write(Z_STRVAL_PP(d),Z_STRLEN_PP(d));
		php_printf("'\n");
		i++;
	}
*/
	//step 2: call userland
	zval** args[5], *retval_ptr;

	args[0]=&arg_data;
	args[1]=&arg_arg;
	args[2]=&arg_key;
	args[3]=&arg_argc;
	args[4]=&arg_argv;
	FUSEG(proc_fci).retval_ptr_ptr=&retval_ptr;
	FUSEG(proc_fci).param_count=5;
	FUSEG(proc_fci).params=args;

	if (zend_call_function(&FUSEG(proc_fci),&FUSEG(proc_fcic) TSRMLS_CC)==SUCCESS) {
		if(!retval_ptr)
			php_error(E_ERROR,"php_fuse_opt_parse_proc: retval_ptr is null");
		if(Z_TYPE_P(retval_ptr)!=IS_LONG)
			php_error(E_ERROR,"php_fuse_opt_parse_proc: typeof(retval)!=int");
//		php_printf("php_fuse_opt_parse_proc: returned %d from userland\n",Z_LVAL_P(retval_ptr));
		ret=(int)Z_LVAL_P(retval_ptr);
		arg_argv_hash=Z_ARRVAL_P(arg_argv);
		arg_argv_size=zend_hash_num_elements(arg_argv_hash);
		if(arg_argv_size!=Z_LVAL_P(arg_argc))
			php_error(E_ERROR,"php_fuse_opt_parse_proc: size mismatch in modified argc=%li/sizeof(argv)=%i",Z_LVAL_P(arg_argc),arg_argv_size);
		i=0;
		for(zend_hash_internal_pointer_reset_ex(arg_argv_hash, &arg_argv_ptr); zend_hash_get_current_data_ex(arg_argv_hash, (void**) &d, &arg_argv_ptr) == SUCCESS; zend_hash_move_forward_ex(arg_argv_hash, &arg_argv_ptr)) {
			if(Z_TYPE_PP(d)!=IS_STRING) {
				php_error(E_WARNING,"php_fuse_opt_parse_proc: modified argv key %d is not a string (type %d/%s), converting silently",i,Z_TYPE_PP(d),zend_get_type_by_const(Z_TYPE_PP(d)));
				convert_to_string_ex(d);
			}
//			php_printf("Element %d: '",i);
//			php_write(Z_STRVAL_PP(d),Z_STRLEN_PP(d));
//			php_printf("'\n");
			fuse_opt_add_arg(outargs,Z_STRVAL_PP(d)); //one by one, add the stuff back
			i++;
		}
	} else {
		php_error(E_ERROR,"php_fuse_opt_parse_proc: Userland returned failure");
	}
	
	//step 3: write back changes in userdata
	php_fuse_set_udata(data,arg_data);
	
	//step 4: clean up
//	php_printf("opt_parse_proc returned from userland. outargs.argc is %d, outargs.argv are:\n",outargs->argc);
//	for(i=0;i<outargs->argc;i++)
//		php_printf("'%s'\n",outargs->argv[i]);
//	php_printf("----\n");
	
	if(retval_ptr)
		zval_ptr_dtor(&retval_ptr);
	zval_ptr_dtor(&arg_data);
	zval_ptr_dtor(&arg_arg);
	zval_ptr_dtor(&arg_key);
	zval_ptr_dtor(&arg_argc);
	zval_ptr_dtor(&arg_argv);
	return ret;
}


static PHP_METHOD(Fuse, opt_parse) {
	zval *object = getThis();
	int i;
	
	//int $argc
	zval* z_ac;
	long ac;
	//string[] $argv
	zval* av;
	HashTable* av_hash;
	HashPosition av_ptr;
	int av_size;
	//mixed[] $data (unused for now!)
	zval* data;
	HashTable* data_hash;
	HashPosition data_ptr;
	int data_size;
	//mixed[] $opts
	zval* opts;
	HashTable* opts_hash;
	HashPosition opts_ptr;
	int opts_size;
	//callable $proc is stored in globals (FUSEG(proc_fci), FUSEG(proc_fcic))

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zaaaf", &z_ac, &av, &data, &opts, &FUSEG(proc_fci), &FUSEG(proc_fcic)) == FAILURE) {
		return;
	}
	
	//Sanity checks for argc/argv
	if(Z_TYPE_P(z_ac)!=IS_LONG)
		php_error(E_ERROR,"Fuse.args_init: argc is not an integer");
	ac=Z_LVAL_P(z_ac);
	av_hash=Z_ARRVAL_P(av);
	av_size=zend_hash_num_elements(av_hash);
	
	if(ac!=av_size) {
		php_error(E_ERROR,"Fuse.args_init: argc/sizeof(argv) mismatch: argc is %ld, sizeof(argv) is %d",ac,av_size);
		RETURN_FALSE;
	}
	if(ac==0) {
		php_error(E_NOTICE,"Fuse.args_init: argv is empty");
		RETURN_NULL();
	}
	
	
	//make a C array out of argv
	zval** d;
	struct fuse_args fargs= { 0, NULL, 0 };
	i=0;
	for(zend_hash_internal_pointer_reset_ex(av_hash, &av_ptr); zend_hash_get_current_data_ex(av_hash, (void**) &d, &av_ptr) == SUCCESS; zend_hash_move_forward_ex(av_hash, &av_ptr)) {
		convert_to_string_ex(d);
		fuse_opt_add_arg(&fargs, Z_STRVAL_PP(d));
		i++;
	}
	
	//now, make a fuse_args out of argc and the converted argv
//	php_printf("Fuse.opt_parse: going into fuse_opt_parse, fargs is now %d\n",fargs.argc);
//	for(i=0;i<fargs.argc;i++)
//		php_printf("'%s'\n",fargs.argv[i]);
	
	//now, convert $opts
	opts_hash=Z_ARRVAL_P(opts);
	opts_size=zend_hash_num_elements(opts_hash);
	
	struct fuse_opt* fopts=safe_emalloc(sizeof(struct fuse_opt),opts_size+1,0);
	i=0;
//	php_printf("walking through %d opts, assigned %li bytes of RAM for %li bytes wide struct\n",opts_size,(sizeof(struct fuse_opt)*(opts_size+1)),sizeof(struct fuse_opt));
	for(zend_hash_internal_pointer_reset_ex(opts_hash, &opts_ptr); zend_hash_get_current_data_ex(opts_hash, (void**) &d, &opts_ptr) == SUCCESS; zend_hash_move_forward_ex(opts_hash, &opts_ptr)) {
		char* key;
		uint key_len;
		ulong index;
		if(Z_TYPE_PP(d)!=IS_LONG)
			php_error(E_ERROR,"Value of element %d is not an integer",i);
		if(zend_hash_get_current_key_ex(opts_hash, &key, &key_len, &index, 0, &opts_ptr) != HASH_KEY_IS_STRING)
			php_error(E_ERROR,"Key of element %d is not a string",i);

		fopts[i].templ=estrndup(key,key_len);
		fopts[i].offset=-1U;
		fopts[i].value=Z_LVAL_PP(d);
		i++;
        }
	fopts[i].templ=NULL;
	fopts[i].offset=0;
	fopts[i].value=0;

	//now, get the udata "struct"
	void* udata=php_fuse_init_udata(data,&fopts,&opts_size);
	
	int ret=fuse_opt_parse(&fargs,udata,fopts,php_fuse_opt_parse_proc);

	if(ret==-1)
		php_error(E_ERROR,"Fuse.opt_parse: fuse_opt_parse returned error");

//	php_printf("Fuse.opt_parse: returned from fuse_opt_parse, fargs is now %d\n",fargs.argc);
	
	//copy over to zval $argv
	zend_hash_clean(av_hash);
	for(i=0;i<fargs.argc;i++) {
//		php_printf("'%s'\n",fargs.argv[i]);
		add_index_string(av,i,fargs.argv[i],1);
	}
	//copy over to zval $argc
	ZVAL_LONG(z_ac,fargs.argc);
	
	php_fuse_free_udata(udata);
	for(i=0;i<opts_size;i++) {
		if(fopts[i].templ) {
			efree((char*) fopts[i].templ); //manually discard const flag
		}
	}
	efree(fopts);
	fuse_opt_free_args(&fargs);
	return;
}

/* }}} */

/* {{{ fuse method entries */
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_fuse_main, 0, 0, 1)
	ZEND_ARG_INFO(0, path)				// string
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_opt_parse, 0, 0, 2)
	ZEND_ARG_INFO(1, argc)				// int
	ZEND_ARG_INFO(1, argv)				// [ref] array
	ZEND_ARG_INFO(1, data)				// [ref] array
	ZEND_ARG_INFO(0, opts)				// array
	ZEND_ARG_INFO(0, proc)				// callable
ZEND_END_ARG_INFO()

/*
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_getattr, 1, 0, 2)
	ZEND_ARG_INFO(0, path)				// string
	ZEND_ARG_INFO(1, list_retval)		// [ref] array
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_readlink, 1, 0, 2)
	ZEND_ARG_INFO(0, path)				// string
	ZEND_ARG_INFO(1, str_retval)		// [ref] string
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_getdir, 1, 0, 2)
	ZEND_ARG_INFO(0, path)				// string
	ZEND_ARG_INFO(1, list_retval)		// [ref] array
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_mknod, 0, 0, 2)
	ZEND_ARG_INFO(0, path)				// string
	ZEND_ARG_INFO(0, mode)				// int
	ZEND_ARG_INFO(0, dev)				// int
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_mkdir, 0, 0, 2)
	ZEND_ARG_INFO(0, path)				// string
	ZEND_ARG_INFO(0, mode)				// int
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_unlink, 0, 0, 1)
	ZEND_ARG_INFO(0, path)				// string
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_rmdir, 0, 0, 1)
	ZEND_ARG_INFO(0, path)				// string
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_symlink, 0, 0, 2)
	ZEND_ARG_INFO(0, path_from)			// string
	ZEND_ARG_INFO(0, path_to)			// string
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_rename, 0, 0, 2)
	ZEND_ARG_INFO(0, path_from)			// string
	ZEND_ARG_INFO(0, path_to)			// string
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_link, 0, 0, 2)
	ZEND_ARG_INFO(0, path_from)			// string
	ZEND_ARG_INFO(0, path_to)			// string
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_chmod, 0, 0, 2)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(0, mode)			// int
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_chown, 0, 0, 3)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(0, uid)			// int
	ZEND_ARG_INFO(0, gid)			// int
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_truncate, 0, 0, 2)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(0, len)			// int
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_utime, 0, 0, 2)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(0, buf)			// array
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_open, 0, 0, 2)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(1, mode)			// int
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_create, 0, 0, 2)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(1, mode)			// int
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_read, 1, 0, 5)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(0, handle)		// int
	ZEND_ARG_INFO(0, offset)		// int
	ZEND_ARG_INFO(0, buf_len)			// int
	ZEND_ARG_INFO(1, str_retval)	// string
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_write, 0, 0, 4)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(0, handle)		// int
	ZEND_ARG_INFO(0, offset)		// int
	ZEND_ARG_INFO(0, buf)			// string
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_statfs, 1, 0, 2)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(1, list_retval)	// array
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_flush, 0, 0, 2)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(0, handle)		// int
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_release, 0, 0, 3)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(0, handle)		// int
	ZEND_ARG_INFO(0, flag)			// int
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_fsync, 0, 0, 4)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(0, handle)		// int
	ZEND_ARG_INFO(0, mode)			// int
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_setxattr, 0, 0, 4)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(0, name)			// string
	ZEND_ARG_INFO(0, value)			// string
	ZEND_ARG_INFO(0, mode)			// int
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_getxattr, 1, 0, 3)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(0, name)			// string
	ZEND_ARG_INFO(1, str_retval)	// string
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_listxattr, 1, 0, 2)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(1, list_retval)	// array
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_removexattr, 0, 0, 2)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(0, name)			// string
ZEND_END_ARG_INFO()
*/

zend_function_entry php_fuse_methods[] = {
	ZEND_MALIAS(Fuse,	__construct,	fuse_constructor,	NULL,	ZEND_ACC_PUBLIC)
	PHP_ME(Fuse,	fuse_main,		arginfo_fuse_fuse_main,		ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Fuse,	opt_parse,		arginfo_fuse_opt_parse,		ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	/*
    PHP_ME(Fuse,	getattr,		arginfo_fuse_getattr,		ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	readlink,		arginfo_fuse_readlink,		ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	getdir,			arginfo_fuse_getdir,		ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	mknod,			arginfo_fuse_mknod,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	mkdir,			arginfo_fuse_mkdir,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	unlink,			arginfo_fuse_unlink,		ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	rmdir,			arginfo_fuse_rmdir,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	symlink,		arginfo_fuse_symlink,		ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	rename,			arginfo_fuse_rename,		ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	link,			arginfo_fuse_link,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	chmod,			arginfo_fuse_chmod,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	chown,			arginfo_fuse_chown,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	truncate,		arginfo_fuse_truncate,		ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	utime,			arginfo_fuse_utime,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	open,			arginfo_fuse_open,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	create,			arginfo_fuse_create,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	read,			arginfo_fuse_read,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	write,			arginfo_fuse_write,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	statfs,			arginfo_fuse_statfs,		ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	flush,			arginfo_fuse_flush,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	release,		arginfo_fuse_release,		ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	fsync,			arginfo_fuse_fsync,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	setxattr,		arginfo_fuse_setxattr,		ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	getxattr,		arginfo_fuse_getxattr,		ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	listxattr,		arginfo_fuse_listxattr,		ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	removexattr,	arginfo_fuse_removexattr,	ZEND_ACC_PUBLIC)
	*/

	{NULL, NULL, NULL}
};
/* }}} */

static php_fuse_globals_ctor(zend_fuse_globals *globals TSRMLS_DC) {
	pthread_mutex_init(&globals->m, NULL);
}

static php_fuse_globals_dtor(zend_fuse_globals *globals TSRMLS_DC) {
	pthread_mutex_destroy(&globals->m);
}

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(fuse) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "Fuse", php_fuse_methods); 
	php_fuse_ce = zend_register_internal_class(&ce TSRMLS_CC);
	php_fuse_ce->create_object = php_fuse_object_handler_new;

	memcpy(&php_fuse_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	memcpy(&php_fuse_object_handlers_ze1, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	/* constants */
	REGISTER_LONG_CONSTANT("FUSE_S_IFMT", S_IFMT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IFSOCK", S_IFSOCK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IFLNK", S_IFLNK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IFREG", S_IFREG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IFBLK", S_IFBLK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IFDIR", S_IFDIR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IFCHR", S_IFCHR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IFIFO", S_IFIFO, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_ISUID", S_ISUID, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_ISGID", S_ISGID, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_ISVTX", S_ISVTX, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IRWXU", S_IRWXU, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IRUSR", S_IRUSR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IWUSR", S_IWUSR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IXUSR", S_IXUSR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IRWXG", S_IRWXG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IRGRP", S_IRGRP, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IWGRP", S_IWGRP, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IXGRP", S_IXGRP, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IRWXO", S_IRWXO, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IROTH", S_IROTH, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IWOTH", S_IWOTH, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IXOTH", S_IXOTH, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("FUSE_EPERM", EPERM, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ENOENT", ENOENT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ESRCH", ESRCH, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EINTR", EINTR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EIO", EIO, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ENXIO", ENXIO, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_E2BIG", E2BIG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ENOEXEC", ENOEXEC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EBADF", EBADF, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ECHILD", ECHILD, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EAGAIN", EAGAIN, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ENOMEM", ENOMEM, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EACCES", EACCES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EFAULT", EFAULT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ENOTBLK", ENOTBLK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EBUSY", EBUSY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EEXIST", EEXIST, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EXDEV", EXDEV, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ENODEV", ENODEV, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ENOTDIR", ENOTDIR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EISDIR", EISDIR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EINVAL", EINVAL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ENFILE", ENFILE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EMFILE", EMFILE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ENOTTY", ENOTTY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ETXTBSY", ETXTBSY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EFBIG", EFBIG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ENOSPC", ENOSPC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ESPIPE", ESPIPE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EROFS", EROFS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EMLINK", EMLINK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EPIPE", EPIPE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EDOM", EDOM, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ERANGE", ERANGE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ENOSYS", ENOSYS, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("FUSE_DT_UKNOWN", DT_UNKNOWN, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_DT_REG", DT_REG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_DT_DIR", DT_DIR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_DT_FIFO", DT_FIFO, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_DT_SOCK", DT_SOCK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_DT_CHR", DT_CHR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_DT_BLK", DT_BLK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_DT_LNK", DT_LNK, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("FUSE_O_RDONLY", O_RDONLY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_O_WRONLY", O_WRONLY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_O_RDWR", O_RDWR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_O_CREAT", O_CREAT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_O_EXCL", O_EXCL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_O_NOCTTY", O_NOCTTY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_O_TRUNC", O_TRUNC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_O_APPEND", O_APPEND, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_O_NONBLOCK", O_NONBLOCK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_O_NDELAY", O_NDELAY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_O_SYNC", O_SYNC, CONST_CS | CONST_PERSISTENT);
#ifdef O_FSYNC
	REGISTER_LONG_CONSTANT("FUSE_O_FSYNC", O_FSYNC, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef O_DSYNC
	REGISTER_LONG_CONSTANT("FUSE_O_DSYNC", O_DSYNC, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef O_FASYNC
	REGISTER_LONG_CONSTANT("FUSE_O_FASYNC", O_FASYNC, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef O_DIRECT
	REGISTER_LONG_CONSTANT("FUSE_O_DIRECT", O_DIRECT, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef O_LARGEFILE
//On x64 systems, O_LARGEFILE is defined to 0, but still 0100000 gets passed as mode to open()!
	REGISTER_LONG_CONSTANT("FUSE_O_LARGEFILE", 0100000, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef O_DIRECTORY
	REGISTER_LONG_CONSTANT("FUSE_O_DIRECTORY", O_DIRECTORY, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef O_NOFOLLOW
	REGISTER_LONG_CONSTANT("FUSE_O_NOFOLLOW", O_NOFOLLOW, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef O_NOATIME
	REGISTER_LONG_CONSTANT("FUSE_O_NOATIME", O_NOATIME, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef O_CLOEXEC
	REGISTER_LONG_CONSTANT("FUSE_O_CLOEXEC", O_CLOEXEC, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef O_ACCMODE
	REGISTER_LONG_CONSTANT("FUSE_O_ACCMODE", O_ACCMODE, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef O_PATH
	REGISTER_LONG_CONSTANT("FUSE_O_PATH", O_PATH, CONST_CS | CONST_PERSISTENT);
#endif



	REGISTER_LONG_CONSTANT("FUSE_XATTR_CREATE", XATTR_CREATE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_XATTR_REPLACE", XATTR_REPLACE, CONST_CS | CONST_PERSISTENT);
	
	REGISTER_LONG_CONSTANT("FUSE_OPT_KEY_NONOPT", FUSE_OPT_KEY_NONOPT, CONST_CS | CONST_PERSISTENT);
	
	ZEND_INIT_MODULE_GLOBALS(fuse, php_fuse_globals_ctor, php_fuse_globals_dtor);
	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION(fuse) {
	FUSEG(active_object) = NULL;
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(fuse) {
	php_info_print_box_start(0);
	php_printf("<p>FUSE(File system in USEr space) Bindings for PHP</p>\n");
	php_printf("<p>Version 0.9.2 (2009-10-15)</p>\n");
	php_info_print_box_end();
}
/* }}} */

#endif /* HAVE_FUSE */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
