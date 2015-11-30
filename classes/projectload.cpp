// Functions used to populate the global variables in SWMM using data from the SWMMLoader class
// Basically will have functionality of project_readinput() and probably project_open()

// check errorcode in aoptions vs. _errCode (class member)

#include "SWMMLoader.h"
#define  EXTERN extern
#include "globals.h"
#include <malloc.h>
#include <string.h>

//void projectload_readinput();
//void projectload_open(char *f2, char* f3);

// pass in input file here? or do in projectopen?
void projectload_readinput(char* path)
{
	// for testing
	
	SWMMLoader swmmloader(path);

	// allocate memory for SWMM hashtable
	ProjectCreateHashTables(); 
	
	// get class hashtable
	HTtable** classHT = swmmloader.GetHtable();

	// get empty SWMM hashtable from project.c
	HTtable** Htable = ProjectGetHTable(); 

	for (int i = 0; i < 16; i++)
		memcpy(Htable[i], classHT[i], sizeof(struct HTentry)*HTMAXSIZE);

	// analysis options
	AnalysisOptions _aoptions = swmmloader.GetAnalysisOptions();

	UnitSystem = _aoptions.UnitSystem;						// Unit system
	FlowUnits = _aoptions.FlowUnits;						// Flow units
	InfilModel = _aoptions.InfilModel;						// Infiltration method
	RouteModel = _aoptions.RouteModel;					    // Flow routing method
	ForceMainEqn = _aoptions.ForceMainEqn;					// Flow equation for force mains
	LinkOffsets = _aoptions.LinkOffsets;			        // Link offset convention
	AllowPonding = _aoptions.AllowPonding;				    // Allow water to pond at nodes
	InertDamping = _aoptions.InertDamping;					// Degree of inertial damping
	NormalFlowLtd = _aoptions.NormalFlowLtd;				// Normal flow limited
	SlopeWeighting = _aoptions.SlopeWeighting;				// Use slope weighting
	Compatibility = _aoptions.Compatibility;				// SWMM 5/3/4 compatibility
	SkipSteadyState = _aoptions.SkipSteadyState;			// Skip over steady state periods
	IgnoreRainfall = _aoptions.IgnoreRainfall;				// Ignore rainfall/runoff
	IgnoreRDII = _aoptions.IgnoreRainfall;					// Ignore RDII                     //(5.1.004)
	IgnoreSnowmelt = _aoptions.IgnoreSnowmelt;			    // Ignore snowmelt
	IgnoreGwater = _aoptions.IgnoreGwater;					// Ignore groundwater
	IgnoreRouting = _aoptions.IgnoreRouting;				// Ignore flow routing
	IgnoreQuality = _aoptions.IgnoreQuality;				// Ignore water quality
	ErrorCode = _aoptions.ErrorCode;			            // Error code number
	WarningCode = _aoptions.WarningCode;			        // Warning code number
	WetStep = _aoptions.WetStep;							// Runoff wet time step (sec)
	DryStep = _aoptions.DryStep;						    // Runoff dry time step (sec)
	ReportStep = _aoptions.ReportStep;						// Reporting time step (sec)
	SweepStart = _aoptions.SweepStart;						// Day of year when sweeping starts
	SweepEnd = _aoptions.SweepEnd;						    // Day of year when sweeping ends
	MaxTrials = _aoptions.MaxTrials;						// Max. trials for DW routing
	NumThreads = _aoptions.NumThreads;						// Number of parallel threads used //(5.1.008)

	LengtheningStep = _aoptions.LengtheningStep;			// Time step for lengthening (sec)
	StartDryDays = _aoptions.StartDryDays;					// Antecedent dry days
	CourantFactor = _aoptions.CourantFactor;				// Courant time step factor
	MinSurfArea = _aoptions.MinSurfArea;					// Minimum nodal surface area
	MinSlope = _aoptions.MinSlope;							// Minimum conduit slope

	RouteStep = _aoptions.RouteStep;						// Routing time step (sec)
	MinRouteStep = _aoptions.MinRouteStep;					// Minimum variable time step (sec) //(5.1.008)

	HeadTol = _aoptions.HeadTol;							// DW routing head tolerance (ft)
	SysFlowTol = _aoptions.SysFlowTol;						// Tolerance for steady system flow
	LatFlowTol = _aoptions.LatFlowTol;						// Tolerance for steady nodal inflow     

	// time list
	TimeList _timelist = swmmloader.GetTimeList();

	StartDate = _timelist.StartDate;                // Starting date
	StartTime = _timelist.StartTime;                // Starting time
	StartDateTime = _timelist.StartDateTime;        // Starting Date+Time
	EndDate = _timelist.EndDate;                    // Ending date
	EndTime = _timelist.EndTime;                    // Ending time
	EndDateTime = _timelist.EndDateTime;            // Ending Date+Time
	ReportStartDate = _timelist.ReportStartDate;    // Report start date
	ReportStartTime = _timelist.ReportStartTime;    // Report start time
	ReportStart = _timelist.ReportStart;            // Report start Date+Time

	ReportTime = _timelist.ReportTime;              // Current reporting time (msec)
	OldRunoffTime = _timelist.OldRunoffTime;        // Previous runoff time (msec)
	NewRunoffTime = _timelist.NewRunoffTime;        // Current runoff time (msec)
	OldRoutingTime = _timelist.OldRoutingTime;      // Previous routing time (msec)
	NewRoutingTime = _timelist.NewRoutingTime;      // Current routing time (msec)
	TotalDuration = _timelist.TotalDuration;        // Simulation duration (msec)

	// gages
	Nobjects[GAGE] = swmmloader.GetGageCount();
	Gage = (TGage *)calloc(Nobjects[GAGE], sizeof(TGage));
	TGage* _gages = swmmloader.GetGages();
	memcpy(Gage, _gages, sizeof(TGage));

	// subcatchments
	Nobjects[SUBCATCH] = swmmloader.GetSubcatchCount();
	Subcatch = (TSubcatch *)calloc(Nobjects[SUBCATCH], sizeof(TSubcatch));
	TSubcatch* _subcatches = swmmloader.GetSubcatches();
	memcpy(Subcatch, _subcatches, sizeof(TSubcatch));

	// nodes
	Nobjects[NODE] = swmmloader.GetNodeCount();
	Node = (TNode *)calloc(Nobjects[NODE], sizeof(TNode));
	TNode* _node = swmmloader.GetNodes();
	memcpy(Node, _node, sizeof(TNode));

	// timeseries
	Nobjects[TSERIES] = swmmloader.GetTSeriesCount();
	Tseries = (TTable *)calloc(Nobjects[TSERIES], sizeof(TTable));
	TTable* _tseries = swmmloader.GetTSeries();
	memcpy(Tseries, _tseries, sizeof(TTable));

	// infiltration
	//infil_create(Nobjects[SUBCATCH], InfilModel);
	switch (InfilModel)
	{
	case HORTON:
		HortInfil = (THorton *)calloc(1, sizeof(THorton));
		if (HortInfil == NULL) ErrorCode = ERR_MEMORY;
		THorton* _hortinfil = swmmloader.GetInfiltration();
		memcpy(HortInfil, _hortinfil, sizeof(HortInfil)); // check this sizeof
	}

	// 2 options -- either allocate memory as above or call createObjects()
	// that would need a wrapper in project

}


// initPointers() is wrapped by InitPointers()
// decide where to setDefaults() -- in project_open or in SWMMLoader?
// setDefaults() is wrapped by SetDefaults()
// change openFiles() to only open the report and binary files

void projectload_open(char *f2, char* f3)
{
	InitPointers();
	SetDefaults();

	// don't open input file here

	// --- initialize file pointers to NULL
	Frpt.file = NULL;
	Fout.file = NULL;

	// --- save file names
	sstrncpy(Frpt.name, f2, MAXFNAME);
	sstrncpy(Fout.name, f3, MAXFNAME);

	// --- check that file names are not identical
	if (strcomp(f2, f3))
	{
		writecon(FMT11);
		ErrorCode = ERR_FILE_NAME;
		return;
	}

	// --- open report file
	if ((Frpt.file = fopen(f2, "wt")) == NULL)
	{
		writecon(FMT13);
		ErrorCode = ERR_RPT_FILE;
		return;
	}

	// binary file is opened in swmm_start
}


