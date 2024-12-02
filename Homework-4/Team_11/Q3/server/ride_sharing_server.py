import grpc
from concurrent import futures
import threading
import sys
import time
from ridesharing_pb2 import RideResponse, RideStatusResponse, RideStatusRequest, RideRequest
from ridesharing_pb2_grpc import RideSharingServicer, add_RideSharingServicer_to_server
from queue import Queue

# Global Queue for ride requests
ride_request_queue = Queue()

class RideSharingService(RideSharingServicer):
    def __init__(self):
        self.rides = {}
        self.driver_status = {
            'driver_001': True,
            'driver_002': True,
            'driver_003': True,
            'driver_004': True,
            'driver_005': True
        }
        self.lock = threading.Lock()

    def RequestRide(self, request, context):
        print(f"Handling ride request for {request.rider_id}.")

        with self.lock:
            # Check if the server is busy
            if ride_request_queue.qsize() >= 1:
                print(f"Server is busy. Adding request for {request.rider_id} to the queue.")
                ride_request_queue.put(request)
                return RideResponse(
                    ride_id='',
                    status='queued',
                    driver_id='none',
                    estimated_time='N/A'
                )

        # Process the ride immediately if the server is free
        return self.process_ride_request(request)

    def process_ride_request(self, request):
        ride_id = f"ride_{len(self.rides) + 1}"
        self.rides[ride_id] = {
            'rider_id': request.rider_id,
            'pickup_location': request.pickup_location,
            'destination': request.destination,
            'status': 'pending',
            'driver_id': None
        }

        assigned_driver = self.assign_driver()
        if assigned_driver:
            self.rides[ride_id]['driver_id'] = assigned_driver
            self.rides[ride_id]['status'] = 'assigned'
            threading.Thread(target=self.prompt_driver, args=(ride_id, assigned_driver)).start()
            return RideResponse(
                ride_id=ride_id,
                status='assigned',
                driver_id=assigned_driver,
                estimated_time="10 minutes"
            )
        else:
            return RideResponse(
                ride_id=ride_id,
                status='no drivers available',
                driver_id='none',
                estimated_time='N/A'
            )

    def prompt_driver(self, ride_id, driver_id):
        print(f"\nDriver {driver_id}, you have been assigned ride {ride_id}.")
        print("Press 1 to accept or 0 to reject:")
        
        # Create an event for driver response and a shared variable for response
        driver_response_event = threading.Event()
        driver_response = [None]  # Use a list to allow modification in the thread

        # Start a thread to get the driver's response
        driver_response_thread = threading.Thread(target=self.get_driver_response, args=(driver_response_event, driver_response))
        driver_response_thread.start()

        # Wait for the driver to respond or timeout
        if driver_response_event.wait(timeout=10):  # Wait for a maximum of 10 seconds
            if driver_response[0] == 1:
                self.accept_ride(ride_id, driver_id)
            elif driver_response[0] == 0:
                self.reject_ride(ride_id, driver_id, by_timeout=False)
            else:
                print(f"Invalid input from Driver {driver_id}. Automatically rejecting the ride.")
                self.reject_ride(ride_id, driver_id, by_timeout=False)
        else:
            # If the timeout occurs
            print(f"Ride {ride_id} was not accepted by Driver {driver_id} in time. Reassigning ride...")
            self.reject_ride(ride_id, driver_id, by_timeout=True)

    def get_driver_response(self, event, driver_response):
        try:
            response = int(input().strip())
            driver_response[0] = response  # Update response
            event.set()  # Notify that response is received
        except ValueError:
            print("Invalid input. Please press 1 to accept or 0 to reject.")

    def assign_driver(self):
        for driver_id, available in self.driver_status.items():
            if available:
                self.driver_status[driver_id] = False  # Mark driver as unavailable
                return driver_id
        return None

    def accept_ride(self, ride_id, driver_id):
        self.rides[ride_id]['status'] = 'on the way'
        print(f"Driver {driver_id} accepted ride {ride_id}.")
        self.wait_for_ride_completion(ride_id, driver_id)

    def wait_for_ride_completion(self, ride_id, driver_id):
        while True:
            try:
                completed_response = int(input(f"Driver {driver_id}, press 2 to complete ride {ride_id}: "))
                if completed_response == 2:
                    self.complete_ride(ride_id, driver_id)
                    break
                else:
                    print("Invalid input. Please press 2 to complete the ride.")
            except ValueError:
                print("Invalid input. Please press 2 to complete the ride.")

    def reject_ride(self, ride_id, driver_id, by_timeout=False):
        self.rides[ride_id]['status'] = 'pending'
        if not by_timeout:
            print(f"Driver {driver_id} rejected ride {ride_id}. Reassigning ride...")

        new_driver = self.assign_driver()
        if new_driver:
            self.rides[ride_id]['driver_id'] = new_driver
            print(f"Ride {ride_id} reassigned to {new_driver}.")
            threading.Thread(target=self.prompt_driver, args=(ride_id, new_driver)).start()
        else:
            self.rides[ride_id]['status'] = 'no drivers available'
            print(f"No available drivers for ride {ride_id}. Ride is now pending.")

        self.check_queued_requests()

    def complete_ride(self, ride_id, driver_id):
        if ride_id in self.rides:
            self.rides[ride_id]['status'] = 'completed'
            print(f"Ride {ride_id} is now completed by Driver: {driver_id}.")
            self.driver_status[driver_id] = True  # Mark driver as available

            # Check for queued requests
            self.check_queued_requests()

    def check_queued_requests(self):
        if not ride_request_queue.empty():
            next_request = ride_request_queue.get()
            print(f"Processing queued request for {next_request.rider_id}.")
            self.process_ride_request(next_request)

    def GetRideStatus(self, request, context):
        ride = self.rides.get(request.ride_id)
        if ride:
            return RideStatusResponse(status=ride['status'])
        return RideStatusResponse(status='not found')

def serve(port):
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    add_RideSharingServicer_to_server(RideSharingService(), server)

    # Load SSL certificates for secure communication
    with open('certs/server.key', 'rb') as f:
        server_key = f.read()
    with open('certs/server.crt', 'rb') as f:
        server_cert = f.read()

    server_credentials = grpc.ssl_server_credentials(((server_key, server_cert),))
    server.add_secure_port(f'[::]:{port}', server_credentials)

    server.start()
    print(f"Server started on port {port}.")

    try:
        while True:
            time.sleep(86400)  # Keep server running
    except KeyboardInterrupt:
        server.stop(0)

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python ride_sharing_server.py <port>")
        sys.exit(1)

    port = int(sys.argv[1])
    serve(port)
import grpc
from concurrent import futures
import threading
import sys
import time
from ridesharing_pb2 import RideResponse, RideStatusResponse, RideStatusRequest, RideRequest
from ridesharing_pb2_grpc import RideSharingServicer, add_RideSharingServicer_to_server
from queue import Queue

# Global Queue for ride requests
ride_request_queue = Queue()

class RideSharingService(RideSharingServicer):
    def __init__(self):
        self.rides = {}
        self.driver_status = {
            'driver_001': True,
            'driver_002': True,
            'driver_003': True,
            'driver_004': True,
            'driver_005': True
        }
        self.lock = threading.Lock()

    def RequestRide(self, request, context):
        print(f"Handling ride request for {request.rider_id}.")

        with self.lock:
            # Check if the server is busy
            if ride_request_queue.qsize() >= 1:
                print(f"Server is busy. Adding request for {request.rider_id} to the queue.")
                ride_request_queue.put(request)
                return RideResponse(
                    ride_id='',
                    status='queued',
                    driver_id='none',
                    estimated_time='N/A'
                )

        # Process the ride immediately if the server is free
        return self.process_ride_request(request)

    def process_ride_request(self, request):
        ride_id = f"ride_{len(self.rides) + 1}"
        self.rides[ride_id] = {
            'rider_id': request.rider_id,
            'pickup_location': request.pickup_location,
            'destination': request.destination,
            'status': 'pending',
            'driver_id': None
        }

        assigned_driver = self.assign_driver()
        if assigned_driver:
            self.rides[ride_id]['driver_id'] = assigned_driver
            self.rides[ride_id]['status'] = 'assigned'
            threading.Thread(target=self.prompt_driver, args=(ride_id, assigned_driver)).start()
            return RideResponse(
                ride_id=ride_id,
                status='assigned',
                driver_id=assigned_driver,
                estimated_time="10 minutes"
            )
        else:
            return RideResponse(
                ride_id=ride_id,
                status='no drivers available',
                driver_id='none',
                estimated_time='N/A'
            )

    def prompt_driver(self, ride_id, driver_id):
        print(f"\nDriver {driver_id}, you have been assigned ride {ride_id}.")
        print("Press 1 to accept or 0 to reject:")
        
        # Create an event for driver response and a shared variable for response
        driver_response_event = threading.Event()
        driver_response = [None]  # Use a list to allow modification in the thread

        # Start a thread to get the driver's response
        driver_response_thread = threading.Thread(target=self.get_driver_response, args=(driver_response_event, driver_response))
        driver_response_thread.start()

        # Wait for the driver to respond or timeout
        if driver_response_event.wait(timeout=10):  # Wait for a maximum of 10 seconds
            if driver_response[0] == 1:
                self.accept_ride(ride_id, driver_id)
            elif driver_response[0] == 0:
                self.reject_ride(ride_id, driver_id, by_timeout=False)
            else:
                print(f"Invalid input from Driver {driver_id}. Automatically rejecting the ride.")
                self.reject_ride(ride_id, driver_id, by_timeout=False)
        else:
            # If the timeout occurs
            print(f"Ride {ride_id} was not accepted by Driver {driver_id} in time. Reassigning ride...")
            self.reject_ride(ride_id, driver_id, by_timeout=True)

    def get_driver_response(self, event, driver_response):
        try:
            response = int(input().strip())
            driver_response[0] = response  # Update response
            event.set()  # Notify that response is received
        except ValueError:
            print("Invalid input. Please press 1 to accept or 0 to reject.")

    def assign_driver(self):
        for driver_id, available in self.driver_status.items():
            if available:
                self.driver_status[driver_id] = False  # Mark driver as unavailable
                return driver_id
        return None

    def accept_ride(self, ride_id, driver_id):
        self.rides[ride_id]['status'] = 'on the way'
        print(f"Driver {driver_id} accepted ride {ride_id}.")
        self.wait_for_ride_completion(ride_id, driver_id)

    def wait_for_ride_completion(self, ride_id, driver_id):
        while True:
            try:
                completed_response = int(input(f"Driver {driver_id}, press 2 to complete ride {ride_id}: "))
                if completed_response == 2:
                    self.complete_ride(ride_id, driver_id)
                    break
                else:
                    print("Invalid input. Please press 2 to complete the ride.")
            except ValueError:
                print("Invalid input. Please press 2 to complete the ride.")

    def reject_ride(self, ride_id, driver_id, by_timeout=False):
        self.rides[ride_id]['status'] = 'pending'
        if not by_timeout:
            print(f"Driver {driver_id} rejected ride {ride_id}. Reassigning ride...")

        new_driver = self.assign_driver()
        if new_driver:
            self.rides[ride_id]['driver_id'] = new_driver
            print(f"Ride {ride_id} reassigned to {new_driver}.")
            threading.Thread(target=self.prompt_driver, args=(ride_id, new_driver)).start()
        else:
            self.rides[ride_id]['status'] = 'no drivers available'
            print(f"No available drivers for ride {ride_id}. Ride is now pending.")

        self.check_queued_requests()

    def complete_ride(self, ride_id, driver_id):
        if ride_id in self.rides:
            self.rides[ride_id]['status'] = 'completed'
            print(f"Ride {ride_id} is now completed by Driver: {driver_id}.")
            self.driver_status[driver_id] = True  # Mark driver as available

            # Check for queued requests
            self.check_queued_requests()

    def check_queued_requests(self):
        if not ride_request_queue.empty():
            next_request = ride_request_queue.get()
            print(f"Processing queued request for {next_request.rider_id}.")
            self.process_ride_request(next_request)

    def GetRideStatus(self, request, context):
        ride = self.rides.get(request.ride_id)
        if ride:
            return RideStatusResponse(status=ride['status'])
        return RideStatusResponse(status='not found')

def serve(port):
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    add_RideSharingServicer_to_server(RideSharingService(), server)

    # Load SSL certificates for secure communication
    with open('certs/server.key', 'rb') as f:
        server_key = f.read()
    with open('certs/server.crt', 'rb') as f:
        server_cert = f.read()

    server_credentials = grpc.ssl_server_credentials(((server_key, server_cert),))
    server.add_secure_port(f'[::]:{port}', server_credentials)

    server.start()
    print(f"Server started on port {port}.")

    try:
        while True:
            time.sleep(86400)  # Keep server running
    except KeyboardInterrupt:
        server.stop(0)

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python ride_sharing_server.py <port>")
        sys.exit(1)

    port = int(sys.argv[1])
    serve(port)
