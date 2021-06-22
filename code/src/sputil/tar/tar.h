/* Extended tar format from POSIX.1.
   Copyright (C) 1992, 1996 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by David J. MacKenzie.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef    _TAR_H
#define    _TAR_H    1

/* A tar archive consists of 512-byte blocks.
   Each file in the archive has a header block followed by 0+ data blocks.
   Two blocks of NUL bytes indicate the end of the archive.  */

/* The fields of header blocks:
   All strings are stored as ISO 646 (approximately ASCII) strings.

   Fields are numeric unless otherwise noted below; numbers are ISO 646
   representations of octal numbers, with leading zeros as needed.

   linkname is only valid when typeflag==LNKTYPE.  It doesn't use prefix
   files that are links to pathnames >100 chars long can not be stored
   in a tar archive.

   If typeflag=={LNKTYPE,SYMTYPE,DIRTYPE} then size must be 0.

   devmajor and devminor are only valid for typeflag=={BLKTYPE,CHRTYPE}.

   chksum contains the sum of all 512 bytes in the header block,
   treating each byte as an 8-bit unsigned value and treating the
   8 bytes of chksum as blank characters.

   uname and gname are used in preference to uid and gid, if those
   names exist locally.

   Field Name	Byte Offset	Length in Bytes	Field Type
   name		0		100		NUL-terminated if NUL fits
   mode		100		8
   uid		108		8
   gid		116		8
   size		124		12
   mtime	136		12
   chksum	148		8
   typeflag	156		1		see below
   linkname	157		100		NUL-terminated if NUL fits
   magic	257		6		must be TMAGIC (NUL term.)
   version	263		2		must be TVERSION
   uname	265		32		NUL-terminated
   gname	297		32		NUL-terminated
   devmajor	329		8
   devminor	337		8
   prefix	345		155		NUL-terminated if NUL fits

   If the first character of prefix is '\0', the file name is name,
   otherwise, it is prefix/name.  Files whose pathnames don't fit in that
   length can not be stored in a tar archive.  */

/* The bits in mode: */
constexpr int TSUID = 04000;
constexpr int TSGID = 02000;
constexpr int TSVTX = 01000;
constexpr int TUREAD = 00400;
constexpr int TUWRITE = 00200;
constexpr int TUEXEC = 00100;
constexpr int TGREAD = 00040;
constexpr int TGWRITE = 00020;
constexpr int TGEXEC = 00010;
constexpr int TOREAD = 00004;
constexpr int TOWRITE = 00002;
constexpr int TOEXEC = 00001;

/* The values for typeflag:
   Values 'A'-'Z' are reserved for custom implementations.
   All other values are reserved for future POSIX.1 revisions.  */

constexpr char REGTYPE = '0';    /* Regular file (preferred code).  */
constexpr char AREGTYPE = '\0';    /* Regular file (alternate code).  */
constexpr char LNKTYPE = '1';    /* Hard link.  */
constexpr char SYMTYPE = '2';    /* Symbolic link (hard if not supported).  */
constexpr char CHRTYPE = '3';    /* Character special.  */
constexpr char BLKTYPE = '4';    /* Block special.  */
constexpr char DIRTYPE = '5';    /* Directory.  */
constexpr char FIFOTYPE = '6';    /* Named pipe.  */
constexpr char CONTTYPE = '7';    /* Contiguous file */
/* (regular file if not supported).  */

/* Contents of magic field and its length.  */
constexpr const char* TMAGIC = "ustar";
constexpr int TMAGLEN = 6;

/* Contents of the version field and its length.  */
constexpr const char* TVERSION = "00";
constexpr int TVERSLEN = 2;

#endif /* tar.h */
