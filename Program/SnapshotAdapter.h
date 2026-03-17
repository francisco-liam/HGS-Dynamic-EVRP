#ifndef SNAPSHOT_ADAPTER_H
#define SNAPSHOT_ADAPTER_H

#include "PlanningSnapshot.h"
#include "Planner.h"

class SnapshotAdapter
{
public:
	static void validate(const PlanningSnapshot& snapshot);
	static PlannerRequest toPlannerRequest(const PlanningSnapshot& snapshot, const AlgorithmParameters& ap, bool verbose);
	static PlanResult toPlanResult(const PlanningSnapshot& snapshot, const PlannerResult& result);
};

#endif
