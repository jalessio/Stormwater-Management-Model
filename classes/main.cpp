#include "SWMMRunner.h"

int main(int argc, char* argv[])
{
	// note that the MAXFNAME defined in constants.h as 259

	char* pathinp = "c:\\users\\cbarr02\\desktop\\github\\swmm\\stormwater-management-model\\examples\\spz2_externalrainfall.inp";
	char* pathrpt = "c:\\users\\cbarr02\\desktop\\github\\swmm\\stormwater-management-model\\examples\\spz2_externalrainfall.rpt";
	char* pathout = "c:\\users\\cbarr02\\desktop\\github\\swmm\\stormwater-management-model\\examples\\spz2_externalrainfall.out";
	char* pathrain = "c:\\users\\cbarr02\\desktop\\github\\swmm\\stormwater-management-model\\examples\\rainfall_for_SPZ2.dat";

	SWMMRunner swmmrunner(pathinp, pathrpt, pathout, pathrain);
	int err = swmmrunner.Run();
	if (err)
		return 1;

	return 0;
}

