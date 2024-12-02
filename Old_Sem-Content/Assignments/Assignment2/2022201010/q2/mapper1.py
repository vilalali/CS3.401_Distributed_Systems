#!/usr/bin/env python3
"""mapper1.py"""

import sys

for line in sys.stdin:
    line = line.strip().split("\t")
    node = line[0]
    neighbor = line[1]

    neighbor_list=neighbor.split(' ')

    for i in range(0,len(neighbor_list)-1):
        for j in range(i+1,len(neighbor_list)):
            a=int(neighbor_list[i])
            b=int(neighbor_list[j])

            lst=[a,b]
            lst.sort()
            key=""
            key+=str(lst[0])+"_"+str(lst[1])

            print("{}\t{}".format(key, node))
        
