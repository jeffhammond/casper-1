/*
 * win-lockall-overhead.c
 *
 *  This benchmark evaluates the overhead of Win_lock_all with
 *  user-specified number of processes (>= 2). Rank 0 locks
 *  all the processes and issues accumulate operations to all
 *  of them.
 *
 *  Author: Min Si
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>

//#define DEBUG
#define CHECK
#define ITER_S 1000
#define ITER_L 500
#define NPROCS_M 16

double *winbuf = NULL;
double locbuf[1];
int rank, nprocs;
MPI_Win win = MPI_WIN_NULL;
int ITER = ITER_S;

static int run_test()
{
    int i, x, errs = 0;
    MPI_Status stat;
    int dst;
    int winbuf_offset = 0;
    double t0, avg_total_time = 0.0, t_total = 0.0;
    double sum = 0.0;

    if (nprocs <= NPROCS_M) {
        ITER = ITER_S;
    }
    else {
        ITER = ITER_L;
    }

    if (rank == 0) {
        t0 = MPI_Wtime();

        for (x = 0; x < ITER; x++) {
            MPI_Win_lock_all(0, win);
            /* Send to all the other processes including itself in order to
             * make sure that all the targets are exactly locked. */
            for (dst = 0; dst < nprocs; dst++) {
                MPI_Accumulate(&locbuf[0], 1, MPI_DOUBLE, dst, 0, 1, MPI_DOUBLE, MPI_SUM, win);
            }
            MPI_Win_unlock_all(win);
        }

        t_total += MPI_Wtime() - t0;
        t_total /= ITER;
    }

    MPI_Barrier(MPI_COMM_WORLD);

#ifdef CHECK
    /* need lock on self rank for checking window buffer.
     * otherwise, the result may be incorrect because flush/unlock
     * doesn't wait for target completion in exclusive lock */
    MPI_Win_lock(MPI_LOCK_SHARED, rank, 0, win);
    sum = 1.0 * ITER;

    /* It is wrong to load/store local winbuf with no_local_load_store */
    double result = 0;
    MPI_Get(&result, 1, MPI_DOUBLE, rank, 0, 1, MPI_DOUBLE, win);
    if (result != sum) {
        fprintf(stderr, "[%d]computation error : winbuf %.2lf != %.2lf\n", rank, i, result, sum);
        errs += 1;
    }

    MPI_Win_unlock(rank, win);
#endif

    if (rank == 0) {
#ifdef ASP
        fprintf(stdout, "asp: nprocs %d total_time %lf\n", nprocs, t_total);
#else
        fprintf(stdout, "orig: nprocs %d total_time %lf\n", nprocs, t_total);
#endif
    }

    return errs;
}

int main(int argc, char *argv[])
{
    int errs;
    MPI_Info win_info = MPI_INFO_NULL;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (nprocs < 2) {
        fprintf(stderr, "Please run using at least 2 processes\n");
        goto exit;
    }

    MPI_Info_create(&win_info);
    MPI_Info_set(win_info, (char *) "no_local_load_store", (char *) "true");

    locbuf[0] = (rank + 1) * 1.0;
    MPI_Win_allocate(sizeof(double), sizeof(double), win_info, MPI_COMM_WORLD, &winbuf, &win);

    errs = run_test();

  exit:

    if (win != MPI_WIN_NULL)
        MPI_Win_free(&win);
    MPI_Finalize();

    return 0;
}
