#include <mpi.h>
#include <iostream>
#include <complex>
#include <vector>
#include <stdexcept>
#include <fstream>

// Function to compute the Julia set for a given complex number z0 and constant c
/**
 * Function: juliaSetIterations
 * Purpose: Computes whether a given complex number z0 belongs to the Julia set defined by the constant c.
 * Arguments:
 *  - std::complex<double> z0: The complex number to test.
 *  - std::complex<double> c: The constant defining the Julia set.
 *  - int K: The maximum number of iterations.
 *  - double threshold: The threshold for divergence.
 * Returns:
 *  - int: 1 if z0 is in the Julia set (did not diverge within K iterations), 0 otherwise.
 */
int juliaSetIterations(std::complex<double> z0, std::complex<double> c, int K, double threshold) {
    std::complex<double> z = z0;
    int iterations = 0;
    while (iterations < K && std::abs(z) <= threshold) {
        z = z * z + c;
        iterations++;
    }
    return iterations == K ? 1 : 0;  // 1 if in the Julia set, 0 otherwise
}

// Function to handle input from file and broadcast parameters
/**
 * Function: inputHandler
 * Purpose: Handles input by reading from a file and broadcasting the data to all processes.
 * Arguments:
 *  - int& N: Reference to the number of rows in the grid.
 *  - int& M: Reference to the number of columns in the grid.
 *  - int& K: Reference to the maximum number of iterations.
 *  - double& creal: Reference to the real part of the constant c.
 *  - double& cimag: Reference to the imaginary part of the constant c.
 *  - int world_rank: The rank of the current process.
 *  - const char* filename: The name of the input file.
 * If an error occurs (e.g., invalid input or file read error), the program will stop.
 */
void inputHandler(int& N, int& M, int& K, double& creal, double& cimag, int world_rank, const char* filename) {
    if (world_rank == 0) {
        std::ifstream infile(filename);
        if (!infile) {
            std::cerr << "Error opening file: " << filename << std::endl;
            MPI_Abort(MPI_COMM_WORLD, -1);  // Abort all processes
        }

        try {
            infile >> N >> M >> K;
            if (N <= 0 || M <= 0 || K <= 0) {
                throw std::invalid_argument("N, M, and K must be positive integers.");
            }
            infile >> creal >> cimag;
        } catch (const std::exception& e) {
            std::cerr << "Input error: " << e.what() << std::endl;
            MPI_Abort(MPI_COMM_WORLD, -1);  // Abort all processes
        }
    }

    // Broadcast the input parameters to all processes
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&M, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&K, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&creal, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&cimag, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

// Function to compute the Julia set for a range of points
/**
 * Function: computeJuliaSet
 * Purpose: Computes the Julia set for a range of grid points.
 * Arguments:
 *  - int start: The starting index of the range.
 *  - int end: The ending index of the range.
 *  - int M: The number of columns in the grid.
 *  - int N: The number of rows in the grid.
 *  - int K: The maximum number of iterations.
 *  - std::complex<double> c: The constant defining the Julia set.
 *  - double threshold: The threshold for divergence.
 * Returns:
 *  - std::vector<int>: A vector containing the results of the Julia set computation for the range.
 */
std::vector<int> computeJuliaSet(int start, int end, int M, int N, int K, std::complex<double> c, double threshold) {
    std::vector<int> local_result(end - start, 0);
    for (int idx = start; idx < end; ++idx) {
        int i = idx / M;
        int j = idx % M;

        // Map the grid coordinates to the complex plane [-1.5, 1.5] x [-1.5, 1.5]
        double real_part = -1.5 + 3.0 * j / (M - 1);
        double imag_part = -1.5 + 3.0 * i / (N - 1);
        std::complex<double> z0(real_part, imag_part);

        // Compute whether the point is in the Julia set
        local_result[idx - start] = juliaSetIterations(z0, c, K, threshold);
    }
    return local_result;
}

// Function to gather and print the results
/**
 * Function: outputHandler
 * Purpose: Gathers the results from all processes and prints them.
 * Arguments:
 *  - const std::vector<int>& local_result: The local results from each process.
 *  - int total_points: The total number of points in the grid.
 *  - int points_per_process: The number of points each process is responsible for, excluding remainder.
 *  - int remainder: The remainder points that need to be distributed.
 *  - int N: The number of rows in the grid.
 *  - int M: The number of columns in the grid.
 *  - int world_rank: The rank of the current process.
 *  - int world_size: The total number of processes.
 * If an error occurs during gathering or printing, the program will stop.
 */
void outputHandler(const std::vector<int>& local_result, int total_points, int points_per_process, int remainder, int N, int M, int world_rank, int world_size) {
    std::vector<int> global_result;
    if (world_rank == 0) {
        global_result.resize(total_points);
    }

    // Gather the results with varying sizes
    std::vector<int> recv_counts(world_size);
    std::vector<int> displacements(world_size);

    for (int i = 0; i < world_size; ++i) {
        recv_counts[i] = points_per_process + (i < remainder ? 1 : 0);
        displacements[i] = (i > 0) ? displacements[i - 1] + recv_counts[i - 1] : 0;
    }

    MPI_Gatherv(local_result.data(), local_result.size(), MPI_INT,
                global_result.data(), recv_counts.data(), displacements.data(), MPI_INT,
                0, MPI_COMM_WORLD);

    // Print the results (only by the root process)
    if (world_rank == 0) {
        std::cout << "==========RESULT==========" << std::endl;
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < M; ++j) {
                std::cout << global_result[i * M + j] << " ";
            }
            std::cout << std::endl;
        }
    }
}

int main(int argc, char** argv) {
    // Initialize MPI environment
    MPI_Init(&argc, &argv);

    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Check if number of processes is within the valid range
    if (world_size < 1 || world_size > 12) {
        if (world_rank == 0) {
            std::cerr << "Number of processes (np) should be from 1 to 12." << std::endl;
        }
        MPI_Abort(MPI_COMM_WORLD, -1);  // Abort all processes
    }

    // Measure start time for profiling
    double start_time = MPI_Wtime();

    // Check command-line arguments
    if (argc != 2) {
        if (world_rank == 0) {
            std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        }
        MPI_Abort(MPI_COMM_WORLD, -1);  // Abort all processes
    }

    const char* filename = argv[1];

    // Input parameters
    int N, M, K;
    double creal, cimag;

    // Handle input and broadcast parameters
    inputHandler(N, M, K, creal, cimag, world_rank, filename);

    std::complex<double> c(creal, cimag);
    double threshold = 2.0;

    // Determine the workload for each process
    int total_points = N * M;
    int points_per_process = total_points / world_size;
    int remainder = total_points % world_size;

    // Adjust the range of points each process will handle
    int start = world_rank * points_per_process + std::min(world_rank, remainder);
    int end = start + points_per_process + (world_rank < remainder ? 1 : 0);

    // Compute the Julia set for the assigned points
    std::vector<int> local_result = computeJuliaSet(start, end, M, N, K, c, threshold);

    // Gather and print results
    outputHandler(local_result, total_points, points_per_process, remainder, N, M, world_rank, world_size);

    // Measure end time for profiling
    double end_time = MPI_Wtime();
    if (world_rank == 0) {
        std::cout << "\nExecution Time: " << end_time - start_time << " seconds.\n" << std::endl;
    }

    // Finalize the MPI environment
    MPI_Finalize();
    return 0;
}
