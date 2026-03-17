#include "Planner.h"
#include <cassert>
#include <unordered_set>
#include <iostream>

static PlanningSnapshot buildExampleSnapshot()
{
    PlanningSnapshot snapshot;
    snapshot.snapshotId = "snapshot_001";
    snapshot.timestamp = 120.0;

    snapshot.depot.id = 0;
    snapshot.depot.position.x = 50.0;
    snapshot.depot.position.y = 50.0;

    snapshot.vehicles.count = 3;
    snapshot.vehicles.energyCapacity = 100.0;

    snapshot.vehicleStates = {
        {0, {50.0, 50.0}, 100.0, {0, 3}, 3},
        {1, {20.0, 80.0}, 65.0, {0, 5, 7}, 7},
        {2, {75.0, 30.0}, 40.0, {0}, -1}
    };

    snapshot.customers = {
        {1, 10.0, 15.0, 1.0, 0.0},
        {2, 80.0, 10.0, 1.0, 0.0},
        {3, 60.0, 70.0, 1.0, 0.0},
        {4, 30.0, 25.0, 1.0, 0.0},
        {5, 15.0, 60.0, 1.0, 0.0},
        {6, 90.0, 55.0, 1.0, 0.0},
        {7, 25.0, 85.0, 1.0, 0.0},
        {8, 65.0, 20.0, 1.0, 0.0}
    };

    snapshot.activeCustomers = {1, 2, 4, 6, 8};
    snapshot.requiredEdges = {{4, 6}};
    snapshot.costModel.metric = "euclidean";
    snapshot.costModel.energyPerDistance = 1.0;

    return snapshot;
}

int main()
{
    PlanningSnapshot snapshot = buildExampleSnapshot();

    AlgorithmParameters ap = default_algorithm_parameters();
    ap.seed = 1;
    ap.nbIter = 500;
    ap.timeLimit = 0.0;

    PlanResult result = Planner::solveSnapshot(snapshot, false, ap);

    assert(result.objectiveCost > 0.0);

    std::unordered_set<int> active(snapshot.activeCustomers.begin(), snapshot.activeCustomers.end());
    std::unordered_set<int> visited;

    for (const PlanRoute& route : result.routes)
    {
        for (int customerId : route.route)
        {
            assert(active.find(customerId) != active.end());
            visited.insert(customerId);
        }
    }

    assert(visited.size() == active.size());

    std::cout << "snapshot_test passed" << std::endl;
    return 0;
}
