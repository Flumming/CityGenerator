#pragma once

#include "CoreMinimal.h"

enum class ERoadType {
	MainRoad,
	SecondaryRoad,
	SmallRoad
};

struct SeedNode {
	FVector2D position;
	float splitNum;
};

struct Neighbor;

struct Node{
	int id;
	FVector2d position;
	TArray<Neighbor> neighbors;

	int depth;

	bool operator==(const Node& other) const {
		return id == other.id;
	}
};

struct Neighbor{
	Node* node;
	ERoadType roadType;

	FORCEINLINE bool operator==(const Neighbor& other) const {
		return node == other.node && roadType == other.roadType;
	}

	FORCEINLINE bool operator==(const Node* otherNode) const {
		return node == otherNode;
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
	BoundaryResult GeneratePoints(int iteration, TArray<float> roadSize);
	void GenerateBoundary(FVector2D center, FVector2D radii, int numPoints, float variation);

	TArray<FVector2D> points;
	TArray<FVector2D> openList;
	TArray<FVector2D> boundary;

private:
	FVector2d RandomPointInBoundary(TArray<float> roadSize, int iteration);

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
	void GenerateRoad(TArray<FVector2d> startpositions, float splitNumStart);

	TArray<Node*> outNodes;
	TArray<TArray<Node*>> outFaces;
	float size = 50000.0f;

private:
	void GenerateNodes(
		TArray<Node*>& nodes,
		int iteration,
		TArray<Node*> inputBoundary,
		TMap<FVector2D, Node*>& existingNodes
	);
	void ConnectNodes(TArray<Node*>& nodes, int iteration);
	bool roadConstraints(TArray<Node*>& nodes, Node& node1, Node& node2);

	void SplitEdges(TArray<Node*>& nodes, int iteration, TMap<FVector2D, Node*>& existingNodes);
	void CleanNodes(TArray<Node*>& nodes);
	void DivideFaces(TArray<Node*>& nodes, TArray<TArray<Node*>>& faces);

	void RemoveDuplicatesByID();
	Node* GetOrCreateNode(const FVector2D& position, int depth, TMap<FVector2D, Node*>& existingNodes);

	int IDCounter = 0;
	TArray<float> roadSize;
};
