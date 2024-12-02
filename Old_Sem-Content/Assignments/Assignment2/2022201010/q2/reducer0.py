#!/usr/bin/env python3
"""reducer0.py"""

import sys

neighbor_nodes={}

for line in sys.stdin:
    friend_1, friend_2 = line.strip().split('\t')

    if friend_1 in neighbor_nodes:
        neighbor_nodes[friend_1].add(friend_2)
    else:
        lst=set()
        lst.add(friend_2)
        neighbor_nodes[friend_1] = lst

for key in neighbor_nodes:
    lst = list(neighbor_nodes[key])
    sorted_nodes = sorted(map(int, lst))
    neighbor_values_str = ' '.join(map(str, sorted_nodes)) 
    print("{}\t{}".format(key, neighbor_values_str))
