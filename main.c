#include <stdio.h>
#include <math.h>
#include <mpi.h>

int MPI_FlattreeColectiva(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);
int MPI_BinomialColectiva(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm);

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int numprocs, rank, namelen;
    int i, done = 0, n;
    double PI25DT = 3.141592653589793238462643;
    double pi, h, sum, x, sumTotal = 0.0, recibido, salto;
    MPI_Status status;

    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    while (!done)
    {
        sum = 0;
        sumTotal = 0;

        // COMUNICACION
        if (rank == 0)
        {
            printf("Enter the number of intervals: (0 quits) \n");
            scanf("%d", &n);
        }
        // MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_BinomialColectiva(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

        if (n == 0)
            break;

        // CALCULOS
        h = 1.0 / (double)n;

        for (int i = rank; i < n; i += numprocs)
        {
            x = h * ((double)i - 0.5);
            sum += 4.0 / (1.0 + x * x);
        }

        // COMUNICACION
        // MPI_Reduce(&sum, &sumTotal, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_FlattreeColectiva(&sum, &sumTotal, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        if (rank == 0)
        {
            pi = h * sumTotal;
            printf("pi is approximately %.16f, Error is %.16f\n", pi, fabs(pi - PI25DT));
        }
    }
    MPI_Finalize();
}

int MPI_FlattreeColectiva(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
{
    int numprocs, rank;
    double localSum = 0, rec;
    MPI_Status status;

    if (datatype != MPI_DOUBLE)
        return MPI_ERR_TYPE;

    MPI_Comm_size(comm, &numprocs);
    MPI_Comm_rank(comm, &rank);

    if (rank == root)
    {
        localSum = *(double *)sendbuf;
        for (int i = 1; i < numprocs; i++)
        {
            MPI_Recv(&rec, count, datatype, MPI_ANY_SOURCE, 0, comm, &status);
            localSum += rec;
        }
        *(double *)recvbuf = localSum;
    }
    else
    {
        MPI_Send(sendbuf, count, datatype, root, 0, MPI_COMM_WORLD);
    }
    return MPI_SUCCESS;
}

int MPI_BinomialColectiva(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
{
    int numprocs, rank;

    MPI_Comm_size(comm, &numprocs);
    MPI_Comm_rank(comm, &rank);

    for (int i = 1; pow(2, i - 1) <= numprocs; i++)
    {
        if (rank < pow(2, i - 1) && rank + pow(2, i - 1) < numprocs)
            MPI_Send(buffer, count, datatype, rank + (int)pow(2, i - 1), 0, comm);
        if (rank >= pow(2, i - 1) && rank < pow(2, i))
            MPI_Recv(buffer, count, datatype, rank - (int)pow(2, i - 1), 0, comm, MPI_STATUS_IGNORE);
    }
    return MPI_SUCCESS;
}
