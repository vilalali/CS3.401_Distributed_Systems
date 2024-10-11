const grpc = require('@grpc/grpc-js');
const protoLoader = require('@grpc/proto-loader');
const path = require('path');
const fs = require('fs');

// Load the gRPC proto file
const PROTO_PATH = path.join(__dirname, 'knn.proto');
const packageDefinition = protoLoader.loadSync(PROTO_PATH, {
  keepCase: true,
  longs: String,
  enums: String,
  defaults: true,
  oneofs: true,
});
const knnProto = grpc.loadPackageDefinition(packageDefinition).knn;

let serverData = []; // Will hold the subset data for the server

function findKNearestNeighbors(call, callback) {
  const { dataPoint, k, serverId } = call.request; // Expecting serverId from request
  const point = JSON.parse(dataPoint);

  // Calculate Euclidean distance for all points in the server's subset
  const distances = serverData.map((p) => ({
    point: p,
    distance: Math.sqrt((p[0] - point[0]) ** 2 + (p[1] - point[1]) ** 2),
  }));

  // Sort by distance and return k nearest neighbors
  const nearestNeighbors = distances
    .sort((a, b) => a.distance - b.distance)
    .slice(0, k);

  // Log nearest neighbors and distances
  console.log(`The nearest neighbors from Server ${serverId} are:`);
  nearestNeighbors.forEach((neighbor) => {
    console.log(`Point: ${JSON.stringify(neighbor.point)}, Distance: ${neighbor.distance}`);
  });

  // Send back the nearest neighbors to the client
  callback(null, { neighbors: JSON.stringify(nearestNeighbors.map(n => n.point)) });
}

function loadDataset(datasetFile) {
  const data = JSON.parse(fs.readFileSync(datasetFile, 'utf8'));
  console.log('Dataset Loaded Successfully:', data);
  return data;
}

function startServer(port, subsetData) {
  const server = new grpc.Server();
  
  // Store the subset data for this server
  serverData = subsetData;

  server.addService(knnProto.KNNService.service, { findKNearestNeighbors });
  server.bindAsync(`127.0.0.1:${port}`, grpc.ServerCredentials.createInsecure(), () => {
    server.start();
    console.log(`Server successfully running on port number: ${port}`);
    console.log(`Subset data loaded on server ${port}:`, subsetData);
  });
}

// Parsing arguments from the command line
const args = process.argv.slice(2);
if (args.length < 2) {
  console.error('Usage: node server.js <dataset_file> <num_servers>');
  process.exit(1);
}

const datasetFile = args[0]; // Dataset file passed as the first argument
const numServers = parseInt(args[1], 10); // Number of servers passed as the second argument
const basePort = 50051;

// Load the full dataset
const fullDataset = loadDataset(datasetFile);

// Split the dataset across multiple servers
const dataSubsetSize = Math.ceil(fullDataset.length / numServers);
for (let i = 0; i < numServers; i++) {
  const subset = fullDataset.slice(i * dataSubsetSize, (i + 1) * dataSubsetSize);
  startServer(basePort + i, subset);
}
