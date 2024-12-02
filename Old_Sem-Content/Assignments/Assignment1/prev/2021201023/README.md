# Distributed Systems: ASSIGNMENT 1

## Problem 1:  Mandelbrot Set
**INPUT Description:**   
`M*N` are number points are given . 
`K` indicates number of iteration for each point to find out whether the point is lie in the Mandel brot set or not  .
And `P` is the number of process running parallely.
### 1. Time Complexity
`T(m,n,p,k) = O(((M * N)/P) * K)`  

### 2. Message Complexity
Message complexity =` O(P)` 
### 3. Space Requirements
`S(m,n,p,k) = O(M*N)`

### 4. Performance Scaling
Below table shows the performance scaling for input 
```
16 16 1000
```
| no. of processes | Timetaken  | 
| ---------------- | ---------- | 
| 1                |0.000917946  |
| 2                |0.000633473  |
| 3                |0.000788592 |
| 4                |0.000647427  |
| 5                |0.000558031 |
| 6                |0.000603014  |
| 7                |0.000684014  |
| 8                |0.000634942 |
| 9                |0.000597212  |
| 10               |0.000650552  |
| 11               |0.000666491  |
| 12               |0.000661091 |

-----------------------------------------------------------------------------
## Problem 2: Tony Stark and Pym Particles
**Input Description:**  
`NxM` is the grid size. `K` is the number of pym particles are placed in the grid and `T` indicates number of moves that each particles would take in their respective direction and rules.

### 1. Time Complexity
`T(m,n,k,t) = O(K * T)`
### 2. Message Complexity
Here I have created a custom MPI Datatype message named as ` MPI_PARTICLE` which holds the cordinates `x`,`y` and direction `dir` of a pym particle.

So, message complexity = `O( K * size of MPI_PARTICLE)`
### 3. Space Requirements
`S(m,n,k,t) = O(K)` 
### 4. Performance Scaling
Below table shows the performace scaling for the input 
```
10 12 16 100
0 1 R
2 1 L
3 3 U
9 9 L
6 8 D
4 8 L
7 3 R
5 6 U
0 1 U
2 1 D
3 3 R
9 9 U
6 8 L
4 8 D
7 3 U
5 6 R
```
| no. of processes | Timetaken  | 
| ---------------- | ---------- | 
| 1                |0.00605447  |
| 2                |0.00414707 |
| 3                |0.00402515 |
| 4                |0.00363642  |
| 5                |0.00331802  |
| 6                |0.00401674 |
| 7                |0.00402616  |
| 8                |0.00433609  |
| 9                |0.003775  |
| 10               |0.00326441  |
| 11               |0.00397102  |
| 12               |0.00317863  |
---------------------------------------------------------------------
## Problem 3:  Jar of Words
**Input Description:**  
`N` is the number of nodes in the optimal binary search tree and the next N lines containes node value and frequency of a node.
### Approach :
1. Dividing the given array of freq and key into different processes and then each process will apply the simple sort algorithm on thier parts of the array and send it back to the root process. After that root process will merge all the sorted clustered array into the resultant sorted array.
2. Parallelizing each diagonal calculation to different processes.
3. Each process maintains its own table for optimal binary search tree where each element in the table contains two values one is cost and another is root.
4. Each process calculates part of the diagonal and send the update to the root process which handles all the updation of the main DP table. Again this updated table is sent to all the process for the next diagonal calculation.
5. At each iteration the diagonals decreases and at last we find the ans at the root process.

### 1. Time complexity

* Merge sort time = `(N/P)log(N/P)`
* Time complexity = `O(N^3)`
### 2. Message Complexity
Here I have created a custom MPI datatype named as **MPI_NODE** which is used for implementing a structure data type in the MPI and this datatype have placed for two integer.
* While sorting the array in mergesort algorithm sending a vector to each process. So in this case  
message complexity =  `O(N/P) * (size of MPI_NODE)`
* While collecting diagonals from all processes and scattering them to all process I am using vector of `MPI_NODE`. So in this case,  
message complexity = `O((diagonal length / P) * (size of MPI_NODE))`
### 3. Space Requirements

I am Using an array of structure where each structure node contains node value and frequency And another array of structure in which each node contains cost and parent of the optimal binary search tree.


Space Complexity = `O(N^2)`
### Performance scaling
For below input 
```
8
5 7
3 6
7 5
6 17
1 16
4 8
2 13
8 28
```
| no. of processes | Timetaken  | 
| ---------------- | ---------- | 
| 1                |7.9382e-05  |
| 2                |0.000344708 |
| 3                |0.000934842 |
| 4                |0.000938659 |
| 5                |0.00105418 |
| 6                |0.00141326 |
| 7                |0.0243161 |
| 8                |0.0597142 |
| 9                |0.128652 |
| 10               |0.2682 |
| 11               |0.251979 |
| 12               |0.320427 |