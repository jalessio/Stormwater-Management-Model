//#include "SWMMInput.h"
//#include <cstring>
//#include <math.h>
//
//
//
//const int SWMMInput::MAXERRS = 100;        // Max. input errors reported
//
//SWMMInput::SWMMInput()
//:_inFile(NULL), _gages(NULL), _subcatches(NULL), _tseries(NULL), _tlist(NULL)
//{
//	ClearErr();
//	ClearCounts();
//}
//
////SWMMInput::SWMMInput(const char* path)
////:_gages(NULL), _subcatches(NULL)
////{
////	ClearErr();
////	OpenFile(path);
////}
//
//SWMMInput::~SWMMInput()
//{
//	ClearObjArrays();
//	//project_deleteHashTables();
//}
//
//void SWMMInput::SetTimeDefaults(void)
//{
//	_tlist->StartDate = datetime_encodeDate(2004, 1, 1);
//	_tlist->StartTime = datetime_encodeTime(0, 0, 0);
//	_tlist->StartDateTime = _tlist->StartDate + _tlist->StartTime;
//	_tlist->EndDate = _tlist->StartDate;
//	_tlist->EndTime = 0.0;
//	_tlist->ReportStartDate = NO_DATE;
//	_tlist->ReportStartTime = NO_DATE;
//	//_tlist->SweepStart = 1;
//	//_tlist->SweepEnd = 365;
//}
//
//void SWMMInput::SetAnalysisDefaults(void)
//{
//	// Analysis options
//	_aoptions->UnitSystem = US;               // US unit system
//	_aoptions->FlowUnits = CFS;              // CFS flow units
//	_aoptions->InfilModel = HORTON;           // Horton infiltration method
//	_aoptions->RouteModel = KW;               // Kin. wave flow routing method
//	_aoptions->AllowPonding = FALSE;            // No ponding at nodes
//	_aoptions->InertDamping = SOME;             // Partial inertial damping
//	_aoptions->NormalFlowLtd = BOTH;             // Default normal flow limitation
//	_aoptions->ForceMainEqn = H_W;              // Hazen-Williams eqn. for force mains
//	_aoptions->LinkOffsets = DEPTH_OFFSET;     // Use depth for link offsets
//	_aoptions->LengtheningStep = 0;                // No lengthening of conduits
//	_aoptions->CourantFactor = 0.0;              // No variable time step 
//	_aoptions->MinSurfArea = 0.0;              // Force use of default min. surface area
//	_aoptions->SkipSteadyState = FALSE;            // Do flow routing in steady state periods 
//	_aoptions->IgnoreRainfall = FALSE;            // Analyze rainfall/runoff
//	_aoptions->IgnoreRDII = FALSE;            // Analyze RDII                         //(5.1.004)
//	_aoptions->IgnoreSnowmelt = FALSE;            // Analyze snowmelt 
//	_aoptions->IgnoreGwater = FALSE;            // Analyze groundwater 
//	_aoptions->IgnoreRouting = FALSE;            // Analyze flow routing
//	_aoptions->IgnoreQuality = FALSE;            // Analyze water quality
//	_aoptions->WetStep = 300;              // Runoff wet time step (secs)
//	_aoptions->DryStep = 3600;             // Runoff dry time step (secs)
//	_aoptions->RouteStep = 300.0;            // Routing time step (secs)
//	_aoptions->MinRouteStep = 0.5;              // Minimum variable time step (sec)     //(5.1.008)
//	_aoptions->ReportStep = 900;              // Reporting time step (secs)
//	_aoptions->StartDryDays = 0.0;              // Antecedent dry days
//	_aoptions->MaxTrials = 0;                // Force use of default max. trials 
//	_aoptions->HeadTol = 0.0;              // Force use of default head tolerance
//	_aoptions->SysFlowTol = 0.05;             // System flow tolerance for steady state
//	_aoptions->LatFlowTol = 0.05;             // Lateral flow tolerance for steady state
//	_aoptions->NumThreads = 0;                // Number of parallel threads to use
//}
//
//
//
////bool SWMMInput::OpenFile(const char* path)
////{
////	_inFile = fopen(path, "rt");
////	if (!_inFile)
////	{
////		_errCode = ERR_INP_FILE;
////		return false;
////	}
////
////	if (!CountObjects())
////		return false;
////
////	AllocObjArrays();
////
////	ReadData();
////
////	return _errCode == 0;
////
////}
//
//void SWMMInput::ClearErr()
//{
//	_errCode = 0;
//
//	//set all chars to null character so any inserted char sequence will be valid
//	for (int i = 0; i < 512; ++i)
//		_errString[i] = '\0';
//}
//
//void SWMMInput::SetNObjects(int ngage, int nsubcatch, int ntseries)
//{
//	ClearCounts();
//
//	_Nobjects[GAGE] = ngage;
//	_Nobjects[SUBCATCH] = nsubcatch;
//	_Nobjects[TSERIES] = ntseries;
//
//	AllocObjArrays();
//	project_createHashTables();
//	SetTimeDefaults();
//	//	SetAnalysisDefaults();
//}
//
//TGage* SWMMInput::GetGages()
//{
//	return _gages;
//}
//
//TGage* SWMMInput::GetGage(const int & i)
//{
//	if (i<0 || i >= _Nobjects[GAGE])
//		return NULL;
//
//	return &_gages[i];
//}
//
//int SWMMInput::GetGageCount() const
//{
//	return _Nobjects[GAGE];
//}
//
//TSubcatch* SWMMInput::GetSubcatches()
//{
//	return _subcatches;
//}
//
//TSubcatch* SWMMInput::GetSubcatch(const int & i)
//{
//	if (i<0 || i >= _Nobjects[SUBCATCH])
//		return NULL;
//
//	return &_subcatches[i];
//}
//
//int SWMMInput::GetSubcatchCount() const
//{
//	return _Nobjects[SUBCATCH];
//}
//
//int SWMMInput::GetTSeriesCount() const
//{
//	return _Nobjects[TSERIES];
//}
//
//TTable* SWMMInput::GetTSeries(const int & i)
//{
//	if (i < 0 || i >= _Nobjects[TSERIES])
//		return NULL;
//
//	return &_tseries[i];
//}
//
//
//int* SWMMInput::GetAllCounts()
//{
//	return _Nobjects;
//}
//
//
//void SWMMInput::ClearCounts()
//{
//	//set all counts to zero (could also possibly initialize with {})
//	int i = 0;
//	for (i = 0; i < MAX_OBJ_TYPES; ++i)
//	{
//		_Nobjects[i] = 0;
//		_Mobjects[i] = 0;
//	}
//}
//
//void SWMMInput::SetError(const int & errcode, const char* s)
//{
//	strncpy(_errString, s, 512 - 1);
//	_errCode = errcode;
//}
//
//
//void SWMMInput::ClearObjArrays()
//{
//	delete[] _gages;
//	delete[] _subcatches;
//	delete[] _tseries;
//	delete[] _tlist;
//
//	_gages = NULL;
//	_subcatches = NULL;
//	_tseries = NULL;
//	_tlist = NULL;
//}
//
//
//void SWMMInput::AllocObjArrays()
//{
//	// note that the function createObjects() in project.c does something like this, but using calloc
//
//	//make sure any previous values are disposed of 
//	ClearObjArrays();
//
//	//() sets all space to zero
//	_gages = new TGage[_Nobjects[GAGE]]();
//	_subcatches = new TSubcatch[_Nobjects[SUBCATCH]]();
//	_tseries = new TTable[_Nobjects[TSERIES]]();
//	_tlist = new TTimeList[9]();
//	//add more as needed
//
//}
//
//int SWMMInput::SetGage(int index, char* rID, int rainfalltype, float raininterval,
//	char* tsID, double times[], double rainseries[], int nitems)
//	//	Format:
//	//		[RAINGAGES]
//	//		Name Form Intvl SCF TIMESERIES Tseries 
//	//		Name Form Intvl SCF FILE Fname Sta Units
//	//
//	//		Name	name assigned to rain gage.
//	//		Form	form of recorded rainfall, either INTENSITY, VOLUME or CUMULATIVE.
//	//		Intvl	time interval between gage readings in decimal hours or hours :
//	//				minutes format(e.g., 0:15 for 15 - minute readings).
//	//		SCF		snow catch deficiency correction factor(use 1.0 for no adjustment). -- not included right now
//	//		Tseries name of time series in[TIMESERIES] section with rainfall data.
//	//		Fname	name of external file with rainfall data.
//	//				Rainfall files are discussed in Section 11.3.
//	//		Sta		name of recording station used in the rain file.
//	//		Units	rain depth units used in the rain file, either IN(inches) or MM(millimeters).
//{
//	//char     fname[MAXFNAME + 1];
//	//char     staID[MAXMSG + 1];
//
//	char* id = rID;
//	int sect = s_RAINGAGE;
//
//	int		 err;
//
//	addObject(sect, id);
//
//	if (index < 0 || index >= _Nobjects[GAGE])
//		return 999; //error
//
//
//	_gages[index].ID = id;
//	// --- assign default parameter values
//	//	_gages[j].ID = -1.0;				// No time series index
//	//	_gages[j].rainType = 1.0;           // Rain type is volume
//	//	_gages[j].rainInterval = 3600.0;    // Recording freq. is 3600 sec
//	_gages[index].snowFactor = 1.0;         // Snow catch deficiency factor
//	_gages[index].startDate = NO_DATE;      // Default is no start/end date
//	_gages[index].endDate = NO_DATE;
//	_gages[index].rainUnits = 0.0;          // US units
//	//strcpy(fname, "");
//	//strcpy(staID, "");
//
//	//if (ntoks < 5) return error_setInpError(ERR_ITEMS, "");
//	if (rainfalltype == RAIN_TSERIES)
//	{
//		err = SetTSeries(tsID, times, rainseries, nitems);
//		_gages[index].dataSource = RAIN_TSERIES;
//	}
//	else if (rainfalltype == RAIN_FILE)
//	{
//		//	if (ntoks < 8) return error_setInpError(ERR_ITEMS, "");
//		//	sstrncpy(fname, tok[5], MAXFNAME);
//		//	sstrncpy(staID, tok[6], MAXMSG);
//		//	err = readGageFileFormat(tok, ntoks, x);
//
//		_gages[index].dataSource = RAIN_FILE;
//		//if (_gages[j].dataSource == RAIN_FILE)
//		//{
//		//	sstrncpy(_gages[j].fname, fname, MAXFNAME);
//		//	sstrncpy(_gages[j].staID, staID, MAXMSG);
//		//	_gages[j].startFileDate = x[4];
//		//	_gages[j].endFileDate = x[5];
//		//}
//	}
//
//	//	else return error_setInpError(ERR_KEYWORD, tok[4]);
//
//	// --- save parameters to rain gage object
//	//	if (err > 0) return err;
//	_gages[index].ID = rID;
//	//	_gages[j].tSeries = (int)1; // int  rainfall data time series index
//	_gages[index].rainType = (int)rainfalltype;
//	_gages[index].rainInterval = (int)raininterval;
//
//	//if (_gages[j].tSeries >= 0) 
//	//else                        
//	_gages[index].unitsFactor = 1.0;
//	_gages[index].coGage = -1;
//	_gages[index].isUsed = FALSE;
//	return 0;
//
//}
//
//
//
//
//int SWMMInput::SetSubcatch(char* sID, int gIndex, int outIndex,
//	double area, double fracimperv, double width, double slope, double clength)
//	//	Format:
//	//		[SUBCATCHMENTS]
//	//		Name Rgage OutID Area %Imperv Width Slope Clength (Spack)
//	//
//	//		Name	name assigned to subcatchment.
//	//		Rgage	name of rain gage in[RAINGAGES] section assigned to subcatchment.
//	//		OutID	name of node or subcatchment that receives runoff from subcatchment.
//	//		Area	area of subcatchment(acres or hectares). 
//	//		%Imperv percent imperviousness of subcatchment.
//	//		Width	characteristic width of subcatchment(ft or meters).
//	//		Slope	subcatchment slope(percent).
//	//		Clength total curb length(any length units).
//	//		Spack	name of snow pack object(from[SNOWPACKS] section) that
//	//				characterizes snow accumulation and melting over the subcatchment.
//{
//
//	char* id = sID;
//	int index = 0;
//
//	int sect = s_SUBCATCH;
//	// --- check that named subcatch exists 
//	// this uses hash tables -- strings have already been moved to hashtable based on countobjects in original swmm
//	//id = project_findID(SUBCATCH, tok[0]); 
//	//if (id == NULL) return error_setInpError(ERR_NAME, tok[0]);
//
//	addObject(sect, id);
//
//	// --- check that rain gage exists
//	//k = project_findObject(GAGE, tok[1]);
//	//if (k < 0) return error_setInpError(ERR_NAME, tok[1]);
//	//x[0] = k;
//
//	//// --- check that outlet node or subcatch exists
//	//m = project_findObject(NODE, tok[2]);
//	//x[1] = m;
//	//m = project_findObject(SUBCATCH, tok[2]);
//	//x[2] = m;
//	//if (x[1] < 0.0 && x[2] < 0.0)
//	//	return error_setInpError(ERR_NAME, tok[2]);
//
//	//// --- if snowmelt object named, check that it exists
//	//x[8] = -1;
//	//if (ntoks > 8)
//	//{
//	//	k = project_findObject(SNOWMELT, tok[8]);
//	//	if (k < 0) return error_setInpError(ERR_NAME, tok[8]);
//	//	x[8] = k;
//	//}
//
//
//	if (index < 0 || index >= _Nobjects[SUBCATCH])
//		return 111; // error
//
//	_subcatches[index].ID = id;
//	_subcatches[index].gage = gIndex;
//	_subcatches[index].outNode = outIndex;
//	_subcatches[index].outSubcatch = index;
//	_subcatches[index].infil = index;
//	// skip subarea for now
//	_subcatches[index].width = width / UCF(LENGTH);
//	_subcatches[index].area = area / UCF(LANDAREA);
//	_subcatches[index].fracImperv = fracimperv / 100.0;
//	_subcatches[index].slope = slope / 100.0;
//	_subcatches[index].curbLength = clength;
//
//	// --- create the snow pack object if it hasn't already been created
//	//if (_subcatches[index].snowpack  )
//	//{
//	//	if (!snow_createSnowpack(j, (int)x[8]))
//	//		return error_setInpError(ERR_MEMORY, "");
//	//}
//
//
//	return 0;
//}
//
//int  SWMMInput::addObject(int objType, char* id)
////
////  Input:   objType = object type index
////           id = object's ID string
////  Output:  returns an error code
////  Purpose: adds a new object to the project.
////
//{
//	int errcode = 0;
//	switch (objType)
//	{
//	case s_RAINGAGE:
//		if (!project_addObject(GAGE, id, _Nobjects[GAGE]))
//			errcode = error_setInpError(ERR_DUP_NAME, id);
//		//_Nobjects[GAGE]++;
//		break;
//
//	case s_SUBCATCH:
//		if (!project_addObject(SUBCATCH, id, _Nobjects[SUBCATCH]))
//			errcode = error_setInpError(ERR_DUP_NAME, id);
//		//_Nobjects[SUBCATCH]++;
//		break;
//
//	case s_TIMESERIES:
//		// --- a Time Series can span several lines
//		if (project_findObject(TSERIES, id) < 0)
//		{
//			if (!project_addObject(TSERIES, id, _Nobjects[TSERIES]))
//				errcode = error_setInpError(ERR_DUP_NAME, id);
//			//	_Nobjects[TSERIES]++;
//		}
//		break;
//	}
//	return errcode;
//}
//
//int SWMMInput::SetTSeries(char* tsID, DateTime times[], double rainseries[], int nitems)
//{
//	int    j;                          // time series index
//	int    k;                          // token index
//	int    state;                      // 1: next token should be a date
//	// 2: next token should be a time
//	// 3: next token should be a value 
//	double x, y;                       // time & value table entries
//	//	DateTime d;                        // day portion of date/time value
//	//	DateTime t;                        // time portion of date/time value
//
//	j = 0; // for now
//	x = 0.0;
//	k = 0;
//	state = 1;               // start off looking for a date
//
//	int sect = s_TIMESERIES;
//	char* id = tsID;
//
//	//addObject(sect, id);
//
//
//	while (k < nitems)
//	{
//		switch (state)
//		{
//		case 1:
//			_tseries[j].lastDate = _tlist->StartDate; // set date to start date for now
//			state = 2;
//			break;
//		case 2:
//			// --- save date + time in x
//			x = _tseries[j].lastDate + times[k] / 24.0;
//			state = 3;
//			break;
//		case 3:
//			y = rainseries[k];
//			// --- add date/time & value to time series
//			table_addEntry(&_tseries[j], x, y);
//
//			// --- start over looking first for a date
//			k++;
//			state = 1;
//			break;
//
//		}
//	}
//	return 0;
//}
