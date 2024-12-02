import sys
sys.path.insert(0, './generated')
import json
import grpc
from concurrent import futures
import knn_pb2  # Import your generated protobuf classes
import knn_pb2_grpc  # Import your generated gRPC classes
import threading


class KNNService(knn_pb2_grpc.KNNServiceServicer):
    def __init__(self, data, port):
        self.data = data
        self.port = port  # Store the server port

    def FindNearest(self, request, context):
        data_point = request.data_point  # Assuming it's a repeated float
        k = request.k  # Number of nearest neighbors to return
        
        # Log the received request with the server port
        print(f"Server {self.port} received request for data_point: {data_point}, k: {k}")

        # Find the nearest neighbors
        nearest_neighbors = self.find_nearest_neighbors(data_point, k)

        # Log the nearest neighbors found
        print(f"Server {self.port} nearest neighbors found: {nearest_neighbors}")

        # Create the response object
        response = knn_pb2.KNNResponse()
        for neighbor in nearest_neighbors:
            response.nearest_neighbors.add(coordinates=neighbor)  # Assuming coordinates is a repeated field

        return response

    def find_nearest_neighbors(self, data_point, k):
        # Placeholder logic for finding nearest neighbors
        distances = []
        for entry in self.data:
            dist = sum((entry[i] - data_point[i]) ** 2 for i in range(len(data_point))) ** 0.5
            distances.append((dist, entry))
        distances.sort(key=lambda x: x[0])  # Sort by distance
        return [neighbor for _, neighbor in distances[:k]]  # Return the k nearest neighbors


def start_server(port, data):
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    knn_pb2_grpc.add_KNNServiceServicer_to_server(KNNService(data, port), server)

    # Attempt to bind to the specified port, incrementing if it fails
    while True:
        try:
            server.add_insecure_port(f'[::]:{port}')
            break  # Exit loop if binding is successful
        except RuntimeError as e:
            print(f"\nPort {port} is already in use. Trying next port.")
            port += 1  # Increment port and try again

    server.start()
    #print(f'Server started on port {port}')
    server.wait_for_termination()  # This line will block, but it can be in a thread


def main():
    if len(sys.argv) != 3:
        print("Usage: python server.py <path_to_dataset.json> <number_of_servers>")
        return

    dataset_path = sys.argv[1]
    number_of_servers = int(sys.argv[2])

    # Load dataset
    with open(dataset_path, 'r') as f:
        data = json.load(f)

    # Print the loaded dataset
    print("Dataset Loaded successfully:")
    print(data)

    # Split data into subsets
    subset_size = len(data) // number_of_servers
    servers = []
    used_ports = []  # List to track used ports

    for i in range(number_of_servers):
        port = 50051 + i  # Assign different ports for each server
        server_data = data[i * subset_size:(i + 1) * subset_size]  # Fix the indexing
        servers.append((port, server_data))
        used_ports.append(port)  # Track the used port

        # Print the subset loaded by each server
        print(f'\nServer started on port {port}')
        print(f"Server: {port} loaded the Subset of above data as:")
        print(server_data)

    # Start each server in a separate thread
    print("\n")
    threads = []
    for port, server_data in servers:
        thread = threading.Thread(target=start_server, args=(port, server_data))
        thread.start()  # Start the server thread
        threads.append(thread)  # Keep track of the threads

    # Print the used ports in sorted order
    print(f'\nServer started on port: {sorted(used_ports)}\n')

    # Optionally join threads if you want to wait for them to finish
    for thread in threads:
        thread.join()  # This will block until all threads are done, which won't happen here

if __name__ == '__main__':
    main()
