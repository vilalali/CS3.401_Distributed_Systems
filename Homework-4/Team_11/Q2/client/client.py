import sys
sys.path.insert(0, './generated')
import grpc
import knn_pb2
import knn_pb2_grpc
import json


def run(data_point, k, number_of_servers):
    data_point = json.loads(data_point)
    k = int(k)

    all_neighbors = []
    # List to store neighbors from each server
    neighbors_from_all_servers = []

    # Query each server
    for i in range(number_of_servers):
        port = 50051 + i
        channel = grpc.insecure_channel(f'localhost:{port}')
        stub = knn_pb2_grpc.KNNServiceStub(channel)

        # Create a request
        request = knn_pb2.KNNRequest(data_point=data_point, k=k)
        
        # Log the request to the server
        print(f"\nSending request to server running on port {port} with data_point: {data_point} and k: {k}")
        
        try:
            response = stub.FindNearest(request)

            # Extend the neighbors with the received neighbors
            neighbors_from_server = [list(dp.coordinates) for dp in response.nearest_neighbors]
            neighbors_from_all_servers.append(neighbors_from_server)

            # Log the received neighbors from this server
            print(f"Received neighbors from server {port}:\n{neighbors_from_server}")
        except grpc.RpcError:
            # Skip this server if there's an error and print a simplified message
            print(f"Server on port {port} is unavailable, skipping...")

    # Combine all neighbors from all servers
    for server_neighbors in neighbors_from_all_servers:
        all_neighbors.extend(server_neighbors)

    # Sort all neighbors by distance to aggregate globally
    all_neighbors = sorted(all_neighbors, key=lambda x: sum([(x[i] - data_point[i]) ** 2 for i in range(len(data_point))]))

    # Get final k nearest neighbors
    final_neighbors = all_neighbors[:k]

    # Log the aggregated neighbors
    print("\n\nAggregated results from all servers:")
    for neighbor in all_neighbors:
        print(neighbor)

    # Log the final k-NN result
    # print("\n===========FINAL OUTPUT===========\n")
    # print(f'\nFinal aggregated {k} nearest neighbors:\n{final_neighbors}')

    return final_neighbors


if __name__ == '__main__':
    if len(sys.argv) != 4:
        print("Usage: python client.py <data_point> <k> <number_of_servers>")
        sys.exit(1)

    data_point = sys.argv[1]
    k = sys.argv[2]
    number_of_servers = int(sys.argv[3])

    result = run(data_point, k, number_of_servers)
    print("\n===========FINAL OUTPUT===========")
    print(f'Final aggregated {k} nearest neighbors:\n{result}')
