#ifndef PLANNING_SNAPSHOT_H
#define PLANNING_SNAPSHOT_H

#include <string>
#include <vector>

struct SnapshotPosition
{
	double x = 0.0;
	double y = 0.0;
};

struct SnapshotDepot
{
	int id = 0;
	SnapshotPosition position;
};

struct SnapshotVehicleSpec
{
	int count = 0;
	double energyCapacity = 0.0;
};

struct SnapshotVehicleState
{
	int vehicleId = -1;
	SnapshotPosition position;
	double energyRemaining = 0.0;
	std::vector<int> lockedPrefix;
	int currentTarget = -1; // -1 means null / no target
};

struct SnapshotCustomer
{
	int id = -1;
	double x = 0.0;
	double y = 0.0;
	double demand = 0.0;
	double serviceTime = 0.0;
};

struct SnapshotRequiredEdge
{
	int from = -1;
	int to = -1;
};

struct SnapshotCostModel
{
	std::string metric = "euclidean";
	double energyPerDistance = 1.0;
};

struct PlanningSnapshot
{
	std::string snapshotId;
	double timestamp = 0.0;
	SnapshotDepot depot;
	SnapshotVehicleSpec vehicles;
	std::vector<SnapshotVehicleState> vehicleStates;
	std::vector<SnapshotCustomer> customers;
	std::vector<int> activeCustomers;
	std::vector<SnapshotRequiredEdge> requiredEdges;
	SnapshotCostModel costModel;
};

struct PlanRoute
{
	int vehicleId = -1;
	std::vector<int> route;
};

struct PlanResult
{
	std::vector<PlanRoute> routes;
	double objectiveCost = 0.0;
};

#endif
