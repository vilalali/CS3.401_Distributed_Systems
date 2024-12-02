import grpc
import time
import threading
from ridesharing_pb2 import RideRequest, RideStatusRequest
from ridesharing_pb2_grpc import RideSharingStub

class RideClient:
    def __init__(self, load_balancer_address):
        self.load_balancer_address = load_balancer_address
        self.keep_running = True  # Flag to control the polling loop

    def load_credentials(self):
        # Load SSL certificates
        with open('certs/client.key', 'rb') as f:
            client_key = f.read()
        with open('certs/client.crt', 'rb') as f:
            client_cert = f.read()
        with open('certs/ca.crt', 'rb') as f:
            ca_cert = f.read()

        # Create SSL credentials
        credentials = grpc.ssl_channel_credentials(ca_cert, client_key, client_cert)
        return credentials

    def request_ride(self, pickup_location, destination):
        ride_request = RideRequest(rider_id="rider_001", pickup_location=pickup_location, destination=destination)

        # Load the SSL credentials
        credentials = self.load_credentials()

        # Create a secure channel with the credentials
        with grpc.secure_channel(self.load_balancer_address, credentials) as channel:
            stub = RideSharingStub(channel)
            try:
                # Request the ride
                response = stub.RequestRide(ride_request)
                print(f"Ride Requested**: \nRide ID: {response.ride_id}, \nStatus: {response.status}, \nEstimated Time: {response.estimated_time}")
                print("Waiting for driver to accept the ride...")

                # Start a thread to listen for exit command
                exit_thread = threading.Thread(target=self.check_exit_command)
                exit_thread.daemon = True  # Ensure the thread exits when the main program does
                exit_thread.start()

                # Keep checking the status
                while self.keep_running:
                    time.sleep(5)  # Polling interval
                    try:
                        # Call another RPC to get the latest status
                        status_response = stub.GetRideStatus(RideStatusRequest(ride_id=response.ride_id))
                        response.status = status_response.status  # Update the status from the server

                        # Print the current status
                        print(f"Current Ride Status: {response.status}")

                        # Break the loop if we receive a final status
                        if response.status in ["completed", "rejected", "no drivers available"]:
                            print(f"Final Ride Status: {response.status}")
                            break

                    except grpc.RpcError as e:
                        print(f"Failed to get ride status: {e.code()}: {e.details()}")

            except grpc.RpcError as e:
                print(f"An error occurred while requesting the ride: {e.code()}: {e.details()}")

    def check_exit_command(self):
        input("Press Enter to exit the ride request and terminate the client...\n")
        self.keep_running = False  # Stop the polling loop

if __name__ == "__main__":
    load_balancer_address = input("Enter the load balancer address (default: localhost:50051): ") or "localhost:50051"
    pickup_location = input("Enter Pickup Location (A, B, C, D...): ")
    destination = input("Enter Destination (I, J, K, L...): ")
    
    client = RideClient(load_balancer_address)
    client.request_ride(pickup_location, destination)
