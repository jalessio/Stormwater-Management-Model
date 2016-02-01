#include "SWMMLoader.h"
#include <cstring>
#include <stdlib.h>

const int SWMMLoader::MAXERRS = 100;        // Max. input errors reported



SWMMLoader::SWMMLoader()
:_inFile(NULL), _gages(NULL), _subcatches(NULL), _nodes(NULL), _tseries(NULL), _hortinfil(NULL),
_gainfil(NULL), _cninfil(NULL), _lidProcs(NULL), _lidGroups(NULL), _landuse(NULL)
{
	_status = 0;

	ClearErr();
	ClearCounts();

}

SWMMLoader::SWMMLoader(const char* path)
:_gages(NULL), _subcatches(NULL), _nodes(NULL), _tseries(NULL), _hortinfil(NULL), 
_gainfil(NULL), _cninfil(NULL), _lidProcs(NULL), _lidGroups(NULL), _landuse(NULL) //TODO check other variables and _lidGroups
{
	_status = 0;

	ClearErr(); // ClearCounts() called by OpenFile
	OpenFile(path);

	_aoptions.ErrorCode = _errCode;
}

SWMMLoader::~SWMMLoader()
{
	ClearObjArrays();

	for (int j = 0; j < MAX_OBJ_TYPES; j++)
	{
		if (_Htable[j] != NULL) HTfree(_Htable[j]);
	}
}

bool SWMMLoader::OpenFile(const char* path)
{
	_inFile = fopen(path, "rt");
	if (!_inFile)
	{
		_errCode = ERR_INP_FILE;
		return false;
	}

	// usually handled by swmm_open but here for now
	datetime_setDateFormat(M_D_Y);

	CreateHashTables();

	if (CountObjects() != ERR_NONE)
		return false;

	AllocObjArrays();
	ReadData();

	// from project_readInput
	// --- establish starting & ending date/time
	_timelist.StartDateTime = _timelist.StartDate + _timelist.StartTime;
	_timelist.EndDateTime = _timelist.EndDate + _timelist.EndTime;
	_timelist.ReportStart = _timelist.ReportStartDate + _timelist.ReportStartTime;
	_timelist.ReportStart = MAX(_timelist.ReportStart, _timelist.StartDateTime);

	// --- check for valid starting & ending date/times
	if (_timelist.EndDateTime <= _timelist.StartDateTime)
	{
		SetError(ERR_START_DATE, "");
	}
	else if (_timelist.EndDateTime <= _timelist.ReportStart)
	{
		SetError(ERR_REPORT_DATE, "");
	}
	else
	{
		////  Following code segment was modified for release 5.1.009.  ////           //(5.1.009)
		////
		// --- compute total duration of simulation in seconds
		_timelist.TotalDuration = floor((_timelist.EndDateTime - _timelist.StartDateTime) * SECperDAY);

		// --- reporting step must be <= total duration
		if ((double)_aoptions.ReportStep > _timelist.TotalDuration)
		{
			_aoptions.ReportStep = (int)(_timelist.TotalDuration);
		}

		// --- reporting step can't be < routing step
		if ((double)_aoptions.ReportStep < _aoptions.RouteStep)
		{
			SetError(ERR_REPORT_STEP, "");
		}

		// --- convert total duration to milliseconds
		_timelist.TotalDuration *= 1000.0;
	}

	fclose(_inFile);
	_inFile = NULL;

	return _errCode == ERR_NONE;
}

void SWMMLoader::ClearErr()
{
	_errCode = 0;

	//set all chars to null character so any inserted char sequence will be valid
	for (int i = 0; i < 512; ++i)
		_errString[i] = '\0';
}

TGage* SWMMLoader::GetGages()
{
	return _gages;
}

TGage* SWMMLoader::GetGage(const int & i)
{
	if (i < 0 || i >= _Nobjects[GAGE])
		return NULL;

	return &_gages[i];
}

int SWMMLoader::GetGageCount() const
{
	return _Nobjects[GAGE];
}

void SWMMLoader::SetGageCount(const int n)
{
	_Nobjects[GAGE] = n;
}

TSubcatch* SWMMLoader::GetSubcatches()
{
	return _subcatches;
}

TSubcatch* SWMMLoader::GetSubcatch(const int & i)
{
	if (i <0 || i >= _Nobjects[SUBCATCH])
		return NULL;

	return &_subcatches[i];
}

int SWMMLoader::GetSubcatchCount() const
{
	return _Nobjects[SUBCATCH];
}

TNode* SWMMLoader::GetNodes()
{
	return _nodes;
}

TNode* SWMMLoader::GetNode(const int & i)
{
	if (i < 0 || i >= _Nobjects[NODE])
		return NULL;

	return &_nodes[i];
}

int SWMMLoader::GetNodeCount() const
{
	return _Nobjects[NODE];
}

TOutfall* SWMMLoader::GetOutfalls()
{
	return _outfalls;
}

int SWMMLoader::GetOutfallCount() const
{
	return _Nnodes[OUTFALL];
}

TTable* SWMMLoader::GetTSeries()
{
	return _tseries;
}

int SWMMLoader::GetTSeriesCount() const
{
	return _Nobjects[TSERIES];
}

THorton* SWMMLoader::GetHortInfil()
{
	return _hortinfil;
}

TGrnAmpt* SWMMLoader::GetGAInfil()
{
	return _gainfil;
}

TCurveNum* SWMMLoader::GetCNInfil()
{
	return _cninfil;
}

TEvap SWMMLoader::GetEvap()
{
	return _evap;
}

int SWMMLoader::GetLidCount() const
{
	return _Nobjects[LID];
}

TLidGroup* SWMMLoader::GetLidGroups()
{
	return _lidGroups;
}

TLidProc* SWMMLoader::GetLidProcs()
{
	return _lidProcs;
}

int SWMMLoader::GetLanduseCount() const
{
	return _Nobjects[LANDUSE];
}

TLanduse* SWMMLoader::GetLanduse()
{
	return _landuse;
}

AnalysisOptions SWMMLoader::GetAnalysisOptions()
{
	return _aoptions;
}

TimeList SWMMLoader::GetTimeList()
{
	return _timelist;
}

TRptFlags SWMMLoader::GetRptFlags() const
{
	return _rptFlags;
}


HTtable** SWMMLoader::GetHtable()
{
	return _Htable;
} 

int* SWMMLoader::GetAllCounts()
{
	return _Nobjects;
}

void SWMMLoader::ClearCounts()
{
	//set all counts to zero
	int i = 0;
	for (i = 0; i < MAX_OBJ_TYPES; ++i)
	{
		_Nobjects[i] = 0;
		_Mobjects[i] = 0;
	}

	for (i = 0; i < MAX_NODE_TYPES; ++i)
	{
		_Nnodes[i] = 0;
		_Mnodes[i] = 0;
	}

	LidCount = 0;
	GroupCount = 0;
}

void SWMMLoader::SetError(const int & errcode, const char* s)
{
	strncpy(_errString, s, 512 - 1);
	_errCode = errcode;
}


void SWMMLoader::CreateHashTables()
{
	int j;
	_MemPoolAllocated = FALSE;
	for (j = 0; j < MAX_OBJ_TYPES; j++)
	{
		_Htable[j] = HTcreate(); // uses calloc
		if (_Htable[j] == NULL) SetError(ERR_MEMORY, "");
	}

	// --- initialize memory pool used to store object ID's
	if (AllocInit() == NULL) SetError(ERR_MEMORY, ""); // allocinit called directly from mempool.c
	else _MemPoolAllocated = TRUE;
}


int SWMMLoader::ProjectAddObject(int type, char *id, int n)
//
//  Input:   type = object type
//           id   = object ID string
//           n    = object index
//  Output:  returns 0 if object already added, 1 if not, -1 if hashing fails
//  Purpose: adds an object ID to a hash table
//
{
	int  result;
	int  len;
	char *newID;

	// --- do nothing if object already placed in hash table
	if (ProjectFindObject(type, id) >= 0) return 0;

	// --- use memory from the hash tables' common memory pool to store
	//     a copy of the object's ID string
	len = strlen(id) + 1;
	newID = (char *)Alloc(len*sizeof(char));
	strcpy(newID, id);

	// --- insert object's ID into the hash table for that type of object
	result = HTinsert(_Htable[type], newID, n);
	if (result == 0) result = -1;
	return result;
}

int SWMMLoader::ProjectFindObject(int type, char *id)
{
	return HTfind(_Htable[type], id);
}

char *SWMMLoader::ProjectFindID(int type, char *id)
//
//  Input:   type = object type
//           id   = ID name being sought
//  Output:  returns pointer to location where object's ID string is stored
//  Purpose: uses hash table to find address of given string entry.
//
{
	return HTfindKey(_Htable[type], id);
}

int SWMMLoader::CountObjects()
{
	char  line[MAXLINE + 1];           // line from input data file     
	char  wLine[MAXLINE + 1];          // working copy of input line   
	char  *tok;                        // first string token of line          
	int   sect = -1, newsect;          // input data sections          
	int   errcode = 0;                 // error code
	int   errsum = 0;                  // number of errors found                   
	long  lineCount = 0;

	// --- initialize number of objects & set default values
	ClearCounts();

	// --- make pass through data file counting number of each object
	while (fgets(line, MAXLINE, _inFile) != NULL)
	{
		// --- skip blank lines & those beginning with a comment
		lineCount++;
		strcpy(wLine, line);           // make working copy of line
		tok = strtok(wLine, SEPSTR);   // get first text token on line
		if (tok == NULL) continue;
		if (*tok == ';') continue;

		// --- check if line begins with a new section heading
		if (*tok == '[')
		{
			// --- look for heading in list of section keywords
			newsect = findmatch(tok, SectWords);
			if (newsect >= 0)
			{
				sect = newsect;
				continue;
			}
			else
			{
				sect = -1;
				errcode = ERR_KEYWORD;
			}
		}

		//.... continue here ....

		// --- if in OPTIONS section then read the option setting
		//     otherwise add object and its ID name (tok) to project
		if (sect == s_OPTION) errcode = ReadOption(line); 
		else if (sect >= 0)
		{
			switch (sect)
			{
			case s_RAINGAGE:
				// add object to hashtable
				ProjectAddObject(GAGE, tok, _Nobjects[GAGE]);
				_Nobjects[GAGE]++;
				break;

			case s_SUBCATCH:
				ProjectAddObject(SUBCATCH, tok, _Nobjects[SUBCATCH]);
				_Nobjects[SUBCATCH]++;
				break;

			case s_JUNCTION:
				ProjectAddObject(NODE, tok, _Nobjects[NODE]);
				_Nobjects[NODE]++;
				_Nnodes[JUNCTION]++; 
				break;

			case s_OUTFALL:
				ProjectAddObject(NODE, tok, _Nobjects[NODE]);
				_Nobjects[NODE]++;
				_Nnodes[OUTFALL]++;
				break;

			case s_TIMESERIES:
				// --- a Time Series can span several lines
				if (ProjectFindObject(TSERIES, tok) < 0)
				{
					ProjectAddObject(TSERIES, tok, _Nobjects[TSERIES]);
					_Nobjects[TSERIES]++;
				}
				break;
				

			case s_LID_CONTROL:
				// --- an LID object can span several lines
				if (ProjectFindObject(LID, tok) < 0)
				{
					ProjectAddObject(LID, tok, _Nobjects[LID]);
					_Nobjects[LID]++;
				}
				break;
				//add more cases as needed
			}
		}

		// can be expanded for more detailed error reporting
		// --- report any error found
		 if ( errcode )
		 {
		      //report_writeInputErrorMsg(errcode, sect, line, lineCount);
		     errsum++;
		     if (errsum >= MAXERRS ) break;
		 }
	}

	// --- set global error code if input errors were found
	if (errsum > 0) _errCode = ERR_INPUT;
	return _errCode;
}

// from input.c -- called by input_countObjects()
int SWMMLoader::ReadOption(char* line)
{
	_Ntokens = GetTokens(line);
	if ( _Ntokens < 2 ) return 0;
	return ProjectReadOption(_Tok[0], _Tok[1]);
}

// from project.c
int SWMMLoader::ProjectReadOption(char* s1, char* s2)
{
	int      k, m, h, s;
	double   tStep;
	char     strDate[25];
	DateTime aTime;
	DateTime aDate;

	// --- determine which option is being read
	k = findmatch(s1, OptionWords);
	if (k < 0) return error_setInpError(ERR_KEYWORD, s1);
	switch (k)
	{
	// --- choice of flow units
	case FLOW_UNITS:
		m = findmatch(s2, FlowUnitWords);
		if (m < 0) return error_setInpError(ERR_KEYWORD, s2);
		_aoptions.FlowUnits = m;
		if (_aoptions.FlowUnits <= MGD) _aoptions.UnitSystem = US;
		else                    _aoptions.UnitSystem = SI;
		break;

	// --- choice of infiltration modeling method
	case INFIL_MODEL:
		m = findmatch(s2, InfilModelWords);
		if (m < 0) return error_setInpError(ERR_KEYWORD, s2);
		_aoptions.InfilModel = m;
		break;

	// --- choice of flow routing method
	case ROUTE_MODEL:
		m = findmatch(s2, RouteModelWords);
		if (m < 0) m = findmatch(s2, OldRouteModelWords);
		if (m < 0) return error_setInpError(ERR_KEYWORD, s2);
		if (m == NO_ROUTING) _aoptions.IgnoreRouting = TRUE;
		else _aoptions.RouteModel = m;
		if (_aoptions.RouteModel == EKW) _aoptions.RouteModel = KW;
		break;

	// these are stored in TTimeList
	// --- simulation start date
	case START_DATE:
		if (!datetime_strToDate(s2, &_timelist.StartDate))
		{
			return error_setInpError(ERR_DATETIME, s2);
		}
		break;

	// --- simulation start time of day
	case START_TIME:
		if (!datetime_strToTime(s2, &_timelist.StartTime))
		{
			return error_setInpError(ERR_DATETIME, s2);
		}
		break;

	// --- simulation ending date
	case END_DATE:
		if (!datetime_strToDate(s2, &_timelist.EndDate))
		{
			return error_setInpError(ERR_DATETIME, s2);
		}
		break;

	// --- simulation ending time of day
	case END_TIME:
		if (!datetime_strToTime(s2, &_timelist.EndTime))
		{
			return error_setInpError(ERR_DATETIME, s2);
		}
		break;

	// --- reporting start date
	case REPORT_START_DATE:
		if (!datetime_strToDate(s2, &_timelist.ReportStartDate))
		{
			return error_setInpError(ERR_DATETIME, s2);
		}
		break;

	// --- reporting start time of day
	case REPORT_START_TIME:
		if (!datetime_strToTime(s2, &_timelist.ReportStartTime))
		{
			return error_setInpError(ERR_DATETIME, s2);
		}
		break;

	// --- day of year when street sweeping begins or when it ends
	//     (year is arbitrarily set to 1947 so that the dayOfYear
	//      function can be applied)
	case SWEEP_START:
	case SWEEP_END:
		strcpy(strDate, s2);
		strcat(strDate, "/1947");
		if (!datetime_strToDate(strDate, &aDate))
		{
			return error_setInpError(ERR_DATETIME, s2);
		}
		m = datetime_dayOfYear(aDate);
		if (k == SWEEP_START) _aoptions.SweepStart = m;
		else _aoptions.SweepEnd = m;
		break;

	// --- number of antecedent dry days
	case START_DRY_DAYS:
		_aoptions.StartDryDays = atof(s2);
		if (_aoptions.StartDryDays < 0.0)
		{
			return error_setInpError(ERR_NUMBER, s2);
		}
		break;

	// --- runoff or reporting time steps
	//     (input is in hrs:min:sec format, time step saved as seconds)
	case WET_STEP:
	case DRY_STEP:
	case REPORT_STEP:
		if (!datetime_strToTime(s2, &aTime))
		{
			return error_setInpError(ERR_DATETIME, s2);
		}
		datetime_decodeTime(aTime, &h, &m, &s);
		h += 24 * (int)aTime;
		s = s + 60 * m + 3600 * h;
		if (s <= 0) return error_setInpError(ERR_NUMBER, s2);
		switch (k)
		{
		case WET_STEP:     _aoptions.WetStep = s;     break;
		case DRY_STEP:     _aoptions.DryStep = s;     break;
		case REPORT_STEP:  _aoptions.ReportStep = s;  break;
		}
		break;

	// --- type of damping applied to inertial terms of dynamic wave routing
	case INERT_DAMPING:
		m = findmatch(s2, InertDampingWords);
		if (m < 0) return error_setInpError(ERR_KEYWORD, s2);
		else _aoptions.InertDamping = m;
		break;

	// --- Yes/No options (NO = 0, YES = 1)
	case ALLOW_PONDING:
	case SLOPE_WEIGHTING:
	case SKIP_STEADY_STATE:
	case IGNORE_RAINFALL:
	case IGNORE_SNOWMELT:
	case IGNORE_GWATER:
	case IGNORE_ROUTING:
	case IGNORE_QUALITY:
	case IGNORE_RDII:                                                        //(5.1.004)
		m = findmatch(s2, NoYesWords);
		if (m < 0) return error_setInpError(ERR_KEYWORD, s2);
		switch (k)
		{
		case ALLOW_PONDING:     _aoptions.AllowPonding = m;  break;
		case SLOPE_WEIGHTING:   _aoptions.SlopeWeighting = m;  break;
		case SKIP_STEADY_STATE: _aoptions.SkipSteadyState = m;  break;
		case IGNORE_RAINFALL:   _aoptions.IgnoreRainfall = m;  break;
		case IGNORE_SNOWMELT:   _aoptions.IgnoreSnowmelt = m;  break;
		case IGNORE_GWATER:     _aoptions.IgnoreGwater = m;  break;
		case IGNORE_ROUTING:    _aoptions.IgnoreRouting = m;  break;
		case IGNORE_QUALITY:    _aoptions.IgnoreQuality = m;  break;
		case IGNORE_RDII:       _aoptions.IgnoreRDII = m;  break;                 //(5.1.004)
		}
		break;

	case NORMAL_FLOW_LTD:
		m = findmatch(s2, NormalFlowWords);
		if (m < 0) m = findmatch(s2, NoYesWords);
		if (m < 0) return error_setInpError(ERR_KEYWORD, s2);
		_aoptions.NormalFlowLtd = m;
		break;

	case FORCE_MAIN_EQN:
		m = findmatch(s2, ForceMainEqnWords);
		if (m < 0) return error_setInpError(ERR_KEYWORD, s2);
		_aoptions.ForceMainEqn = m;
		break;

	case LINK_OFFSETS:
		m = findmatch(s2, LinkOffsetWords);
		if (m < 0) return error_setInpError(ERR_KEYWORD, s2);
		_aoptions.LinkOffsets = m;
		break;

	// --- compatibility option for selecting solution method for
	//     dynamic wave flow routing (NOT CURRENTLY USED)
	//case COMPATIBILITY:
	//	if (strcomp(s2, "3")) _aoptions.Compatibility = SWMM3;
	//	else if (strcomp(s2, "4")) _aoptions.Compatibility = SWMM4;
	//	else if (strcomp(s2, "5")) _aoptions.Compatibility = SWMM5;
	//	else return error_setInpError(ERR_KEYWORD, s2);
	//	break;

	// --- routing or lengthening time step (in decimal seconds)
	//     (lengthening time step is used in Courant stability formula
	//     to artificially lengthen conduits for dynamic wave flow routing
	//     (a value of 0 means that no lengthening is used))
	case ROUTE_STEP:
	case LENGTHENING_STEP:
		if (!getDouble(s2, &tStep))
		{
			if (!datetime_strToTime(s2, &aTime))
			{
				return error_setInpError(ERR_NUMBER, s2);
			}
			else
			{
				datetime_decodeTime(aTime, &h, &m, &s);
				h += 24 * (int)aTime;
				s = s + 60 * m + 3600 * h;
				tStep = s;
			}
		}
		if (k == ROUTE_STEP)
		{
			if (tStep <= 0.0) return error_setInpError(ERR_NUMBER, s2);
			_aoptions.RouteStep = tStep;
		}
		else _aoptions.LengtheningStep = MAX(0.0, tStep);
		break;

	////  Following code section added to release 5.1.008.  ////                   //(5.1.008)

	// --- minimum variable time step for dynamic wave routing
	case MIN_ROUTE_STEP:
		if (!getDouble(s2, &_aoptions.MinRouteStep) || _aoptions.MinRouteStep < 0.0)
			return error_setInpError(ERR_NUMBER, s2);
		break;

	case NUM_THREADS:
		m = atoi(s2);
		if (m < 0) return error_setInpError(ERR_NUMBER, s2);
		_aoptions.NumThreads = m;
		break;

	// --- safety factor applied to variable time step estimates under
	//     dynamic wave flow routing (value of 0 indicates that variable
	//     time step option not used)
	case VARIABLE_STEP:
		if (!getDouble(s2, &_aoptions.CourantFactor))
			return error_setInpError(ERR_NUMBER, s2);
		if (_aoptions.CourantFactor < 0.0 || _aoptions.CourantFactor > 2.0)
			return error_setInpError(ERR_NUMBER, s2);
		break;

	// --- minimum surface area (ft2 or sq. meters) associated with nodes
	//     under dynamic wave flow routing 
	case MIN_SURFAREA:
		_aoptions.MinSurfArea = atof(s2);
		break;

	// --- minimum conduit slope (%)
	case MIN_SLOPE:
		if (!getDouble(s2, &_aoptions.MinSlope))
			return error_setInpError(ERR_NUMBER, s2);
		if (_aoptions.MinSlope < 0.0 || _aoptions.MinSlope >= 100)
			return error_setInpError(ERR_NUMBER, s2);
		_aoptions.MinSlope /= 100.0;
		break;

	// --- maximum trials / time step for dynamic wave routing
	case MAX_TRIALS:
		m = atoi(s2);
		if (m < 0) return error_setInpError(ERR_NUMBER, s2);
		_aoptions.MaxTrials = m;
		break;

	// --- head convergence tolerance for dynamic wave routing
	case HEAD_TOL:
		if (!getDouble(s2, &_aoptions.HeadTol))
		{
			return error_setInpError(ERR_NUMBER, s2);
		}
		break;

	// --- steady state tolerance on system inflow - outflow
	case SYS_FLOW_TOL:
		if (!getDouble(s2, &_aoptions.SysFlowTol))
		{
			return error_setInpError(ERR_NUMBER, s2);
		}
		_aoptions.SysFlowTol /= 100.0;
		break;

	// --- steady state tolerance on nodal lateral inflow
	case LAT_FLOW_TOL:
		if (!getDouble(s2, &_aoptions.LatFlowTol))
		{
			return error_setInpError(ERR_NUMBER, s2);
		}
		_aoptions.LatFlowTol /= 100.0;
		break;

		//case TEMPDIR: // Temporary Directory
		//	sstrncpy(TempDir, s2, MAXFNAME);
		//	break;

	}
	return 0;
}

void SWMMLoader::ClearObjArrays()
{
	if (_aoptions.InfilModel == HORTON || _aoptions.InfilModel == MOD_HORTON)
	{
		delete[] _hortinfil;
		_hortinfil = NULL;
	}
	else if (_aoptions.InfilModel == GREEN_AMPT || _aoptions.InfilModel == MOD_GREEN_AMPT)
	{
		delete[] _gainfil;
		_gainfil = NULL;
	}
	else if (_aoptions.InfilModel == CURVE_NUMBER)
	{
		delete[] _cninfil;
		_cninfil = NULL;
	}

	delete[] _gages;
	delete[] _subcatches;
	delete[] _nodes;
	delete[] _outfalls;
	delete[] _tseries;
	delete[] _landuse;

	_gages = NULL;
	_subcatches = NULL;
	_nodes = NULL;
	_outfalls = NULL;
	_tseries = NULL;
	_landuse = NULL;

	if (_status == 1) // destructor is being called
	{
		for (int j = 0; j < _Nobjects[SUBCATCH]; j++)
		{
			DeleteLidGroup(j);
		}
	}

	delete[] _lidGroups;
	delete[] _lidProcs;
	//_GroupCount = 0;
	//_LidCount = 0;

	_lidGroups = NULL;
	_lidProcs = NULL;

}

// similar to createObjects() in SWMM
void SWMMLoader::AllocObjArrays()
{
	int j;

	// make sure any previous values are disposed of 
	ClearObjArrays(); 
	_status = 1;

	//() sets all space to zero
	_gages = new TGage[_Nobjects[GAGE]]();
	_subcatches = new TSubcatch[_Nobjects[SUBCATCH]]();
	_nodes = new TNode[_Nobjects[NODE]]();
	_outfalls = new TOutfall[_Nobjects[OUTFALL]]();
	_landuse = new TLanduse[_Nobjects[LANDUSE]]();
	_tseries = new TTable[_Nobjects[TSERIES]]();
	
	// --- allocate memory for infiltration data
	InfilCreate(_Nobjects[SUBCATCH], _aoptions.InfilModel); // uses new

	LidCreate(_Nobjects[LID], _Nobjects[SUBCATCH]); // uses new
	
	//add more as needed
	
	// --- initialize rain gage properties
	for (j = 0; j < _Nobjects[GAGE]; j++)
	{
		_gages[j].tSeries = -1;
		strcpy(_gages[j].fname, "");
	}

	// --- initialize subcatchment properties
	for (j = 0; j < _Nobjects[SUBCATCH]; j++)
	{
		_subcatches[j].outSubcatch = -1;
		_subcatches[j].outNode = -1;
		_subcatches[j].infil = -1;
		_subcatches[j].groundwater = NULL;
		_subcatches[j].gwLatFlowExpr = NULL;                                      //(5.1.007)
		_subcatches[j].gwDeepFlowExpr = NULL;                                     //(5.1.007)
		_subcatches[j].snowpack = NULL;
		_subcatches[j].lidArea = 0.0;
		//for (k = 0; k < _Nobjects[POLLUT]; k++)
		//{
		//	Subcatch[j].initBuildup[k] = 0.0;
		//}
	}

	//  --- initialize curves, time series, and time patterns
	//for (j = 0; j < Nobjects[CURVE]; j++)   table_init(&Curve[j]);
	for (j = 0; j < _Nobjects[TSERIES]; j++) table_init(&_tseries[j]);
	//for (j = 0; j < Nobjects[TIMEPATTERN]; j++) inflow_initDwfPattern(j);

}

int SWMMLoader::ReadData()
{
	char  line[MAXLINE + 1];      // line from input data file
	char  wLine[MAXLINE + 1];     // working copy of input line
	char* comment;                // ptr. to start of comment in input line
	int   sect, newsect;          // data sections
	int   inperr, errsum;         // error code & total error count
	int   lineLength;             // number of characters in input line
	int   i;
	long  lineCount = 0;

	// --- read each line from input file
	rewind(_inFile);
	sect = 0;
	errsum = 0;

	// --- initialize starting date for all time series
	for (i = 0; i < _Nobjects[TSERIES]; i++)
	{
		_tseries[i].lastDate = _timelist.StartDate + _timelist.StartTime;
	}

	while (fgets(line, MAXLINE, _inFile) != NULL)
	{
		// --- make copy of line and scan for tokens
		lineCount++;
		strcpy(wLine, line);
		_Ntokens = GetTokens(wLine);

		// --- skip blank lines and comments
		if (_Ntokens == 0) continue;
		if (*_Tok[0] == ';') continue;

		// --- check if max. line length exceeded
		lineLength = strlen(line);
		if (lineLength >= MAXLINE)
		{
			// --- don't count comment if present
			comment = strchr(line, ';');
			if (comment) lineLength = comment - line;    // Pointer math here
			if (lineLength >= MAXLINE)
			{
				inperr = ERR_LINE_LENGTH;
				// report_writeInputErrorMsg(inperr, sect, line, lineCount);
				errsum++;
			}
		}

		// --- check if at start of a new input section
		if (*_Tok[0] == '[')
		{
			// --- match token against list of section keywords
			newsect = findmatch(_Tok[0], SectWords);
			if (newsect >= 0)
			{
				// // --- SPECIAL CASE FOR TRANSECTS
				// //     finish processing the last set of transect data
				// if ( sect == s_TRANSECT )
				//     transect_validate(Nobjects[TRANSECT]-1);

				// --- begin a new input section
				sect = newsect;
				continue;
			}
			else
			{
				SetError(ERR_KEYWORD, _Tok[0]);
				//report_writeInputErrorMsg(inperr, sect, line, lineCount);
				errsum++;
				break;
			}
		}

		// --- otherwise parse tokens from input line
		else
		{
			inperr = ParseLine(sect, line);
			if (inperr > 0)
			{
				//errsum++; //TODO figure out what to do about this
				//if ( errsum > MAXERRS ) report_writeLine(FMT19);
				//else report_writeInputErrorMsg(inperr, sect, line, lineCount);
			}
		}

		// --- stop if reach end of file or max. error count
		if (errsum > MAXERRS) break;
	}   /* End of while */

	// --- check for errors
	if (errsum > 0)  _errCode = ERR_INPUT;
	return _errCode;

}

int  SWMMLoader::ParseLine(int sect, char *line)
{
	int j, err;
	switch (sect)
	{
	//add more as needed.
	//see parseLine() in input.c

	case s_RAINGAGE:
		j = _Mobjects[GAGE];
		err = ReadGageParams(j, _Tok, _Ntokens);
		_Mobjects[GAGE]++;
		return err;

	case s_EVAP:
		return ClimateReadEvapParams(_Tok, _Ntokens);

	case s_SUBCATCH:
		j = _Mobjects[SUBCATCH];
		err = ReadSubcatchParams(j, _Tok, _Ntokens);
		_Mobjects[SUBCATCH]++;
		return err;

	case s_SUBAREA:
		return ReadSubareaParams(_Tok, _Ntokens);

	case s_INFIL:
		return InfilReadParams(_aoptions.InfilModel, _Tok, _Ntokens);

	case s_JUNCTION:
		return ReadNode(JUNCTION);					

	case s_OUTFALL:
		return ReadNode(OUTFALL);

	case s_LANDUSE:
		j = _Mobjects[LANDUSE];
		return LanduseReadParams(j, _Tok, _Ntokens);
		_Mobjects[LANDUSE]++;
		return err;

	case s_TIMESERIES:
		return TableReadTimeseries(_Tok, _Ntokens); 

	case s_REPORT:
		return ReportReadOptions(_Tok, _Ntokens);

	case s_LID_CONTROL:
		return LidReadProcParams(_Tok, _Ntokens);

	case s_LID_USAGE:
		return LidReadGroupParams(_Tok, _Ntokens);

	default: return 0;
	}
}

int  SWMMLoader::LanduseReadParams(int j, char* tok[], int ntoks)
//
//  Input:   j = land use index
//           tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads landuse parameters from a tokenized line of input.
//
//  Data format is:
//    landuseID  (sweepInterval sweepRemoval sweepDays0)
//
{
	char *id;
	if (ntoks < 1) return error_setInpError(ERR_ITEMS, "");
	id = project_findID(LANDUSE, tok[0]);
	if (id == NULL) return error_setInpError(ERR_NAME, tok[0]);
	_landuse[j].ID = id;
	if (ntoks > 1)
	{
		if (ntoks < 4) return error_setInpError(ERR_ITEMS, "");
		if (!getDouble(tok[1], &_landuse[j].sweepInterval))
			return error_setInpError(ERR_NUMBER, tok[1]);
		if (!getDouble(tok[2], &_landuse[j].sweepRemoval))
			return error_setInpError(ERR_NUMBER, tok[2]);
		if (!getDouble(tok[3], &_landuse[j].sweepDays0))
			return error_setInpError(ERR_NUMBER, tok[3]);
	}
	else
	{
		_landuse[j].sweepInterval = 0.0;
		_landuse[j].sweepRemoval = 0.0;
		_landuse[j].sweepDays0 = 0.0;
	}
	if (_landuse[j].sweepRemoval < 0.0
		|| _landuse[j].sweepRemoval > 1.0)
		return error_setInpError(ERR_NUMBER, tok[2]);
	return 0;
}

int SWMMLoader::ReportReadOptions(char* tok[], int ntoks)
//
//  Input:   tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads reporting options from a line of input
//
{
	char  k;
	int   j, m, t;
	if (ntoks < 2) return error_setInpError(ERR_ITEMS, "");
	k = (char)findmatch(tok[0], ReportWords);
	if (k < 0) return error_setInpError(ERR_KEYWORD, tok[0]);
	switch (k)
	{
	case 0: // Input
		m = findmatch(tok[1], NoYesWords);
		if (m == YES) _rptFlags.input = TRUE;
		else if (m == NO)  _rptFlags.input = FALSE;
		else                 return error_setInpError(ERR_KEYWORD, tok[1]);
		return 0;

	case 1: // Continuity
		m = findmatch(tok[1], NoYesWords);
		if (m == YES) _rptFlags.continuity = TRUE;
		else if (m == NO)  _rptFlags.continuity = FALSE;
		else                 return error_setInpError(ERR_KEYWORD, tok[1]);
		return 0;

	case 2: // Flow Statistics
		m = findmatch(tok[1], NoYesWords);
		if (m == YES) _rptFlags.flowStats = TRUE;
		else if (m == NO)  _rptFlags.flowStats = FALSE;
		else                 return error_setInpError(ERR_KEYWORD, tok[1]);
		return 0;

	case 3: // Controls
		m = findmatch(tok[1], NoYesWords);
		if (m == YES) _rptFlags.controls = TRUE;
		else if (m == NO)  _rptFlags.controls = FALSE;
		else                 return error_setInpError(ERR_KEYWORD, tok[1]);
		return 0;

	case 4:  m = SUBCATCH;  break;  // Subcatchments
	case 5:  m = NODE;      break;  // Nodes
	case 6:  m = LINK;      break;  // Links

	case 7: // Node Statistics
		m = findmatch(tok[1], NoYesWords);
		if (m == YES) _rptFlags.nodeStats = TRUE;
		else if (m == NO)  _rptFlags.nodeStats = FALSE;
		else                 return error_setInpError(ERR_KEYWORD, tok[1]);
		return 0;

	default: return error_setInpError(ERR_KEYWORD, tok[1]);
	}
	k = (char)findmatch(tok[1], NoneAllWords);
	if (k < 0)
	{
		k = SOME;
		for (t = 1; t < ntoks; t++)
		{
			j = ProjectFindObject(m, tok[t]);
			if (j < 0) return error_setInpError(ERR_NAME, tok[t]);
			switch (m)
			{
			case SUBCATCH:  _subcatches[j].rptFlag = TRUE;  break;
			case NODE:      _nodes[j].rptFlag = TRUE;  break;
			//case LINK:      _links[j].rptFlag = TRUE;  break;
			}
		}
	}
	switch (m)
	{
	case SUBCATCH: _rptFlags.subcatchments = k;  break;
	case NODE:     _rptFlags.nodes = k;  break;
	case LINK:     _rptFlags.links = k;  break;
	}
	return 0;
}


// if you move to reading from a separate file, you'll need to move the external variable Frain (type Tfile)
// into the class
int SWMMLoader::TableReadTimeseries(char* tok[], int ntoks)
//
//  Input:   tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads a tokenized line of data for a time series table.
//
{
	int    j;                          // time series index
	int    k;                          // token index
	int    state;                      // 1: next token should be a date
	// 2: next token should be a time
	// 3: next token should be a value 
	double x, y;                       // time & value table entries
	DateTime d;                        // day portion of date/time value
	DateTime t;                        // time portion of date/time value

	// --- check for minimum number of tokens
	if (ntoks < 3) return error_setInpError(ERR_ITEMS, "");

	// --- check that time series exists in database
	j = ProjectFindObject(TSERIES, tok[0]);
	if (j < 0) return error_setInpError(ERR_NAME, tok[0]);

	// --- if first line of data, assign ID pointer
	if (_tseries[j].ID == NULL)
		_tseries[j].ID = ProjectFindID(TSERIES, tok[0]);

	// --- check if time series data is in an external file
	if (strcomp(tok[1], w_FILE))
	{
		sstrncpy(_tseries[j].file.name, tok[2], MAXFNAME);
		_tseries[j].file.mode = USE_FILE;
		return 0;
	}

	// --- parse each token of input line
	x = 0.0;
	k = 1;
	state = 1;               // start off looking for a date
	while (k < ntoks)
	{
		switch (state)
		{
		case 1:            // look for a date entry
			if (datetime_strToDate(tok[k], &d))
			{
				_tseries[j].lastDate = d;
				k++;
			}

			// --- next token must be a time
			state = 2;
			break;

		case 2:            // look for a time entry
			if (k >= ntoks) return error_setInpError(ERR_ITEMS, "");

			// --- first check for decimal hours format
			if (getDouble(tok[k], &t)) t /= 24.0;

			// --- then for an hrs:min format
			else if (!datetime_strToTime(tok[k], &t))
				return error_setInpError(ERR_NUMBER, tok[k]);

			// --- save date + time in x
			x = _tseries[j].lastDate + t;

			// --- next token must be a numeric value
			k++;
			state = 3;
			break;

		case 3:
			// --- extract a numeric value from token
			if (k >= ntoks) return error_setInpError(ERR_ITEMS, "");
			if (!getDouble(tok[k], &y))
				return error_setInpError(ERR_NUMBER, tok[k]);

			// --- add date/time & value to time series -- called directly from table.c
			table_addEntry(&_tseries[j], x, y);

			// --- start over looking first for a date
			k++;
			state = 1;
			break;
		}
	}
	return 0;
}

int SWMMLoader::ReadNode(int type)
//
//  Input:   type = type of node
//  Output:  returns error code
//  Purpose: reads data for a node from a line of input.
//
{
	int j = _Mobjects[NODE];
	int k = _Mnodes[type];
	int err = ReadNodeParams(j, type, k, _Tok, _Ntokens);
	_Mobjects[NODE]++;
	_Mnodes[type]++;
	return err;
}

int SWMMLoader::ReadNodeParams(int j, int type, int k, char* tok[], int ntoks)
//
//  Input:   j = node index
//           type = node type code
//           k = index of node type
//           tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads node properties from a tokenized line of input.
//
{
	switch (type)
	{
	case JUNCTION: return JuncReadParams(j, k, tok, ntoks);
	case OUTFALL:  return OutfallReadParams(j, k, tok, ntoks);
	//case STORAGE:  return storage_readParams(j, k, tok, ntoks);
	//case DIVIDER:  return divider_readParams(j, k, tok, ntoks);
	default:       return 0;
	}
}

int SWMMLoader::JuncReadParams(int j, int k, char* tok[], int ntoks)
//
//  Input:   j = node index
//           k = junction index
//           tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error message
//  Purpose: reads a junction's properties from a tokenized line of input.
//
//  Format of input line is:
//     nodeID  elev  maxDepth  initDepth  surDepth  aPond 
{
	int    i;
	double x[6];
	char*  id;

	if (ntoks < 2) return error_setInpError(ERR_ITEMS, "");
	id = ProjectFindID(NODE, tok[0]);
	if (id == NULL) return error_setInpError(ERR_NAME, tok[0]);

	// --- parse invert elev., max. depth, init. depth, surcharged depth,
	//     & ponded area values
	for (i = 1; i <= 5; i++)
	{
		x[i - 1] = 0.0;
		if (i < ntoks)
		{
			if (!getDouble(tok[i], &x[i - 1]))
				return error_setInpError(ERR_NUMBER, tok[i]);
		}
	}

	// --- check for non-negative values (except for invert elev.)
	for (i = 1; i <= 4; i++)
	{
		if (x[i] < 0.0) return error_setInpError(ERR_NUMBER, tok[i + 1]);
	}

	// --- add parameters to junction object
	_nodes[j].ID = id;
	NodeSetParams(j, JUNCTION, k, x);
	return 0;
}

void  SWMMLoader::NodeSetParams(int j, int type, int k, double x[])
{
	_nodes[j].type = type;
	_nodes[j].subIndex = k;
	_nodes[j].invertElev = x[0] / UCF(LENGTH);
	_nodes[j].crownElev = _nodes[j].invertElev;
	_nodes[j].initDepth = 0.0;
	_nodes[j].newVolume = 0.0;
	_nodes[j].fullVolume = 0.0;
	_nodes[j].fullDepth = 0.0;
	_nodes[j].surDepth = 0.0;
	_nodes[j].pondedArea = 0.0;
	_nodes[j].degree = 0;
	switch (type)
	{
	case JUNCTION:
		_nodes[j].fullDepth = x[1] / UCF(LENGTH);
		_nodes[j].initDepth = x[2] / UCF(LENGTH);
		_nodes[j].surDepth = x[3] / UCF(LENGTH);
		_nodes[j].pondedArea = x[4] / (UCF(LENGTH)*UCF(LENGTH));
		break;

	case OUTFALL:
		_outfalls[k].type = (int)x[1];
		_outfalls[k].fixedStage = x[2] / UCF(LENGTH);
		_outfalls[k].tideCurve = (int)x[3];
		_outfalls[k].stageSeries = (int)x[4];
		_outfalls[k].hasFlapGate = (char)x[5];

		////  Following code segment added to release 5.1.008.  ////                   //(5.1.008)

		_outfalls[k].routeTo = (int)x[6];
		_outfalls[k].wRouted = NULL;
		if (_outfalls[k].routeTo >= 0)
		{
			_outfalls[k].wRouted =
				(double *)calloc(_Nobjects[POLLUT], sizeof(double));
		}
		////
		break;

	//case STORAGE:
	//	Node[j].fullDepth = x[1] / UCF(LENGTH);
	//	Node[j].initDepth = x[2] / UCF(LENGTH);
	//	Storage[k].aCoeff = x[3];
	//	Storage[k].aExpon = x[4];
	//	Storage[k].aConst = x[5];
	//	Storage[k].aCurve = (int)x[6];
	//	// x[7] (ponded depth) is deprecated.                                  //(5.1.007)
	//	Storage[k].fEvap = x[8];
	//	break;

	//case DIVIDER:
	//	Divider[k].link = (int)x[1];
	//	Divider[k].type = (int)x[2];
	//	Divider[k].flowCurve = (int)x[3];
	//	Divider[k].qMin = x[4] / UCF(FLOW);
	//	Divider[k].dhMax = x[5];
	//	Divider[k].cWeir = x[6];
	//	Node[j].fullDepth = x[7] / UCF(LENGTH);
	//	Node[j].initDepth = x[8] / UCF(LENGTH);
	//	Node[j].surDepth = x[9] / UCF(LENGTH);
	//	Node[j].pondedArea = x[10] / (UCF(LENGTH)*UCF(LENGTH));
	//	break;
	}
}

int SWMMLoader::OutfallReadParams(int j, int k, char* tok[], int ntoks)
//
//  Input:   j = node index
//           k = outfall index
//           tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error message
//  Purpose: reads an outfall's properties from a tokenized line of input.
//
//  Format of input line is:
//    nodeID  elev  FIXED  fixedStage (flapGate) (routeTo)
//    nodeID  elev  TIDAL  curveID (flapGate) (routeTo)
//    nodeID  elev  TIMESERIES  tseriesID (flapGate) (routTo)
//    nodeID  elev  FREE (flapGate) (routeTo)
//    nodeID  elev  NORMAL (flapGate) (routeTo)
//
{
	int    i, m, n;
	double x[7];                                                               //(5.1.008)
	char*  id;

	if (ntoks < 3) return error_setInpError(ERR_ITEMS, "");
	id = ProjectFindID(NODE, tok[0]);                      // node ID
	if (id == NULL)
		return error_setInpError(ERR_NAME, tok[0]);
	if (!getDouble(tok[1], &x[0]))                       // invert elev. 
		return error_setInpError(ERR_NUMBER, tok[1]);
	i = findmatch(tok[2], OutfallTypeWords);               // outfall type
	if (i < 0) return error_setInpError(ERR_KEYWORD, tok[2]);
	x[1] = i;                                              // outfall type
	x[2] = 0.0;                                            // fixed stage
	x[3] = -1.;                                            // tidal curve
	x[4] = -1.;                                            // tide series
	x[5] = 0.;                                             // flap gate
	x[6] = -1.;                                            // route to subcatch//(5.1.008)

	n = 4;
	if (i >= FIXED_OUTFALL)
	{
		if (ntoks < 4) return error_setInpError(ERR_ITEMS, "");
		n = 5;
		switch (i)
		{
		case FIXED_OUTFALL:                                // fixed stage
			if (!getDouble(tok[3], &x[2]))
				return error_setInpError(ERR_NUMBER, tok[3]);
			break;
		case TIDAL_OUTFALL:                                // tidal curve
			m = ProjectFindObject(CURVE, tok[3]);
			if (m < 0) return error_setInpError(ERR_NAME, tok[3]);
			x[3] = m;
			break;
		//case TIMESERIES_OUTFALL:                           // stage time series
		//	m = project_findObject(TSERIES, tok[3]);
		//	if (m < 0) return error_setInpError(ERR_NAME, tok[3]);
		//	x[4] = m;
		//	_Tseries[m].refersTo = TIMESERIES_OUTFALL;
		}
	}
	if (ntoks == n)
	{
		m = findmatch(tok[n - 1], NoYesWords);               // flap gate
		if (m < 0) return error_setInpError(ERR_KEYWORD, tok[n - 1]);
		x[5] = m;
	}

	////  Added for release 5.1.008.  ////                                         //(5.1.008)
	if (ntoks == n + 1)
	{
		m = ProjectFindObject(SUBCATCH, tok[n]);
		if (m < 0) return error_setInpError(ERR_NAME, tok[n]);
		x[6] = m;
	}
	////

	_nodes[j].ID = id;
	NodeSetParams(j, OUTFALL, k, x);
	return 0;
}

int SWMMLoader::ReadGageParams(int j, char* tok[], int ntoks)
{
	int      k, err;
	char     *id;
	char     fname[MAXFNAME + 1];
	char     staID[MAXMSG + 1];
	double   x[7];

	// --- check that gage exists
	if (ntoks < 2) return error_setInpError(ERR_ITEMS, "");
	id = ProjectFindID(GAGE, tok[0]);
	if (id == NULL) return error_setInpError(ERR_NAME, tok[0]);

	// --- assign default parameter values
	x[0] = -1.0;         // No time series index
	x[1] = 1.0;          // Rain type is volume
	x[2] = 3600.0;       // Recording freq. is 3600 sec
	x[3] = 1.0;          // Snow catch deficiency factor
	x[4] = NO_DATE;      // Default is no start/end date
	x[5] = NO_DATE;
	x[6] = 0.0;          // US units
	strcpy(fname, "");
	strcpy(staID, "");

	if (ntoks < 5)
	{
		SetError(ERR_ITEMS, "");
		return _errCode;
	}

	k = findmatch(tok[4], GageDataWords);
	if (k == RAIN_TSERIES)
	{
		err = GageReadSeriesFormat(tok, ntoks, x);
	}
	else if (k == RAIN_FILE)
	{
		if (ntoks < 8)
		{
			SetError(ERR_ITEMS, "");
			return _errCode;
		}
		strncpy(fname, tok[5], MAXFNAME);
		strncpy(staID, tok[6], MAXMSG);
		// if you want to read from a file, scrape from gage.c and change to use correct hash table
		//  err = gage_readFileFormat(tok, ntoks, x);
	}
	else
	{
		SetError(ERR_KEYWORD, tok[4]);
		return _errCode;
	}

	// --- save parameters to rain gage object
	if (err > 0) return err;
	_gages[j].ID = id;
	_gages[j].tSeries = (int)x[0];
	_gages[j].rainType = (int)x[1];
	_gages[j].rainInterval = (int)x[2];
	_gages[j].snowFactor = x[3];
	_gages[j].rainUnits = (int)x[6];
	if (_gages[j].tSeries >= 0) _gages[j].dataSource = RAIN_TSERIES;
	else                        _gages[j].dataSource = RAIN_FILE;
	if (_gages[j].dataSource == RAIN_FILE)
	{
		strncpy(_gages[j].fname, fname, MAXFNAME);
		strncpy(_gages[j].staID, staID, MAXMSG);
		_gages[j].startFileDate = x[4];
		_gages[j].endFileDate = x[5];
	}
	_gages[j].unitsFactor = 1.0;
	_gages[j].coGage = -1;
	_gages[j].isUsed = FALSE;
	return 0;
}

int SWMMLoader::GageReadSeriesFormat(char* tok[], int ntoks, double x[])
{
	int m, ts;
	DateTime aTime;

	if (ntoks < 6) return error_setInpError(ERR_ITEMS, "");

	// --- determine type of rain data
	m = findmatch(tok[1], RainTypeWords);
	if (m < 0) return error_setInpError(ERR_KEYWORD, tok[1]);
	x[1] = (double)m;

	// --- get data time interval & convert to seconds
	if (getDouble(tok[2], &x[2])) x[2] = floor(x[2] * 3600 + 0.5);
	else if (datetime_strToTime(tok[2], &aTime))
	{
		x[2] = floor(aTime*SECperDAY + 0.5);
	}
	else return error_setInpError(ERR_DATETIME, tok[2]);
	if (x[2] <= 0.0) return error_setInpError(ERR_DATETIME, tok[2]);

	// --- get snow catch deficiency factor
	if (!getDouble(tok[3], &x[3]))
		return error_setInpError(ERR_DATETIME, tok[3]);;

	// --- get time series index
	ts = ProjectFindObject(TSERIES, tok[5]);
	if (ts < 0) return error_setInpError(ERR_NAME, tok[5]);
	x[0] = (double)ts;
	strcpy(tok[2], "");
	return 0;
}

int  SWMMLoader::ReadSubcatchParams(int j, char* tok[], int ntoks)
{
	int    i, k, m;
	char*  id;
	double x[9];

	// --- check for enough tokens
	if (ntoks < 8)
	{
		SetError(ERR_ITEMS, "");
		return _errCode;
	}

	// --- check that named subcatch exists
	id = ProjectFindID(SUBCATCH, tok[0]);
	if (id == NULL) return error_setInpError(ERR_NAME, tok[0]);

	// --- check that rain gage exists
	k = ProjectFindObject(GAGE, tok[1]);
	if (k < 0) return error_setInpError(ERR_NAME, tok[1]);
	x[0] = k;

	// --- check that outlet node or subcatch exists
	m = ProjectFindObject(NODE, tok[2]);
	x[1] = m;
	m = ProjectFindObject(SUBCATCH, tok[2]);
	x[2] = m;
	if (x[1] < 0.0 && x[2] < 0.0)
		return error_setInpError(ERR_NAME, tok[2]);

	// --- read area, %imperv, width, slope, & curb length
	for (i = 3; i < 8; i++)
	{
		if (!getDouble(tok[i], &x[i]) || x[i] < 0.0)
			return error_setInpError(ERR_NUMBER, tok[i]);
	}

	// --- if snowmelt object named, check that it exists
	x[8] = -1;
	if (ntoks > 8)
	{
		k = ProjectFindObject(SNOWMELT, tok[8]);
		if (k < 0) return error_setInpError(ERR_NAME, tok[8]);
		x[8] = k;
	}

	// --- assign input values to subcatch's properties
	_subcatches[j].ID = id;
	_subcatches[j].gage = (int)x[0];
	_subcatches[j].outNode     = (int)x[1];
	_subcatches[j].outSubcatch = (int)x[2]; 
	_subcatches[j].area = x[3] / UCF(LANDAREA);
	_subcatches[j].fracImperv = x[4] / 100.0;
	_subcatches[j].width = x[5] / UCF(LENGTH);
	_subcatches[j].slope = x[6] / 100.0;
	_subcatches[j].curbLength = x[7];

	// --- create the snow pack object if it hasn't already been created -- don't have snowpacks yet
	// if ( x[8] >= 0 )
	// {
	//     if ( !snow_createSnowpack(j, (int)x[8]) )
	//         return error_setInpError(ERR_MEMORY, "");
	// }
	return 0;
}

int SWMMLoader::ReadSubareaParams(char* tok[], int ntoks)
//
//
//  Input:   tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads subcatchment's subarea parameters from a tokenized 
//           line of input data.
//
//  Data has format:
//    Subcatch  Imperv_N  Perv_N  Imperv_S  Perv_S  PctZero  RouteTo (PctRouted)
//
{
	int    i, j, k, m;
	double x[7];
	char** RunoffRoutingWords = SubcatchGetRunoffRoutingWords();

	// --- check for enough tokens
	if (ntoks < 7) return error_setInpError(ERR_ITEMS, "");

	// --- check that named subcatch exists
	j = ProjectFindObject(SUBCATCH, tok[0]);
	if (j < 0) return error_setInpError(ERR_NAME, tok[0]);

	// --- read in Mannings n, depression storage, & PctZero values
	for (i = 0; i < 5; i++)
	{
		if (!getDouble(tok[i + 1], &x[i]) || x[i] < 0.0)
			return error_setInpError(ERR_NAME, tok[i + 1]);
	}

	// --- check for valid runoff routing keyword
	m = findmatch(tok[6], RunoffRoutingWords);
	if (m < 0) return error_setInpError(ERR_KEYWORD, tok[6]);

	// --- get percent routed parameter if present (default is 100)
	x[5] = m;
	x[6] = 1.0;
	if (ntoks >= 8)
	{
		if (!getDouble(tok[7], &x[6]) || x[6] < 0.0 || x[6] > 100.0)
			return error_setInpError(ERR_NUMBER, tok[7]);
		x[6] /= 100.0;
	}

	// --- assign input values to each type of subarea
	_subcatches[j].subArea[IMPERV0].N = x[0];
	_subcatches[j].subArea[IMPERV1].N = x[0];
	_subcatches[j].subArea[PERV].N = x[1];

	_subcatches[j].subArea[IMPERV0].dStore = 0.0;
	_subcatches[j].subArea[IMPERV1].dStore = x[2] / UCF(RAINDEPTH);
	_subcatches[j].subArea[PERV].dStore = x[3] / UCF(RAINDEPTH);

	_subcatches[j].subArea[IMPERV0].fArea = _subcatches[j].fracImperv * x[4] / 100.0;
	_subcatches[j].subArea[IMPERV1].fArea = _subcatches[j].fracImperv * (1.0 - x[4] / 100.0);
	_subcatches[j].subArea[PERV].fArea = (1.0 - _subcatches[j].fracImperv);

	// --- assume that all runoff from each subarea goes to subcatch outlet
	for (i = IMPERV0; i <= PERV; i++)
	{
		_subcatches[j].subArea[i].routeTo = TO_OUTLET;
		_subcatches[j].subArea[i].fOutlet = 1.0;
	}

	// --- modify routing if pervious runoff routed to impervious area
	//     (fOutlet is the fraction of runoff not routed)

	k = (int)x[5];
	if (_subcatches[j].fracImperv == 0.0
		|| _subcatches[j].fracImperv == 1.0) k = TO_OUTLET;
	if (k == TO_IMPERV && _subcatches[j].fracImperv)
	{
		_subcatches[j].subArea[PERV].routeTo = k;
		_subcatches[j].subArea[PERV].fOutlet = 1.0 - x[6];
	}

	// --- modify routing if impervious runoff routed to pervious area
	if (k == TO_PERV)
	{
		_subcatches[j].subArea[IMPERV0].routeTo = k;
		_subcatches[j].subArea[IMPERV1].routeTo = k;
		_subcatches[j].subArea[IMPERV0].fOutlet = 1.0 - x[6];
		_subcatches[j].subArea[IMPERV1].fOutlet = 1.0 - x[6];
	}
	return 0;
}

int SWMMLoader::ClimateReadEvapParams(char* tok[], int ntoks)
//
//  Input:   tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns error code
//  Purpose: reads evaporation parameters from input line of data.
//
//  Data formats are:
//    CONSTANT  value
//    MONTHLY   v1 ... v12
//    TIMESERIES name
//    TEMPERATURE
//    FILE      (v1 ... v12)
//    RECOVERY   name
//    DRY_ONLY   YES/NO
//
{
	int i, k;
	double x;

	// --- find keyword indicating what form the evaporation data is in
	k = findmatch(tok[0], EvapTypeWords);
	if (k < 0) return error_setInpError(ERR_KEYWORD, tok[0]);

	// --- check for RECOVERY pattern data
	if (k == RECOVERY)
	{
		if (ntoks < 2) return error_setInpError(ERR_ITEMS, "");
		i = project_findObject(TIMEPATTERN, tok[1]);
		if (i < 0) return error_setInpError(ERR_NAME, tok[1]);
		_evap.recoveryPattern = i;
		return 0;
	}

	// --- check for no evaporation in wet periods
	if (k == DRYONLY)
	{
		if (ntoks < 2) return error_setInpError(ERR_ITEMS, "");
		if (strcomp(tok[1], w_NO))  _evap.dryOnly = FALSE;
		else if (strcomp(tok[1], w_YES)) _evap.dryOnly = TRUE;
		else return error_setInpError(ERR_KEYWORD, tok[1]);
		return 0;
	}

	// --- process data depending on its form
	_evap.type = k;
	if (k != TEMPERATURE_EVAP && ntoks < 2)
		return error_setInpError(ERR_ITEMS, "");
	switch (k)
	{
	case CONSTANT_EVAP:
		// --- for constant evap., fill monthly avg. values with same number
		if (!getDouble(tok[1], &x))
			return error_setInpError(ERR_NUMBER, tok[1]);
		for (i = 0; i<12; i++) _evap.monthlyEvap[i] = x;
		break;

	case MONTHLY_EVAP:
		// --- for monthly evap., read a value for each month of year
		if (ntoks < 13) return error_setInpError(ERR_ITEMS, "");
		for (i = 0; i<12; i++)
		if (!getDouble(tok[i + 1], &_evap.monthlyEvap[i]))
			return error_setInpError(ERR_NUMBER, tok[i + 1]);
		break;

	case TIMESERIES_EVAP:
		// --- for time series evap., read name of time series
		i = ProjectFindObject(TSERIES, tok[1]);
		if (i < 0) return error_setInpError(ERR_NAME, tok[1]);
		_evap.tSeries = i;
		_tseries[i].refersTo = TIMESERIES_EVAP;
		break;

	case FILE_EVAP:
		// --- for evap. from climate file, read monthly pan coeffs.
		//     if they are provided (default values are 1.0)
		if (ntoks > 1)
		{
			if (ntoks < 13) return error_setInpError(ERR_ITEMS, "");
			for (i = 0; i<12; i++)
			{
				if (!getDouble(tok[i + 1], &_evap.panCoeff[i]))
					return error_setInpError(ERR_NUMBER, tok[i + 1]);
			}
		}
		break;
	}
	return 0;
}

void SWMMLoader::LidCreate(int lidCount, int subcatchCount)
//
//  Purpose: creates an array of LID objects.
//  Input:   n = number of LID processes
//  Output:  none
//
{
	int j;

	//... assign NULL values to LID arrays
	_lidProcs = NULL;
	_lidGroups = NULL;
	_LidCount = lidCount;

	//... create LID groups
	_GroupCount = subcatchCount;
	if (_GroupCount == 0) return;
	_lidGroups = new TLidGroup[_GroupCount]();
	if (_lidGroups == NULL)
	{
		_errCode = ERR_MEMORY;
		return;
	}

	/*... initialize LID groups
	_GroupCount is _Nobjects[SUBCATCH]
	each subcatch has a _lidGroups[subcatch index]
	if a subcatch has no associated LIDs, _lidGroups[subcatch index] = NULL*/
	for (j = 0; j < _GroupCount; j++) _lidGroups[j] = NULL; 

	/*... create LID objects
	_LidCount is the actual number of LIDs in the .inp file (_Nobjects[LID])
	each LID is stored as a _lidProc[lid process index]
	lid process indices are in the order listed in the [LID_CONTROLS] section of the .inp file*/
	if (_LidCount == 0) return;
	_lidProcs = new TLidProc[_LidCount]();		
	if (_lidProcs == NULL)
	{
		_errCode = ERR_MEMORY;
		return;
	}

	//... initialize LID objects
	for (j = 0; j < _LidCount; j++)
	{
		_lidProcs[j].lidType = -1;
		_lidProcs[j].surface.thickness = 0.0;
		_lidProcs[j].surface.voidFrac = 1.0;
		_lidProcs[j].surface.roughness = 0.0;
		_lidProcs[j].surface.surfSlope = 0.0;
		_lidProcs[j].pavement.thickness = 0.0;
		_lidProcs[j].soil.thickness = 0.0;
		_lidProcs[j].storage.thickness = 0.0;
		_lidProcs[j].storage.kSat = 0.0;
		_lidProcs[j].drain.coeff = 0.0;
		_lidProcs[j].drain.offset = 0.0;
		_lidProcs[j].drainMat.thickness = 0.0;
		_lidProcs[j].drainMat.roughness = 0.0;
	}
}

int SWMMLoader::LidReadProcParams(char* toks[], int ntoks)
//
//  Purpose: reads LID process information from line of input data file
//  Input:   toks = array of string tokens
//           ntoks = number of tokens
//  Output:  returns error code
//
//  Format for first line that defines a LID process is:
//    LID_ID  LID_Type
//
//  Followed by some combination of lines below depending on LID_Type:
//    LID_ID  SURFACE   <parameters>
//    LID_ID  PAVEMENT  <parameters>
//    LID_ID  SOIL      <parameters>
//    LID_ID  STORAGE   <parameters>
//    LID_ID  DRAIN     <parameters>
//    LID_ID  DRAINMAT  <parameters>
//
{
	int j, m;

	// --- check for minimum number of tokens
	if (ntoks < 2) return error_setInpError(ERR_ITEMS, "");

	// --- check that LID exists in database
	j = ProjectFindObject(LID, toks[0]);
	if (j < 0) return error_setInpError(ERR_NAME, toks[0]);

	// --- assign ID if not done yet
	if (_lidProcs[j].ID == NULL)
		_lidProcs[j].ID = ProjectFindID(LID, toks[0]);

	// --- check if second token is the type of LID
	m = findmatch(toks[1], LidTypeWords);
	if (m >= 0)
	{
		_lidProcs[j].lidType = m;
		return 0;
	}

	// --- check if second token is name of LID layer
	else m = findmatch(toks[1], LidLayerWords);

	 //--- read input parameters for the identified layer
	switch (m)
	{
	case SURF:  return ReadSurfaceData(j, toks, ntoks);
	case SOIL:  return ReadSoilData(j, toks, ntoks);
	case STOR:  return ReadStorageData(j, toks, ntoks);
	case PAVE:  return ReadPavementData(j, toks, ntoks);
	case DRAIN: return ReadDrainData(j, toks, ntoks);
	case DRAINMAT: return ReadDrainMatData(j, toks, ntoks);
	}
	return error_setInpError(ERR_KEYWORD, toks[1]);
}

int SWMMLoader::ReadSurfaceData(int j, char* toks[], int ntoks)
//
//  Purpose: reads surface layer data for a LID process from line of input
//           data file
//  Input:   j = LID process index 
//           toks = array of string tokens
//           ntoks = number of tokens
//  Output:  returns error code
//
//  Format of data is:
//  LID_ID  SURFACE  StorageHt  VegVolFrac  Roughness  SurfSlope  SideSlope  DamHt
//
{
	int    i;
	double x[5];

	if (ntoks < 7) return error_setInpError(ERR_ITEMS, "");
	for (i = 2; i < 7; i++)
	{
		if (!getDouble(toks[i], &x[i - 2]) || x[i - 2] < 0.0)
			return error_setInpError(ERR_NUMBER, toks[i]);
	}
	if (x[1] >= 1.0) return error_setInpError(ERR_NUMBER, toks[3]);
	if (x[0] == 0.0) x[1] = 0.0;

	_lidProcs[j].surface.thickness = x[0] / UCF(RAINDEPTH);
	_lidProcs[j].surface.voidFrac = 1.0 - x[1];
	_lidProcs[j].surface.roughness = x[2];
	_lidProcs[j].surface.surfSlope = x[3] / 100.0;
	_lidProcs[j].surface.sideSlope = x[4];
	return 0;
}

//=============================================================================

int SWMMLoader::ReadPavementData(int j, char* toks[], int ntoks)
//
//  Purpose: reads pavement layer data for a LID process from line of input
//           data file
//  Input:   j = LID process index 
//           toks = array of string tokens
//           ntoks = number of tokens
//  Output:  returns error code
//
//  Format of data is:
//    LID_ID PAVEMENT  Thickness  VoidRatio  FracImperv  Permeability  ClogFactor
//
{
	int    i;
	double x[5];

	if (ntoks < 7) return error_setInpError(ERR_ITEMS, "");
	for (i = 2; i < 7; i++)
	{
		if (!getDouble(toks[i], &x[i - 2]) || x[i - 2] < 0.0)
			return error_setInpError(ERR_NUMBER, toks[i]);
	}

	//... convert void ratio to void fraction
	x[1] = x[1] / (x[1] + 1.0);

	_lidProcs[j].pavement.thickness = x[0] / UCF(RAINDEPTH);
	_lidProcs[j].pavement.voidFrac = x[1];
	_lidProcs[j].pavement.impervFrac = x[2];
	_lidProcs[j].pavement.kSat = x[3] / UCF(RAINFALL);
	_lidProcs[j].pavement.clogFactor = x[4];
	return 0;
}

//=============================================================================

int SWMMLoader::ReadSoilData(int j, char* toks[], int ntoks)
//
//  Purpose: reads soil layer data for a LID process from line of input
//           data file
//  Input:   j = LID process index 
//           toks = array of string tokens
//           ntoks = number of tokens
//  Output:  returns error code
//
//  Format of data is:
//    LID_ID  SOIL  Thickness  Porosity  FieldCap  WiltPt Ksat  Kslope  Suction
//
{
	int    i;
	double x[7];

	if (ntoks < 9) return error_setInpError(ERR_ITEMS, "");
	for (i = 2; i < 9; i++)
	{
		if (!getDouble(toks[i], &x[i - 2]) || x[i - 2] < 0.0)
			return error_setInpError(ERR_NUMBER, toks[i]);
	}
	_lidProcs[j].soil.thickness = x[0] / UCF(RAINDEPTH);
	_lidProcs[j].soil.porosity = x[1];
	_lidProcs[j].soil.fieldCap = x[2];
	_lidProcs[j].soil.wiltPoint = x[3];
	_lidProcs[j].soil.kSat = x[4] / UCF(RAINFALL);
	_lidProcs[j].soil.kSlope = x[5];
	_lidProcs[j].soil.suction = x[6] / UCF(RAINDEPTH);
	return 0;
}

//=============================================================================

int SWMMLoader::ReadStorageData(int j, char* toks[], int ntoks)
//
//  Purpose: reads drainage layer data for a LID process from line of input
//           data file
//  Input:   j = LID process index 
//           toks = array of string tokens
//           ntoks = number of tokens
//  Output:  returns error code
//
//  Format of data is:
//    LID_ID STORAGE  Thickness  VoidRatio  Ksat  ClogFactor 
//
{
	int    i;
	double x[6];

	//... read numerical parameters
	if (ntoks < 6) return error_setInpError(ERR_ITEMS, "");
	for (i = 2; i < 6; i++)
	{
		if (!getDouble(toks[i], &x[i - 2]) || x[i - 2] < 0.0)
			return error_setInpError(ERR_NUMBER, toks[i]);
	}

	//... convert void ratio to void fraction
	x[1] = x[1] / (x[1] + 1.0);

	//... save parameters to LID storage layer structure
	_lidProcs[j].storage.thickness = x[0] / UCF(RAINDEPTH);
	_lidProcs[j].storage.voidFrac = x[1];
	_lidProcs[j].storage.kSat = x[2] / UCF(RAINFALL);
	_lidProcs[j].storage.clogFactor = x[3];
	return 0;
}

//=============================================================================

int SWMMLoader::ReadDrainData(int j, char* toks[], int ntoks)
//
//  Purpose: reads underdrain data for a LID process from line of input
//           data file
//  Input:   j = LID process index 
//           toks = array of string tokens
//           ntoks = number of tokens
//  Output:  returns error code
//
//  Format of data is:
//    LID_ID DRAIN  coeff  expon  offset  delay
//
{
	int    i;
	double x[4];

	//... read numerical parameters
	if (ntoks < 6) return error_setInpError(ERR_ITEMS, "");
	for (i = 2; i < 6; i++)
	{
		if (!getDouble(toks[i], &x[i - 2]) || x[i - 2] < 0.0)
			return error_setInpError(ERR_NUMBER, toks[i]);
	}

	//... save parameters to LID drain layer structure
	_lidProcs[j].drain.coeff = x[0];
	_lidProcs[j].drain.expon = x[1];
	_lidProcs[j].drain.offset = x[2] / UCF(RAINDEPTH);
	_lidProcs[j].drain.delay = x[3] * 3600.0;
	return 0;
}

//=============================================================================

int SWMMLoader::ReadDrainMatData(int j, char* toks[], int ntoks)
//
//  Purpose: reads drainage mat data for a LID process from line of input
//           data file
//  Input:   j = LID process index 
//           toks = array of string tokens
//           ntoks = number of tokens
//  Output:  returns error code
//
//  Format of data is:
//    LID_ID DRAINMAT  thickness  voidRatio  roughness
//
{
	int    i;
	double x[3];

	//... read numerical parameters
	if (ntoks < 5) return error_setInpError(ERR_ITEMS, "");
	if (_lidProcs[j].lidType != GREEN_ROOF) return 0;
	for (i = 2; i < 5; i++)
	{
		if (!getDouble(toks[i], &x[i - 2]) || x[i - 2] < 0.0)
			return error_setInpError(ERR_NUMBER, toks[i]);
	}

	//... save parameters to LID drain layer structure
	_lidProcs[j].drainMat.thickness = x[0] / UCF(RAINDEPTH);;
	_lidProcs[j].drainMat.voidFrac = x[1];
	_lidProcs[j].drainMat.roughness = x[2];
	return 0;
}


int SWMMLoader::LidReadGroupParams(char* toks[], int ntoks)
//
//  Purpose: reads input data for a LID unit placed in a subcatchment.
//  Input:   toks = array of string tokens
//           ntoks = number of tokens
//  Output:  returns error code
//
//  Format of input data line is:
//    Subcatch_ID  LID_ID  Number  Area  Width  InitSat  FromImp  ToPerv
//                                                       (RptFile DrainTo)     //(5.1.008)
//  where:
//    Subcatch_ID    = name of subcatchment
//    LID_ID         = name of LID process
//    Number     (n) = number of replicate units
//    Area    (x[0]) = area of each unit
//    Width   (x[1]) = outflow width of each unit
//    InitSat (x[2]) = % that LID is initially saturated
//    FromImp (x[3]) = % of impervious runoff sent to LID
//    ToPerv  (x[4]) = 1 if outflow goes to pervious sub-area; 0 if not
//    RptFile        = name of detailed results file (optional)                //(5.1.008)
//    DrainTo        = name of subcatch/node for drain flow (optional)         //(5.1.008)
//
{
	int        i, j, k, n;
	double     x[5];
	char*      fname = NULL;                                                   //(5.1.008)
	int        drainSubcatch = -1, drainNode = -1;                             //(5.1.008)

	//... check for valid number of input tokens
	if (ntoks < 8) return error_setInpError(ERR_ITEMS, "");

	//... find subcatchment
	j = ProjectFindObject(SUBCATCH, toks[0]);
	if (j < 0) return error_setInpError(ERR_NAME, toks[0]);

	//... find LID process in list of LID processes
	k = ProjectFindObject(LID, toks[1]);
	if (k < 0) return error_setInpError(ERR_NAME, toks[1]);

	//... get number of replicates
	n = atoi(toks[2]);
	if (n < 0) return error_setInpError(ERR_NUMBER, toks[2]);
	if (n == 0) return 0;

	//... convert next 4 tokens to doubles
	for (i = 3; i <= 7; i++)
	{
		if (!getDouble(toks[i], &x[i - 3]) || x[i - 3] < 0.0)
			return error_setInpError(ERR_NUMBER, toks[i]);
	}

	//... check for valid percentages on tokens 5 & 6 (x[2] & x[3])
	for (i = 2; i <= 3; i++) if (x[i] > 100.0)
		return error_setInpError(ERR_NUMBER, toks[i + 3]);

	//... read optional report file name
	if (ntoks >= 9 && strcmp(toks[8], "*") != 0) fname = toks[8];

	////  ----  Following code segment added to release 5.1.008.  ----  ////       //(5.1.008)
	////
	//... read optional underdrain outlet
	if (ntoks >= 10 && strcmp(toks[9], "*") != 0)
	{
		drainSubcatch = ProjectFindObject(SUBCATCH, toks[9]);
		if (drainSubcatch < 0)
		{
			drainNode = ProjectFindObject(NODE, toks[9]);
			if (drainNode < 0) return error_setInpError(ERR_NAME, toks[9]);
		}
	}
	////

	//... create a new LID unit and add it to the subcatchment's LID group
	return AddLidUnit(j, k, n, x, fname, drainSubcatch, drainNode);
}

int SWMMLoader::AddLidUnit(int j, int k, int n, double x[], char* fname,
	int drainSubcatch, int drainNode)                                          //(5.1.008)
//
//  Purpose: adds an LID unit to a subcatchment's LID group.
//  Input:   j = subcatchment index
//           k = LID control index
//           n = number of replicate units
//           x = LID unit's parameters
//           fname = name of detailed performance report file
//           drainSubcatch = index of subcatchment receiving underdrain flow   //(5.1.008)
//           drainNode = index of node receiving underdrain flow               //(5.1.008)
//  Output:  returns an error code
//
{
	TLidUnit*  lidUnit;
	TLidList*  lidList;
	TLidGroup  lidGroup;

	//... create a LID group (pointer to an LidGroup struct)
	//    if one doesn't already exist
	lidGroup = _lidGroups[j];
	if (!lidGroup)
	{
		lidGroup = new LidGroup();
		if (!lidGroup) return error_setInpError(ERR_MEMORY, "");
		lidGroup->lidList = NULL;
		_lidGroups[j] = lidGroup;
	}

	//... create a new LID unit to add to the group
	lidUnit = new TLidUnit();
	if (!lidUnit) return error_setInpError(ERR_MEMORY, "");
	lidUnit->rptFile = NULL;

	//... add the LID unit to the group
	lidList = new TLidList();
	if (!lidList)
	{
		delete(lidUnit);
		return error_setInpError(ERR_MEMORY, "");
	}
	lidList->lidUnit = lidUnit;
	lidList->nextLidUnit = lidGroup->lidList;
	lidGroup->lidList = lidList;

	//... assign parameter values to LID unit
	lidUnit->lidIndex = k;
	lidUnit->number = n;
	lidUnit->area = x[0] / SQR(UCF(LENGTH));
	lidUnit->fullWidth = x[1] / UCF(LENGTH);
	lidUnit->initSat = x[2] / 100.0;
	lidUnit->fromImperv = x[3] / 100.0;
	lidUnit->toPerv = (x[4] > 0.0);
	lidUnit->drainSubcatch = drainSubcatch;                                //(5.1.008)
	lidUnit->drainNode = drainNode;                                        //(5.1.008)

	//... open report file if it was supplied
	//if (fname != NULL)
	//{
	//	if (!CreateLidRptFile(lidUnit, fname))
	//		return error_setInpError(ERR_RPT_FILE, fname);
	//}
	return 0;
}

void SWMMLoader::DeleteLidGroup(int j)
//
//  Purpose: frees all LID units associated with a subcatchment.
//  Input:   j = group (or subcatchment) index
//  Output:  none
//
{
	TLidGroup  lidGroup = _lidGroups[j];
	TLidList*  lidList;
	TLidUnit*  lidUnit;
	TLidList*  nextLidUnit;

	if (lidGroup == NULL) return;
	lidList = lidGroup->lidList;
	while (lidList)
	{
		lidUnit = lidList->lidUnit;
		if (lidUnit->rptFile)
		{
			if (lidUnit->rptFile->file) fclose(lidUnit->rptFile->file);
			delete[](lidUnit->rptFile);
		}
		nextLidUnit = lidList->nextLidUnit;
		delete[](lidUnit);
		delete[](lidList);
		lidList = nextLidUnit;
	}
	delete[](lidGroup);
	_lidGroups[j] = NULL;
}

void SWMMLoader::InfilCreate(int subcatchCount, int model)
//
//  Purpose: creates an array of infiltration objects.
//  Input:   n = number of subcatchments
//           m = infiltration method code
//  Output:  none
//
{
	switch (model)
	{
	case HORTON:
	case MOD_HORTON:
		_hortinfil = new THorton[_Nobjects[SUBCATCH]]();
		if (_hortinfil == NULL) _errCode = ERR_MEMORY;
		break;
	case GREEN_AMPT:
	case MOD_GREEN_AMPT:                                                       //(5.1.010)
		_gainfil = new TGrnAmpt[_Nobjects[SUBCATCH]]();
		if (_gainfil == NULL) _errCode = ERR_MEMORY;
		break;
	//case CURVE_NUMBER:
	//	CNInfil = (TCurveNum *)calloc(subcatchCount, sizeof(TCurveNum));
	//	if (CNInfil == NULL) ErrorCode = ERR_MEMORY;
	//	break;
	default: _errCode = ERR_MEMORY;
	}
}

int SWMMLoader::InfilReadParams(int m, char* tok[], int ntoks)
//
//  Input:   m = infiltration method code
//           tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: sets infiltration parameters from a line of input data.
//
//  Format of data line is:
//     subcatch  p1  p2 ...
{
	int   i, j, n, status;
	double x[5];

	// --- check that subcatchment exists
	j = ProjectFindObject(SUBCATCH, tok[0]);
	if (j < 0) return error_setInpError(ERR_NAME, tok[0]);

	// --- number of input tokens depends on infiltration model m
	if (m == HORTON)       n = 5;
	else if (m == MOD_HORTON)   n = 5;
	else if (m == GREEN_AMPT)   n = 4;
	else if (m == MOD_GREEN_AMPT)   n = 4;                                   //(5.1.010)
	else if (m == CURVE_NUMBER) n = 4;
	else return 0;
	if (ntoks < n) return error_setInpError(ERR_ITEMS, "");

	// --- parse numerical values from tokens
	for (i = 0; i < 5; i++) x[i] = 0.0;
	for (i = 1; i < n; i++)
	{
		if (!getDouble(tok[i], &x[i - 1]))
			return error_setInpError(ERR_NUMBER, tok[i]);
	}

	// --- special case for Horton infil. - last parameter is optional
	if ((m == HORTON || m == MOD_HORTON) && ntoks > n)
	{
		if (!getDouble(tok[n], &x[n - 1]))
			return error_setInpError(ERR_NUMBER, tok[n]);
	}

	// --- assign parameter values to infil. object
	_subcatches[j].infil = j;
	switch (m)
	{
	case HORTON:
	case MOD_HORTON:   status = HortonSetParams(&_hortinfil[j], x);
		break;
	case GREEN_AMPT:
	case MOD_GREEN_AMPT:                                                     //(5.1.010)
		status = GrnamptSetParams(&_gainfil[j], x);
		break;
	case CURVE_NUMBER: status = CurvenumSetParams(&CNInfil[j], x);
		break;
	default:           status = TRUE;
	}
	if (!status) return error_setInpError(ERR_NUMBER, "");
	return 0;
}

int SWMMLoader::HortonSetParams(THorton *infil, double p[])
//
//  Input:   infil = ptr. to Horton infiltration object
//           p[] = array of parameter values
//  Output:  returns TRUE if parameters are valid, FALSE otherwise
//  Purpose: assigns Horton infiltration parameters to a subcatchment.
//
{
	int k;
	for (k = 0; k<5; k++) if (p[k] < 0.0) return FALSE;

	// --- max. & min. infil rates (ft/sec)
	infil->f0 = p[0] / UCF(RAINFALL);
	infil->fmin = p[1] / UCF(RAINFALL);

	// --- convert decay const. to 1/sec
	infil->decay = p[2] / 3600.;

	// --- convert drying time (days) to a regeneration const. (1/sec)
	//     assuming that former is time to reach 98% dry along an
	//     exponential drying curve
	if (p[3] == 0.0) p[3] = TINY;
	infil->regen = -log(1.0 - 0.98) / p[3] / SECperDAY;

	// --- optional max. infil. capacity (ft) (p[4] = 0 if no value supplied)
	infil->Fmax = p[4] / UCF(RAINDEPTH);
	if (infil->f0 < infil->fmin) return FALSE;
	return TRUE;
}

int SWMMLoader::GrnamptSetParams(TGrnAmpt *infil, double p[])
//
//  Input:   infil = ptr. to Green-Ampt infiltration object
//           p[] = array of parameter values
//  Output:  returns TRUE if parameters are valid, FALSE otherwise
//  Purpose: assigns Green-Ampt infiltration parameters to a subcatchment.
//
{
	double ksat;                       // sat. hyd. conductivity in in/hr

	if (p[0] < 0.0 || p[1] <= 0.0 || p[2] < 0.0) return FALSE;               //(5.1.007)
	infil->S = p[0] / UCF(RAINDEPTH);   // Capillary suction head (ft)
	infil->Ks = p[1] / UCF(RAINFALL);    // Sat. hyd. conductivity (ft/sec)
	infil->IMDmax = p[2];                    // Max. init. moisture deficit

	// --- find depth of upper soil zone (ft) using Mein's eqn.
	ksat = infil->Ks * 12. * 3600.;
	infil->Lu = 4.0 * sqrt(ksat) / 12.;
	return TRUE;
}

int SWMMLoader::CurvenumSetParams(TCurveNum *infil, double p[])
//
//  Input:   infil = ptr. to Curve Number infiltration object
//           p[] = array of parameter values
//  Output:  returns TRUE if parameters are valid, FALSE otherwise
//  Purpose: assigns Curve Number infiltration parameters to a subcatchment.
//
{

	// --- convert Curve Number to max. infil. capacity
	if (p[0] < 10.0) p[0] = 10.0;
	if (p[0] > 99.0) p[0] = 99.0;
	infil->Smax = (1000.0 / p[0] - 10.0) / 12.0;
	if (infil->Smax < 0.0) return FALSE;

	//// ---- linear regeneration constant and inter-event --- ////
	////      time now computed directly from drying time;     ////
	////      hydraulic conductivity no longer used.           ////

	// --- convert drying time (days) to a regeneration const. (1/sec)
	if (p[2] > 0.0)  infil->regen = 1.0 / (p[2] * SECperDAY);
	else return FALSE;

	// --- compute inter-event time from regeneration const. as in Green-Ampt
	infil->Tmax = 0.06 / infil->regen;

	return TRUE;
}

int  SWMMLoader::GetTokens(char *s)
//
//  Input:   s = a character string
//  Output:  returns number of tokens found in s
//  Purpose: scans a string for tokens, saving pointers to them
//           in shared variable Tok[].
//
//  Notes:   Tokens can be separated by the characters listed in SEPSTR
//           (spaces, tabs, newline, carriage return) which is defined
//           in CONSTS.H. Text between quotes is treated as a single token.
//

// Now saving to class variable _Tok
{
	int  len, m, n;
	char *c;

	// --- begin with no tokens
	for (n = 0; n < MAXTOKS; n++) _Tok[n] = NULL;
	n = 0;

	// --- truncate s at start of comment 
	c = strchr(s, ';');
	if (c) *c = '\0';
	len = strlen(s);

	// --- scan s for tokens until nothing left
	while (len > 0 && n < MAXTOKS)
	{
		m = strcspn(s, SEPSTR);              // find token length 
		if (m == 0) s++;                    // no token found
		else
		{
			if (*s == '"')                  // token begins with quote
			{
				s++;                        // start token after quote
				len--;                      // reduce length of s
				m = strcspn(s, "\"\n");      // find end quote or new line
			}
			s[m] = '\0';                    // null-terminate the token
			_Tok[n] = s;                     // save pointer to token 
			n++;                            // update token count
			s += m + 1;                       // begin next token
		}
		len -= m + 1;                         // update length of s
	}
	return(n);
}

int SWMMLoader::SetSubcatch(int index, double fracimperv)
//Format:
//	[SUBCATCHMENTS]
//	Name Rgage OutID Area %Imperv Width Slope Clength (Spack)

//	Name	name assigned to subcatchment.
//	Rgage	name of rain gage in[RAINGAGES] section assigned to subcatchment.
//	OutID	name of node or subcatchment that receives runoff from subcatchment.
//	Area	area of subcatchment(acres or hectares). 
//	%Imperv percent imperviousness of subcatchment.
//	Width	characteristic width of subcatchment(ft or meters).
//	Slope	subcatchment slope(percent).
//	Clength total curb length(any length units).
//	Spack	name of snow pack object(from[SNOWPACKS] section) that
//			characterizes snow accumulation and melting over the subcatchment.
{
	//-- check that subcatch exists
	TSubcatch* subcatch = GetSubcatch(index);
	
	if (subcatch == NULL) return 444; // error -- add to table or find one that makes sense

	_subcatches[index].fracImperv = fracimperv / 100.0;
	
	return 0;
}