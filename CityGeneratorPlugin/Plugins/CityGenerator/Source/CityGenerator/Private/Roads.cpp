#include "Roads.h"
#include "GeomTools.h"

void RoadGraph::GenerateRoad(TArray<FVector2d> startpositions, float splitNumStart) {
	outNodes.Empty();
	outFaces.Empty();
	IDCounter = 0;
	
	roadSize = {size, size / 3.0f, size / 6.5f};

	TArray<Node*> nodes;
	TArray<TArray<Node*>> faces;
	TMap<FVector2D, Node*> existingNodes;

	// Generate nodes
	GenerateNodes(nodes, 0, {}, existingNodes);
	// Connect nodes based on constraints
	ConnectNodes(nodes, 0);
	// Split edges longer than 500 units
	SplitEdges(nodes, 0, existingNodes);
	// Divide graph into faces
	DivideFaces(nodes, faces);

	outNodes = nodes;	

	
	for (auto face : faces) {
		TArray<Node*> newNodes;
		TArray<TArray<Node*>> newFaces;

		GenerateNodes(newNodes, 1, face, existingNodes);
		ConnectNodes(newNodes, 1);

		outNodes.Append(newNodes);
	}
	

	RemoveDuplicatesByID();

	outFaces.Empty();
	DivideFaces(outNodes, outFaces);
}

Node* RoadGraph::GetOrCreateNode(const FVector2D& position, int depth, TMap<FVector2D, Node*>& existingNodes) {
	if (existingNodes.Contains(position)) {
		return existingNodes[position];
	}

	Node* node = new Node();
	node->id = IDCounter++;
	node->depth = depth;
	node->position = position;
	existingNodes.Add(position, node);
	return node;
}


void RoadGraph::RemoveDuplicatesByID() {
	TSet<int32> seenIDs;
	TArray<Node*> uniqueNodes;

	for (Node* node : outNodes) {
		if (!node) continue;

		if (!seenIDs.Contains(node->id)) {
			seenIDs.Add(node->id);
			uniqueNodes.Add(node);
		}
	}

	outNodes = uniqueNodes;
}

void RoadGraph::GenerateNodes(
	TArray<Node*>& nodes,
	int iteration,
	TArray<Node*> inputBoundary,
	TMap<FVector2D, Node*>& existingNodes
) {
	BoundaryPoints boundaryPoints;

	if (iteration == 0) {
		boundaryPoints.GenerateBoundary(FVector2D(0.0f, 0.0f), FVector2D(size, size), 20, 0.2f);
	}
	else {
		TArray<FVector2D> boundary;
		for (auto node : inputBoundary) {
			boundary.Add(node->position);
		}
		boundaryPoints.boundary = boundary;
	}

	auto points = boundaryPoints.GeneratePoints(iteration, roadSize);

	// Add nodes at all points, reusing existing ones
	for (auto point : points.BoundaryPoints) {
		Node* node = nullptr;

		if (existingNodes.Contains(point)) {
			node = existingNodes[point];
		}
		else {
			node = new Node();
			node->id = IDCounter++;
			node->depth = iteration;
			node->position = point;
			existingNodes.Add(point, node);
		}

		nodes.Add(node);
	}

	// Connect sequential boundary nodes
	for (int i = 0; i < nodes.Num(); i++) {
		Node* node = nodes[i];
		Node* next = nodes[(i + 1) % nodes.Num()];

		Neighbor nodeN = {nodes[i], static_cast<ERoadType>(iteration)};
		Neighbor nextN = {next, static_cast<ERoadType>(iteration)};


		if (!node->neighbors.Contains(next)) node->neighbors.Add(nextN);
		if (!next->neighbors.Contains(node)) next->neighbors.Add(nodeN);
	}

	// Add inner points
	for (auto point : points.InnerPoints) {
		if (points.BoundaryPoints.Contains(point)) continue;

		Node* node = nullptr;
		if (existingNodes.Contains(point)) {
			node = existingNodes[point];
		}
		else {
			node = new Node();
			node->id = IDCounter++;
			node->depth = iteration;
			node->position = point;
			existingNodes.Add(point, node);
		}
		nodes.Add(node);
	}
}


void RoadGraph::ConnectNodes(TArray<Node*>& nodes, int iteration) {
	// Get random node
	float connectionRadius = roadSize[iteration] / 2.0f;

	TArray<Node*> tempNodes = nodes;
	bool pickNewNode = true;
	Node* currentNode = tempNodes[0];
	while (!tempNodes.IsEmpty()) {
		//get index from currentNOde
		if (pickNewNode) {
			// Pick new random node
			int index = FMath::RandRange(0, tempNodes.Num() - 1);
			currentNode = tempNodes[index];
		}
		else {
			//
		}

		if (currentNode->neighbors.Num() < 4) {
			// Find nearby nodes
			TArray<Node*> nearbyNodes;
			for (Node* otherNode : nodes) {
				if (otherNode->id != currentNode->id &&
					FVector2D::Distance(FVector2D(currentNode->position), FVector2D(otherNode->position)) < connectionRadius) {
					nearbyNodes.Add(otherNode);
				}
			}

			TArray<Node*> candidates;
			for (Node* nearbyNode : nearbyNodes) {
				if (currentNode->neighbors.Contains(nearbyNode) || nearbyNode->neighbors.Contains(currentNode)) {
					continue;
				}

				if (roadConstraints(nodes, *currentNode, *nearbyNode)) {
					candidates.Add(nearbyNode);
				}
			}

			if (candidates.Num() > 0) {
				// chosenNode = closest node
				Node* chosenNode = candidates[0];
				float minDistance = 10000.0f;
				for (Node* candidate : candidates) {
					float distance = FVector2D::Distance(FVector2D(currentNode->position), FVector2D(candidate->position));
					if (distance < minDistance) {
						minDistance = distance;
						chosenNode = candidate;
					}
				}

				if (chosenNode->neighbors.Num() < 4 || (chosenNode->neighbors.Num() != 1 && FMath::FRandRange(0.0f, 1.0f) >= 0.5)) {
					// Connect nodes
					Neighbor chosenNodeN = {chosenNode, static_cast<ERoadType>(iteration)};
					Neighbor currentNodeN = {currentNode, static_cast<ERoadType>(iteration)};

					currentNode->neighbors.Add(chosenNodeN);
					chosenNode->neighbors.Add(currentNodeN);
					pickNewNode = false;
					tempNodes.Remove(currentNode);
					currentNode = chosenNode;
					continue;
				} 

				pickNewNode = false;
				tempNodes.Remove(currentNode);
				currentNode = chosenNode;
			}
			else {
				// No candidate found
				pickNewNode = true;
				tempNodes.Remove(currentNode);
			}

			//Create street from node
			// Pick a direction
			// Pick best node: no crossing, no doubling up (no triangle), no small edges (> 50 degrees)
			// Link together

		}
		else {
			tempNodes.Remove(currentNode);
			pickNewNode = true;
		}
	}

	CleanNodes(nodes);
	CleanNodes(nodes);
}

bool RoadGraph::roadConstraints(TArray<Node*>& nodes, Node& node1, Node& node2) {

	// Check if already connected
	if (node1.neighbors.Contains(&node2)) {
		return false;
	}

	// Check if already have 4 connections
	if (node1.neighbors.Num() >= 4 || node2.neighbors.Num() >= 4) {
		return false;
	}

	// Check if connecting these nodes creates a triangle
	for (Neighbor neighbor1 : node1.neighbors) {
		auto neighborN1 = neighbor1.node;
		for (Neighbor neighbor2 : node2.neighbors) {
			auto neighborN2 = neighbor2.node;
			if (neighborN1->id == neighborN2->id) {
				return false;
			}
		}
	}

	// Check for angle between node1, node2 and node1, its neighbours
	float minAngleDegrees = 60.0f;
	for (Neighbor neighbor : node1.neighbors) {
		auto neighborN = neighbor.node;
		FVector2D vec1 = FVector2D(node2.position) - FVector2D(node1.position);
		FVector2D vec2 = FVector2D(neighborN->position) - FVector2D(node1.position);
		vec1.Normalize();
		vec2.Normalize();
		float dotProduct = FVector2D::DotProduct(vec1, vec2);
		float angleRadians = FMath::Acos(dotProduct);
		float angleDegrees = FMath::RadiansToDegrees(angleRadians);
		if (angleDegrees < minAngleDegrees) {
			return false;
		}
	}

	// Check for crossing with existing roads
	for (Node* n : nodes) {
		for (Neighbor neighbor : n->neighbors) {
			auto neighborN = neighbor.node;	

			if (n->id == node1.id || n->id == node2.id || neighborN->id == node1.id || neighborN->id == node2.id) {
				continue;
			}
			FVector intersection;
			if (FMath::SegmentIntersection2D(
				FVector(node1.position.X, node1.position.Y, 0.0),
				FVector(node2.position.X, node2.position.Y, 0.0),
				FVector(n->position.X, n->position.Y, 0.0),
				FVector(neighborN->position.X, neighborN->position.Y, 0.0),
				intersection)) 
			{
				UE_LOG(LogTemp, Warning, TEXT("Intersection at: %s"), *intersection.ToString());
				return false;
			}
		}
	}
	/*
	// Aldo check for crossing with existing roads in outNodes array
	for (Node* n : outNodes) {
		for (Node* neighbor : n->neighbors) {
			if (n->id == node1.id || n->id == node2.id || neighbor->id == node1.id || neighbor->id == node2.id) {
				continue;
			}
			FVector intersection;
			if (FMath::SegmentIntersection2D(
				FVector(node1.position.X, node1.position.Y, 0.0),
				FVector(node2.position.X, node2.position.Y, 0.0),
				FVector(n->position.X, n->position.Y, 0.0),
				FVector(neighbor->position.X, neighbor->position.Y, 0.0),
				intersection)) {
				UE_LOG(LogTemp, Warning, TEXT("Intersection at: %s"), *intersection.ToString());
				return false;
			}
		}
	}
	*/

	return true;
}

void RoadGraph::SplitEdges(TArray<Node*>& nodes, int iteration, TMap<FVector2D, Node*>& existingNodes) {
	CleanNodes(nodes);

	TArray<TPair<Node*, Node*>> edgesToSplit;

	// Collect edges
	for (auto node : nodes) {
		for (auto& neighborStruct : node->neighbors) {
			Node* neighborNode = neighborStruct.node;
			if (node->id < neighborNode->id) {
				edgesToSplit.Add({node, neighborNode});
			}
		}
	}

	for (auto& edge : edgesToSplit) {
		Node* node = edge.Key;
		Node* neighbor = edge.Value;

		float distance = FVector2D::Distance(node->position, neighbor->position);
		if (distance > roadSize[iteration] / 4.0f) {
			FVector2D midPoint = (node->position + neighbor->position) / 2;

			Node* newNode = GetOrCreateNode(midPoint, iteration, existingNodes);

			// Remove old connections
			node->neighbors.RemoveAll([neighbor](const Neighbor& n) { return n.node == neighbor; });
			neighbor->neighbors.RemoveAll([node](const Neighbor& n) { return n.node == node; });

			// Add new connections
			auto addNeighbor = [](Node* src, Node* dest, ERoadType type) {
				Neighbor n{dest, type};
				if (!src->neighbors.Contains(n)) {
					src->neighbors.Add(n);
				}
			};

			addNeighbor(node, newNode, static_cast<ERoadType>(iteration));
			addNeighbor(newNode, node, static_cast<ERoadType>(iteration));
			addNeighbor(neighbor, newNode, static_cast<ERoadType>(iteration));
			addNeighbor(newNode, neighbor, static_cast<ERoadType>(iteration));

			if (!nodes.Contains(newNode)) nodes.Add(newNode);
		}
	}
}


void RoadGraph::CleanNodes(TArray<Node*>& nodes) {
	TArray<Node*> nodesToDelete;

	// Find nodes with less than 2 neighbors
	for (Node* node : nodes) {
		if (node->neighbors.Num() < 2) {
			nodesToDelete.Add(node);
			continue;
		}
	}

	// Remove connections and nodes
	for (Node* nodeToDelete : nodesToDelete) {
		for (Neighbor& neighbor : nodeToDelete->neighbors) {
			Node* neighborNode = neighbor.node;
			neighborNode->neighbors.Remove({nodeToDelete, neighbor.roadType});
		}
		nodes.Remove(nodeToDelete);
	}
}

void RoadGraph::DivideFaces(TArray<Node*>& nodes, TArray<TArray<Node*>>& faces) {
	//Create a set cointaining an edge,direction twice for each direction of the edge
	TArray<DirectedEdge> directedEdges;

	CleanNodes(nodes);

	for (Node* node : nodes) {
		for (Neighbor neighbor : node->neighbors) {
			Node* neighborNode = neighbor.node;
			directedEdges.Add({node, neighborNode});
		}
	}

	while (!directedEdges.IsEmpty()) {
		auto currentEdge = directedEdges[0];
		auto startingNode = currentEdge.from;

		TArray<Node*> face;
		while (currentEdge.to != startingNode) {
			FVector2d direction = (currentEdge.to->position - currentEdge.from->position).GetSafeNormal();
			FVector2d right = FVector2d(-direction.Y, direction.X);  // Perpendicular to the right
			// Get right most edge
			DirectedEdge nextEdge = currentEdge;

			TArray<Node*> rightNodes;

			bool newEdgeFound = false;
			float biggestAngle = -0.1;
			for (Neighbor neighbor : currentEdge.to->neighbors) {
				Node* n = neighbor.node;
				// For each neighbor check if the n->position is to the right of the current direction
				if (n->id == currentEdge.from->id) continue;
				auto edgeToFind = DirectedEdge{currentEdge.to, n};
				if (!directedEdges.Contains(edgeToFind)) continue;
				if (!nodes.Contains(n)) continue;

				FVector2d toNeighbor = (n->position - currentEdge.to->position).GetSafeNormal();
				float angle = FMath::Acos(FVector2d::DotProduct(direction, toNeighbor));
				//Check if toNeighbor is to the right of direction using cross product
				if (FVector2d::CrossProduct(direction, toNeighbor) >= 0) {
					rightNodes.Add(n);
					if (angle > biggestAngle) {
						biggestAngle = angle;
						nextEdge = {currentEdge.to, n};
						newEdgeFound = true;
					}
				}
			}

			// If no right edge found, pick the right most edge on the left side
			float smallestAngle = PI + 0.1;
			if (!newEdgeFound) {
				// No right edge found look at the left side
				for (Neighbor neighbors : currentEdge.to->neighbors) {
					Node* n = neighbors.node;
					// For each neighbor check if the n->position is to the right of the current direction
					if (n->id == currentEdge.from->id) continue;
					auto edgeToFind = DirectedEdge{currentEdge.to, n};
					if (!directedEdges.Contains(edgeToFind)) continue;
					if (!nodes.Contains(n)) continue;

					FVector2d toNeighbor = (n->position - currentEdge.to->position).GetSafeNormal();
					float angle = FMath::Acos(FVector2d::DotProduct(direction, toNeighbor));
					//Check if toNeighbor is to the left of direction using cross product
					if (FVector2d::CrossProduct(direction, toNeighbor) <= 0) {
						if (angle < smallestAngle) {
							smallestAngle = angle;
							nextEdge = {currentEdge.to, n};
						}
					}
				}
			}
			//prrint next edge
			if (currentEdge == nextEdge) {
				UE_LOG(LogTemp, Warning, TEXT("No next edge found, stuck in loop"));
				break;
			}
			face.Add(currentEdge.from);
			directedEdges.Remove(currentEdge);
			currentEdge = nextEdge;
		}
		face.Add(currentEdge.from);
		directedEdges.Remove(currentEdge);
		if (face.Num() > 2) {
			faces.Add(face);
		}
	}

	// Remove face that runs along the boundary  shoelace area
	// Find the face with the most nodes
	int outerFaceIndex = 0;
	int maxNodes = faces[0].Num();

	for (int i = 1; i < faces.Num(); i++) {
		if (faces[i].Num() > maxNodes) {
			maxNodes = faces[i].Num();
			outerFaceIndex = i;
		}
	}

	// Remove the outer face
	faces.RemoveAt(outerFaceIndex);
}

BoundaryResult BoundaryPoints::GeneratePoints(int iteration, TArray<float> roadSize) {
	points = boundary;
	points.Add(RandomPointInBoundary(roadSize, iteration));
	openList.Add(points.Last());

	while (!openList.IsEmpty()) {
		FVector2D currentPoint = openList[0];
		openList.RemoveAt(0);
		int numCandidates = 16;
		//numCandidates = 4;
		TArray<FVector2D> candidates;
		float minDistance = roadSize[iteration] / 4.0f;

		for (int i = 0; i < numCandidates; i++) {
			float angle = (2 * PI / numCandidates) * i;
			float distance = FMath::FRandRange(minDistance, minDistance * 1.5);

			FVector2D candidate = currentPoint + FVector2D(FMath::Cos(angle), FMath::Sin(angle)) * distance;
			candidates.Add(candidate);
		}

		for (auto candidate : candidates) {
			if (FGeomTools2D::IsPointInPolygon(candidate, boundary)) {
				bool tooClose = false;
				for (auto point : points) {
					if (FVector2D::Distance(candidate, point) < minDistance) {
						tooClose = true;
						break;
					}
				}
				for (int i = 0; i < boundary.Num(); i++) {
					FVector2D a = boundary[i];
					FVector2D b = boundary[(i + 1) % boundary.Num()];

					float distanceToEdge = FMath::PointDistToSegment(FVector(candidate, 0.0f), FVector(a, 0.0f), FVector(b, 0.0f));
					if (distanceToEdge < minDistance / 2.0f) {
						tooClose = true;
						break;
					}
				}
				if (!tooClose) {
					openList.Add(candidate);
					points.Add(candidate);
				}
			}
		}
	}
	return {boundary, points};
}
void BoundaryPoints::GenerateBoundary(FVector2D center, FVector2D radii, int numPoints, float variation) {
	boundary.Empty();
	for (int i = 0; i < numPoints; i++) {
		float angle = (2 * PI / numPoints) * i;
		float rX = radii.X * FMath::FRandRange(1.0f - variation, 1.0f + variation);
		float rY = radii.Y * FMath::FRandRange(1.0f - variation, 1.0f + variation);

		//rX = radii.X;
		//rY = radii.Y;

		FVector2D point = center + FVector2D(FMath::Cos(angle) * rX, FMath::Sin(angle) * rY);
		boundary.Add(point);
	}
}

FVector2d BoundaryPoints::RandomPointInBoundary(TArray<float> roadSize, int iteration) {

	// Get center from boundary
	if (boundary.Num() <= 2) return FVector2D::ZeroVector;

	FVector2D sum(0.0f, 0.0f);
	for (const FVector2D& p : boundary) {
		sum += p;
	}

	FVector2D center = sum / boundary.Num();

	FVector2D point = center;
	//do {
	//	float size = roadSize[iteration] / 10.0f;
	//	float x = FMath::FRandRange(-size, size);
	//	float y = FMath::FRandRange(-size, size);
	//
	//	point = center + FVector2D(x, y);
	//}
	//while (!FGeomTools2D::IsPointInPolygon(point, boundary));

	//point = center;

	return point;
}

TArray<Node*> SeedPoints::GenerateSeedPoints(TArray<FVector2d> startpositions, float splitNumStart) {
	tempNodes.Empty();
	outputNodes.Empty();
	for (FVector2d pos : startpositions) {
		tempNodes.Add({pos, splitNumStart});
	}

	while (!tempNodes.IsEmpty()) {
		SeedNode currentNode = tempNodes[0];
		tempNodes.RemoveAt(0);
		//add currentNode to nodes without any neighbors with id as size of nodes
		Node* newNode = new Node{outputNodes.Num(), currentNode.position, {}};
		outputNodes.Add(newNode);

		if (currentNode.splitNum >= 1.0f) {
			// Split road
			SplitRoad(currentNode);
		}
		else if (currentNode.splitNum > 0.0f) {
			// Random change of splitting road
			float randValue = FMath::FRand();
			if (randValue < currentNode.splitNum) {
				SplitRoad(currentNode);
			}
		}
	}

	return outputNodes;
}
void SeedPoints::SplitRoad(SeedNode currentNode) {
	// Pick 10 candidate with random distance in a circle
	const int numCandidates = 10;
	TArray<FVector2D> candidates;
	float minDistance = 800.0f;

	for (int i = 0; i < numCandidates; i++) {
		float angle = (2 * PI / numCandidates) * i;
		//angle += FMath::FRandRange(-PI / (numCandidates * 2), PI / (numCandidates * 2));

		float distance = FMath::FRandRange(500.0f, 750.0f);
		//distance = 600.0f;
		FVector2D candidate = currentNode.position + FVector2D(FMath::Cos(angle), FMath::Sin(angle)) * distance;
		candidates.Add(candidate);
	}

	for (FVector2D candidate : candidates) {
		// Check if candidate is too close to existing nodes
		bool tooClose = false;
		for (Node* node : outputNodes) {
			if (FVector2D::Distance(candidate, node->position) < minDistance && currentNode.position != node->position) {
				tooClose = true;
				break;
			}
		}
		for (SeedNode node : tempNodes) {
			if (FVector2D::Distance(candidate, node.position) < minDistance && currentNode.position != node.position) {
				tooClose = true;
				break;
			}
		}
		if (!tooClose) {
			float newSplitNum = currentNode.splitNum - 1.0f;
			if (newSplitNum < 0.0f) {
				newSplitNum = 0.0f;
			}
			tempNodes.Add({candidate, newSplitNum});
		}
	}
}