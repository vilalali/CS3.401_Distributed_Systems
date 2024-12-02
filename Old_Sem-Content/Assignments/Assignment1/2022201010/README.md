### Q1-B ###
**Input Description:**

The program takes a single constant N (the size of the chessboard) as input.

#### Time and Space Complexity
The total time complexity of your approach : O(N!)

The total message complexity of your approach : O(N)

The space requirements of your solution : O(N*N)

*Performance scaling:*


| no. of processes | Timetaken(in seconds)  | n | 
| ---------------- | ---------- | ---------------- |
| 1                |0.1170 s  | 12 |
| 2                |0.0989 s  | 12 |
| 3                |0.0989 s | 12 |
| 4                |0.0985 s  | 12 |
| 5                |0.0608 s | 12 |
| 6                |0.1188 s  | 12 |
| 7                |0.0982 s  | 12 |
| 8                |0.0815 s | 12 |
| 9                |0.0596 s  | 12 |
| 10               |0.0398 s  | 12 |
| 11               |0.0351 s  | 12 |
| 12               |0.2082 s | 12 |

----------------------------------------------------------------------------

### Q2 ###

**Input Description:**

The first line contains a single integer N, the number of vertices in the graph. Each of the next N lines contain N space separated integers. These represent the N×N adjacency matrix of the graph. `A[i][j]` = weight of edge from node i to node j. (It will be −1 if there exists no edge from node i to node j)

#### Time and Space Complexity

The total time complexity of your approach : O(N^2)

The total message complexity of your approach : O(N*N)

The space requirements of your solution : O(N*N)

*Performance Scaling:*

```
n=50
```

| no. of processes | Timetaken  | 
| ---------------- | ---------- | 
| 1                |6.4645 ms  |
| 2                |4.1082 ms  |
| 3                |3.4466 ms |
| 4                |3.8028 ms  |
| 5                |5.0154 ms |
| 6                |4.8069 ms  |
| 7                |4.9601 ms  |
| 8                |6.1135 ms |
| 9                |8.3082 ms  |
| 10               |8.8351 ms  |
| 11               |9.8386 ms  |
| 12               |9.8623 ms |

----------------------------------------------------------------------------

### Q3 ###

**Input Description:**

The first line of input contains three space separated integers N,M, T representing the size of the grid and the number of generations to simulate. The next N lines would contain M space separated integers on each line, representing a grid of N rows andM columns. A value of 0 in this grid represents a “dead” cell and a value of 1 represents an “alive” cell.

#### Time and Space Complexity

The total time complexity of your approach : O(T*N)

The total message complexity of your approach : O(T*N)

The space requirements of your solution : O(N*N)

*Performance scaling:*

```
n: 100 
m: 100 
t: 100
```

| no. of processes | Timetaken  | 
| ---------------- | ---------- | 
| 1                |38.7458 ms  |
| 2                |20.2188 ms  |
| 3                |18.1674 ms |
| 4                |21.3917 ms  |
| 5                |18.6894 ms |
| 6                |18.7038 ms  |
| 7                |20.7438 ms  |
| 8                |21.3846 ms |
| 9                |26.7816 ms  |
| 10               |23.3443 ms  |
| 11               |18.2314 ms  |
| 12               |28.1201 ms |