#include "circular_buffer.h"
#include <string.h>
#include <monapi/syscall.h>

int cb_index_inc(int x, int limit)
{
    if( (x+1) == limit ) return 0; else return x+1;
}

int cb_index_dec(int x, int limit)
{
    if( (x-1) < 0 ) return limit-1; else return x-1;
}

CB *cb_alloc()
{
    CB *cb;
    int result;
    cb = (CB*)calloc(1, sizeof(CB));
    return cb;
}

CB *cb_init(CB *cb, int blocksize, int maxblocks)
{
    if( cb == NULL ) return NULL;
    if( blocksize == 0 ) blocksize = CB_DEFAULT_BLOCKSIZE;
    if( maxblocks == 0 ) maxblocks = CB_DEFAULT_MAX_BLOCKS;
    cb->blocksize = blocksize;
    cb->maxblocks = maxblocks;
    cb->fi = -1;
    cb->ei = 0;
    cb->p = (char*)malloc(maxblocks*blocksize);
    if( cb->p == NULL ) return NULL;
    cb->mutex = syscall_mutex_create(0);
    cb->semaphore = syscall_semaphore_create(maxblocks, 0);
    return cb;
}

int cb_free(CB *cb)
{
    syscall_mutex_destroy(cb->mutex);
    syscall_semaphore_destroy(cb->semaphore);
    free(cb->p);
    free(cb);
    return 0;
}

#define CB_FIRST(a, b) { b; return a; }
int cb_write(CB *cb, void *p, int flag)
{
    int index;
    static int semIndex = 0;
BEGIN:
    if( cb == NULL ) return -1;
//    logprintf("%s %s:%d\n", __func__, __FILE__, __LINE__);
    syscall_semaphore_down(cb->semaphore);
//    logprintf("%s %s:%d\n", __func__, __FILE__, __LINE__);
    syscall_mutex_lock(cb->mutex);
//    logprintf("%s %s:%d\n", __func__, __FILE__, __LINE__);
    if( cb->ei == -1 )
    {
        CB_FIRST(0, syscall_mutex_unlock(cb->mutex));
    }
    index = cb->blocksize*cb->ei;
    memcpy(cb->p+index, p, cb->blocksize);
    if( cb->fi == -1 ) cb->fi = cb->ei;
    cb->ei = cb_index_inc(cb->ei, cb->maxblocks);
    if( cb->fi == cb->ei ) cb->ei = -1;
    syscall_mutex_unlock(cb->mutex);
    return 1;
}

int cb_is_empty(CB* cb)
{
    return cb->fi == cb->ei;
}

int cb_read(CB *cb, void *p)
{
    int index;
    if( cb == NULL ) return -1;
BEGIN:
    if( cb->fi == -1 )
    {
//        logprintf("%s %s:%d\n", __func__, __FILE__, __LINE__);
        goto BEGIN;
    }
//    logprintf("%s %s:%d\n", __func__, __FILE__, __LINE__);
    syscall_mutex_lock(cb->mutex);
//    logprintf("%s %s:%d\n", __func__, __FILE__, __LINE__);
    index = cb->blocksize*cb->fi;
    memcpy(p, cb->p+index, cb->blocksize);
    if( cb->ei == -1 ) cb->ei = cb->fi;
    cb->fi = cb_index_inc(cb->fi, cb->maxblocks);
    if( cb->ei == cb->fi ) cb->fi = -1;
    syscall_mutex_unlock(cb->mutex);
    syscall_semaphore_up(cb->semaphore);
    return 1;
}