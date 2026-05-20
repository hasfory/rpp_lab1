#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <mpi.h>

int ProcNum = 0;
int ProcRank = 0;

int IsPrime(long long n) {
    if (n < 2) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    for (long long i = 3; i <= (long long)sqrt((double)n); i += 2)
        if (n % i == 0) return 0;
    return 1;
}

void ProcessInitialization(long long& RangeStart, long long& RangeEnd,
    long long& ProcStart, long long& ProcEnd) {
    if (ProcRank == 0) {
        printf("\nEnter the start of the range: ");
        fflush(stdout);
        scanf("%lld", &RangeStart);

        printf("Enter the end of the range: ");
        fflush(stdout);
        scanf("%lld", &RangeEnd);
    }

    MPI_Bcast(&RangeStart, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&RangeEnd, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

    long long total = RangeEnd - RangeStart + 1;
    long long chunk = total / ProcNum;
    long long rem = total % ProcNum;
    ProcStart = RangeStart + ProcRank * chunk + (ProcRank < rem ? ProcRank : rem);
    long long mySize = chunk + (ProcRank < rem ? 1 : 0);
    ProcEnd = ProcStart + mySize - 1;
}

long long ParallelPrimeCount(long long ProcStart, long long ProcEnd) {
    long long localCount = 0;
    for (long long n = ProcStart; n <= ProcEnd; n++)
        if (IsPrime(n)) localCount++;

    long long globalCount = 0;
    MPI_Reduce(&localCount, &globalCount, 1,
        MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    return globalCount;
}

long long SequentialPrimeCount(long long start, long long end) {
    long long count = 0;
    for (long long n = start; n <= end; n++)
        if (IsPrime(n)) count++;
    return count;
}

int main(int argc, char* argv[]) {
    long long RangeStart, RangeEnd, ProcStart, ProcEnd;
    double    Start, Finish;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

    ProcessInitialization(RangeStart, RangeEnd, ProcStart, ProcEnd);

    Start = MPI_Wtime();
    long long result = ParallelPrimeCount(ProcStart, ProcEnd);
    Finish = MPI_Wtime();

    if (ProcRank == 0) {
        printf("Number of primes: %lld\n", result);
        printf("Time = %f sec\n", Finish - Start);
        fflush(stdout);
    }

    MPI_Finalize();
    return 0;
}