//#include "SWMMInput.h"
#include "SWMMLoader.h"

extern void projectload_readinput();
extern void projectload_open(char* path2, char* path3);

int main()
{

	//SWMMLoader.c test stuff
	const char* path = "C:\\Users\\cbarr02\\Desktop\\GitHub\\swmm\\Stormwater-Management-Model\\parkinglot_simple.inp";
	char* pathrpt = "C:\\Users\\cbarr02\\Desktop\\GitHub\\swmm\\Stormwater-Management-Model\\parkinglot_simple.rpt";
	char* pathout = "C:\\Users\\cbarr02\\Desktop\\GitHub\\swmm\\Stormwater-Management-Model\\parkinglot_simple.out";

	//int nsubcatch;
	AnalysisOptions aoptions;

	SWMMLoader swmmloader(path);
	aoptions = swmmloader.GetAnalysisOptions();

	HTtable** htable;
	htable = swmmloader.GetHtable();

	projectload_open(pathrpt, pathout);
	projectload_readinput();
//	report_writeTitle();
	project_validate();

}





	// SWMMInput.c test stuff

	//	int Nsubcatch = 2;
	//	int Ngage = 1;
	//	int Ntseries = 1;
	//
	//	int numgages;
	//	int numsubcatches;
	//	int* totobjects;
	//	int err;
	////	int err2;
	//	TSubcatch* subcatch;
	//	TGage* gage;
	////	HTtable* htable;
	//	int index = 0;
	//	int index2 = 17;
	//	char* sID = "S0";
	//	char* rID = "R0";
	//	char* tsID = "TS1";
	//	
	//	SWMMInput swmmInput;
	//	
	//	swmmInput.SetNObjects(Ngage, Nsubcatch, Ntseries);
	//
	//	numgages	  = swmmInput.GetGageCount();
	//	numsubcatches = swmmInput.GetSubcatchCount();
	//	totobjects	  = swmmInput.GetAllCounts();
	//	
	//	err  = swmmInput.SetSubcatch(sID, 0, 0, 100, 50, 400, 10, 30);
	//
	//	subcatch = swmmInput.GetSubcatch(0);
	//
	//	double times1[] = { 0.0, 0.25, 0.5, 0.75, 1.0, 2.0 };
	//	double rainseries1[] = { 0.1, 1.0, 0.5, 0.1, 0.0, 0.4 };
	//
	//	int nitems1 = 6;
	//
	//	swmmInput.SetGage(0, rID, RAINFALL_INTENSITY, 20, tsID, times1, rainseries1, nitems1);
	//	gage = swmmInput.GetGage(0);
	//


