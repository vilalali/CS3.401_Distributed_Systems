#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include<bits/stdc++.h>
using namespace std;

bool isMandelbrot(double x, double y, int K){
    double zi = 0.0;
    double xk = 0.0, yk = 0.0, cx = x, cy = y;
    double modz = sqrt((x*x)+(y*y));
    if(modz > 2){
        return false;
    }
    for(int k=2;k<=K;k++){
        xk = (x*x) - (y*y) + cx;
        yk = (2*x*y + cy);
        x = xk;
        y = yk;
        modz = sqrt((x*x)+(y*y));
        if(modz > 2){
            return false;
        }
    }
    return true;
}

int main(int argc, char **argv) {
    int size, rank;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int N,M,K;
    double dx,dy;
    int avg_size = 0;
    int *my_arr;
    
    int count=0;
    double starttime;
    if(rank==0){
        cin>>N>>M>>K;
        starttime = MPI_Wtime();    
    }
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&M, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&K, 1, MPI_INT, 0, MPI_COMM_WORLD);

    avg_size = ceill((double)(N*M)/size);
    // printf("rank= %d, size = %d",rank,avg_size);
    my_arr = new int[N*M];
    int result[N*M];
    for(int i=0;i<N*M;i++){
        my_arr[i] = count++;
    }
    
    int localdata[avg_size];

    MPI_Scatter(my_arr, avg_size, MPI_INT, localdata, avg_size, MPI_INT, 0, MPI_COMM_WORLD);
    // each process will execute
    dx = 2.5/(N-1);
    dy = 2.0/(M-1);
    for(int i=0;i<avg_size;i++){
        int n = localdata[i]/M;
        int m = localdata[i]%M;

        double x=0.0,y=0.0;
        x = -1.5 + m*(dx);
        y = 1 - n*(dy);
        
        if(isMandelbrot(x,y,K)){
            localdata[i] = 1;
        }
        else{
            localdata[i] = 0;
        }
    }
    // till here
    MPI_Gather(localdata, avg_size, MPI_INT, result, avg_size, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        int i=0;
        for(int n=0;n<N;n++){
            for(int m=0;m<M;m++){
                cout<<result[i++]<<" ";
            }
            cout<<endl;
        }
        double time=MPI_Wtime() -starttime;
        // cout<<"Time elapsed= "<<time;
    }


    MPI_Finalize();
    return 0;
}