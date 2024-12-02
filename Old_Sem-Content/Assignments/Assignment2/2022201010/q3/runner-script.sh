#!/usr/bin/bash

STREAM_JAR=$1
LOCAL_INP=$2
HDFS_INP=$3
HDFS_OUT=$4
FILES=$5
ITERATIONS=$6

hadoop fs -mkdir -p /dummyOutput/
hdfs dfs -put ${LOCAL_INP} ${HDFS_INP}q3_input
hadoop jar $STREAM_JAR \
-input ${HDFS_INP}q3_input \
-output /dummyOutput/dummyOutput_0 \
-mapper ./mapper0.py \
-reducer ./reducer0.py \
-file ./mapper0.py \
-file ./reducer0.py

for ((i=1; i<=ITERATIONS; i++))
do
    d1=$(expr $i - 1)
    
    hadoop jar $STREAM_JAR \
    -input /dummyOutput/dummyOutput_$d1/part-00000 \
    -output /dummyOutput/dummyOutput_$i \
    -mapper ./mapper1.py \
    -reducer ./reducer1.py \
    -file ./mapper1.py \
    -file ./reducer1.py

done
    
hadoop jar $STREAM_JAR \
-input /dummyOutput/dummyOutput_${ITERATIONS}/part-00000 \
-output ${HDFS_OUT} \
-mapper ./mapper2.py \
-reducer ./reducer2.py \
-file ./mapper2.py \
-file ./reducer2.py

hadoop fs -get ${HDFS_OUT}/part-00000 ${FILES}
hadoop fs -test -d /dummyOutput && hadoop fs -rm -r /dummyOutput