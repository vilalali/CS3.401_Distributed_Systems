#!/usr/bin/env python3
"""reducer0.py"""

import sys

curr_node = None
curr_neighbors = []
firstNode = "1"
count = 0

def helper(node, neighbors, first_node=False):
    neighbors = sorted(neighbors, key=lambda y: y[0])
    path = []
    for (nb, dist) in neighbors:
        path.append('{}:{}'.format(nb, dist))
    
    distance = 9999
    if first_node:
        distance = 0
    
    print('{}\t{}\t{}'.format(node, distance,','.join(path)))

for line in sys.stdin:
    line = line.strip().split('\t')
    node, adjNode = [str(i) for i in line]
    count += 1

    if count == 1:
        firstNode = node
        
    if node == curr_node:
        curr_neighbors.append((adjNode, str(1)))
    else:
        if curr_node:
            helper(curr_node, curr_neighbors, curr_node == firstNode)
        curr_node = node
        curr_neighbors = [(adjNode, 1)]

helper(curr_node, curr_neighbors)
