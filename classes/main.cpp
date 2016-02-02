#include "SWMMRunner.h"

int main(int argc, char* argv[])
{
	// note that the MAXFNAME defined in constants.h as 259

	//char* pathinp = "C:\\Users\\cbarr02\\Desktop\\GitHub\\swmm\\Stormwater-Management-Model\\examples\\parkinglot_simple_.inp";
	//char* pathrpt = "C:\\Users\\cbarr02\\Desktop\\GitHub\\swmm\\Stormwater-Management-Model\\examples\\parkinglot_simple_runner.rpt";
	//char* pathout = "C:\\Users\\cbarr02\\Desktop\\GitHub\\swmm\\Stormwater-Management-Model\\examples\\parkinglot_simple_runner.out";
	
	char* pathinp2 = "C:\\Users\\cbarr02\\Desktop\\GitHub\\swmm\\Stormwater-Management-Model\\examples\\Example4.inp";
	char* pathrpt2 = "C:\\Users\\cbarr02\\Desktop\\GitHub\\swmm\\Stormwater-Management-Model\\examples\\Example4_runner.rpt";
	char* pathout2 = "C:\\Users\\cbarr02\\Desktop\\GitHub\\swmm\\Stormwater-Management-Model\\examples\\Example4_runner.out";

	//SWMMRunner swmmrunner(pathinp, pathrpt, pathout);
	//int err = swmmrunner.Run();
	//if (err)
	//	return 1;

	SWMMRunner swmmrunner2(pathinp2, pathrpt2, pathout2);
	int err2 = swmmrunner2.Run();
	if (err2)
		return 2;

	return 0;
}

