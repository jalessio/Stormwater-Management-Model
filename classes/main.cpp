#include "SWMMRunner.h"

int main()
{
	// note that the maximum number of characters for a filename is 259 

	char* pathinp = "C:\\Users\\cbarr02\\Desktop\\GitHub\\swmm\\Stormwater-Management-Model\\examples\\parkinglot_simple.inp";
	char* pathrpt = "C:\\Users\\cbarr02\\Desktop\\GitHub\\swmm\\Stormwater-Management-Model\\examples\\parkinglot_simple_runner.rpt";
	char* pathout = "C:\\Users\\cbarr02\\Desktop\\GitHub\\swmm\\Stormwater-Management-Model\\examples\\parkinglot_simple_runner.out";


	char* pathinp2 = "C:\\Users\\cbarr02\\Desktop\\GitHub\\swmm\\Stormwater-Management-Model\\examples\\LID_bioretentialcell.inp";
	char* pathrpt2 = "C:\\Users\\cbarr02\\Desktop\\GitHub\\swmm\\Stormwater-Management-Model\\examples\\LID_bioretentialcell_runner.rpt";
	char* pathout2 = "C:\\Users\\cbarr02\\Desktop\\GitHub\\swmm\\Stormwater-Management-Model\\examples\\LID_bioretentialcell_runner.out";


	SWMMRunner swmmrunner(pathinp, pathrpt, pathout);
	swmmrunner.Run();

	SWMMRunner swmmrunner2(pathinp2, pathrpt2, pathout2);
	swmmrunner2.Run();

	return 0;
}

