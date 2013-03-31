/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * Slabs memory allocation, based on powers-of-N. Slabs are up to 1MB in size
 * and are divided into chunks. The chunk sizes start off at the size of the
 * "item" structure plus space for a small key and value. They increase by
 * a multiplier factor from there, up to half the maximum slab size. The last
 * slab size is always 1MB, since that's the maximum item size allowed by the
 * memcached protocol.
 */
#include <chaos/common/data/cache/memcached.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

/* powers-of-N allocation structures */

typedef struct {
    unsigned int size;      /* sizes of items */
    unsigned int perslab;   /* how many items per slab */

    void **slots;           /* list of item ptrs */
    unsigned int sl_total;  /* size of previous array */
    unsigned int sl_curr;   /* first free slot */

    void *end_page_ptr;         /* pointer to next free item at end of page, or 0 */
    unsigned int end_page_free; /* number of items remaining at end of last alloced page */

    unsigned int slabs;     /* how many slabs were allocated for this class */

    void **slab_list;       /* array of slab pointers */
    unsigned int list_size; /* size of prev array */

    unsigned int killing;  /* index+1 of dying slab, or zero if none */
    size_t requested; /* The number of requested bytes */
} slabclass_t;

static slabclass_t slabclass[MAX_NUMBER_OF_SLAB_CLASSES];
static size_t mem_limit = 0;
static size_t mem_malloced = 0;
static int power_largest;

static void *mem_base = NULL;
static void *mem_current = NULL;
static size_t mem_avail = 0;

/**
 * Access to the slab allocator is protected by this lock
 */
static pthread_mutex_t slabs_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * Forward Declarations
 */
static int do_slabs_newslab(const unsigned int id);
static void *memory_allocate(size_t size);

#ifndef DONT_PREALLOC_SLABS
/* Preallocate as many slab pages as possible (called from slabs_init)
   on start-up, so users don't get confused out-of-memory errors when
   they do have free (in-slab) space, but no space to make new slabs.
   if maxslabs is 18 (POWER_LARGEST - POWER_SMALLEST + 1), then all
   slab types can be made.  if max memory is less than 18 MB, only the
   smaller ones will be made.  */
static void slabs_preallocate (const unsigned int maxslabs);
#endif

/*
 * Figures out which slab class (chunk size) is required to store an item of
 * a given size.
 *
 * Given object size, return id to use when allocating/freeing memory for object
 * 0 means error: can't store such a large object
 */

unsigned int slabs_clsid(const size_t size) {
    int res = POWER_SMALLEST;

    if (size == 0)
        return 0;
    while (size > slabclass[res].size)
        if (res++ == power_largest)     /* won't fit in the biggest slab */
            return 0;
    return res;
}

/**
 * Determines the chunk sizes and initializes the slab class descriptors
 * accordingly.
 */
void slabs_init(const size_t limit, const double factor, const int prealloc) {
    int i = POWER_SMALLEST - 1;
    unsigned int size = sizeof(item) + settings.chunk_size;

    mem_limit = limit;

    if (prealloc) {
        /* Allocate everything in a big chunk with malloc */
        mem_base = malloc(mem_limit);
        if (mem_base != NULL) {
            mem_current = mem_base;
            mem_avail = mem_limit;
        } else {
            
        }
    }

    memset(slabclass, 0, sizeof(slabclass));

    while (++i < POWER_LARGEST && size <= settings.item_size_max / factor) {
        /* Make sure items are always n-byte aligned */
        if (size % CHUNK_ALIGN_BYTES)
            size += CHUNK_ALIGN_BYTES - (size % CHUNK_ALIGN_BYTES);

        slabclass[i].size = size;
        slabclass[i].perslab = settings.item_size_max / slabclass[i].size;
        size *= factor;
    }

    power_largest = i;
    slabclass[power_largest].size = settings.item_size_max;
    slabclass[power_largest].perslab = 1;
    /* for the test suite:  faking of how much we've already malloc'd */
    {
        char *t_initial_malloc = getenv("T_MEMD_INITIAL_MALLOC");
        if (t_initial_malloc) {
            mem_malloced = (size_t)atol(t_initial_malloc);
        }

    }

#ifndef DONT_PREALLOC_SLABS
    {
        char *pre_alloc = getenv("T_MEMD_SLABS_ALLOC");

        if (pre_alloc == NULL || atoi(pre_alloc) != 0) {
            slabs_preallocate(power_largest);
        }
    }
#endif
}

#ifndef DONT_PREALLOC_SLABS
static void slabs_preallocate (const unsigned int maxslabs) {
    int i;
    unsigned int prealloc = 0;

    /* pre-allocate a 1MB slab in every size class so people don't get
       confused by non-intuitive "SERVER_ERROR out of memory"
       messages.  this is the most common question on the mailing
       list.  if you really don't want this, you can rebuild without
       these three lines.  */

    for (i = POWER_SMALLEST; i <= POWER_LARGEST; i++) {
        if (++prealloc > maxslabs)
            return;
        do_slabs_newslab(i);
    }

}
#endif

static int grow_slab_list (const unsigned int id) {
    slabclass_t *p = &slabclass[id];
    if (p->slabs == p->list_size) {
        size_t new_size =  (p->list_size != 0) ? p->list_size * 2 : 16;
        void *new_list = realloc(p->slab_list, new_size * sizeof(void *));
        if (new_list == 0) return 0;
        p->list_size = new_size;
        p->slab_list = (void **)new_list;
    }
    return 1;
}

static int do_slabs_newslab(const unsigned int id) {
    slabclass_t *p = &slabclass[id];
    int len = p->size * p->perslab;
    char *ptr;

    if ((mem_limit && mem_malloced + len > mem_limit && p->slabs > 0) ||
        (grow_slab_list(id) == 0) ||
        ((ptr = (char*)memory_allocate((size_t)len)) == 0)) {

        return 0;
    }

    memset(ptr, 0, (size_t)len);
    p->end_page_ptr = ptr;
    p->end_page_free = p->perslab;

    p->slab_list[p->slabs++] = ptr;
    mem_malloced += len;

    return 1;
}

/*@null@*/
static void *do_slabs_alloc(const size_t size, unsigned int id) {
    slabclass_t *p;
    void *ret = NULL;

    if (id < POWER_SMALLEST || id > power_largest) {
        return NULL;
    }

    p = &slabclass[id];
    assert(p->sl_curr == 0 || ((item *)p->slots[p->sl_curr - 1])->slabs_clsid == 0);

#ifdef USE_SYSTEM_MALLOC
    if (mem_limit && mem_malloced + size > mem_limit) {
        return 0;
    }
    mem_malloced += size;
    ret = malloc(size);
    return ret;
#endif

    /* fail unless we have space at the end of a recently allocated page,
       we have something on our freelist, or we could allocate a new page */
    if (! (p->end_page_ptr != 0 || p->sl_curr != 0 ||
           do_slabs_newslab(id) != 0)) {
        /* We don't have more memory available */
        ret = NULL;
    } else if (p->sl_curr != 0) {
        /* return off our freelist */
        ret = p->slots[--p->sl_curr];
    } else {
        /* if we recently allocated a whole page, return from that */
        assert(p->end_page_ptr != NULL);
        ret = p->end_page_ptr;
        if (--p->end_page_free != 0) {
            p->end_page_ptr = ((caddr_t)p->end_page_ptr) + p->size;
        } else {
            p->end_page_ptr = 0;
        }
    }

    if (ret) {
        p->requested += size;
    }

    return ret;
}

static void do_slabs_free(void *ptr, const size_t size, unsigned int id) {
    slabclass_t *p;

    assert(((item *)ptr)->slabs_clsid == 0);
    assert(id >= POWER_SMALLEST && id <= power_largest);
    if (id < POWER_SMALLEST || id > power_largest)
        return;

    p = &slabclass[id];

#ifdef USE_SYSTEM_MALLOC
    mem_malloced -= size;
    free(ptr);
    return;
#endif

    if (p->sl_curr == p->sl_total) { /* need more space on the free list */
        int new_size = (p->sl_total != 0) ? p->sl_total * 2 : 16;  /* 16 is arbitrary */
        void **new_slots = (void**)realloc(p->slots, new_size * sizeof(void *));
        if (new_slots == 0)
            return;
        p->slots = new_slots;
        p->sl_total = new_size;
    }
    p->slots[p->sl_curr++] = ptr;
    p->requested -= size;
    return;
}

static int nz_strcmp(int nzlength, const char *nz, const char *z) {
    size_t zlength=strlen(z);
    return (zlength == nzlength) && (strncmp(nz, z, zlength) == 0) ? 0 : -1;
}

static void *memory_allocate(size_t size) {
    void *ret;

    if (mem_base == NULL) {
        /* We are not using a preallocated large memory chunk */
        ret = malloc(size);
    } else {
        ret = mem_current;

        if (size > mem_avail) {
            return NULL;
        }

        /* mem_current pointer _must_ be aligned!!! */
        if (size % CHUNK_ALIGN_BYTES) {
            size += CHUNK_ALIGN_BYTES - (size % CHUNK_ALIGN_BYTES);
        }

        mem_current = ((char*)mem_current) + size;
        if (size < mem_avail) {
            mem_avail -= size;
        } else {
            mem_avail = 0;
        }
    }

    return ret;
}

void *slabs_alloc(size_t size, unsigned int id) {
    void *ret;

    pthread_mutex_lock(&slabs_lock);
    ret = do_slabs_alloc(size, id);
    pthread_mutex_unlock(&slabs_lock);
    return ret;
}

void slabs_free(void *ptr, size_t size, unsigned int id) {
    pthread_mutex_lock(&slabs_lock);
    do_slabs_free(ptr, size, id);
    pthread_mutex_unlock(&slabs_lock);
}

void slabs_adjust_mem_requested(unsigned int id, size_t old, size_t ntotal)
{
    pthread_mutex_lock(&slabs_lock);
    slabclass_t *p;
    if (id < POWER_SMALLEST || id > power_largest) {
        abort();
    }

    p = &slabclass[id];
    p->requested = p->requested - old + ntotal;
    pthread_mutex_unlock(&slabs_lock);
}
