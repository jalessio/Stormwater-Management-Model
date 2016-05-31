// Functions used to populate the global variables in SWMM using data from the SWMMLoader class
// Basically will have functionality of project_readinput() and probably project_open()

#include "SWMMLoader.h"
#include <string.h>
#include <stdlib.h>
#define  EXTERN extern
#include "globals.h"

extern void InitPointers(); // might need to modify this
extern void SetDefaults();
extern void ProjectCreateHashTables();
extern HTtable** ProjectGetHTable();
extern TLidGroup* GetLidGroups();

void projectload_readinput(char *f1, char *f2)
{
	// mimic project_readinput
	SWMMLoader swmmloader(f1, f2);

	// check for errors
	ErrorCode = swmmloader.GetErr();
	if (ErrorCode == ERR_INP_FILE)
	{
		writecon(FMT12);
		return;
	}
	else if (ErrorCode) return;


	int k;

	HTtable** classHT;
	HTtable** Htable;
	ProjectCreateHashTables();

	classHT = swmmloader.GetHtable();  		// get class hashtable
	Htable = ProjectGetHTable();			// get empty SWMM hashtable from project.c

	int  len;
	char *newID;

	// populate SWMM hashtable -- consider making this a function in hash.c 
	for (int i = 0; i < MAX_OBJ_TYPES; i++)
	{
		for (int j = 0; j < HTMAXSIZE; j++)
		{
			if (classHT[i][j] != NULL)
			{
				len = strlen((*classHT[i][j]).key) + 1;
				newID = (char *)Alloc(len*sizeof(char));
				strcpy(newID, (*classHT[i][j]).key);

				struct HTentry *entry;
				entry = (struct HTentry *) malloc(sizeof(struct HTentry));

				if (entry == NULL)
				{
					ErrorCode = ERR_MEMORY;
					return;
				}

				entry->key = newID;
				entry->data = (*classHT[i][j]).data;
				entry->next = Htable[i][j];
				Htable[i][j] = entry;

			}
		}
	}


	// get all counts needed
	Nobjects[GAGE] = swmmloader.GetGageCount();
	Nobjects[SUBCATCH] = swmmloader.GetSubcatchCount();
	Nobjects[NODE] = swmmloader.GetNodeCount();
	Nnodes[OUTFALL] = swmmloader.GetOutfallCount();
	Nobjects[TSERIES] = swmmloader.GetTSeriesCount();
	Nobjects[LID] = swmmloader.GetLidCount();
	Nobjects[LANDUSE] = swmmloader.GetLanduseCount();

	// allocate memory for each category of object
	if (ErrorCode) return;
	Gage = (TGage *)calloc(Nobjects[GAGE], sizeof(TGage));
	Subcatch = (TSubcatch *)calloc(Nobjects[SUBCATCH], sizeof(TSubcatch));
	Node = (TNode *)calloc(Nobjects[NODE], sizeof(TNode));
	Outfall = (TOutfall *)calloc(Nnodes[OUTFALL], sizeof(TOutfall));
	//Divider = (TDivider *)calloc(Nnodes[DIVIDER], sizeof(TDivider));
	//Storage = (TStorage *)calloc(Nnodes[STORAGE], sizeof(TStorage));
	//Link = (TLink *)calloc(Nobjects[LINK], sizeof(TLink));
	//Conduit = (TConduit *)calloc(Nlinks[CONDUIT], sizeof(TConduit));
	//Pump = (TPump *)calloc(Nlinks[PUMP], sizeof(TPump));
	//Orifice = (TOrifice *)calloc(Nlinks[ORIFICE], sizeof(TOrifice));
	//Weir = (TWeir *)calloc(Nlinks[WEIR], sizeof(TWeir));
	//Outlet = (TOutlet *)calloc(Nlinks[OUTLET], sizeof(TOutlet));
	//Pollut = (TPollut *)calloc(Nobjects[POLLUT], sizeof(TPollut));
	Landuse = (TLanduse *)calloc(Nobjects[LANDUSE], sizeof(TLanduse));
	Pattern = (TPattern *)calloc(Nobjects[TIMEPATTERN], sizeof(TPattern));
	//Curve = (TTable *)calloc(Nobjects[CURVE], sizeof(TTable));
	Tseries = (TTable *)calloc(Nobjects[TSERIES], sizeof(TTable));
	//Aquifer = (TAquifer *)calloc(Nobjects[AQUIFER], sizeof(TAquifer));
	//UnitHyd = (TUnitHyd *)calloc(Nobjects[UNITHYD], sizeof(TUnitHyd));
	//Snowmelt = (TSnowmelt *)calloc(Nobjects[SNOWMELT], sizeof(TSnowmelt));
	//Shape = (TShape *)calloc(Nobjects[SHAPE], sizeof(TShape));

	AnalysisOptions _aoptions;
	_aoptions = swmmloader.GetAnalysisOptions();

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
	IgnoreRDII = _aoptions.IgnoreRDII;					    // Ignore RDII                     //(5.1.004)
	IgnoreSnowmelt = _aoptions.IgnoreSnowmelt;			    // Ignore snowmelt
	IgnoreGwater = _aoptions.IgnoreGwater;					// Ignore groundwater
	IgnoreRouting = _aoptions.IgnoreRouting;				// Ignore flow routing
	IgnoreQuality = _aoptions.IgnoreQuality;				// Ignore water quality
	//ErrorCode = _aoptions.ErrorCode;			            // Error code number -- set above to know to continue
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

	TimeList _timelist;
	_timelist = swmmloader.GetTimeList();

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

	TRptFlags _rptFlags;
	_rptFlags = swmmloader.GetRptFlags();

	RptFlags.report = _rptFlags.report;				   // TRUE if results report generated
	RptFlags.input = _rptFlags.input;				   // TRUE if input summary included
	RptFlags.subcatchments = _rptFlags.subcatchments;  // TRUE if subcatchment results reported
	RptFlags.nodes = _rptFlags.nodes;				   // TRUE if node results reported
	RptFlags.links = _rptFlags.links;				   // TRUE if link results reported
	RptFlags.continuity = _rptFlags.continuity;        // TRUE if continuity errors reported
	RptFlags.flowStats = _rptFlags.flowStats;		   // TRUE if routing link flow stats. reported
	RptFlags.nodeStats = _rptFlags.nodeStats;		   // TRUE if routing node depth stats. reported
	RptFlags.controls = _rptFlags.controls;			   // TRUE if control actions reported
	RptFlags.linesPerPage = _rptFlags.linesPerPage;    // number of lines printed per page

	// --- create LID objects
	lid_create(Nobjects[LID], Nobjects[SUBCATCH]);

	// --- create control rules
	ErrorCode = controls_create(Nobjects[CONTROL]);
	if (ErrorCode) return;

	// --- create cross section transects
	ErrorCode = transect_create(Nobjects[TRANSECT]);
	if (ErrorCode) return;

	// --- allocate memory for infiltration data
	infil_create(Nobjects[SUBCATCH], InfilModel);

	// --- allocate memory for water quality state variables
	//for (int j = 0; j < Nobjects[SUBCATCH]; j++)
	//{
	//	Subcatch[j].initBuildup =
	//		(double *)calloc(Nobjects[POLLUT], sizeof(double));
	//	Subcatch[j].oldQual = (double *)calloc(Nobjects[POLLUT], sizeof(double));
	//	Subcatch[j].newQual = (double *)calloc(Nobjects[POLLUT], sizeof(double));
	//	Subcatch[j].pondedQual = (double *)calloc(Nobjects[POLLUT], sizeof(double));
	//	Subcatch[j].totalLoad = (double *)calloc(Nobjects[POLLUT], sizeof(double));
	//}
	//for (int j = 0; j < Nobjects[NODE]; j++)
	//{
	//	Node[j].oldQual = (double *)calloc(Nobjects[POLLUT], sizeof(double));
	//	Node[j].newQual = (double *)calloc(Nobjects[POLLUT], sizeof(double));
	//	Node[j].extInflow = NULL;
	//	Node[j].dwfInflow = NULL;
	//	Node[j].rdiiInflow = NULL;
	//	Node[j].treatment = NULL;
	//}
	//for (int j = 0; j < Nobjects[LINK]; j++)
	//{
	//	Link[j].oldQual = (double *)calloc(Nobjects[POLLUT], sizeof(double));
	//	Link[j].newQual = (double *)calloc(Nobjects[POLLUT], sizeof(double));
	//	Link[j].totalLoad = (double *)calloc(Nobjects[POLLUT], sizeof(double));
	//}

	// --- allocate memory for land use buildup/washoff functions
	//for (int j = 0; j < Nobjects[LANDUSE]; j++)
	//{
	//	Landuse[j].buildupFunc =
	//		(TBuildup *)calloc(Nobjects[POLLUT], sizeof(TBuildup));
	//	Landuse[j].washoffFunc =
	//		(TWashoff *)calloc(Nobjects[POLLUT], sizeof(TWashoff));
	//}

	// --- allocate memory for subcatchment landuse factors
	//for (int j = 0; j < Nobjects[SUBCATCH]; j++)
	//{
	//	Subcatch[j].landFactor =
	//		(TLandFactor *)calloc(Nobjects[LANDUSE], sizeof(TLandFactor));
	//	for (k = 0; k < Nobjects[LANDUSE]; k++)
	//	{
	//		Subcatch[j].landFactor[k].buildup =
	//			(double *)calloc(Nobjects[POLLUT], sizeof(double));
	//	}
	//}

	// --- initialize buildup & washoff functions
	//for (int j = 0; j < Nobjects[LANDUSE]; j++)
	//{
	//	for (k = 0; k < Nobjects[POLLUT]; k++)
	//	{
	//		Landuse[j].buildupFunc[k].funcType = NO_BUILDUP;
	//		Landuse[j].buildupFunc[k].normalizer = PER_AREA;
	//		Landuse[j].washoffFunc[k].funcType = NO_WASHOFF;
	//	}
	//}

	// --- initialize rain gage properties
	for (int j = 0; j < Nobjects[GAGE]; j++)
	{
		Gage[j].tSeries = -1;
		strcpy(Gage[j].fname, "");
	}

	// --- initialize subcatchment properties
	for (int j = 0; j < Nobjects[SUBCATCH]; j++)
	{
		Subcatch[j].outSubcatch = -1;
		Subcatch[j].outNode = -1;
		Subcatch[j].infil = -1;
		Subcatch[j].groundwater = NULL;
		Subcatch[j].gwLatFlowExpr = NULL;                                      //(5.1.007)
		Subcatch[j].gwDeepFlowExpr = NULL;                                     //(5.1.007)
		Subcatch[j].snowpack = NULL;
		Subcatch[j].lidArea = 0.0;
		for (k = 0; k < Nobjects[POLLUT]; k++)
		{
			Subcatch[j].initBuildup[k] = 0.0;
		}
	}

	// --- initialize RDII unit hydrograph properties
	for (int j = 0; j < Nobjects[UNITHYD]; j++) rdii_initUnitHyd(j);

	// --- initialize snowmelt properties
	for (int j = 0; j < Nobjects[SNOWMELT]; j++) snow_initSnowmelt(j);

	// --- initialize storage node exfiltration                                //(5.1.007)
	for (int j = 0; j < Nnodes[STORAGE]; j++) Storage[j].exfil = NULL;             //(5.1.007)

	// --- initialize link properties
	for (int j = 0; j < Nobjects[LINK]; j++)
	{
		Link[j].xsect.type = -1;
		Link[j].cLossInlet = 0.0;
		Link[j].cLossOutlet = 0.0;
		Link[j].cLossAvg = 0.0;
		Link[j].hasFlapGate = FALSE;
	}
	for (int j = 0; j < Nlinks[PUMP]; j++) Pump[j].pumpCurve = -1;

	// --- initialize reporting flags
	for (int j = 0; j < Nobjects[SUBCATCH]; j++) Subcatch[j].rptFlag = FALSE;
	for (int j = 0; j < Nobjects[NODE]; j++) Node[j].rptFlag = FALSE;
	for (int j = 0; j < Nobjects[LINK]; j++) Link[j].rptFlag = FALSE;

	//  --- initialize curves, time series, and time patterns
	for (int j = 0; j < Nobjects[CURVE]; j++)   table_init(&Curve[j]);
	for (int j = 0; j < Nobjects[TSERIES]; j++) table_init(&Tseries[j]);
	for (int j = 0; j < Nobjects[TIMEPATTERN]; j++) inflow_initDwfPattern(j);



	// then copy data now that everything has been allocated
	TGage* _gages;
	_gages = swmmloader.GetGages();
	memcpy(Gage, _gages, sizeof(TGage)*Nobjects[GAGE]);

	TSubcatch* _subcatches;
	_subcatches = swmmloader.GetSubcatches();
	memcpy(Subcatch, _subcatches, sizeof(TSubcatch)*Nobjects[SUBCATCH]);

	if (InfilModel == HORTON)
	{
		THorton* _hortinfil;
		_hortinfil = swmmloader.GetHortInfil();
		extern THorton* HortInfil;
		memcpy(HortInfil, _hortinfil, sizeof(THorton)*Nobjects[SUBCATCH]);
	}
	if (InfilModel == GREEN_AMPT)
	{
		TGrnAmpt* _gainfil;
		_gainfil = swmmloader.GetGAInfil();
		extern TGrnAmpt* GAInfil;
		memcpy(GAInfil, _gainfil, sizeof(TGrnAmpt)*Nobjects[SUBCATCH]);
	}
	if (InfilModel == CURVE_NUMBER)
	{
		TCurveNum* _cninfil;
		_cninfil = swmmloader.GetCNInfil();
		extern TCurveNum* CNInfil;
		memcpy(CNInfil, _cninfil, sizeof(TCurveNum)*Nobjects[SUBCATCH]);
	}

	TNode* _nodes;
	_nodes = swmmloader.GetNodes();
	memcpy(Node, _nodes, sizeof(TNode)*Nobjects[NODE]);

	TOutfall* _outfalls;
	_outfalls = swmmloader.GetOutfalls();
	memcpy(Outfall, _outfalls, sizeof(TOutfall)*Nnodes[OUTFALL]);

	TTable* _tseries;
	_tseries = swmmloader.GetTSeries();
	int Tseriescount = Nobjects[TSERIES];
	for (int i = 0; i < Nobjects[TSERIES]; i++)
	{
		memcpy(&Tseries[i].ID, &_tseries[i].ID, sizeof(_tseries[i].ID));
		memcpy(&Tseries[i].curveType, &_tseries[i].curveType, sizeof(_tseries[i].curveType));
		memcpy(&Tseries[i].refersTo, &_tseries[i].refersTo, sizeof(_tseries[i].refersTo));
		memcpy(&Tseries[i].dxMin, &_tseries[i].dxMin, sizeof(_tseries[i].dxMin));
		memcpy(&Tseries[i].x1, &_tseries[i].x1, sizeof(_tseries[i].x1));
		memcpy(&Tseries[i].x2, &_tseries[i].x2, sizeof(_tseries[i].x2));
		memcpy(&Tseries[i].y1, &_tseries[i].y1, sizeof(_tseries[i].y1));
		memcpy(&Tseries[i].y2, &_tseries[i].y2, sizeof(_tseries[i].y2));

		TableEntry* testtableentry = _tseries[i].firstEntry;
		do {
			table_addEntry(&Tseries[i], testtableentry->x, testtableentry->y);
			testtableentry = testtableentry->next;
		} while (testtableentry != NULL);
	}

	TEvap _evap;
	_evap = swmmloader.GetEvap();
	Evap = _evap;

	// set LidCount and GroupCount (now externs in lid.c)
	LidCount = Nobjects[LID];
	GroupCount = Nobjects[SUBCATCH];

	// lidprocs first
	TLidProc* _lidprocs;
	_lidprocs = swmmloader.GetLidProcs();
	extern TLidProc* LidProcs;
	memcpy(LidProcs, _lidprocs, sizeof(TLidProc)*Nobjects[LID]);

	TLidGroup* _lidgroups; 	// a lidGroup holds all the lidUnits associated with a given subcatchment
	_lidgroups = swmmloader.GetLidGroups();

	for (int j = 0; j < Nobjects[SUBCATCH]; j++)
	{
		if (_lidgroups[j] != NULL)
		{
			TLidList* testLidUnit = _lidgroups[j]->lidList;
			do 	// check linked list for more lidUnits for this subcatch
			{
				lid_copyunit(j, testLidUnit->lidUnit);
				testLidUnit = testLidUnit->nextLidUnit;
			} while (testLidUnit != NULL);
		}
	}

	TLanduse* _landuse;
	_landuse = swmmloader.GetLanduse();
	memcpy(Landuse, _landuse, sizeof(TLanduse)*Nobjects[LANDUSE]);
}


void projectload_open(char *f2, char* f3)
{
	// wrapped functions
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
