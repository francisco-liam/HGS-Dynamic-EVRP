#include "SnapshotAdapter.h"
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <algorithm>

namespace
{
	double euclideanDistance(double x1, double y1, double x2, double y2)
	{
		double dx = x1 - x2;
		double dy = y1 - y2;
		return std::sqrt(dx * dx + dy * dy);
	}
}

void SnapshotAdapter::validate(const PlanningSnapshot& snapshot)
{
	if (snapshot.vehicles.count <= 0)
		throw std::string("Snapshot validation error: vehicles.count must be positive");

	if ((int)snapshot.vehicleStates.size() != snapshot.vehicles.count)
		throw std::string("Snapshot validation error: vehicles.count does not match number of vehicle states");

	std::unordered_set<int> customerIds;
	for (const SnapshotCustomer& customer : snapshot.customers)
	{
		if (customer.id <= 0)
			throw std::string("Snapshot validation error: customer ids must be positive");
		if (!customerIds.insert(customer.id).second)
			throw std::string("Snapshot validation error: duplicate customer id");
	}

	std::unordered_set<int> activeIds;
	for (int customerId : snapshot.activeCustomers)
	{
		if (customerIds.find(customerId) == customerIds.end())
			throw std::string("Snapshot validation error: active customer not found in customers list");
		if (!activeIds.insert(customerId).second)
			throw std::string("Snapshot validation error: duplicate active customer id");
	}

	std::unordered_set<int> vehicleIds;
	for (const SnapshotVehicleState& vehicle : snapshot.vehicleStates)
	{
		if (!vehicleIds.insert(vehicle.vehicleId).second)
			throw std::string("Snapshot validation error: duplicate vehicle id");

		for (int lockedCustomer : vehicle.lockedPrefix)
		{
			if (lockedCustomer != snapshot.depot.id && customerIds.find(lockedCustomer) == customerIds.end())
				throw std::string("Snapshot validation error: locked prefix references unknown customer");
		}

		if (vehicle.currentTarget != -1 && vehicle.currentTarget != snapshot.depot.id && customerIds.find(vehicle.currentTarget) == customerIds.end())
			throw std::string("Snapshot validation error: current_target references unknown customer");
	}

	for (const SnapshotRequiredEdge& edge : snapshot.requiredEdges)
	{
		bool validFrom = (edge.from == snapshot.depot.id) || (customerIds.find(edge.from) != customerIds.end());
		bool validTo = (edge.to == snapshot.depot.id) || (customerIds.find(edge.to) != customerIds.end());
		if (!validFrom || !validTo)
			throw std::string("Snapshot validation error: required edge references unknown node");
	}
}

PlannerRequest SnapshotAdapter::toPlannerRequest(const PlanningSnapshot& snapshot, const AlgorithmParameters& ap, bool verbose)
{
	validate(snapshot);

	std::unordered_map<int, SnapshotCustomer> customerById;
	for (const SnapshotCustomer& customer : snapshot.customers)
		customerById[customer.id] = customer;

	PlannerRequest request;
	request.verbose = verbose;
	request.nbVeh = snapshot.vehicles.count;
	request.ap = ap;

	request.originalNodeIds.push_back(snapshot.depot.id);
	for (int customerId : snapshot.activeCustomers)
		request.originalNodeIds.push_back(customerId);

	int nbNodes = (int)request.originalNodeIds.size();
	request.instance.x_coords.assign(nbNodes, 0.0);
	request.instance.y_coords.assign(nbNodes, 0.0);
	request.instance.demands.assign(nbNodes, 0.0);
	request.instance.service_time.assign(nbNodes, 0.0);
	request.instance.dist_mtx.assign(nbNodes, std::vector<double>(nbNodes, 0.0));

	request.instance.x_coords[0] = snapshot.depot.position.x;
	request.instance.y_coords[0] = snapshot.depot.position.y;

	for (int i = 1; i < nbNodes; i++)
	{
		const SnapshotCustomer& customer = customerById[request.originalNodeIds[i]];
		request.instance.x_coords[i] = customer.x;
		request.instance.y_coords[i] = customer.y;
		request.instance.demands[i] = customer.demand;
		request.instance.service_time[i] = customer.serviceTime;
	}

	for (int i = 0; i < nbNodes; i++)
	{
		for (int j = 0; j < nbNodes; j++)
		{
			request.instance.dist_mtx[i][j] = euclideanDistance(
				request.instance.x_coords[i],
				request.instance.y_coords[i],
				request.instance.x_coords[j],
				request.instance.y_coords[j]);
		}
	}

	request.instance.vehicleCapacity = snapshot.vehicles.energyCapacity;
	request.instance.durationLimit = 1.e30;
	request.instance.isDurationConstraint = false;

	return request;
}

PlanResult SnapshotAdapter::toPlanResult(const PlanningSnapshot& snapshot, const PlannerResult& result)
{
	PlanResult plan;
	plan.objectiveCost = result.bestCost;

	int maxVehicles = std::min((int)result.routes.size(), snapshot.vehicles.count);
	for (int vehicleIndex = 0; vehicleIndex < maxVehicles; vehicleIndex++)
	{
		PlanRoute route;
		route.vehicleId = snapshot.vehicleStates[vehicleIndex].vehicleId;
		route.route = result.routes[vehicleIndex];
		plan.routes.push_back(route);
	}

	return plan;
}
