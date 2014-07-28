#include <stdio.h>
#include <stdlib.h>
#include "mtcore.h"

int MPI_Finalize(void)
{
    static const char FCNAME[] = "MPI_Finalize";
    int mpi_errno = MPI_SUCCESS;
    int rank, nprocs, local_rank, local_nprocs;

    MTCORE_DBG_PRINT_FCNAME();

    /* Helpers do not need user process information because it is a global call. */
    MTCORE_Func_start(MTCORE_FUNC_FINALIZE, 0, 0, 0, MTCORE_COMM_USER_LOCAL);

    if (MTCORE_COMM_USER_WORLD) {
        MTCORE_DBG_PRINT(" free MTCORE_COMM_USER_WORLD\n");
        PMPI_Comm_free(&MTCORE_COMM_USER_WORLD);
    }

    if (MTCORE_COMM_LOCAL) {
        MTCORE_DBG_PRINT(" free MTCORE_COMM_LOCAL\n");
        PMPI_Comm_free(&MTCORE_COMM_LOCAL);
    }

    if (MTCORE_COMM_USER_LOCAL) {
        MTCORE_DBG_PRINT(" free MTCORE_COMM_USER_LOCAL\n");
        PMPI_Comm_free(&MTCORE_COMM_USER_LOCAL);
    }

    if (MTCORE_COMM_USER_ROOTS) {
        MTCORE_DBG_PRINT(" free MTCORE_COMM_USER_ROOTS\n");
        PMPI_Comm_free(&MTCORE_COMM_USER_ROOTS);
    }

    if (MTCORE_GROUP_WORLD != MPI_GROUP_NULL)
        PMPI_Group_free(&MTCORE_GROUP_WORLD);
    if (MTCORE_GROUP_LOCAL != MPI_GROUP_NULL)
        PMPI_Group_free(&MTCORE_GROUP_LOCAL);

    if (MTCORE_ALL_NODE_IDS)
        free(MTCORE_ALL_NODE_IDS);

    if (MTCORE_ALL_H_IN_COMM_WORLD)
        free(MTCORE_ALL_H_IN_COMM_WORLD);

    mpi_errno = PMPI_Finalize();
    if (mpi_errno != MPI_SUCCESS)
        goto fn_fail;

    destroy_uh_win_table();

  fn_exit:
    return mpi_errno;

  fn_fail:
    goto fn_exit;
}
