/*
**  Copyright 1998-2003 University of Illinois Board of Trustees
**  Copyright 1998-2003 Mark D. Roth
**  All rights reserved.
**
**  block.c - libtar code to handle tar archive header blocks
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

#include "libtar.h"
#include <cerrno> // BSD
#include <sptk5/String.h>
#include <sptk5/Exception.h>

using namespace std;
using namespace sptk;

#define BIT_ISSET(bitmask, bit) ((bitmask) & (bit))


/* read a header block */
int th_read_internal(TAR *t)
{
    int i;
    int num_zero_blocks = 0;

    while ((i = tar_block_read(t, &(t->th_buf))) == T_BLOCKSIZE)
    {
        /* two all-zero blocks mark EOF */
        if (t->th_buf.name[0] == '\0')
        {
            ++num_zero_blocks;
            if (!BIT_ISSET(t->options, TAR_IGNORE_EOT)
                && num_zero_blocks >= 2)
                return 0;    /* EOF */
            continue;
        }

        /* verify magic and version */
        if (BIT_ISSET(t->options, TAR_CHECK_MAGIC)
            && strncmp(t->th_buf.magic, TMAGIC, TMAGLEN - 1) != 0)
        {
            throw Exception("Unknown magic value in tar header");
        }

        if (BIT_ISSET(t->options, TAR_CHECK_VERSION)
            && strncmp(t->th_buf.version, TVERSION, TVERSLEN) != 0)
        {
            throw Exception("Unknown version value in tar header");
        }

        /* check chksum */
        if (!BIT_ISSET(t->options, TAR_IGNORE_CRC)
            && !th_crc_ok(t))
        {
            throw Exception("Tar header checksum error");
        }

        break;
    }

    return i;
}

void clearTarHeader(struct tar_header* th)
{
    delete [] th->gnu_longname;
    delete [] th->gnu_longlink;
    memset(th, 0, sizeof(struct tar_header));
}

/* wrapper function for th_read_internal() to handle GNU extensions */
int th_read(TAR *t)
{
    int i;
    int j;
    size_t sz;
    char *ptr;

#ifdef LIBTAR_DEBUG
    printf("==> th_read(t=0x%lx)\n", t);
#endif

    clearTarHeader(&(t->th_buf));

    i = th_read_internal(t);
    if (i == 0)
        return 1;
    if (i != T_BLOCKSIZE)
    {
        if (i != -1)
            errno = EINVAL;
        return -1;
    }

    /* check for GNU long link extention */
    if TH_ISLONGLINK(t)
    {
        sz = (size_t) th_get_size(t);
        j = (int) ( (sz / T_BLOCKSIZE) + ((sz % T_BLOCKSIZE) ? 1 : 0) );
        t->th_buf.gnu_longlink = new char[size_t(j) * T_BLOCKSIZE];
        if (t->th_buf.gnu_longlink == nullptr)
            throw Exception("Can't allocate " + to_string(size_t(j) * T_BLOCKSIZE) + " bytes");

        for (ptr = t->th_buf.gnu_longlink; j > 0 && ptr != nullptr; --j, ptr += T_BLOCKSIZE)
        {
            i = tar_block_read(t, ptr);
            if (i != T_BLOCKSIZE)
            {
                throw Exception("Can't read block from tar");
            }
        }

        i = th_read_internal(t);
        if (i != T_BLOCKSIZE)
        {
            throw Exception("Can't read from tar");
        }
    }

    /* check for GNU long name extention */
    if TH_ISLONGNAME(t)
    {
        sz = (size_t) th_get_size(t);
        j = (int) ( (sz / T_BLOCKSIZE) + ((sz % T_BLOCKSIZE) ? 1 : 0) );

        t->th_buf.gnu_longname = new char[size_t(j) * T_BLOCKSIZE];
        if (t->th_buf.gnu_longname == nullptr)
            throw Exception("Can't allocate " + to_string(size_t(j) * T_BLOCKSIZE) + " bytes");

        for (ptr = t->th_buf.gnu_longname; j > 0 && ptr != nullptr; --j, ptr += T_BLOCKSIZE)
        {
            i = tar_block_read(t, ptr);
            if (i != T_BLOCKSIZE)
            {
                throw Exception("Can't read block from tar");
            }
        }

        i = th_read_internal(t);
        if (i != T_BLOCKSIZE)
        {
            throw Exception("Can't read from tar");
        }
    }

    return 0;
}


