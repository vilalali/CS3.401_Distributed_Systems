#include <mpi.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include <stdexcept>
#include <fstream>
using namespace std;

/**
 * Function: inputHandler
 * Purpose: Handles input by reading from a file and broadcasting the data to all processes.
 * Arguments:
 *  - int& N: Reference to the number of elements in the array.
 *  - vector<double>& a: Reference to the vector that stores the array elements.
 *  - int rank: The rank of the current process.
 *  - const string& filename: The name of the input file.
 * If an error occurs (e.g., invalid input or file read error), the program will stop.
 */
void inputHandler(int& N, vector<double>& a, int rank, const string& filename) {
    if (rank == 0) {
        try {
            ifstream infile(filename); // Open file for reading
            if (!infile) {
                throw runtime_error("Error opening file");
            }
            
            infile >> N;
            if (N <= 0) {
                throw invalid_argument("N must be greater than 0");
            }
            a.resize(N);
            for (int i = 0; i < N; i++) {
                infile >> a[i];
            }
        } catch (exception& e) {
            cerr << "Error in input on rank 0: " << e.what() << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    // Broadcast the size of the array to all processes
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Resize vector a to hold data on non-root processes
    if (rank != 0) {
        a.resize(N);
    }

    // Broadcast the array data from the root process to all other processes
    MPI_Bcast(a.data(), N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

/**
 * Function: scatterDataHandler
 * Purpose: Scatters the data from the root process to all other processes.
 * Arguments:
 *  - const vector<double>& a: The vector containing the full array of data.
 *  - vector<double>& local_a: The vector to store the local chunk of data for each process.
 *  - int N: The total number of elements in the array.
 *  - int rank: The rank of the current process.
 *  - int size: The total number of processes.
 * If an error occurs during scattering, the program will stop.
 */
void scatterDataHandler(const vector<double>& a, vector<double>& local_a, int N, int rank, int size) {
    try {
        vector<int> sendcounts(size), displs(size);
        int base_chunk_size = N / size;
        int remainder = N % size;
        int offset = 0;

        for (int i = 0; i < size; i++) {
            sendcounts[i] = (i < remainder) ? (base_chunk_size + 1) : base_chunk_size;
            displs[i] = offset;
            offset += sendcounts[i];
        }

        MPI_Scatterv(a.data(), sendcounts.data(), displs.data(), MPI_DOUBLE,
                     local_a.data(), sendcounts[rank], MPI_DOUBLE, 0, MPI_COMM_WORLD);
    } catch (exception& e) {
        cerr << "Error in scatterDataHandler on rank " << rank << ": " << e.what() << endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
}

/**
 * Function: computeLocalPrefixSum
 * Purpose: Computes the prefix sum of the local chunk of data.
 * Arguments:
 *  - vector<double>& local_a: The local chunk of data for the current process.
 *  - vector<double>& local_p: The vector to store the local prefix sums.
 *  - int sendcount: The number of elements in the local chunk of data.
 * If an error occurs during computation, the program will stop.
 */
void computeLocalPrefixSum(vector<double>& local_a, vector<double>& local_p, int sendcount) {
    try {
        if (sendcount > 0) {
            local_p[0] = local_a[0];
            for (int i = 1; i < sendcount; i++) {
                local_p[i] = local_p[i - 1] + local_a[i];
            }
        }
    } catch (exception& e) {
        cerr << "Error in computeLocalPrefixSum: " << e.what() << endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
}

/**
 * Function: sumsShareBetweenProcesses
 * Purpose: Shares the prefix sums between processes to update local sums.
 * Arguments:
 *  - vector<double>& local_p: The local prefix sums for the current process.
 *  - int sendcount: The number of elements in the local chunk of data.
 *  - int rank: The rank of the current process.
 *  - int size: The total number of processes.
 * If an error occurs during communication, the program will stop.
 */
void sumsShareBetweenProcesses(vector<double>& local_p, int sendcount, int rank, int size) {
    try {
        double prev_sum = 0;

        if (rank > 0) {
            MPI_Recv(&prev_sum, 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 0; i < sendcount; i++) {
                local_p[i] += prev_sum;
            }
        }

        if (sendcount > 0) {
            double local_total_sum = local_p[sendcount - 1];
            if (rank < size - 1) {
                MPI_Send(&local_total_sum, 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);
            }
        }
    } catch (exception& e) {
        cerr << "Error in sumsShareBetweenProcesses on rank " << rank << ": " << e.what() << endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
}

/**
 * Function: gatherResults
 * Purpose: Gathers the results from all processes to the root process.
 * Arguments:
 *  - const vector<double>& local_p: The local prefix sums from the current process.
 *  - vector<double>& p: The vector to store the gathered results at the root process.
 *  - int N: The total number of elements in the array.
 *  - int rank: The rank of the current process.
 *  - int size: The total number of processes.
 * If an error occurs during gathering, the program will stop.
 */
void gatherResults(const vector<double>& local_p, vector<double>& p, int N, int rank, int size) {
    try {
        vector<int> sendcounts(size), displs(size);
        int base_chunk_size = N / size;
        int remainder = N % size;
        int offset = 0;

        for (int i = 0; i < size; i++) {
            sendcounts[i] = (i < remainder) ? (base_chunk_size + 1) : base_chunk_size;
            displs[i] = offset;
            offset += sendcounts[i];
        }

        MPI_Gatherv(local_p.data(), sendcounts[rank], MPI_DOUBLE, p.data(),
                    sendcounts.data(), displs.data(), MPI_DOUBLE, 0, MPI_COMM_WORLD);
    } catch (exception& e) {
        cerr << "Error in gatherResults on rank " << rank << ": " << e.what() << endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
}

/**
 * Function: outputHandler
 * Purpose: Outputs the final results at the root process.
 * Arguments:
 *  - const vector<double>& p: The vector containing the final prefix sums.
 *  - int N: The total number of elements in the array.
 *  - int rank: The rank of the current process.
 * If an error occurs during output, the program will stop.
 */
void outputHandler(const vector<double>& p, int N, int rank) {
    if (rank == 0) {
        try {
            cout << "=========RESULT=========" << endl;
            cout << fixed << setprecision(2);
            for (int i = 0; i < N; i++) {
                cout << p[i];
                if (i < N - 1) cout << " ";
            }
            cout << endl;
        } catch (exception& e) {
            cerr << "Error in outputHandler on rank 0: " << e.what() << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
}

int main(int argc, char** argv) {
    int rank, size, N;
    vector<double> a, p;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Check if number of processes is within the allowed range
    if (size < 1 || size > 12) {
        if (rank == 0) {
            cerr << "Error: Number of processes (NP) should be between 1 and 12." << endl;
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (argc != 2) { // Check for correct number of arguments
        if (rank == 0) {
            cerr << "Usage: " << argv[0] << " <input_file>" << endl;
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    string filename = argv[1]; // Input file name from command line

    double start_time = MPI_Wtime(); // Start timing

    try {
        // Read input from file and broadcast to all processes
        inputHandler(N, a, rank, filename);

        // Calculate local chunk size
        vector<double> local_a((N + size - 1) / size, 0.0);
        vector<double> local_p(local_a.size(), 0.0);

        // Scatter data to all processes
        scatterDataHandler(a, local_a, N, rank, size);

        // Compute local prefix sum
        computeLocalPrefixSum(local_a, local_p, local_a.size());

        // Share sums between processes
        sumsShareBetweenProcesses(local_p, local_a.size(), rank, size);

        // Gather results at root process
        if (rank == 0) p.resize(N);
        gatherResults(local_p, p, N, rank, size);

        // Print the output at the root
        outputHandler(p, N, rank);
    } catch (exception& e) {
        cerr << "Error on rank " << rank << ": " << e.what() << endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    double end_time = MPI_Wtime(); // End timing
    if (rank == 0) {
        std::cout << "\nExecution Time: " << end_time - start_time << " seconds.\n" << std::endl;
    }

    MPI_Finalize();
    return 0;
}