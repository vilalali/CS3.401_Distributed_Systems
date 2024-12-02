#include <iostream>
#include <fstream>
#include <mpi.h>
#include <ctime>

using namespace std;

// Structure to represent a matrix
struct mmatrix {
    int size;      // Size of the matrix (number of rows/columns)
    double *mat;   // Pointer to matrix data (augmented with right-hand side for inverses)
};

// Initialize the matrix based on user input
void initialize_matrix(mmatrix& matrix, int*& procs, int*& proc_prefix_sum, int my_rank, int num_procs, ifstream& input_file) {
    if (my_rank == 0) {
        input_file >> matrix.size;
        int base_procs = matrix.size / num_procs;
        int remaining_procs = matrix.size % num_procs;

        if (num_procs > 0) {
            procs = new int[num_procs];
            proc_prefix_sum = new int[num_procs];
        } else {
            exit(1); // Exit if there are no processes
        }

        // Distribute rows of the matrix among processes
        for (int i = 0; i < num_procs; ++i) {
            procs[i] = base_procs;
            if (remaining_procs > 0) {
                procs[i]++;
                remaining_procs--;
            }
        }

        // Compute prefix sum for row distribution
        for(int i = 0; i < num_procs; ++i) {
            if(i == 0) {
                proc_prefix_sum[i] = 0;
            } else {
                proc_prefix_sum[i] = proc_prefix_sum[i - 1] + procs[i - 1];
            }
        }
    }

    // Broadcast matrix size and row distribution info to all processes
    MPI_Bcast(&matrix.size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (my_rank != 0) {
        procs = new int[num_procs];
        proc_prefix_sum = new int[num_procs];
    }

    MPI_Bcast(procs, num_procs, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(proc_prefix_sum, num_procs, MPI_INT, 0, MPI_COMM_WORLD);

    int local_n_row = procs[my_rank];

    // Allocate memory for the matrix
    if (my_rank == 0) {
        // Root process has the full matrix
        matrix.mat = new double[matrix.size * matrix.size * 2];
    } else {
        // Other processes have partial matrix
        matrix.mat = new double[local_n_row * matrix.size * 2];
    }
}

// Function to get the input matrix from a file and initialize identity matrix
void input_matrix(mmatrix& matrix, int*& procs, int my_rank, int num_procs, ifstream& input_file) {
    if (my_rank == 0) {
        // Read matrix data from the input file
        for (int i = 0; i < matrix.size; ++i) {
            for (int j = 0; j < matrix.size; ++j) {
                input_file >> matrix.mat[i * matrix.size * 2 + j];
            }
            // Initialize the right-hand side to identity matrix
            for (int j = matrix.size; j < matrix.size * 2; ++j) {
                if (j == i + matrix.size) {    
                    matrix.mat[i * matrix.size * 2 + j] = 1;
                } else {
                    matrix.mat[i * matrix.size * 2 + j] = 0;
                }
            }
        }
    }

    int local_n_row = procs[my_rank];

    // Arrays to handle data scattering
    int* sendcounts = nullptr;
    int* displs = nullptr;

    if (my_rank == 0) {
        sendcounts = new int[num_procs];
        displs = new int[num_procs];

        // Fill sendcounts with number of elements for each process
        int displacement = 0;
        for (int i = 0; i < num_procs; ++i) {
            sendcounts[i] = procs[i] * matrix.size * 2;  // Number of elements for each process
            displs[i] = displacement;                    // Displacement for each process
            displacement += sendcounts[i];               // Update the displacement
        }
    }

    // Scatter the matrix from the root process to all processes
    MPI_Scatterv(matrix.mat, sendcounts, displs, MPI_DOUBLE,
                 matrix.mat, local_n_row * matrix.size * 2, MPI_DOUBLE,
                 0, MPI_COMM_WORLD);

    // Clean up
    if (my_rank == 0) {
        delete[] sendcounts;
        delete[] displs;
    }
}

// Free the memory allocated for the matrix
void deallocate_matrix(mmatrix& matrix) {
    delete[] matrix.mat;
}

// Function to perform parallel matrix inversion using Gaussian elimination
void parallel_inverse_matrix(mmatrix& matrix, int*& procs, int*& proc_prefix_sum, int my_rank, int num_procs) {
    int n = matrix.size;
    int local_n_row = procs[my_rank];
    int local_prefix_sum = proc_prefix_sum[my_rank];

    int startIdx = local_prefix_sum;
    int endIdx = startIdx + local_n_row - 1;

    int* pivot_rows_indexes = nullptr;
    double* pivot_rows = nullptr;

    if (my_rank == 0) {
        pivot_rows_indexes = new int[num_procs];
        pivot_rows = new double[n * 2 * num_procs];
    }

    for (int i = 0; i < n; ++i) {
        int adjusted_pivot_row_idx = -1;
        double* pivot_row = new double[n * 2];
        double pivot_value = 0;
        double* row_i = new double[n * 2];

        // Handle row `i` for the current process
        if (i >= startIdx && i <= endIdx) {
            int local_row_idx = i - startIdx;
            for (int j = 0; j < 2 * n; ++j) {
                row_i[j] = matrix.mat[local_row_idx * n * 2 + j];
            }

            if (my_rank != 0) {
                MPI_Send(row_i, n * 2, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
            }

            int pivot_row_idx = local_row_idx;
            pivot_value = matrix.mat[pivot_row_idx * n * 2 + i];

            // Find the pivot row with the maximum absolute value
            for (int row = pivot_row_idx + 1; row < local_n_row; ++row) {
                if (std::abs(matrix.mat[row * n * 2 + i]) > std::abs(pivot_value)) {
                    pivot_row_idx = row;
                    pivot_value = matrix.mat[row * n * 2 + i];
                }
            }

            adjusted_pivot_row_idx = pivot_row_idx + startIdx;

            for (int k = 0; k < n * 2; ++k) {
                pivot_row[k] = matrix.mat[pivot_row_idx * n * 2 + k];
            }
        }

        // Handle pivoting and rescaling at the root
        int i_copy = i;
        int row_i_root = 0;
        while (i_copy >= procs[row_i_root]) {
            i_copy -= procs[row_i_root];
            row_i_root++;
        }

        if (my_rank == 0 && row_i_root != 0) {
            MPI_Recv(row_i, n * 2, MPI_DOUBLE, row_i_root, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        // Gather pivot rows and their indexes to the root process
        MPI_Gather(&adjusted_pivot_row_idx, 1, MPI_INT, pivot_rows_indexes, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Gather(pivot_row, n * 2, MPI_DOUBLE, pivot_rows, n * 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        double* rescaled_pivot_row = new double[n * 2];
        if (my_rank == 0) {
            // Find the pivot row with the maximum absolute value among all processes
            for (int k = 0; k < num_procs; ++k) {
                if (std::abs(pivot_rows[k * n * 2 + i]) > std::abs(pivot_value) && pivot_rows_indexes[k] >= 0) {
                    adjusted_pivot_row_idx = pivot_rows_indexes[k];
                    pivot_value = pivot_rows[k * n * 2 + i];
                    for (int l = 0; l < n * 2; ++l) {
                        pivot_row[l] = pivot_rows[k * n * 2 + l];
                    }
                }
            }

            // Rescale the pivot row
            for (int k = 0; k < n * 2; ++k) {
                rescaled_pivot_row[k] = pivot_row[k] / pivot_value;
            }
        }

        // Broadcast the adjusted pivot row index and rescaled pivot row to all processes
        MPI_Bcast(&adjusted_pivot_row_idx, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(rescaled_pivot_row, n * 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(row_i, n * 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        if (my_rank == row_i_root && local_n_row > 0) {
            // Update the local matrix with the rescaled pivot row
            for (int j = 0; j < 2 * n; ++j) {
                matrix.mat[(i - startIdx) * n * 2 + j] = rescaled_pivot_row[j];
            }
        }

        // Swap rows if necessary
        if (adjusted_pivot_row_idx != i && adjusted_pivot_row_idx >= startIdx && adjusted_pivot_row_idx <= endIdx) {
            for (int j = 0; j < 2 * n; ++j) {
                matrix.mat[(adjusted_pivot_row_idx - startIdx) * n * 2 + j] = row_i[j];
            }
        }

        // Perform row operations to eliminate the pivot column in other rows
        for (int row = 0; row < local_n_row; ++row) {
            if (row + startIdx != i) {
                double d = matrix.mat[row * 2 * n + i];
                for (int col = 0; col < 2 * n; ++col) {
                    matrix.mat[row * 2 * n + col] -= rescaled_pivot_row[col] * d;
                }
            }
        }

        // Clean up
        delete[] row_i;
        delete[] pivot_row;
        delete[] rescaled_pivot_row;
    }

    // Allocate arrays for receiving and displacing gathered data
    int* recvcounts = new int[num_procs];
    int* displs = new int[num_procs];

    // Calculate recvcounts and displacements for MPI_Gatherv
    displs[0] = 0;
    for (int i = 0; i < num_procs; ++i) {
        recvcounts[i] = procs[i] * matrix.size * 2;  // Number of elements to receive from each process
        if (i > 0) {
            displs[i] = displs[i - 1] + recvcounts[i - 1];  // Cumulative displacement
        }
    }

    // Gather all parts of the matrix into the root process
    MPI_Gatherv(matrix.mat, local_n_row * matrix.size * 2, MPI_DOUBLE,
                matrix.mat, recvcounts, displs, MPI_DOUBLE, 
                0, MPI_COMM_WORLD);

    // Clean up
    delete[] recvcounts;
    delete[] displs;
}

// Function to print the resulting matrix
void print_result(mmatrix& matrix, int my_rank) {
    if (my_rank == 0) {
        // Print the resulting matrix
        // cout << "OUTPUT: " << endl;
        for (int i = 0; i < matrix.size; ++i) {
            for (int j = matrix.size; j < 2 * matrix.size; ++j) {
                // cout << matrix.mat[i * matrix.size * 2 + j] << " ";
                printf("%.2f ", matrix.mat[i * matrix.size * 2 + j]);
            }
            cout << endl;
        }
    }
}

int main(int argc, char** argv) {
    int my_rank, num_procs;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    
    mmatrix matrix;
    int* procs;
    int* proc_prefix_sum;

    // Open the input file
    ifstream input_file(argv[1]);
    if (!input_file) {
        cerr << "Error opening input file!" << endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    double start = MPI_Wtime();
    // Initialize the matrix based on input file
    initialize_matrix(matrix, procs, proc_prefix_sum, my_rank, num_procs, input_file);
    
    // Get the input matrix from the file and initialize the identity matrix
    input_matrix(matrix, procs, my_rank, num_procs, input_file);
    
    // Perform matrix inversion using Gaussian elimination
    parallel_inverse_matrix(matrix, procs, proc_prefix_sum, my_rank, num_procs);
    double end = MPI_Wtime();
    // Print the result
    // print_result(matrix, my_rank);
    
    // Free the memory allocated for the matrix
    deallocate_matrix(matrix);
    
    // Close the input file
    input_file.close();
    
    MPI_Finalize();

    if (my_rank == 0) {
        cout << "Time taken: " << end - start << endl;
    }
    
    return 0;
}
