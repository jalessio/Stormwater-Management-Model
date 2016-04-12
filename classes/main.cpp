#include "SWMMRunner.h"

int main(int argc, char* argv[])
{
	// note that the MAXFNAME defined in constants.h as 259

	char* pathinp = "c:\\users\\cbarr02\\desktop\\github\\swmm\\stormwater-management-model\\examples\\spz2_externalrainfall.inp";
	char* pathrpt = "c:\\users\\cbarr02\\desktop\\github\\swmm\\stormwater-management-model\\examples\\spz2_externalrainfall.rpt";
	char* pathout = "c:\\users\\cbarr02\\desktop\\github\\swmm\\stormwater-management-model\\examples\\spz2_externalrainfall.out";
	char* pathrain = "c:\\users\\cbarr02\\desktop\\github\\swmm\\stormwater-management-model\\examples\\rainfall_for_SPZ2.dat";

	char* pathinp2 = "c:\\users\\cbarr02\\desktop\\github\\swmm\\stormwater-management-model\\examples\\spz2.inp";
	char* pathrpt2 = "c:\\users\\cbarr02\\desktop\\github\\swmm\\stormwater-management-model\\examples\\spz2.rpt";
	char* pathout2 = "c:\\users\\cbarr02\\desktop\\github\\swmm\\stormwater-management-model\\examples\\spz2.out";


	SWMMRunner swmmrunner(pathinp, pathrpt, pathout, pathrain);
	int err = swmmrunner.Run();
	if (err)
		return 1;

	SWMMRunner swmmrunner2(pathinp2, pathrpt2, pathout2);
	int err2 = swmmrunner2.Run();
	if (err2)
		return 2;

	return 0;
}

