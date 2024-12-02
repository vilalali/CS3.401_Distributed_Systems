#include<bits/stdc++.h>
#include <mpi.h>

using namespace std;

bool isPossible(int row, int col, int n, vector<string>& chessboard) {
    int duplicaterow = row;
    int duplicatecol = col;

    while (row >= 0 && col >= 0) {
        if (chessboard[row][col] == 'Q')
            return false;
        row--;
        col--;
    }

    row = duplicaterow;
    col = duplicatecol;
    while (col >= 0) {
        if (chessboard[row][col] == 'Q')
            return false;
        col--;
    }

    row = duplicaterow;
    col = duplicatecol;
    while (row < n && col >= 0) 
    {
        if (chessboard[row][col] == 'Q')
        {
            return false;
        }
        row++;
        col--;
    }
    return true;
}

int helper(int col, vector <string>& chessboard, int n) {
    if (col == n) {
        return 1;
    }
    int noOfWays = 0;
    for (int row = 0; row < n; row++) {
        if (isPossible(row, col, n, chessboard)) {
            chessboard[row][col] = 'Q';
            noOfWays = noOfWays + helper(col + 1, chessboard, n);
            chessboard[row][col] = '.';
        }
    }
    return noOfWays;
}

int solveNQueens(int n, int row) 
{
    vector<string> chessboard(n);
    string temp(n, '0');
    for (int i = 0; i < n; i++) 
    {
        chessboard[i] = temp;
    }

    int noOfWays = 0;

    if (isPossible(row, 0, n, chessboard))
    {
        chessboard[row][0] = 'Q';
        noOfWays = noOfWays + helper(0 + 1, chessboard, n);
        chessboard[row][0] = '0';
    }
    return noOfWays;
}

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    int root_rank = 0;
    double starttime = 0.0;
    int mpisize, mpirank;

    MPI_Comm_size(MPI_COMM_WORLD, &mpisize);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpirank);
    int n;

    if (mpirank == root_rank)
    {
        cin >> n;
        starttime = MPI_Wtime();
    }

    MPI_Bcast(&n, 1, MPI_INT, root_rank, MPI_COMM_WORLD);

    int rows_per_process = n / mpisize;

    int noOfWays = 0;
    for (int i = 0; i < rows_per_process; i++)
    {
        noOfWays += solveNQueens(n, mpirank * rows_per_process + i);
    }
    if (n % mpisize != 0 && mpirank == mpisize - 1)
    {
        for (int i = 0; i < (n % mpisize); i++)
        {
            noOfWays = noOfWays + solveNQueens(n, (mpirank + 1) * rows_per_process + i);
        }
    }
    if (mpirank == root_rank)
    {
        int noOfWaysPlacingAQueen = 0;
        MPI_Reduce(&noOfWays, &noOfWaysPlacingAQueen, 1, MPI_INT, MPI_SUM, root_rank, MPI_COMM_WORLD);
        cout<< noOfWaysPlacingAQueen <<endl;
        double time = MPI_Wtime() - starttime;
        //cout << time << endl;
    }
    else
    {
        MPI_Reduce(&noOfWays, NULL, 1, MPI_INT, MPI_SUM, root_rank, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}