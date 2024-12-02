#include<bits/stdc++.h>
#include "mpi.h"
using namespace std;

#define send MPI_Send
#define receive MPI_Recv


int provideOwner(int numberOfRowsPerProcess, int node)
{
    int ownerRow = node / numberOfRowsPerProcess;
    return ownerRow;
}

void floydWarshall(int n, vector<vector<long long>>& processLocalGraph, int mpirank, int numberOfRowsPerProcess, int size)
{
    for (int k = 0; k < n; k++)
    {
        if (((mpirank * numberOfRowsPerProcess <= k) && (mpirank * numberOfRowsPerProcess + numberOfRowsPerProcess > k)) || ((mpirank == size - 1) && (mpirank * numberOfRowsPerProcess <= k) && (mpirank * numberOfRowsPerProcess + numberOfRowsPerProcess + n % size > k)))
        {
            for (int l = 0; l < size; l++)
            {
                if (l != mpirank)
                {
                    send(&processLocalGraph[k - mpirank * numberOfRowsPerProcess][0], n, MPI_LONG_LONG, l, 0, MPI_COMM_WORLD);
                }
            }
            if (mpirank != size - 1)
            {
                for (int i = 0; i < numberOfRowsPerProcess; i++)
                {
                    for (int j = 0; j < n; j++)
                    {
                        processLocalGraph[i][j] = min(processLocalGraph[i][j], processLocalGraph[i][k] + processLocalGraph[k - numberOfRowsPerProcess * mpirank][j]);
                    }
                }
            }
            else 
            {
                for (int i = 0; i < numberOfRowsPerProcess + n % size; i++)
                {
                    for (int j = 0; j < n; j++)
                    {
                        processLocalGraph[i][j] = min(processLocalGraph[i][j], processLocalGraph[i][k] + processLocalGraph[k - numberOfRowsPerProcess * mpirank][j]);
                    }
                }
            }
        }
        else
        {
            int ownerNode = provideOwner(numberOfRowsPerProcess, k);

            vector<vector<long long>> tempgraph(1, vector<long long>(n));
            
            if (ownerNode >= size)
            {
                ownerNode = size - 1;
            }

            receive(tempgraph[0].data(), n, MPI_LONG_LONG, ownerNode, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (mpirank != size - 1)
            {
                for (int i = 0; i < numberOfRowsPerProcess; i++)
                {
                    for (int j = 0; j < n; j++)
                    {
                        processLocalGraph[i][j] = min(processLocalGraph[i][j], processLocalGraph[i][k] + tempgraph[0][j]);
                    }
                }
            }
            else 
            {
                for (int i = 0; i < numberOfRowsPerProcess + n % size; i++)
                {
                    for (int j = 0; j < n; j++)
                    {
                        processLocalGraph[i][j] = min(processLocalGraph[i][j], processLocalGraph[i][k] + tempgraph[0][j]);
                    }
                }
            }
        }
    }


    if (mpirank != 0)
    {
        if (mpirank != size - 1)
        {
            for (int i = 0; i < numberOfRowsPerProcess; i++)
            {
                send(&processLocalGraph[i][0], n, MPI_LONG_LONG, 0, 0, MPI_COMM_WORLD);
            }
        }
        else 
        {
            for (int i = 0; i < numberOfRowsPerProcess + n % size; i++)
            {
                send(&processLocalGraph[i][0], n, MPI_LONG_LONG, 0, 0, MPI_COMM_WORLD);
            }
        }
    }
}

int cellsPerProcess(int n, int size)
{
    return n / size;
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int mpisize, mpirank;

    MPI_Comm_size(MPI_COMM_WORLD, &mpisize);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpirank);

    auto mpiroot = 0;
    int n, size;
    if (mpisize == 1)
    {
        if (mpirank == mpiroot)
        {
            cin >> n;
            int i, j;
            vector<vector<long long>> graphDistance(n, vector<long long>(n));

            for (i = 0; i < n; ++i)
            {
                for (j = 0; j < n; ++j)
                {
                    cin >> graphDistance[i][j];
                    if (graphDistance[i][j] == -1)
                    {
                        graphDistance[i][j] = 1e9 + 1;
                    }
                }
            }
            for (int k = 0; k < n; k++)
            {
                for (int i = 0; i < n; i++)
                {
                    for (int j = 0; j < n; j++)
                    {
                        graphDistance[i][j] = min(graphDistance[i][j], graphDistance[i][k] + graphDistance[k][j]);
                    }
                }
            }

            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < n; ++j)
                {
                    if (graphDistance[i][j] > 1e9)
                    {
                        cout << "-1" << "\t";
                        continue;
                    }

                    cout << graphDistance[i][j] << "\t";
                }
                cout << endl;
            }
            cout << endl;

        }
    }
    else
    {
        if (mpirank == mpiroot)
        {
            cin >> n;
            if (mpisize > n)
            {
                size = n;
            }
            else
            {
                size = mpisize;
            }
        }

        MPI_Bcast(&size, 1, MPI_INT, mpiroot, MPI_COMM_WORLD);
        MPI_Bcast(&n, 1, MPI_INT, mpiroot, MPI_COMM_WORLD);

        int numberOfRowsPerProcess = cellsPerProcess(n, size);
        
        if (mpirank == mpiroot)
        {
            vector<vector<long long>> graphDistance(n, vector<long long>(n));
            int i, j;
            for (i = 0; i < n; ++i)
            {
                for (j = 0; j < n; ++j)
                {
                    cin >> graphDistance[i][j];
                    if (graphDistance[i][j] == -1)
                    {
                        graphDistance[i][j] = 1e9 + 1;
                    }
                }
            }
            
            for (int i = 1; i < size - 1; i++)
            {
                for (int j = 0; j < numberOfRowsPerProcess; j++)
                {
                    send(&graphDistance[i * numberOfRowsPerProcess + j][0], n, MPI_LONG_LONG, i, 0, MPI_COMM_WORLD);
                }
            }
            
            for (int j = 0; j < numberOfRowsPerProcess + n % size; j++)
            {
                send(&graphDistance[numberOfRowsPerProcess * (size - 1) + j][0], n, MPI_LONG_LONG, size - 1, 0, MPI_COMM_WORLD);
            }

            vector<vector<long long>> processLocalGraph(numberOfRowsPerProcess, vector<long long>(n));

            for (int i = 0; i < numberOfRowsPerProcess; i++)
            {
                for (int j = 0; j < n; j++)
                    processLocalGraph[i][j] = graphDistance[i][j];
            }

            floydWarshall(n, processLocalGraph, mpirank, numberOfRowsPerProcess, size);

            vector<vector<long long>> temp_recv_graph(numberOfRowsPerProcess, vector<long long>(n));
            for (int i = 1; i < size - 1; i++)
            {
                for (int j = 0; j < numberOfRowsPerProcess; j++)
                    receive(temp_recv_graph[j].data(), n, MPI_LONG_LONG, i, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                for (int k = i * numberOfRowsPerProcess, e = 0; k < i * numberOfRowsPerProcess + numberOfRowsPerProcess, e < numberOfRowsPerProcess; k++, e++)
                {
                    for (int l = 0; l < n; l++)
                    {
                        graphDistance[k][l] = temp_recv_graph[e][l];
                    }
                }
            }

            vector<vector<long long>> temp_recv_graph2(numberOfRowsPerProcess + n % size, vector<long long>(n));

            for (int i = 0; i < numberOfRowsPerProcess + n % size; i++)
            {
                receive(temp_recv_graph2[i].data(), n, MPI_LONG_LONG, size - 1, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }

            for (int k = numberOfRowsPerProcess * (size - 1), e = 0; k < (size - 1) * numberOfRowsPerProcess + numberOfRowsPerProcess + (n % size), e < numberOfRowsPerProcess + (n % size); k++, e++)
            {
                for (int j = 0; j < n; j++)
                {
                    graphDistance[k][j] = temp_recv_graph2[e][j];
                }
            }

            for (int i = 0; i < numberOfRowsPerProcess; i++)
            {
                for (int j = 0; j < n; j++)
                    graphDistance[i][j] = processLocalGraph[i][j];
            }

            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < n; ++j)
                {
                    if (graphDistance[i][j] > 1e9)
                    {
                        cout << "-1" << "\t";
                        continue;
                    }
                    cout << graphDistance[i][j] << " ";
                }
                cout << endl;
            }
            cout << endl;
        }
        else if (mpirank < size) 
        {

            if (mpirank != size - 1)
            {
                vector<vector<long long>> processLocalGraph(numberOfRowsPerProcess, vector<long long>(n));
                for (int i = 0; i < numberOfRowsPerProcess; i++)
                {
                    receive(processLocalGraph[i].data(), n, MPI_LONG_LONG, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
                floydWarshall(n, processLocalGraph, mpirank, numberOfRowsPerProcess, size);
            }
            else
            {
                vector<vector<long long>> processLocalGraph(numberOfRowsPerProcess + n % size, vector<long long>(n));
                for (int i = 0; i < numberOfRowsPerProcess + n % size; i++)
                {
                    receive(processLocalGraph[i].data(), n, MPI_LONG_LONG, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
                floydWarshall(n, processLocalGraph, mpirank, numberOfRowsPerProcess, size);
            }
        }
    }

    MPI_Finalize();
    return 0;
}