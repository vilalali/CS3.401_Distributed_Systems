#include<bits/stdc++.h>
#include<mpi.h>
using namespace std;

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);

    int mpisize, mpirank;

    MPI_Comm_size(MPI_COMM_WORLD, &mpisize);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpirank);

    auto mpiroot = 0;

    int nRows;
    int nCols;
    int nTime;
    vector<vector<int>> matrix, mymatrix;
    if (mpirank == mpiroot)
    {
        cin >> nRows;
        cin >> nCols;
        cin >> nTime;
        matrix.resize(nRows, vector<int>(nCols, 0));

        for (auto iRow = 0; iRow < nRows; iRow++)
        {
            for (auto iCol = 0; iCol < nCols; iCol++)
            {
                int a;
                cin >> a;
                matrix[iRow][iCol] = a;
            }
        }
        cout<<endl;
    }

    MPI_Bcast(&nRows, 1, MPI_INT, mpiroot, MPI_COMM_WORLD);
    MPI_Bcast(&nCols, 1, MPI_INT, mpiroot, MPI_COMM_WORLD);
    MPI_Bcast(&nTime, 1, MPI_INT, mpiroot, MPI_COMM_WORLD);

    


    auto nRowsLocal = nRows / mpisize;
    if (mpirank == mpisize - 1)
    {
        nRowsLocal += nRows % mpisize;
    }

    auto nRowsLocalWithGhost = nRowsLocal + 2;
    auto nColsWithGhost = nCols + 2;

    vector<vector<int>> currGrid(nRowsLocalWithGhost, vector<int>(nColsWithGhost, 0));
    vector<vector<int>> nextGrid(nRowsLocalWithGhost, vector<int>(nColsWithGhost, 0));
    if (mpirank == mpiroot)
    {
        mymatrix.resize(nRows, vector<int>(nCols, 0));
        for (auto iRow = 0; iRow < nRows; iRow++)
        {
            for (auto iCol = 0; iCol < nCols; iCol++)
            {
                mymatrix[iRow][iCol] = matrix[iRow][iCol];
            }
        }
        /*cout << "mpisize : " << mpisize << endl;
        cout << "nRowsLocal: " << nRowsLocal<<endl;*/
        for (int p=1;p<mpisize;p++)
        {
            for (auto iRow = 0; iRow < nRows; iRow++)
            {
                MPI_Send(&matrix[iRow][0], nCols, MPI_INT, p, 0, MPI_COMM_WORLD);
                //cout << nCols<<" Sent successfully from 0 to " << p << endl;
            }
        }
    }
    else
    {
        mymatrix.resize(nRows, vector<int>(nCols, 0));
        for (auto iRow = 0; iRow < nRows; iRow++)
        {
            MPI_Recv(&mymatrix[iRow][0], nCols, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            //cout << "Received successfully from " << mpirank <<" from 0"<< endl;
        }
    }
    for (auto iRow = 1; iRow <= nRowsLocal; iRow++)
    {
        for (auto iCol = 1; iCol <= nCols; iCol++)
        {
            if (mpirank == mpisize - 1)
            {
                int newsize = nRows / mpisize;
                currGrid[iRow][iCol] = mymatrix[mpirank * newsize + iRow - 1][iCol - 1];
            }
            else
            {
                currGrid[iRow][iCol] = mymatrix[mpirank * nRowsLocal + iRow - 1][iCol - 1];
            }
        }
    }


    auto upperNeighbor = (mpirank == 0) ? mpisize - 1 : mpirank - 1;
    auto lowerNeighbor = (mpirank == mpisize - 1) ? 0 : mpirank + 1;

    int ALIVE = 1;
    int DEAD = 0;

    for (auto iTime = 0; iTime <= nTime; iTime++)
    {
        if (mpirank < mpisize && mpisize>1)
        {
            MPI_Send(&currGrid[1][0], nColsWithGhost, MPI_INT, upperNeighbor, 0, MPI_COMM_WORLD);
            MPI_Send(&currGrid[nRowsLocal][0], nColsWithGhost, MPI_INT, lowerNeighbor, 0, MPI_COMM_WORLD);

            MPI_Recv(&currGrid[nRowsLocal + 1][0], nColsWithGhost, MPI_INT, lowerNeighbor, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            MPI_Recv(&currGrid[0][0], nColsWithGhost, MPI_INT, upperNeighbor, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        if (mpirank == mpiroot)
        {
            for (int j = 0; j < nColsWithGhost; j++)
            {
                currGrid[0][j] = 0;
            }
        }
        if (mpirank == mpisize-1)
        {
            for (int j = 0; j < nColsWithGhost; j++)
            {
                currGrid[nRowsLocal+1][j] = 0;
            }
        }

        
        for (auto iRow = 0; iRow < nRowsLocalWithGhost; iRow++)
        {
            currGrid[iRow][0] = 0;
            currGrid[iRow][nCols + 1] = 0;
        }

        // display

        if (mpirank != mpiroot)
        {
            for (int iRow = 1; iRow <= nRowsLocal; iRow++)
            {
                MPI_Send(&currGrid[iRow][1], nCols, MPI_INT, mpiroot, 0, MPI_COMM_WORLD);
            }
        }

        if (mpirank == mpiroot)
        {
            if (nTime == iTime)
            {
                //cout << "***Output after tTime " << iTime << "***" << endl;
                for (auto iRow = 1; iRow <= nRowsLocal; iRow++)
                {
                    for (auto iCol = 1; iCol <= nCols; iCol++)
                    {
                        cout << currGrid[iRow][iCol] << " ";
                    }
                    cout << endl;
                }
            }

            for (auto sourceRank = 1; sourceRank < mpisize; sourceRank++)
            {
                auto nRecv = nRows / mpisize;
                if (sourceRank == mpisize - 1)
                {
                    nRecv += nRows % mpisize;
                }

                vector<int> buff(nCols, 0);

                for (auto iRecv = 0; iRecv < nRecv; iRecv++)
                {
                    MPI_Recv(&buff[0], nCols, MPI_INT, sourceRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                    if (nTime == iTime)
                    {
                        for (auto a : buff)
                        {
                            cout << a << " ";
                        }
                        cout << endl;
                    }
                }
            }
        }

        // update

        for (auto iRow = 1; iRow <= nRowsLocal; iRow++)
        {
            for (auto iCol = 1; iCol <= nCols; iCol++)
            {
                auto nAliveNeighbors = 0;

                for (auto jRow = iRow - 1; jRow <= iRow + 1; jRow++)
                {
                    for (auto jCol = iCol - 1; jCol <= iCol + 1; jCol++)
                    {
                        if ((jRow != iRow || jCol != iCol) && currGrid[jRow][jCol] == ALIVE)
                        {
                            ++nAliveNeighbors;
                        }
                    }
                }

                if (nAliveNeighbors < 2)
                {
                    nextGrid[iRow][iCol] = DEAD;
                }

                if (currGrid[iRow][iCol] == ALIVE && (nAliveNeighbors == 2 || nAliveNeighbors == 3))
                {
                    nextGrid[iRow][iCol] = ALIVE;
                }
                if (nAliveNeighbors > 3)
                {
                    nextGrid[iRow][iCol] = DEAD;
                }
                if (currGrid[iRow][iCol] == DEAD && nAliveNeighbors == 3)
                {
                    nextGrid[iRow][iCol] = ALIVE;
                }
            }
        }

        for (auto iRow = 1; iRow <= nRowsLocal; ++iRow)
        {
            for (auto iCol = 1; iCol <= nCols; iCol++)
            {
                currGrid[iRow][iCol] = nextGrid[iRow][iCol];
            }
        }
    }

    MPI_Finalize();
    return 0;
}