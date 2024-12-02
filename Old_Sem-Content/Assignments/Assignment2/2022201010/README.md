### Q2 ###
**Input Description:**

There are N people in IIITH currently. You are given information about the existing friendships. You wonder for each pair of people, who are the mutual friends.
Formally, you are given input where each line contains two space-separated integers u v, such that 1 ≤ u < v ≤ N, indicating that u, v are friends. This means that u is a friend of v and vice-versa.
It is guaranteed that there won’t be more than nC2. number of input lines. For each pair of people x, y (with x < y) having atleast one mutual friend, you are required to output one line of the form x y<tab-character><space-separated list of one or more mutual friends of x and y in sorted order>.


**Command to run runner script:**
```
chmod +x runner-script.sh

bash runner-script.sh {STREAM_JAR} {LOCAL_INP} {HDFS_INP} {HDFS_OUT} {FILES}

Ex : bash runner-script.sh ~/hadoop-3.3.6/share/hadoop/tools/lib/hadoop-streaming-3.3.6.jar /home/aman/Q2/input.txt /Q2/ /Q2/output /home/aman/Q2/output
```

- STREAM_JAR - path of STREAM JAR file
- LOCAL_INP - path of local input file
- HDFS_INP - path where you want to store input file in HDFS [condition: path should be correct or should exist in HDFS Filesystem]
- HDFS_OUT - path where you want to store the final output in HDFS Filesystem
- FILES - local path where you want to store the final output file from HDFS filesystem 


----------------------------------------------------------------------------

### Q3 ###

**Input Description:**

Consider a large-scale graph represented as an adjacency list, where each node has a unique identifier in the form of a positive integer and is connected to other nodes with undirected, unweighted edges (that is, each edge has a weight of 1). Your task is to output the shortest distance from node 1 to each node that is at most a distance of 10 from the node 1 using Map-Reduce. The input will be an adjacency list of the graph. So for each node x in the graph, there will be a line of input of the format x<tab-character><list of neighbours of node x>. Note that the graph is undirected; this means that if node A appears as a neighbour of B, then it is guaranteed that B also appears as a neighbour of A. For each node u that is at a distance of at most 10 from node 1, output a line of the form <tab-character><shortest-distance-from-node-1>

**Command to run runner script:**
```
chmod +x runner-script.sh

bash runner-script.sh {STREAM_JAR} {LOCAL_INP} {HDFS_INP} {HDFS_OUT} {FILES} {ITERATIONS}

Ex: bash runner-script.sh ~/hadoop-3.3.6/share/hadoop/tools/lib/hadoop-streaming-3.3.6.jar /home/aman/Q3/input.txt /Q3/ /Q3/output /home/aman/Q3/output 10
```

- STREAM_JAR - path of STREAM JAR file
- LOCAL_INP - path of local input file
- HDFS_INP - path where you want to store input file in HDFS [condition: path should be correct or should exist in HDFS Filesystem]
- HDFS_OUT - path where you want to store the final output in HDFS Filesystem
- FILES - local path where you want to store the final output file from HDFS filesystem 
- ITERATIONS - number of iterations you want to run

----------------------------------------------------------------------------