#!/usr/bin/env python3
"""reducer1.py"""

import sys

mutual_friends = {}

for line in sys.stdin:
    line=line.strip()
    key,neighbor=line.split("\t")

    if key not in mutual_friends:
        lst=set()
        lst.add(neighbor)
        mutual_friends[key]=lst
    else:
        mutual_friends[key].add(neighbor)


for key in mutual_friends:
    a=list(mutual_friends[key])
    a=sorted(map(int,a))
    a=map(str,a)
    output=' '.join(a)

    key1,key2=key.split("_")
    print(f"{key1} {key2}\t{output}")
       