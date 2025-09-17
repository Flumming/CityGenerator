#pragma once

#include "CoreMinimal.h"

struct SeedNode {
	FVector2D position;
	float splitNum;
};

struct Node{
	int id;
	FVector2d position;
	TArray<Node*> neighbors;

	bool operator==(const Node& other) const {
		return id == other.id;
	}
};

struct DirectedEdge {
	Node* from;
	Node* to;

	bool operator==(const DirectedEdge& Other) const {
		return from == Other.from && to == Other.to;
	}
};

struct BoundaryResult {
	TArray<FVector2D> BoundaryPoints;
	TArray<FVector2D> InnerPoints;
};

class BoundaryPoints {
public:
	BoundaryResult GeneratePoints();
	TArray<FVector2D> points;
	TArray<FVector2D> openList;
private:
	TArray<FVector2d> boundary;
	void GenerateBoundary(FVector2D center, FVector2D radii, int numPoints, float variation);
	FVector2d RandomPointInBoundary();

};

class SeedPoints {
public:
	TArray<Node*> GenerateSeedPoints(TArray<FVector2d> startpositions, float splitNumStart);
	TArray<Node*> outputNodes;

private:
	TArray<SeedNode> tempNodes;
	void SplitRoad(SeedNode currentNode);
};

class RoadGraph {
public:
	SeedPoints seedPoints;
	BoundaryPoints boundaryPoints;

	void GenerateRoad(TArray<FVector2d> startpositions, float splitNumStart);
	TArray<Node*> nodes;
	TArray<Node*> tempNodes;

	TArray<TArray<Node*>> faces;

private:
	void ConnectNodes();
	bool roadConstraints(Node& node1, Node& node2);

	void SplitEdges();
	void CleanNodes();
	void DivideFaces();

	TArray<DirectedEdge> directedEdges;
};
