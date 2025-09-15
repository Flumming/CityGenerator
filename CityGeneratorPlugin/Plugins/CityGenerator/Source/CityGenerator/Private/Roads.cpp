#include "Roads.h"
#include "GeomTools.h"

void RoadGraph::GenerateRoad(TArray<FVector2d> startpositions, float splitNumStart) {
	nodes.Empty();
	
	auto points = boundaryPoints.GeneratePoints();
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

	//for (auto point : points.InnerPoints) {
	//	if (points.BoundaryPoints.Contains(point)) {
	//		continue;
	//	}
	//
	//	Node* node = new Node();
	//	node->id = nodes.Num();
	//	node->position = point;
	//	nodes.Add(node);
	//}

	// Connect nodes
	ConnectNodes();

	SplitEdges();
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
			UE_LOG(LogTemp, Warning, TEXT("Amount Node Neighbors: %d"), currentNode->neighbors.Num());
			// Find nearby nodes
			TArray<Node*> nearbyNodes;
			for (Node* otherNode : nodes) {
				if (otherNode->id != currentNode->id &&
					FVector2D::Distance(FVector2D(currentNode->position), FVector2D(otherNode->position)) < connectionRadius) {
					nearbyNodes.Add(otherNode);
				}
			}

			UE_LOG(LogTemp, Warning, TEXT("Nearby Nodes Found: %d"), nearbyNodes.Num());

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

	TArray<Node*> removeNodes;
	//Delete nodes that only have one neightbor
	for (auto node : nodes) {
		if (node->neighbors.Num() == 1) {
			removeNodes.Add(node);
		}
	}

	for (auto nodeToRemove : removeNodes) {
		for (auto neighbor : nodeToRemove->neighbors) {
			neighbor->neighbors.Remove(nodeToRemove);
		}
		nodes.Remove(nodeToRemove);
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