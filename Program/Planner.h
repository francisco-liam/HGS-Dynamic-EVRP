#ifndef PLANNER_H
#define PLANNER_H

#include <string>
#include <vector>
#include <utility>
#include <climits>
#include "AlgorithmParameters.h"

struct PlannerInstanceData
{
	std::vector<double> x_coords;
	std::vector<double> y_coords;
	std::vector<std::vector<double>> dist_mtx;
	std::vector<double> service_time;
	std::vector<double> demands;
	double vehicleCapacity = 1.e30;
	double durationLimit = 1.e30;
	bool isDurationConstraint = false;
};

struct PlannerRequest
{
	PlannerInstanceData instance;
	int nbVeh = INT_MAX;
	bool verbose = true;
	AlgorithmParameters ap = default_algorithm_parameters();
};

struct PlannerProgressEntry
{
	double timeSeconds = 0.0;
	double cost = 0.0;
};

struct PlannerResult
{
	bool hasSolution = false;
	double bestCost = 0.0;
	std::vector<std::vector<int>> routes;
	std::vector<PlannerProgressEntry> progress;
	int seed = 0;
};

class Planner
{
public:
	static PlannerRequest buildRequestFromCVRPLIB(const std::string& instancePath, bool isRoundingInteger, int nbVeh, bool verbose, const AlgorithmParameters& ap);
	static PlannerResult solve(const PlannerRequest& request);

	static void writeCVRPLibSolution(const PlannerResult& result, const std::string& fileName);
	static void writeSearchProgress(const PlannerResult& result, const std::string& fileName, const std::string& instanceName);
};

#endif
