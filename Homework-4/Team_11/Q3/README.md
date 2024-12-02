# MyUber Ride-Sharing System

## Table of Contents
1. [Overview](#overview)
2. [Directory Structure](#directory-structure)
3. [Prerequisites](#prerequisites)
2. [Getting Started](#getting-started)
5. [Running the Project](#running-the-project)
6. [Testing the System](#testing-the-system)
7. [Demo](#demo)
---

## Overview
The MyUber Ride-Sharing System is a distributed system designed to handle ride requests securely using gRPC with SSL/TLS. The system allows riders to request rides, drivers to accept or reject them, and provides load balancing across multiple servers.

## Directory Structure

```plaintext
Q3/
├── q3MyVenv
├── certs
│   ├── ca.crt
│   ├── ca.key
│   ├── ca.srl
│   ├── client.crt
│   ├── client.csr
│   ├── client.key
│   ├── server.crt
│   ├── server.csr
│   └── server.key
├── proto
│   └── ridesharing.proto
├── server
│    ├── ridesharing_pb2_grpc.py
│    ├── ridesharing_pb2.py
│    └── ride_sharing_server.py
├── client
│   ├── ride_sharing_client2.py
│   ├── ride_sharing_client.py
│   ├── ridesharing_pb2_grpc.py
│   └── ridesharing_pb2.py
└── req.txt
└── README.md                      # Project overview and setup instructions
└── Report.pdf                     # Project report and implementation details
```

## Prerequisites
Before you begin, ensure you have the following installed on your machine:

1. **Python 3.6+**
   - Download and install Python from [python.org](https://www.python.org/downloads/).

2. **OpenSSL**
   - OpenSSL is required to generate SSL/TLS certificates. You can download and install it from [OpenSSL](https://www.openssl.org/source/).

## Getting Started:
```
   cd Team_11/Q3
```
### Step 1: Create a Python Virtual Environment and Activate
```bash
# Create a virtual environment
   python -m venv q3MyVenv
```
```bash
# Activate the virtual environment
   source q3MyVenv/bin/activate # For Linux/macOS
# source q3MyVenv\Scripts\activate   # For Windows
```

### Step 2: Install Dependencies
Ensure you have all required Python packages. If you have a `req.txt` file, install the packages:

```bash
   pip install -r req.txt
#or install one by one
   pip install grpcio==1.56.0
   pip install grpcio-tools==1.56.0
   pip install protobuf==5.28.2
   pip install certifi==2024.8.30
```

### Step 3: Generate SSL/TLS Certificates
Open a terminal and navigate to the `certs/` directory. Run the following commands to generate the certificates:

#### Step 1: Create a Certificate Authority (CA)

1. **Create the `certs` directory** (if you haven't already):
   ```bash
   mkdir certs
   cd certs
   ```

2. **Generate a private key for the CA**:
   ```bash
   openssl genrsa -out ca.key 2048
   ```

3. **Create a self-signed root certificate**:
   ```bash
   openssl req -x509 -new -nodes -key ca.key -sha256 -days 365 -out ca.crt -subj "/C=IN/ST=Telangana/L=Hyderabad/O=MyCompany/OU=DSQ3/CN=localhost/emailAddress=vilal.ali@research.iiit.ac.in"
   ```

#### Step 2: Create a `Server` Certificate

1. **Generate a private key for the server**:
   ```bash
   openssl genrsa -out server.key 2048
   ```

2. **Create a certificate signing request (CSR) for the server**:
   ```bash
   openssl req -new -key server.key -out server.csr -subj "/C=IN/ST=Telangana/L=Hyderabad/O=MyCompany/OU=DSQ3/CN=localhost/emailAddress=vilal.ali@research.iiit.ac.in"
   ```

3. **Sign the server CSR with the CA**:
   ```bash
   openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 365 -sha256
   ```

#### Step 3: Create a `Client` Certificate

1. **Generate a private key for the client**:
   ```bash
   openssl genrsa -out client.key 2048
   ```

2. **Create a certificate signing request (CSR) for the client**:
   ```bash
   openssl req -new -key client.key -out client.csr -subj "/C=IN/ST=Telangana/L=Hyderabad/O=MyCompany/OU=DSQ3/CN=localhost/emailAddress=vilal.ali@research.iiit.ac.in"
   ```

3. **Sign the client CSR with the CA**:
   ```bash
   openssl x509 -req -in client.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out client.crt -days 365 -sha256
   ```

### Step 4: Verify the Certificates

1. **Verify the server certificate**:
   ```bash
   openssl verify -CAfile ca.crt server.crt
   ## Expected Output:
      certs/server.crt: OK
   ```

2. **Verify the client certificate**:
   ```bash
   openssl verify -CAfile ca.crt client.crt
      ## Expected Output:
      certs/client.crt: OK
   ```

#### Step 5: Compile the Proto File
###### Make sure you are in root directory, if not then navigate to `Team_11/Q3`.
1. **Generate code in the `server` directory**:

```bash
python -m grpc_tools.protoc -Iproto/ --python_out=server/ --grpc_python_out=server/ proto/ridesharing.proto
```
2. **Generate code in the `client` directory**:
```bash
python -m grpc_tools.protoc -Iproto/ --python_out=client/ --grpc_python_out=client/ proto/ridesharing.proto
```

## Running the Project
### 1. Start the gRPC Server
Open a terminal and navigate to the `Team_11/Q3` directory. Run the following command to start the gRPC server:

```bash
#python <path for server side script> <Port Number>
```

```bash
## Example:
   python server/ride_sharing_server.py 50051
   python server/ride_sharing_server.py 50052
   python server/ride_sharing_server.py 50053
   .
   .
   .
## Server Started as:
   Server started on port 50051
   Server started on port 50052
   Server started on port 50053
```

### 2. Start the Client
Open another terminal and navigate to the `Team_11/Q3` directory. Run the following command to start the client:
```bash
# python <path for client side script> <Port Number>
```
```bash
## Example:
   python client/ride_sharing_client.py
```
```bash
#The client will connect to the server and client can start requesting rides.
```

## Testing the System
Once the server and client are running, you can test the following scenarios:
1. **Rider Requests a Ride**: Use the client to send a ride request with a pickup location and destination.
2. **Driver Accepts/Rejection**: Simulate driver actions to accept or reject ride requests.
3. **Ride Completion**: Test the completion of rides to ensure status updates are correctly handled.

## Demo:
##### Watch the Demo Video:
[![Q3 Demo](https://img.icons8.com/ios-glyphs/30/000000/play-button-circled.png)](./Q3_Demo.mp4 "Q3 Demo Video")

---
Author: Vilal Ali, Ms By Research IIIT Hyderabad, 2024701027.