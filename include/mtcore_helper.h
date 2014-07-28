#ifndef MTCORE_HELPER_H_
#define MTCORE_HELPER_H_

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "mtcore.h"
#include "hash_table.h"

/* #define MTCORE_H_DEBUG */
#ifdef MTCORE_H_DEBUG
#define MTCORE_H_DBG_PRINT(str, ...) do { \
    fprintf(stdout, "[MTCORE-H][N-%d]"str, MTCORE_MY_NODE_ID, ## __VA_ARGS__); fflush(stdout); \
    } while (0)
#else
#define MTCORE_H_DBG_PRINT(str, ...) {}
#endif

#define MTCORE_H_ERR_PRINT(str...) do {fprintf(stderr, str);fflush(stdout);} while (0)

#define MTCORE_H_assert(EXPR) do { if (unlikely(!(EXPR))){ \
            MTCORE_H_ERR_PRINT("[MTCORE-H][N-%d, %d]  assert fail in [%s:%d]: \"%s\"\n", \
                    MTCORE_MY_NODE_ID, MTCORE_MY_RANK_IN_WORLD, __FILE__, __LINE__, #EXPR); \
            PMPI_Abort(MPI_COMM_WORLD, -1); \
        }} while (0)

typedef struct MTCORE_H_win {
    MPI_Aint *user_base_addrs_in_local;

    /* communicator including local processes and helpers */
    MPI_Comm local_uh_comm;
    MPI_Win local_uh_win;
    int max_local_user_nprocs;

    /* communicator including all the user processes and helpers */
    MPI_Comm uh_comm;

    void *base;
    MPI_Win *uh_wins;
    unsigned long mtcore_h_win_handle;
} MTCORE_H_win;

extern hashtable_t *mtcore_h_win_ht;
#define MTCORE_H_WIN_HT_SIZE 256

static inline int mtcore_get_h_win(unsigned long handle, MTCORE_H_win ** win)
{
    *win = (MTCORE_H_win *) ht_get(mtcore_h_win_ht, (ht_key_t) handle);
    return 0;
}

static inline int mtcore_put_h_win(unsigned long key, MTCORE_H_win * win)
{
    return ht_set(mtcore_h_win_ht, (ht_key_t) key, win);
}

static inline int mtcore_init_h_win_table()
{
    mtcore_h_win_ht = ht_create(MTCORE_H_WIN_HT_SIZE);

    if (mtcore_h_win_ht == NULL)
        return -1;

    return 0;
}

static inline void destroy_mtcore_h_win_table()
{
    return ht_destroy(mtcore_h_win_ht);
}

/**
 * Helper receives a new function from user root process
 */
static inline int MTCORE_H_func_start(MTCORE_Func * FUNC, int *local_root, int *user_nprocs,
                                 int *user_local_nprocs, int *uh_tag)
{
    int mpi_errno = MPI_SUCCESS;
    MPI_Status status;
    MTCORE_Func_info info;

    mpi_errno = PMPI_Recv((char *) &info, sizeof(MTCORE_Func_info), MPI_CHAR,
                          MPI_ANY_SOURCE, MPI_ANY_TAG, MTCORE_COMM_LOCAL, &status);

    *FUNC = info.FUNC;
    *user_nprocs = info.user_nprocs;
    *user_local_nprocs = info.user_local_nprocs;
    *local_root = status.MPI_SOURCE;
    *uh_tag = status.MPI_TAG;

    return mpi_errno;
}

static inline int MTCORE_H_func_get_param(char *func_params, int size,
                                        int user_local_root, int uh_tag)
{
    int local_user_rank, i;
    MPI_Status status;
    int mpi_errno = MPI_SUCCESS;

    return PMPI_Recv(func_params, size, MPI_CHAR, user_local_root, uh_tag,
                     MTCORE_COMM_LOCAL, &status);
}

extern int MTCORE_H_win_allocate(int user_local_root, int user_nprocs, int user_local_nprocs,
                            int user_tag);
extern int MTCORE_H_win_free(int user_local_root, int user_nprocs, int user_local_nprocs, int user_tag);

extern int MTCORE_H_finalize(void);

#endif /* MTCORE_HELPER_H_ */
