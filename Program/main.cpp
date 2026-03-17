#include "commandline.h"
#include "Planner.h"
using namespace std;

int main(int argc, char *argv[])
{
	try
	{
		// Reading the arguments of the program
		CommandLine commandline(argc, argv);

		// Print all algorithm parameter values
		if (commandline.verbose) print_algorithm_parameters(commandline.ap);

		// Reading the data file and initializing some data structures
		if (commandline.verbose) std::cout << "----- READING INSTANCE: " << commandline.pathInstance << std::endl;
		PlannerRequest request = Planner::buildRequestFromCVRPLIB(
			commandline.pathInstance,
			commandline.isRoundingInteger,
			commandline.nbVeh,
			commandline.verbose,
			commandline.ap);

		PlannerResult result = Planner::solve(request);
		
		// Exporting the best solution
		if (result.hasSolution)
		{
			if (commandline.verbose) std::cout << "----- WRITING BEST SOLUTION IN : " << commandline.pathSolution << std::endl;
			Planner::writeCVRPLibSolution(result, commandline.pathSolution);
			Planner::writeSearchProgress(result, commandline.pathSolution + ".PG.csv", commandline.pathInstance);
		}
	}
	catch (const string& e) { std::cout << "EXCEPTION | " << e << std::endl; }
	catch (const std::exception& e) { std::cout << "EXCEPTION | " << e.what() << std::endl; }
	return 0;
}
