/* listhash/libtar_list.c.  Generated from list.c.in by configure. */

/*
**  Copyright 1998-2002 University of Illinois Board of Trustees
**  Copyright 1998-2002 Mark D. Roth
**  All rights reserved.
**
**  libtar_list.c - linked list routines
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

#include "libtar_listhash.h"
#include <cstdlib>
#include <cstring>
#include <sptk5/sptk.h>

/*
** libtar_listptr_reset() - reset a list pointer
*/
void libtar_listptr_reset(libtar_listptr_t* lp)
{
    *lp = nullptr;
}


/*
** libtar_listptr_data() - retrieve the data pointed to by lp
*/
void* libtar_listptr_data(libtar_listptr_t* lp)
{
    return (*lp)->data;
}


/*
** libtar_list_new() - create a new, empty list
*/
libtar_list_t* libtar_list_new(int flags, libtar_cmpfunc_t cmpfunc)
{
    libtar_list_t* newlist;

#ifdef LIBTAR_DEBUG2
    printf("in libtar_list_new(%d, 0x%lx)\n", flags, cmpfunc);
#endif

    if (flags != LIST_USERFUNC
        && flags != LIST_STACK
        && flags != LIST_QUEUE) {
        errno = EINVAL;
        return nullptr;
    }

    newlist = new libtar_list_t;
    if (newlist == nullptr)
        return nullptr;
    memset(newlist, 0, sizeof(libtar_list_t));

    if (cmpfunc != nullptr)
        newlist->cmpfunc = cmpfunc;
    else
        newlist->cmpfunc = (libtar_cmpfunc_t) strcmp;
    newlist->flags = flags;

    return newlist;
}


/*
** libtar_list_iterate() - call a function for every element
**                      in a list
*/
int libtar_list_iterate(libtar_list_t* l, libtar_iterate_func_t plugin, void* state)
{
    libtar_listptr_t n;

    if (l == nullptr)
        return -1;

    for (n = l->first; n != nullptr; n = n->next) {
        if ((*plugin)(n->data, state) == -1)
            return -1;
    }

    return 0;
}


/*
** libtar_list_empty() - empty the list
*/
void libtar_list_empty(libtar_list_t* l, libtar_freefunc_t freefunc)
{
    libtar_listptr_t n;

    for (n = l->first; n != nullptr; n = l->first) {
        l->first = n->next;
        if (freefunc != nullptr)
            (*freefunc)(n->data);
        free(n);
    }

    l->nents = 0;
}


/*
** libtar_list_free() - remove and free() the whole list
*/
void libtar_list_free(libtar_list_t* l, libtar_freefunc_t freefunc)
{
    libtar_list_empty(l, freefunc);
    delete l;
}


/*
** libtar_list_nents() - return number of elements in the list
*/
unsigned int libtar_list_nents(libtar_list_t* l)
{
    return l->nents;
}


/*
** libtar_list_add() - adds an element to the list
** returns:
**    0            success
**    -1 (and sets errno)    failure
*/
int libtar_list_add(libtar_list_t* l, void* data)
{
    libtar_listptr_t n;
    libtar_listptr_t m;

#ifdef LIBTAR_DEBUG2
    printf("==> libtar_list_add(\"%s\")\n", (char *)data);
#endif

    n = (libtar_listptr_t) malloc(sizeof(struct libtar_node));
    if (n == nullptr)
        return -1;
    n->data = data;
    l->nents++;

#ifdef LIBTAR_DEBUG2
    printf("    libtar_list_add(): allocated data\n");
#endif

    /* if the list is empty */
    if (l->first == nullptr) {
        l->last = l->first = n;
        n->next = n->prev = nullptr;
#ifdef LIBTAR_DEBUG2
        printf("<== libtar_list_add(): list was empty; "
               "added first element and returning 0\n");
#endif
        return 0;
    }

#ifdef LIBTAR_DEBUG2
    printf("    libtar_list_add(): list not empty\n");
#endif

    if (l->flags == LIST_STACK) {
        n->prev = nullptr;
        n->next = l->first;
        if (l->first != nullptr)
            l->first->prev = n;
        l->first = n;
#ifdef LIBTAR_DEBUG2
        printf("<== libtar_list_add(): LIST_STACK set; "
               "added in front\n");
#endif
        return 0;
    }

    if (l->flags == LIST_QUEUE) {
        n->prev = l->last;
        n->next = nullptr;
        if (l->last != nullptr)
            l->last->next = n;
        l->last = n;
#ifdef LIBTAR_DEBUG2
        printf("<== libtar_list_add(): LIST_QUEUE set; "
               "added at end\n");
#endif
        return 0;
    }

    for (m = l->first; m != nullptr; m = m->next)
        if ((*(l->cmpfunc))(data, m->data) < 0) {
            /*
            ** if we find one that's bigger,
            ** insert data before it
            */
#ifdef LIBTAR_DEBUG2
            printf("    libtar_list_add(): gotcha..."
                   "inserting data\n");
#endif
            if (m == l->first) {
                l->first = n;
                n->prev = nullptr;
                m->prev = n;
                n->next = m;
#ifdef LIBTAR_DEBUG2
                printf("<== libtar_list_add(): "
                       "added first, returning 0\n");
#endif
                return 0;
            }
            m->prev->next = n;
            n->prev = m->prev;
            m->prev = n;
            n->next = m;
#ifdef LIBTAR_DEBUG2
            printf("<== libtar_list_add(): added middle,"
                   " returning 0\n");
#endif
            return 0;
        }

#ifdef LIBTAR_DEBUG2
    printf("    libtar_list_add(): new data larger than current "
           "list elements\n");
#endif

    /* if we get here, data is bigger than everything in the list */
    l->last->next = n;
    n->prev = l->last;
    l->last = n;
    n->next = nullptr;
#ifdef LIBTAR_DEBUG2
    printf("<== libtar_list_add(): added end, returning 0\n");
#endif
    return 0;
}


/*
** libtar_list_del() - remove the element pointed to by n
**                  from the list l
*/
void libtar_list_del(libtar_list_t* l, libtar_listptr_t* n)
{
    libtar_listptr_t m;

#ifdef LIBTAR_DEBUG2
    printf("==> libtar_list_del()\n");
#endif

    l->nents--;

    m = (*n)->next;

    if ((*n)->prev != nullptr)
        (*n)->prev->next = (*n)->next;
    else
        l->first = (*n)->next;
    if ((*n)->next != nullptr)
        (*n)->next->prev = (*n)->prev;
    else
        l->last = (*n)->prev;

    free(*n);
    *n = m;
}


/*
** libtar_list_next() - get the next element in the list
** returns:
**    1            success
**    0            end of list
*/
int libtar_list_next(libtar_list_t* l,
                 libtar_listptr_t* n)
{
    if (*n == nullptr)
        *n = l->first;
    else
        *n = (*n)->next;

    return (*n != nullptr ? 1 : 0);
}


/*
** libtar_list_prev() - get the previous element in the list
** returns:
**    1            success
**    0            end of list
*/
int libtar_list_prev(libtar_list_t* l,
                 libtar_listptr_t* n)
{
    if (*n == nullptr)
        *n = l->last;
    else
        *n = (*n)->prev;

    return (*n != nullptr ? 1 : 0);
}


/*
** libtar_str_match() - string matching function
** returns:
**    1            match
**    0            no match
*/
int libtar_str_match(char* check, char* data)
{
    return !strcmp(check, data);
}


char* libtar_strsep(char** stringp, const char* delim);

/*
** libtar_list_add_str() - splits string str into delim-delimited
**                      elements and adds them to list l
** returns:
**    0            success
**    -1 (and sets errno)    failure
*/
int libtar_list_add_str(libtar_list_t* l, char* str, char* delim)
{
    char tmp[10241];
    char* tokp;
    char* nextp = tmp;

    strncpy(tmp, str, sizeof(tmp));
    tmp[10240] = '\0';
    while ((tokp = libtar_strsep(&nextp, delim)) != nullptr) {
        if (*tokp == '\0')
            continue;
        if (libtar_list_add(l, strdup(tokp)))
            return -1;
    }

    return 0;
}


/*
** libtar_list_search() - find an entry in a list
** returns:
**    1            match found
**    0            no match
*/
int
libtar_list_search(libtar_list_t* l,
                   libtar_listptr_t* n, void* data,
                   libtar_matchfunc_t matchfunc)
{
#ifdef LIBTAR_DEBUG2
    printf("==> libtar_list_search(l=0x%lx, n=0x%lx, \"%s\")\n",
           l, n, (char *)data);
#endif

    if (matchfunc == nullptr)
        matchfunc = (libtar_matchfunc_t) libtar_str_match;

    if (*n == nullptr)
        *n = l->first;
    else
        *n = (*n)->next;

    for (; *n != nullptr; *n = (*n)->next) {
#ifdef LIBTAR_DEBUG2
        printf("checking against \"%s\"\n", (char *)(*n)->data);
#endif
        if ((*(matchfunc))(data, (*n)->data) != 0)
            return 1;
    }

#ifdef LIBTAR_DEBUG2
    printf("no matches found\n");
#endif
    return 0;
}


/*
** libtar_list_dup() - copy an existing list
*/
libtar_list_t*
libtar_list_dup(libtar_list_t* l)
{
    libtar_list_t* newlist;
    libtar_listptr_t n;

    newlist = libtar_list_new(l->flags, l->cmpfunc);
    for (n = l->first; n != nullptr; n = n->next)
        libtar_list_add(newlist, n->data);

#ifdef LIBTAR_DEBUG2
    printf("returning from libtar_list_dup()\n");
#endif
    return newlist;
}


/*
** libtar_list_merge() - merge two lists into a new list
*/
libtar_list_t*
libtar_list_merge(libtar_cmpfunc_t cmpfunc, int flags,
                  libtar_list_t* list1,
                  libtar_list_t* list2)
{
    libtar_list_t* newlist;
    libtar_listptr_t n;

    newlist = libtar_list_new(flags, cmpfunc);

    n = nullptr;
    while (libtar_list_next(list1, &n) != 0)
        libtar_list_add(newlist, n->data);
    n = nullptr;
    while (libtar_list_next(list2, &n) != 0)
        libtar_list_add(newlist, n->data);

    return newlist;
}


