/*
**  Copyright 1998-2003 University of Illinois Board of Trustees
**  Copyright 1998-2003 Mark D. Roth
**  All rights reserved.
**
**  handle.c - libtar code for initializing a TAR handle
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

#include <sptk5/sptk.h>
#include "libtar.h"

#include <fcntl.h>

#ifdef _WIN32
#include <io.h>
#endif


static tartype_t default_type = { (openfunc_t)open, (closefunc_t)close, (readfunc_t)read, (writefunc_t)write };

static int tar_init(TAR **t, char *pathname, tartype_t *type,int oflags, int mode, int options)
{
	if ((oflags & (O_RDWR|O_RDONLY|O_WRONLY)) == O_RDWR)
	{
		errno = EINVAL;
		return -1;
	}

	*t = (TAR *)calloc(1, sizeof(TAR));
	if (*t == nullptr)
		return -1;

	(*t)->pathname = pathname;
	(*t)->options = options;
	(*t)->type = (type ? type : &default_type);
	(*t)->oflags = oflags;

	(*t)->h = libtar_hash_new(256, (libtar_hashfunc_t)path_hashfunc);
	if ((*t)->h == nullptr)
	{
		free(*t);
		return -1;
	}

	return 0;
}


/* open a new tarfile handle */
int
tar_open(TAR **t, char *pathname, tartype_t *type,
	 int oflags, int mode, int options)
{
	if (tar_init(t, pathname, type, oflags, mode, options) == -1)
		return -1;

	if ((options & TAR_NOOVERWRITE) && (oflags & O_CREAT))
		oflags |= O_EXCL;

#ifdef O_BINARY
	oflags |= O_BINARY;
#endif

	(*t)->fd = (*((*t)->type->openfunc))(pathname, oflags, mode);
	if ((*t)->fd == -1)
	{
		free(*t);
		return -1;
	}

	return 0;
}


/* close tarfile handle */
int
tar_close(TAR *t)
{
	int i;

	i = (*(t->type->closefunc))((int)t->fd);

	if (t->h != nullptr)
		libtar_hash_free(t->h, free);
	free(t);

	return i;
}


