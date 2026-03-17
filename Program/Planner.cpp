#include "Planner.h"
#include "InstanceCVRPLIB.h"
#include "SnapshotAdapter.h"
#include "Params.h"
#include "Genetic.h"
#include <fstream>
#include <iostream>
#include <ctime>

PlannerRequest Planner::buildRequestFromCVRPLIB(const std::string& instancePath, bool isRoundingInteger, int nbVeh, bool verbose, const AlgorithmParameters& ap)
{
	InstanceCVRPLIB cvrp(instancePath, isRoundingInteger);

	PlannerRequest request;
	request.instance.x_coords = cvrp.x_coords;
	request.instance.y_coords = cvrp.y_coords;
	request.instance.dist_mtx = cvrp.dist_mtx;
	request.instance.service_time = cvrp.service_time;
	request.instance.demands = cvrp.demands;
	request.instance.vehicleCapacity = cvrp.vehicleCapacity;
	request.instance.durationLimit = cvrp.durationLimit;
	request.instance.isDurationConstraint = cvrp.isDurationConstraint;
	request.nbVeh = nbVeh;
	request.verbose = verbose;
	request.ap = ap;

	return request;
}

PlannerRequest Planner::buildRequestFromSnapshot(const PlanningSnapshot& snapshot, bool verbose, const AlgorithmParameters& ap)
{
	return SnapshotAdapter::toPlannerRequest(snapshot, ap, verbose);
}

PlannerResult Planner::solve(const PlannerRequest& request)
{
	Params params(request.instance.x_coords,
		request.instance.y_coords,
		request.instance.dist_mtx,
		request.instance.service_time,
		request.instance.demands,
		request.instance.vehicleCapacity,
		request.instance.durationLimit,
		request.nbVeh,
		request.instance.isDurationConstraint,
		request.verbose,
		request.ap);

	Genetic solver(params);
	solver.run();

	PlannerResult result;
	result.seed = request.ap.seed;

	const Individual* best = solver.population.getBestFound();
	if (best != nullptr)
	{
		result.hasSolution = true;
		result.bestCost = best->eval.penalizedCost;
		result.routes = best->chromR;
		if (!request.originalNodeIds.empty())
		{
			for (std::vector<int>& route : result.routes)
			{
				for (int& customerId : route)
				{
					if (customerId >= 0 && customerId < (int)request.originalNodeIds.size())
						customerId = request.originalNodeIds[customerId];
				}
			}
		}
	}

	for (const std::pair<clock_t, double>& state : solver.population.getSearchProgress())
	{
		PlannerProgressEntry entry;
		entry.timeSeconds = (double)state.first / (double)CLOCKS_PER_SEC;
		entry.cost = state.second;
		result.progress.push_back(entry);
	}

	return result;
}

PlanResult Planner::solveSnapshot(const PlanningSnapshot& snapshot, bool verbose, const AlgorithmParameters& ap)
{
	PlannerRequest request = buildRequestFromSnapshot(snapshot, verbose, ap);
	PlannerResult plannerResult = solve(request);
	return SnapshotAdapter::toPlanResult(snapshot, plannerResult);
}

void Planner::writeCVRPLibSolution(const PlannerResult& result, const std::string& fileName)
{
	std::ofstream myfile(fileName);
	if (myfile.is_open())
	{
		for (int k = 0; k < (int)result.routes.size(); k++)
		{
			if (!result.routes[k].empty())
			{
				myfile << "Route #" << k + 1 << ":";
				for (int i : result.routes[k]) myfile << " " << i;
				myfile << std::endl;
			}
		}
		myfile << "Cost " << result.bestCost << std::endl;
	}
	else std::cout << "----- IMPOSSIBLE TO OPEN: " << fileName << std::endl;
}

void Planner::writeSearchProgress(const PlannerResult& result, const std::string& fileName, const std::string& instanceName)
{
	std::ofstream myfile(fileName);
	for (const PlannerProgressEntry& state : result.progress)
		myfile << instanceName << ";" << result.seed << ";" << state.cost << ";" << state.timeSeconds << std::endl;
}
