#!/usr/bin/bash

STREAM_JAR=$1
LOCAL_INP=$2
HDFS_INP=$3
HDFS_OUT=$4
FILES=$5


# put files on hdfs
hdfs dfs -put ${LOCAL_INP} ${HDFS_INP}q2_input
hadoop jar $STREAM_JAR \
-input ${HDFS_INP}q2_input \
-output /dummyOutput1/dummyOutput_1 \
-mapper ./mapper0.py \
-reducer ./reducer0.py \
-file ./mapper0.py \
-file ./reducer0.py

hadoop jar $STREAM_JAR \
-input /dummyOutput1/dummyOutput_1/part-00000 \
-output ${HDFS_OUT} \
-mapper ./mapper1.py \
-reducer ./reducer1.py \
-file ./mapper1.py \
-file ./reducer1.py

hadoop fs -get ${HDFS_OUT}/part-00000 ${FILES}
hadoop fs -test -d /dummyOutput1 && hadoop fs -rm -r /dummyOutput1
