#include "SWMMLoader.h"
#include <cstring>
#include <stdlib.h>

/*
The following function definitions need to be added to gage.c
In order for us to be able to add linkages to the called functions.
Functions and variables declared static are typically only available
within the scope of the c file, and cannot be directly extern'd.
Therefore, we need the wrappers below.

int gage_readSeriesFormat(char* tok[], int ntoks, double x[])
{
return readGageSeriesFormat(tok,ntoks,x);
}

int gage_readFileFormat(char* tok[], int ntoks, double x[])
{
return readGageFileFormat(tok,ntoks,x);
}

*/

//for findmatch(),getTokens()
//#include "input.c"
/*
The following wrapper needs to be inserted into input.c; see
previous comment for explanation.

int input_getTokens(char *s)
{
return input_getTokens(s);
}
*/


const int SWMMLoader::MAXERRS = 100;        // Max. input errors reported

SWMMLoader::SWMMLoader()
:_inFile(NULL), _gages(NULL), _subcatches(NULL), _nodes(NULL), _tseries(NULL)
{
	ClearErr();
	ClearCounts();
}

SWMMLoader::SWMMLoader(const char* path)
:_gages(NULL), _subcatches(NULL), _nodes(NULL), _tseries(NULL)
{
	ClearErr();
	OpenFile(path);
}

SWMMLoader::~SWMMLoader()
{
	ClearObjArrays();
}

bool SWMMLoader::OpenFile(const char* path)
{
	_inFile = fopen(path, "rt");
	if (!_inFile)
	{
		_errCode = ERR_INP_FILE;
		return false;
	}

	CreateHashTables();

	if (CountObjects() != ERR_NONE)
		return false;

	AllocObjArrays();

	ReadData();

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
	if (i<0 || i >= _Nobjects[GAGE])
		return NULL;

	return &_gages[i];
}

int SWMMLoader::GetGageCount() const
{
	return _Nobjects[GAGE];
}

TSubcatch* SWMMLoader::GetSubcatches()
{
	return _subcatches;
}

TSubcatch* SWMMLoader::GetSubcatch(const int & i)
{
	if (i<0 || i >= _Nobjects[SUBCATCH])
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
	if (i<0 || i >= _Nobjects[NODE])
		return NULL;

	return &_nodes[i];
}

int SWMMLoader::GetNodeCount() const
{
	return _Nobjects[NODE];
}



int* SWMMLoader::GetAllCounts()
{
	return _Nobjects;
}


void SWMMLoader::ClearCounts()
{
	//set all counts to zero (could also possibly initialize with {})
	int i = 0;
	for (i = 0; i<MAX_OBJ_TYPES; ++i)
	{
		_Nobjects[i] = 0;
		_Nnodes[i] = 0;
		_Mobjects[i] = 0;
		_Mnodes[i] = 0;
	}
}

void SWMMLoader::SetError(const int & errcode, const char* s)
{
	strncpy(_errString, s, 512 - 1);
	_errCode = errcode;
}

// from project.c
void SWMMLoader::CreateHashTables()
{
	int j;
	_MemPoolAllocated = FALSE;
	for (j = 0; j < MAX_OBJ_TYPES; j++)
	{
		_Htable[j] = HTcreate();
		if (_Htable[j] == NULL) report_writeErrorMsg(ERR_MEMORY, "");
	}

	// --- initialize memory pool used to store object ID's
	if (AllocInit() == NULL) report_writeErrorMsg(ERR_MEMORY, ""); // allocinit called directly from mempool.c
	else _MemPoolAllocated = TRUE;
}

// from project.c
void SWMMLoader::DeleteHashTables()
{
	int j;
	for (j = 0; j < MAX_OBJ_TYPES; j++)
	{
		if (_Htable[j] != NULL) HTfree(_Htable[j]);
	}

	// --- free object ID memory pool
	if (_MemPoolAllocated) AllocFreePool();
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
	char  line[MAXLINE + 1];             // line from input data file     
	char  wLine[MAXLINE + 1];            // working copy of input line   
	char  *tok;                        // first string token of line          
	int   sect = -1, newsect;          // input data sections          
	int   errcode = 0;                 // error code
	int   errsum = 0;                  // number of errors found                   
	//	int   i;
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

			case s_TIMESERIES:
				// --- a Time Series can span several lines
				if (ProjectFindObject(TSERIES, tok) < 0)
				{
					if (!ProjectAddObject(TSERIES, tok, _Nobjects[TSERIES]))
						errcode = error_setInpError(ERR_DUP_NAME, tok);
					_Nobjects[TSERIES]++;
				}
				break;
				//add more cases as needed
			}
		}

		// can be expanded for more detailed error reporting
		// --- report any error found
		// if ( errcode )
		// {
		//     report_writeInputErrorMsg(errcode, sect, line, lineCount);
		//     errsum++;
		//     if (errsum >= MAXERRS ) break;
		// }
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
		if (_aoptions.FlowUnits <= MGD) UnitSystem = US;
		else                    UnitSystem = SI;
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
		else RouteModel = m;
		if (RouteModel == EKW) _aoptions.RouteModel = KW;
		break;

		// these are stored in TTimeList
		// --- simulation start date
		//case START_DATE:
		//	if (!datetime_strToDate(s2, &StartDate))
		//	{
		//		return error_setInpError(ERR_DATETIME, s2);
		//	}
		//	break;

		//	// --- simulation start time of day
		//case START_TIME:
		//	if (!datetime_strToTime(s2, &StartTime))
		//	{
		//		return error_setInpError(ERR_DATETIME, s2);
		//	}
		//	break;

		//	// --- simulation ending date
		//case END_DATE:
		//	if (!datetime_strToDate(s2, &EndDate))
		//	{
		//		return error_setInpError(ERR_DATETIME, s2);
		//	}
		//	break;

		//	// --- simulation ending time of day
		//case END_TIME:
		//	if (!datetime_strToTime(s2, &EndTime))
		//	{
		//		return error_setInpError(ERR_DATETIME, s2);
		//	}
		//	break;

		//	// --- reporting start date
		//case REPORT_START_DATE:
		//	if (!datetime_strToDate(s2, &ReportStartDate))
		//	{
		//		return error_setInpError(ERR_DATETIME, s2);
		//	}
		//	break;

		//	// --- reporting start time of day
		//case REPORT_START_TIME:
		//	if (!datetime_strToTime(s2, &ReportStartTime))
		//	{
		//		return error_setInpError(ERR_DATETIME, s2);
		//	}
		//	break;

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
		if (StartDryDays < 0.0)
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
	case COMPATIBILITY:
		if (strcomp(s2, "3")) _aoptions.Compatibility = SWMM3;
		else if (strcomp(s2, "4")) _aoptions.Compatibility = SWMM4;
		else if (strcomp(s2, "5")) _aoptions.Compatibility = SWMM5;
		else return error_setInpError(ERR_KEYWORD, s2);
		break;

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
		//case MIN_ROUTE_STEP:
		//	if (!getDouble(s2, &MinRouteStep) || MinRouteStep < 0.0)
		//		return error_setInpError(ERR_NUMBER, s2);
		//	break;

	case NUM_THREADS:
		m = atoi(s2);
		if (m < 0) return error_setInpError(ERR_NUMBER, s2);
		_aoptions.NumThreads = m;
		break;
		////

		//	// --- safety factor applied to variable time step estimates under
		//	//     dynamic wave flow routing (value of 0 indicates that variable
		//	//     time step option not used)
		//case VARIABLE_STEP:
		//	if (!getDouble(s2, &CourantFactor))
		//		return error_setInpError(ERR_NUMBER, s2);
		//	if (CourantFactor < 0.0 || CourantFactor > 2.0)
		//		return error_setInpError(ERR_NUMBER, s2);
		//	break;

		// --- minimum surface area (ft2 or sq. meters) associated with nodes
		//     under dynamic wave flow routing 
	case MIN_SURFAREA:
		_aoptions.MinSurfArea = atof(s2);
		break;

		// --- minimum conduit slope (%)
	case MIN_SLOPE:
		if (!getDouble(s2, &MinSlope))
			return error_setInpError(ERR_NUMBER, s2);
		if (MinSlope < 0.0 || MinSlope >= 100)
			return error_setInpError(ERR_NUMBER, s2);
		MinSlope /= 100.0;
		break;

		// --- maximum trials / time step for dynamic wave routing
	case MAX_TRIALS:
		m = atoi(s2);
		if (m < 0) return error_setInpError(ERR_NUMBER, s2);
		_aoptions.MaxTrials = m;
		break;

		// --- head convergence tolerance for dynamic wave routing
	case HEAD_TOL:
		if (!getDouble(s2, &HeadTol))
		{
			return error_setInpError(ERR_NUMBER, s2);
		}
		break;

		// --- steady state tolerance on system inflow - outflow
	case SYS_FLOW_TOL:
		if (!getDouble(s2, &SysFlowTol))
		{
			return error_setInpError(ERR_NUMBER, s2);
		}
		_aoptions.SysFlowTol /= 100.0;
		break;

		// --- steady state tolerance on nodal lateral inflow
	case LAT_FLOW_TOL:
		if (!getDouble(s2, &LatFlowTol))
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
	delete[] _gages;
	delete[] _subcatches;
	delete[] _nodes;
	delete[] _tseries;

	_gages = NULL;
	_subcatches = NULL;
	_nodes = NULL;
	_tseries = NULL;
}

// look at createObjects() in project.c -- some values need to be set
void SWMMLoader::AllocObjArrays()
{
	int j;

	//make sure any previous values are disposed of 
	ClearObjArrays();

	//() sets all space to zero
	_gages = new TGage[_Nobjects[GAGE]]();
	_subcatches = new TSubcatch[_Nobjects[SUBCATCH]]();
	_nodes = new TNode[_Nobjects[NODE]]();
	_tseries = new TTable[_Nobjects[TSERIES]]();

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
	char  line[MAXLINE + 1];        // line from input data file
	char  wLine[MAXLINE + 1];       // working copy of input line
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
				//errsum++;
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
		//presently only subcatch and gage captured;
		//add more as needed.
		//see parseLine() in input.c

	case s_RAINGAGE:
		j = _Mobjects[GAGE];
		err = ReadGageParams(j, _Tok, _Ntokens);
		_Mobjects[GAGE]++;
		return err;

	case s_SUBCATCH:
		j = _Mobjects[SUBCATCH];
		err = ReadSubcatchParams(j, _Tok, _Ntokens);
		_Mobjects[SUBCATCH]++;
		return err;

	case s_TIMESERIES:
		return TableReadTimeseries(_Tok, _Ntokens);

	case s_JUNCTION:
		return ReadNode(JUNCTION);

	default: return 0;
	}
}

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

// from input.c
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

// from node.c
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
	//case OUTFALL:  return outfall_readParams(j, k, tok, ntoks);
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

	//case OUTFALL:
	//	Outfall[k].type = (int)x[1];
	//	Outfall[k].fixedStage = x[2] / UCF(LENGTH);
	//	Outfall[k].tideCurve = (int)x[3];
	//	Outfall[k].stageSeries = (int)x[4];
	//	Outfall[k].hasFlapGate = (char)x[5];

	//	////  Following code segment added to release 5.1.008.  ////                   //(5.1.008)

	//	Outfall[k].routeTo = (int)x[6];
	//	Outfall[k].wRouted = NULL;
	//	if (Outfall[k].routeTo >= 0)
	//	{
	//		Outfall[k].wRouted =
	//			(double *)calloc(Nobjects[POLLUT], sizeof(double));
	//	}
	//	////
	//	break;

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
		//called directly from gage.c
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

	// don't have a node object yet
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

