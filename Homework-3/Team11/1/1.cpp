#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <stdexcept>

// Structure to represent a point with x and y coordinates
struct Point {
    double x, y;
};

// Function to calculate the Euclidean distance between two points
// Euclidean distance is a measure of the straight-line distance between two points in a Euclidean space
double calculateDistance(const Point& p1, const Point& p2) {
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}

/**
 * Function: createPointType
 * Purpose: Creates a custom MPI datatype for the Point structure.
 * Arguments: None
 * Returns: An MPI_Datatype representing the Point structure.
 * This allows MPI to correctly send and receive Point objects between processes.
 */
MPI_Datatype createPointType() {
    MPI_Datatype pointType;
    int lengths[2] = {1, 1};
    MPI_Aint displacements[2] = {offsetof(Point, x), offsetof(Point, y)};
    MPI_Datatype types[2] = {MPI_DOUBLE, MPI_DOUBLE};
    MPI_Type_create_struct(2, lengths, displacements, types, &pointType);
    MPI_Type_commit(&pointType);
    return pointType;
}

/**
 * Function: inputHandler
 * Purpose: Handles input by reading from a file and broadcasting the data to all processes.
 * Arguments:
 *  - int& N: Reference to the number of points.
 *  - int& M: Reference to the number of queries.
 *  - int& K: Reference to the number of nearest neighbors to find.
 *  - std::vector<Point>& points: Reference to the vector storing the points.
 *  - std::vector<Point>& queries: Reference to the vector storing the queries.
 *  - int world_rank: The rank of the current process.
 *  - const char* filename: The name of the input file.
 * If an error occurs (e.g., invalid input or file read error), the program will stop.
 */
void inputHandler(int& N, int& M, int& K, std::vector<Point>& points, std::vector<Point>& queries, int world_rank, const char* filename) {
    try {
        if (world_rank == 0) {
            std::ifstream inputFile(filename);
            if (!inputFile) {
                throw std::runtime_error("Unable to open input file.");
            }

            // Read the number of points, queries, and nearest neighbors
            inputFile >> N >> M >> K;

            // Read the points and queries from the file
            points.resize(N);
            for (int i = 0; i < N; ++i) {
                inputFile >> points[i].x >> points[i].y;
            }

            queries.resize(M);
            for (int i = 0; i < M; ++i) {
                inputFile >> queries[i].x >> queries[i].y;
            }

            inputFile.close();
        }

        // Broadcast N, M, and K to all processes
        MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&M, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&K, 1, MPI_INT, 0, MPI_COMM_WORLD);

        // Resize vectors in non-root processes
        if (world_rank != 0) {
            points.resize(N);
            queries.resize(M);
        }

        // Broadcast points and queries to all processes
        MPI_Bcast(points.data(), N, createPointType(), 0, MPI_COMM_WORLD);
        MPI_Bcast(queries.data(), M, createPointType(), 0, MPI_COMM_WORLD);

    } catch (const std::exception& e) {
        std::cerr << "Exception occurred during input or broadcast: " << e.what() << std::endl;
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);  // Terminate MPI environment on failure
    }
}

/**
 * Function: processQueries
 * Purpose: Processes the queries assigned to this process and calculates the K-nearest neighbors for each query.
 * Arguments:
 *  - int N: The number of points.
 *  - int K: The number of nearest neighbors to find for each query.
 *  - const std::vector<Point>& points: A reference to the vector storing all points.
 *  - const std::vector<Point>& queries: A reference to the vector storing all queries.
 *  - int start: The start index of the queries this process will handle.
 *  - int end: The end index of the queries this process will handle.
 * Returns: A vector containing the K-nearest neighbors for the queries handled by this process.
 * If an error occurs (e.g., invalid calculation or memory issue), the program will stop.
 */
std::vector<Point> processQueries(int N, int K, const std::vector<Point>& points, const std::vector<Point>& queries, int start, int end) {
    try {
        std::vector<Point> local_results((end - start) * K);

        // Process each query assigned to this process
        for (int i = start; i < end; ++i) {
            std::vector<std::pair<double, Point>> distances;

            // Calculate distances from query point to all other points
            for (const auto& p : points) {
                double dist = calculateDistance(queries[i], p);
                distances.push_back({dist, p});
            }

            // Sort distances and keep the K smallest distances
            std::sort(distances.begin(), distances.end(),
                      [](const std::pair<double, Point>& a, const std::pair<double, Point>& b) {
                          return a.first < b.first;
                      });

            // Collect K nearest neighbors for this query
            for (int j = 0; j < K; ++j) {
                local_results[(i - start) * K + j] = distances[j].second;
            }
        }

        return local_results;
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred during query processing: " << e.what() << std::endl;
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);  // Terminate MPI environment on failure
        return {};  // Return an empty vector to satisfy return type, though MPI_Abort will terminate the program
    }
}

/**
 * Function: outputHandler
 * Purpose: Handles the output of the program by printing the final results (K-nearest neighbors) to the console.
 * Arguments:
 *  - const std::vector<Point>& global_results: A reference to the vector storing all the final results from all processes.
 *  - int M: The number of queries.
 *  - int K: The number of nearest neighbors for each query.
 *  - int world_rank: The rank of the current process.
 * If an error occurs during output (e.g., output stream failure), the program will stop.
 */
void outputHandler(const std::vector<Point>& global_results, int M, int K, int world_rank) {
    try {
        if (world_rank == 0) {
            std::cout << "=========RESULT=========" << std::endl;
            for (int i = 0; i < M; ++i) {
                for (int j = 0; j < K; ++j) {
                    const Point& p = global_results[i * K + j];
                    std::cout << p.x << " " << p.y << std::endl;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred during result output: " << e.what() << std::endl;
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);  // Terminate MPI environment on failure
    }
}

// Main Function
int main(int argc, char** argv) {
    try {
        // Initialize MPI environment
        MPI_Init(&argc, &argv);

        int world_size, world_rank;
        MPI_Comm_size(MPI_COMM_WORLD, &world_size);
        MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

        // Check if the number of processes exceeds the allowed range (1-12)
        if (world_size < 1 || world_size > 12) {
            if (world_rank == 0) {
                std::cerr << "Error: Number of processes (np) should be between 1 and 12." << std::endl;
            }
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        // Check if the input file is provided as a command-line argument
        if (argc < 2) {
            if (world_rank == 0) {
                std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
            }
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        int N, M, K;  // N = number of points, M = number of queries, K = number of nearest neighbors
        std::vector<Point> points;  // Vector to store points
        std::vector<Point> queries;  // Vector to store query points

        // Start measuring time
        double start_time = MPI_Wtime();

        // Calling Input Function
        inputHandler(N, M, K, points, queries, world_rank, argv[1]);

        // Determine the workload for each process
        int queries_per_process = M / world_size;
        int remainder = M % world_size;

        // Calculate the range of queries this process will handle
        int start = world_rank * queries_per_process + std::min(world_rank, remainder);
        int end = start + queries_per_process + (world_rank < remainder ? 1 : 0);

        // Process queries and calculate K-nearest neighbors
        std::vector<Point> local_results = processQueries(N, K, points, queries, start, end);

        // Gather results from all processes at the root process
        std::vector<int> recvcounts(world_size), displs(world_size);

        if (world_rank == 0) {
            // Calculate receive counts and displacements for Gatherv
            for (int i = 0; i < world_size; ++i) {
                int local_queries = queries_per_process + (i < remainder ? 1 : 0);
                recvcounts[i] = local_queries * K;
                displs[i] = (i == 0) ? 0 : displs[i - 1] + recvcounts[i - 1];
            }
        }

        std::vector<Point> global_results(M * K);

        // Gather results from all processes
        MPI_Gatherv(local_results.data(), local_results.size(), createPointType(),
                    global_results.data(), recvcounts.data(), displs.data(), createPointType(),
                    0, MPI_COMM_WORLD);

        // Calling Output Handler Function
        outputHandler(global_results, M, K, world_rank);

        // End measuring time using MPI_Wtime.
        double end_time = MPI_Wtime();
        if (world_rank == 0) {
            std::cout << "\nExecution Time: " << end_time - start_time << " seconds.\n" << std::endl;
        }

        // Finalize the MPI environment. Is called to clean up the MPI environment.
        MPI_Finalize();

    } catch (const std::exception& e) {
        std::cerr << "Exception occurred in the main program: " << e.what() << std::endl;
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);  // Ensure proper cleanup and termination of MPI
    }

    return 0;
}