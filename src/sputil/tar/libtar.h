/*
**  Copyright 1998-2003 University of Illinois Board of Trustees
**  Copyright 1998-2003 Mark D. Roth
**  All rights reserved.
**
**  libtar.h - header file for libtar library
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

#ifndef LIBTAR_H
#define LIBTAR_H

#include <sys/types.h>
#include <sys/stat.h>
#include "tar.h"
#include <cstddef>
#include <string>

#include "libtar_listhash.h"

#ifdef __cplusplus
extern "C"
{
#endif


/* useful constants */
#define T_BLOCKSIZE		512
#define T_NAMELEN		100
#define T_PREFIXLEN		155
#define T_MAXPATHLEN		(T_NAMELEN + T_PREFIXLEN)

/* GNU extensions for typeflag */
#define GNU_LONGNAME_TYPE	'L'
#define GNU_LONGLINK_TYPE	'K'

/* our version of the tar header structure */
struct tar_header
{
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char chksum[8];
	char typeflag;
	char linkname[100];
	char magic[6];
	char version[2];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char prefix[155];
	char padding[12];
	char *gnu_longname;
	char *gnu_longlink;
};


/***** handle.c ************************************************************/

typedef int (*openfunc_t)(const char *, int, ...);
typedef int (*closefunc_t)(int);
typedef int (*readfunc_t)(int, void *, size_t);
typedef int (*writefunc_t)(int, const void *, size_t);

typedef struct
{
	openfunc_t openfunc;
	closefunc_t closefunc;
	readfunc_t readfunc;
	writefunc_t writefunc;
}
tartype_t;

typedef struct
{
	tartype_t *type;
	char *pathname;
	long fd;
	int oflags;
	int options;
	struct tar_header th_buf;
	libtar_hash_t *h;
}
TAR;

/* constant values for the TAR options field */
#define TAR_GNU			 1	/* use GNU extensions */
#define TAR_VERBOSE		 2	/* output file info to stdout */
#define TAR_NOOVERWRITE		 4	/* don't overwrite existing files */
#define TAR_IGNORE_EOT		 8	/* ignore double zero blocks as EOF */
#define TAR_CHECK_MAGIC		16	/* check magic in file header */
#define TAR_CHECK_VERSION	32	/* check version in file header */
#define TAR_IGNORE_CRC		64	/* ignore CRC in file header */

/* this is obsolete - it's here for backwards-compatibility only */
#define TAR_IGNORE_MAGIC	0

/* open a new tarfile handle */
int tar_open(TAR **t, char *pathname, tartype_t *type,
	     int oflags, int mode, int options);

/* close tarfile handle */
int tar_close(TAR *t);

/* macros for reading/writing tarchive blocks */
#define tar_block_read(t, buf) \
	(*((t)->type->readfunc))((int)(t)->fd, (char *)(buf), T_BLOCKSIZE)

/* read/write a header block */
int th_read(TAR *t);


/***** decode.c ************************************************************/

/* determine file type */
#define TH_ISREG(t)	((t)->th_buf.typeflag == REGTYPE \
			 || (t)->th_buf.typeflag == AREGTYPE \
			 || (t)->th_buf.typeflag == CONTTYPE \
			 || (S_ISREG((mode_t)oct_to_int((t)->th_buf.mode)) \
			     && (t)->th_buf.typeflag != LNKTYPE))
#define TH_ISLNK(t)	((t)->th_buf.typeflag == LNKTYPE)
#define TH_ISSYM(t)	((t)->th_buf.typeflag == SYMTYPE \
			 || S_ISLNK((mode_t)oct_to_int((t)->th_buf.mode)))
#define TH_ISCHR(t)	((t)->th_buf.typeflag == CHRTYPE \
			 || S_ISCHR((mode_t)oct_to_int((t)->th_buf.mode)))
#define TH_ISBLK(t)	((t)->th_buf.typeflag == BLKTYPE \
			 || S_ISBLK((mode_t)oct_to_int((t)->th_buf.mode)))
#define TH_ISDIR(t)	((t)->th_buf.typeflag == DIRTYPE \
			 || S_ISDIR((mode_t)oct_to_int((t)->th_buf.mode)) \
			 || ((t)->th_buf.typeflag == AREGTYPE \
			     && ((t)->th_buf.name[strlen((t)->th_buf.name) - 1] == '/')))
#define TH_ISFIFO(t)	((t)->th_buf.typeflag == FIFOTYPE \
			 || S_ISFIFO((mode_t)oct_to_int((t)->th_buf.mode)))
#define TH_ISLONGNAME(t)	((t)->th_buf.typeflag == GNU_LONGNAME_TYPE)
#define TH_ISLONGLINK(t)	((t)->th_buf.typeflag == GNU_LONGLINK_TYPE)

/* decode tar header info */
#define th_get_crc(t) oct_to_int((t)->th_buf.chksum)
#define th_get_size(t) oct_to_int((t)->th_buf.size)
void th_get_pathname(TAR *t, char* path, size_t sz);


/***** util.c *************************************************************/

/* hashing function for pathnames */
int path_hashfunc(char *key, int numbuckets);

/* calculate header checksum */
int th_crc_calc(TAR *t);
#define th_crc_ok(t) (th_get_crc(t) == th_crc_calc(t))

/* string-octal to integer conversion */
int oct_to_int(char *oct);


#ifdef __cplusplus
}
#endif

#endif /* ! LIBTAR_H */

