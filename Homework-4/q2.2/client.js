const grpc = require('@grpc/grpc-js');
const protoLoader = require('@grpc/proto-loader');
const path = require('path');

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

function findKNearestNeighbors(client, dataPoint, k, serverId) {
  return new Promise((resolve, reject) => {
    client.findKNearestNeighbors({ dataPoint, k, serverId }, (error, response) => {
      if (error) {
        return reject(error);
      }
      const neighbors = JSON.parse(response.neighbors);
      resolve(neighbors);
    });
  });
}

async function tryConnectToServer(address) {
  const client = new knnProto.KNNService(address, grpc.credentials.createInsecure());
  return new Promise((resolve, reject) => {
    client.waitForReady(Date.now() + 1000, (err) => { // 1 second timeout
      if (err) {
        return reject(err); // Server not available
      }
      resolve(client); // Server available
    });
  });
}

async function discoverServers(basePort, maxAttempts) {
  let servers = [];
  for (let i = 0; i < maxAttempts; i++) {
    const port = basePort + i;
    const address = `127.0.0.1:${port}`;
    try {
      const client = await tryConnectToServer(address);
      servers.push({ address, client });
      console.log(`Connected to server at ${address}`);
    } catch (error) {
      console.log(`No server found at ${address}`);
      break; // Stop if no server is found at the current port
    }
  }
  return servers;
}

async function main() {
  const args = process.argv.slice(2);

  if (args.length < 2) {
    console.error('Usage: node client.js <knn_value> <data_point>');
    console.error('Example: node client.js 2 "[1, 2]"');
    process.exit(1);
  }

  const knn = parseInt(args[0], 10); // Number of nearest neighbors (K)
  const dataPoint = args[1]; // Query point in string form

  console.log(`Querying for ${knn} nearest neighbors to point: ${dataPoint}`);

  // Discover available servers, starting from port 50051 and try up to 10 consecutive ports
  const servers = await discoverServers(50051, 10);

  if (servers.length === 0) {
    console.error('No servers available.');
    process.exit(1);
  }

  const promises = servers.map(({ client, address }, index) =>
    findKNearestNeighbors(client, dataPoint, knn, index + 1).then((neighbors) => ({
      address,
      neighbors,
    }))
  );

  const results = await Promise.all(promises);

  // Aggregate the results from all servers
  let allNeighbors = [];
  results.forEach(({ address, neighbors }) => {
    console.log(`The nearest neighbors from server ${address} are:`, neighbors);
    allNeighbors = allNeighbors.concat(neighbors);
  });

  // Calculate distances and select the global K nearest neighbors
  const parsedPoint = JSON.parse(dataPoint);
  const distances = allNeighbors.map((point) => {
    const distance = Math.sqrt(
      (point[0] - parsedPoint[0]) ** 2 + (point[1] - parsedPoint[1]) ** 2
    );
    return { point, distance };
  });

  // Sort by distance and select the K nearest neighbors
  const sortedNeighbors = distances
    .sort((a, b) => a.distance - b.distance)
    .slice(0, knn);

  console.log(`The ${knn} nearest neighbors to ${dataPoint} are:`);
  sortedNeighbors.forEach((neighbor, index) => {
    console.log(`${index + 1}:`, neighbor.point);
  });

  // Optional: Summary message
  console.log(`So, the nearest neighbors are ${sortedNeighbors.map(n => `[${n.point.join(', ')}]`).join(' and ')}.`);
}

main();
