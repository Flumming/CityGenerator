#include "Roads.h"
#include "GeomTools.h"

void RoadGraph::GenerateRoad(TArray<FVector2d> startpositions, float splitNumStart) {
	nodes.Empty();
	// Generate points
	auto points = boundaryPoints.GeneratePoints();

	// Add nodes at all points
	for (auto point : points.BoundaryPoints) {
		Node* node = new Node();
		node->id = nodes.Num();
		node->position = point;
		nodes.Add(node);

		if (nodes.Num() > 1) {
			node->neighbors.Add(nodes[nodes.Num() - 2]);
			nodes[nodes.Num() - 2]->neighbors.Add(node);
		}
	}
	if (nodes.Num() > 2) {
		nodes[0]->neighbors.Add(nodes[nodes.Num() - 1]);
		nodes[nodes.Num() - 1]->neighbors.Add(nodes[0]);
	}
	for (auto point : points.InnerPoints) {
		if (points.BoundaryPoints.Contains(point)) {
			continue;
		}
	
		Node* node = new Node();
		node->id = nodes.Num();
		node->position = point;
		nodes.Add(node);
	}

	// Connect nodes based on constraints
	ConnectNodes();
	// Split edges longer than 500 units
	SplitEdges();
	// Divide graph into faces
	DivideFaces();
}

void RoadGraph::ConnectNodes() {
	// Get random node
	float connectionRadius = 1000.0f;
	tempNodes = nodes;
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

				if (roadConstraints(*currentNode, *nearbyNode)) {
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
					currentNode->neighbors.Add(chosenNode);
					chosenNode->neighbors.Add(currentNode);
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
}

bool RoadGraph::roadConstraints(Node& node1, Node& node2) {

	// Check if already connected
	if (node1.neighbors.Contains(&node2)) {
		return false;
	}

	// Check if connecting these nodes creates a triangle
	for (Node* neighbor1 : node1.neighbors) {
		for (Node* neighbor2 : node2.neighbors) {
			if (neighbor1->id == neighbor2->id) {
				return false;
			}
		}
	}

	// Check for angle between node1, node2 and node1, its neighbours
	float minAngleDegrees = 50.0f;
	for (Node* neighbor : node1.neighbors) {
		FVector2D vec1 = FVector2D(node2.position) - FVector2D(node1.position);
		FVector2D vec2 = FVector2D(neighbor->position) - FVector2D(node1.position);
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
				UE_LOG(LogTemp, Log, TEXT("Intersection at: %s"), *intersection.ToString());
				return false;
			}
		}
	}
	return true;
}

void RoadGraph::SplitEdges() {
	CleanNodes();

	TArray<TPair<Node*, Node*>> edgesToSplit;

	// Get edges to split
	for (auto node : nodes) {
		for (auto neighbor : node->neighbors) {
			if (node->id < neighbor->id) {
				edgesToSplit.Add(TPair<Node*, Node*>(node, neighbor));
			}
		}
	}

	// Split edges
	for (auto& edge : edgesToSplit) {
		Node* node = edge.Key;
		Node* neighbor = edge.Value;

		float distance = FVector2D::Distance(node->position, neighbor->position);
		if (distance > 500.0f) {
			FVector2D midPoint = (FVector2D(node->position) + FVector2D(neighbor->position)) / 2;
			Node* newNode = new Node();
			newNode->id = nodes.Num();
			newNode->position = midPoint;
			nodes.Add(newNode);

			// Remove old connection
			node->neighbors.Remove(neighbor);
			neighbor->neighbors.Remove(node);

			// Add new connections
			node->neighbors.Add(newNode);
			newNode->neighbors.Add(node);
			neighbor->neighbors.Add(newNode);
			newNode->neighbors.Add(neighbor);
		}
	}
}

void RoadGraph::CleanNodes() {
	TArray<Node*> nodesToDelete;
	for (Node* node : nodes) {
		if (node->neighbors.Num() < 2) {
			nodesToDelete.Add(node);
			continue;
		}
	}
	for (Node* nodeToDelete : nodesToDelete) {
		for (Node* neighbor : nodeToDelete->neighbors) {
			neighbor->neighbors.Remove(nodeToDelete);
		}
		nodes.Remove(nodeToDelete);
	}
}

void RoadGraph::DivideFaces() {
	//Create a set cointaining an edge,direction twice for each direction of the edge
	faces.Empty();
	directedEdges.Empty();

	CleanNodes();

	for (Node* node : nodes) {
		for (Node* neighbor : node->neighbors) {
			directedEdges.Add({node, neighbor});
		}
	}

	while (!directedEdges.IsEmpty()) {
		auto currentEdge = directedEdges[0];
		auto startingNode = currentEdge.from;
		UE_LOG(LogTemp, Warning, TEXT("starting Node %i"), startingNode->id);

		TArray<Node*> face;
		// Follow right most edge until we reach the starting edge again
		while (currentEdge.to != startingNode) {
			UE_LOG(LogTemp, Warning, TEXT("------------"));

			UE_LOG(LogTemp, Warning, TEXT("Current edge: %d -> %d"), currentEdge.from->id, currentEdge.to->id);

			FVector2d direction = (currentEdge.to->position - currentEdge.from->position).GetSafeNormal();
			FVector2d right = FVector2d(-direction.Y, direction.X);  // Perpendicular to the right
			// Get right most edge
			DirectedEdge nextEdge = currentEdge;

			TArray<Node*> rightNodes;

			bool newEdgeFound = false; 
			float biggestAngle = -0.1; 
			for (Node* n : currentEdge.to->neighbors) {
				// For each neighbor check if the n->position is to the right of the current direction
				if (n->id == currentEdge.from->id) {
					continue;
				}
				auto edgeToFind = DirectedEdge{currentEdge.to, n};
				if (!directedEdges.Contains(edgeToFind)) {
					continue;
				}

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

			// Print right nodes
			FString rightNodeString = "Right nodes: ";
			
			// If no right edge found, pick the right most edge on the left side
			float smallestAngle = PI + 0.1;
			if (!newEdgeFound) {
				// No right edge found look at the left side
				for (Node* n : currentEdge.to->neighbors) {
					// For each neighbor check if the n->position is to the right of the current direction
					if (n->id == currentEdge.from->id) {
						continue;
					}

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

			UE_LOG(LogTemp, Warning, TEXT("Next edge: %d -> %d"), nextEdge.from->id, nextEdge.to->id);
			face.Add(currentEdge.from);
			directedEdges.Remove(currentEdge);
			currentEdge = nextEdge;
		}
		face.Add(currentEdge.from);
		directedEdges.Remove(currentEdge);
		faces.Add(face);
		UE_LOG(LogTemp, Warning, TEXT("--------"));

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

BoundaryResult BoundaryPoints::GeneratePoints() {
	GenerateBoundary(FVector2D(0.0f, 0.0f), FVector2D(2000.0f, 2000.0f), 20, 0.2f);
	points = boundary;

	openList.Add(RandomPointInBoundary());

	while (!openList.IsEmpty()) {
		FVector2D currentPoint = openList[0];
		openList.RemoveAt(0);
		const int numCandidates = 10;
		TArray<FVector2D> candidates;
		float minDistance = 500.0f;

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

		FVector2D point = center + FVector2D(FMath::Cos(angle) * rX, FMath::Sin(angle) * rY);
		boundary.Add(point);
	}
}

FVector2d BoundaryPoints::RandomPointInBoundary() {
	FVector2D point;
	do {
		float x = FMath::FRandRange(-2000.0f, 2000.0f);
		float y = FMath::FRandRange(-2000.0f, 2000.0f);

		point = FVector2D(x, y);
	}
	while (!FGeomTools2D::IsPointInPolygon(point, boundary));

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