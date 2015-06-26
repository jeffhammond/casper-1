/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 * (C) 2015 by Argonne National Laboratory.
 *     See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include "csp.h"

static inline int CSP_proc_null_rget_accumualte_impl(const void *origin_addr, int origin_count,
                                                     MPI_Datatype origin_datatype,
                                                     void *result_addr, int result_count,
                                                     MPI_Datatype result_datatype, int target_rank,
                                                     MPI_Aint target_disp, int target_count,
                                                     MPI_Datatype target_datatype, MPI_Op op,
                                                     CSP_win * ug_win, MPI_Request * request)
{
    MPI_Win *win_ptr = NULL;
    CSP_win_target *target = NULL;

    /* We cannot create MPI_Request and complete it here, thus we simply pass to MPI
     * through an window owned by a random target.*/
    CSP_target_get_epoch_win(0, target, ug_win, win_ptr);

    return PMPI_Rget_accumulate(origin_addr, origin_count, origin_datatype,
                                result_addr, result_count, result_datatype,
                                target_rank, target_disp, target_count,
                                target_datatype, op, *win_ptr, request);
}

static int CSP_rget_accumulate_impl(const void *origin_addr, int origin_count,
                                    MPI_Datatype origin_datatype, void *result_addr,
                                    int result_count, MPI_Datatype result_datatype,
                                    int target_rank, MPI_Aint target_disp, int target_count,
                                    MPI_Datatype target_datatype, MPI_Op op,
                                    CSP_win * ug_win, MPI_Request * request)
{
    int mpi_errno = MPI_SUCCESS;
    MPI_Aint ug_target_disp = 0;
    int rank;
    CSP_win_target *target = NULL;

    if (target_rank == MPI_PROC_NULL) {
        mpi_errno = CSP_proc_null_rget_accumualte_impl(origin_addr, origin_count, origin_datatype,
                                                       result_addr, result_count, result_datatype,
                                                       target_rank, target_disp, target_count,
                                                       target_datatype, op, ug_win, request);
        goto fn_exit;
    }

    PMPI_Comm_rank(ug_win->user_comm, &rank);
    target = &(ug_win->targets[target_rank]);

#ifdef CSP_ENABLE_EPOCH_STAT_CHECK
    CSP_target_check_epoch_per_op(target, ug_win);
#endif

#warning MPI_Rget_accumulate is not implemented in segment-lock mode for now. \
Please do not set CSP_LOCK_METHOD=segment when using MPI_Rget_accumulate.

    /* Should not do local RMA in accumulate because of atomicity issue */
    /* TODO: Implement get_acc for segmentation */
    {
        /* Translation for intra/inter-node operations.
         *
         * We do not use force flush + shared window for optimizing operations to local targets.
         * Because: 1) we lose lock optimization on force flush; 2) Although most implementation
         * does shared-communication for operations on shared windows, MPI standard doesn’t
         * require it. Some implementation may use network even for shared targets for
         * shorter CPU occupancy.
         */
        int target_g_rank_in_ug = -1;
        int data_size ATTRIBUTE((unused)) = 0;
        MPI_Aint target_g_offset = 0;
        MPI_Win *win_ptr = NULL;

        CSP_target_get_epoch_win(0, target, ug_win, win_ptr);

#if defined(CSP_ENABLE_RUNTIME_LOAD_OPT)
        if (CSP_ENV.load_opt == CSP_LOAD_BYTE_COUNTING) {
            PMPI_Type_size(datatype, &data_size);
        }
#endif
        mpi_errno = CSP_target_get_ghost(target_rank, 0, 1, data_size, ug_win,
                                         &target_g_rank_in_ug, &target_g_offset);
        if (mpi_errno != MPI_SUCCESS)
            goto fn_fail;

        ug_target_disp = target_g_offset + target->disp_unit * target_disp;

        /* Issue operation to the ghost process in corresponding ug-window of target process. */
        mpi_errno = PMPI_Rget_accumulate(origin_addr, origin_count, origin_datatype,
                                         result_addr, result_count, result_datatype,
                                         target_g_rank_in_ug, ug_target_disp, target_count,
                                         target_datatype, op, *win_ptr, request);

        CSP_DBG_PRINT("CASPER Rget_accumulate to (ghost %d, win 0x%x [%s]) instead of "
                      "target %d, 0x%lx(0x%lx + %d * %ld)\n",
                      target_g_rank_in_ug, *win_ptr,
                      CSP_target_get_epoch_stat_name(target, ug_win),
                      target_rank, ug_target_disp, target_g_offset, target->disp_unit, target_disp);
    }

  fn_exit:
    return mpi_errno;

  fn_fail:
    goto fn_exit;
}

int MPI_Rget_accumulate(const void *origin_addr, int origin_count, MPI_Datatype origin_datatype,
                        void *result_addr, int result_count, MPI_Datatype result_datatype,
                        int target_rank, MPI_Aint target_disp, int target_count,
                        MPI_Datatype target_datatype, MPI_Op op, MPI_Win win, MPI_Request * request)
{
    int mpi_errno = MPI_SUCCESS;
    CSP_win *ug_win;

    CSP_DBG_PRINT_FCNAME();

    CSP_fetch_ug_win_from_cache(win, ug_win);

    if (ug_win) {
        /* casper window */
        mpi_errno = CSP_rget_accumulate_impl(origin_addr, origin_count, origin_datatype,
                                             result_addr, result_count, result_datatype,
                                             target_rank, target_disp, target_count,
                                             target_datatype, op, ug_win, request);
    }
    else {
        /* normal window */
        mpi_errno = PMPI_Rget_accumulate(origin_addr, origin_count, origin_datatype,
                                         result_addr, result_count, result_datatype,
                                         target_rank, target_disp, target_count,
                                         target_datatype, op, win, request);
    }

    return mpi_errno;
}