# 2. K Nearest Neighbors (15 points)

In this question we implements a distributed k-Nearest Neighbors (k-NN) system using gRPC. The system allows multiple gRPC servers to hold portions of a dataset, enabling efficient querying of nearest neighbors. The client queries the servers and aggregates the results to find the global k-nearest neighbors.

## Table of Contents

- [Features](#features)
- [Requirements](#requirements)
- [Getting Start](#getting-start)
- [Results](#usage)


## Features

- **Distributed Architecture**: Multiple gRPC servers handle subsets of the dataset for parallel processing.
- **Dynamic Query Handling**: The client efficiently contacts multiple servers and aggregates results.
- **Euclidean Distance Metric**: The system uses Euclidean distance for calculating nearest neighbors.

## Requirements

- python 3.10
- pip packages
- gRPC and Protocol Buffers

## Getting Start
0. Directory Structure:
   ```markdown
   Q2
   ├── client
   │   └── client.py
   ├── server
   │   └── server.py
   ├── generated
   │   ├── knn_pb2_grpc.py
   │   └── knn_pb2.py      
   ├── protofiles
   │   └── knn.proto      
   ├── dataset
   │   ├── dataset_10000.json
   │   ├── dataset_1000.json
   │   ├── dataset_100.json
   │   └── dataset.json
   └── README.md
   └── Report.pdf
   └── req.txt
   ```
## Prerequisites
Before you begin, ensure you have the following installed on your machine:

1. **Python 3.6+**
   - Download and install Python from [python.org](https://www.python.org/downloads/).


## Getting Started:
```
   cd Team_11/Q3
```

### Step 1: Create a Python Virtual Environment and Activate
```bash
# Create a virtual environment
   python -m venv q2MyVenv
```

```bash
# Activate the virtual environment
   source q2MyVenv/bin/activate # For Linux/macOS
# source q2MyVenv\Scripts\activate   # For Windows
```

### Step 2: Install Dependencies
Ensure you have all required Python packages. If you have a `req.txt` file, install the packages:

```bash
   pip install -r req.txt
#or install one by one
   pip install grpcio==1.68.0
   pip install grpcio-tools==1.68.0
   pip install protobuf==5.28.2
```

### Step 3: Prepare Dataset in JSON format
Prepare dataset in JSON format. The dataset should be structured as an array of arrays, where each inner array represents a data point, e.g.:
   ```json
   [
       [1.0, 2.0],
       [2.0, 3.0],
       [3.0, 4.0],
       [1.5, 2.5],
       [4.0, 5.0],
       [5.0, 6.0]
   ]
   ```
   And store the data in the file : `Q2/dataset/dataset.json`

#### Step 4: Compile the Proto File
Make sure you are in root directory, if not then navigate to `Team_11/Q2`.
```bash
python -m grpc_tools.protoc -I=./protofiles --python_out=./generated --grpc_python_out=./generated ./protofiles/knn.proto
```

   ## Usage

   To start the distributed k-NN system, use the following command to run the server:

   ```bash
   python server/server.py <path_to_dataset.json> <number_of_servers>
   ```

   ### Example

   ```bash
   python server/server.py dataset/dataset.json 3
   ```

   After starting the server, you can run the client to query the nearest neighbors:

   ```bash
   python client/client.py <data_point> <k> <server_number>
   ```

   ### Example Client Command

   ```bash
   python client/client.py "[1, 2]" 2 3 
   ```

   ## Results
   ### Command: 
   ```bash
   python server/server.py dataset/dataset.json 2
   ```
   #### Console Output:

   ```
   1-Server Successfully running on port number: 50051
   2-Server Successfully running on port number: 50052

   Dataset Loaded successfully:
   [
      [1.0, 2.0],
      [2.0, 3.0],
      [3.0, 4.0],
      [1.5, 2.5],
      [4.0, 5.0],
      [5.0, 6.0]
   ]

   Server: 50051 loaded the Subset of above data as:
   [
      [1.0, 2.0],
      [2.0, 3.0],
      [3.0, 4.0],
   ]

   Server: 50052 loaded the Subset of above data as:
   [
      [1.5, 2.5],
      [4.0, 5.0],
      [5.0, 6.0]
   ]
   ```

   ### Command: 
   ```bash
   python client/client.py "[1, 2]" 2 3 
   ```

   #### Console Output:

   ```
   **********OUTPUT**********
   The nearest neighbors from Server 50051 are:
   [1.0, 2.0] 
   [2.0, 3.0]

   The nearest neighbors from Server 50052 are:
   [1.5, 2.5] 

   Now, aggregate the results from all servers:
   [1.0, 2.0]
   [1.5, 2.5]
   [2.0, 3.0]

   The 2 nearest neighbors to [1, 2] are:
   [1.0, 2.0]
   [1.5, 2.5]

   So, the nearest neighbors are [1.0, 2.0] and [1.5, 2.5].
   ```
## Demo:
https://github.com/user-attachments/assets/7c90f106-5763-416e-b1f3-598b5253d09b


## Acknowledgments
- [gRPC](https://grpc.io/) - For efficient communication between services.
- [Protocol Buffers](https://developers.google.com/protocol-buffers) - For data serialization.
- Inspiration from the k-NN algorithm literature and distributed computing frameworks.


---
Author: Vilal Ali, Ms By Research IIIT Hyderabad, 2024701027.




