#include <mpi.h>
#include <vector>
#include <climits>
#include <fstream>
#include <iostream>

using namespace std;

void initialize_MPI(int argc, char **argv, int &num_procs, int &rank)
{
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
}

void read_and_broadcast_input(int rank, int &N, vector<int> &d, ifstream &infile)
{
    if (rank == 0)
    {
        infile >> N;
        d.resize(N + 1);
        for (int i = 0; i <= N; i++)
        {
            infile >> d[i];
        }
        infile.close();
    }
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (rank != 0)
    {
        d.resize(N + 1);
    }
    MPI_Bcast(d.data(), N + 1, MPI_INT, 0, MPI_COMM_WORLD);
}

void initialize_dp_table(int N, vector<vector<long long>> &dp)
{
    dp.resize(N + 1, vector<long long>(N + 1, LLONG_MAX));
    for (int i = 1; i <= N; i++)
    {
        dp[i][i] = 0;
    }
}

void matrix_chain_multiplication(int N, const vector<int> &d, vector<vector<long long>> &dp, int rank, int num_procs)
{
    for (int length = 2; length <= N; length++)
    {
        for (int i = 1; i <= N - length + 1; i++)
        {
            int j = i + length - 1;
            long long local_min = LLONG_MAX;

            // Divide work among processes
            for (int k = i + rank; k < j; k += num_procs)
            {
                long long cost = dp[i][k] + dp[k + 1][j] + d[i - 1] * d[k] * d[j];
                local_min = min(local_min, cost);
            }

            // Gather local minimum costs and find the global minimum
            MPI_Allreduce(&local_min, &dp[i][j], 1, MPI_LONG_LONG, MPI_MIN, MPI_COMM_WORLD);
        }
    }
}

void finalize_MPI()
{
    MPI_Finalize();
}

int main(int argc, char **argv)
{
    int rank, num_procs;
    initialize_MPI(argc, argv, num_procs, rank);

    int N;
    vector<int> d;
    ifstream infile(argv[1]);

    if (!infile)
    {
        cerr << "Failed to open input file." << endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    double start = MPI_Wtime();

    read_and_broadcast_input(rank, N, d, infile);

    vector<vector<long long>> dp;
    initialize_dp_table(N, dp);

    matrix_chain_multiplication(N, d, dp, rank, num_procs);

    double end = MPI_Wtime();

    if (rank == 0)
    {
        cout << dp[1][N] << endl;
        cout << "Time taken: " << end - start << " seconds" << endl;
    }

    finalize_MPI();
    return 0;
}
