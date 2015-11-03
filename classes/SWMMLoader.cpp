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
  :_inFile(NULL),_gages(NULL),_subcatches(NULL)
{
  ClearErr();
  ClearCounts();
}

SWMMLoader::SWMMLoader(const char* path)
  :_gages(NULL),_subcatches(NULL)
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
  _inFile=fopen(path,"rt");
  if(!_inFile)
    {
      _errCode=ERR_INP_FILE;
      return false;
    }

  if(CountObjects() != ERR_NONE)
    return false;

  AllocObjArrays();

  ReadData();

  return _errCode == ERR_NONE;
  
}

void SWMMLoader::ClearErr()
{
  _errCode=0;

  //set all chars to null character so any inserted char sequence will be valid
  for (int i = 0; i < 512; ++i)
    _errString[i]='\0';
}

TGage* SWMMLoader::GetGages()
{
  return _gages;
}

TGage* SWMMLoader::GetGage(const int & i)
{
  if(i<0 || i>=_Nobjects[GAGE])
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
  if(i<0 || i>=_Nobjects[SUBCATCH])
    return NULL;

  return &_subcatches[i];
}

int SWMMLoader::GetSubcatchCount() const
{
  return _Nobjects[SUBCATCH];
}

int* SWMMLoader::GetAllCounts()
{
  return _Nobjects;
}


void SWMMLoader::ClearCounts()
{
  //set all counts to zero (could also possibly initialize with {})
  int i=0;  
  for(i=0; i<MAX_OBJ_TYPES;++i)
    {
    _Nobjects[i]=0;
    _Mobjects[i]=0;
    }
}

void SWMMLoader::SetError(const int & errcode, const char* s)
{
  strncpy(_errString, s, 512-1);
  _errCode=errcode;
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
		if (sect == s_OPTION) errcode = ReadOption(line); // ProjectReadOption
		else if (sect >= 0)
		{
			switch (sect)
			{
			case s_RAINGAGE:
				_Nobjects[GAGE]++;
				break;

			case s_SUBCATCH:
				_Nobjects[SUBCATCH]++;
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

//
int SWMMLoader::ReadOption(char* line)
{
  //_Ntokens = input_getTokens(line);
  //if ( _Ntokens < 2 ) return 0;

  //return ReadOption(Tok[0], Tok[1]);
	return 0;
}

int SWMMLoader::ProjectReadOption(char* s1, char* s2, AnalysisOptions *aoptions)
{
	int      k, m, h, s;
	double   tStep;
	char     strDate[25];
	DateTime aTime;
	DateTime aDate;

  //stub for handling options.
  //when ready, look at project_readOptions() in project.c



	// --- determine which option is being read
	k = findmatch(s1, OptionWords);
	if (k < 0) return error_setInpError(ERR_KEYWORD, s1);
	switch (k)
	{
		// --- choice of flow units
	case FLOW_UNITS:
		m = findmatch(s2, FlowUnitWords);
		if (m < 0) return error_setInpError(ERR_KEYWORD, s2);
		aoptions->FlowUnits = m;
		if (aoptions->FlowUnits <= MGD) UnitSystem = US;
		else                    UnitSystem = SI;
		break;

		// --- choice of infiltration modeling method
	case INFIL_MODEL:
		m = findmatch(s2, InfilModelWords);
		if (m < 0) return error_setInpError(ERR_KEYWORD, s2);
		aoptions->InfilModel = m;
		break;

		// --- choice of flow routing method
	case ROUTE_MODEL:
		m = findmatch(s2, RouteModelWords);
		if (m < 0) m = findmatch(s2, OldRouteModelWords);
		if (m < 0) return error_setInpError(ERR_KEYWORD, s2);
		if (m == NO_ROUTING) aoptions->IgnoreRouting = TRUE;
		else RouteModel = m;
		if (RouteModel == EKW) aoptions->RouteModel = KW;
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
		if (k == SWEEP_START) aoptions->SweepStart = m;
		else aoptions->SweepEnd = m;
		break;

		// --- number of antecedent dry days
	case START_DRY_DAYS:
		aoptions->StartDryDays = atof(s2);
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
		case WET_STEP:     aoptions->WetStep = s;     break;
		case DRY_STEP:     aoptions->DryStep = s;     break;
		case REPORT_STEP:  aoptions->ReportStep = s;  break;
		}
		break;

		// --- type of damping applied to inertial terms of dynamic wave routing
	case INERT_DAMPING:
		m = findmatch(s2, InertDampingWords);
		if (m < 0) return error_setInpError(ERR_KEYWORD, s2);
		else aoptions->InertDamping = m;
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
		case ALLOW_PONDING:     aoptions->AllowPonding = m;  break;
		case SLOPE_WEIGHTING:   aoptions->SlopeWeighting = m;  break;
		case SKIP_STEADY_STATE: aoptions->SkipSteadyState = m;  break;
		case IGNORE_RAINFALL:   aoptions->IgnoreRainfall = m;  break;
		case IGNORE_SNOWMELT:   aoptions->IgnoreSnowmelt = m;  break;
		case IGNORE_GWATER:     aoptions->IgnoreGwater = m;  break;
		case IGNORE_ROUTING:    aoptions->IgnoreRouting = m;  break;
		case IGNORE_QUALITY:    aoptions->IgnoreQuality = m;  break;
		case IGNORE_RDII:       aoptions->IgnoreRDII = m;  break;                 //(5.1.004)
		}
		break;

	case NORMAL_FLOW_LTD:
		m = findmatch(s2, NormalFlowWords);
		if (m < 0) m = findmatch(s2, NoYesWords);
		if (m < 0) return error_setInpError(ERR_KEYWORD, s2);
		aoptions->NormalFlowLtd = m;
		break;

	case FORCE_MAIN_EQN:
		m = findmatch(s2, ForceMainEqnWords);
		if (m < 0) return error_setInpError(ERR_KEYWORD, s2);
		aoptions->ForceMainEqn = m;
		break;

	case LINK_OFFSETS:
		m = findmatch(s2, LinkOffsetWords);
		if (m < 0) return error_setInpError(ERR_KEYWORD, s2);
		aoptions->LinkOffsets = m;
		break;

		// --- compatibility option for selecting solution method for
		//     dynamic wave flow routing (NOT CURRENTLY USED)
	case COMPATIBILITY:
		if (strcomp(s2, "3")) aoptions->Compatibility = SWMM3;
		else if (strcomp(s2, "4")) aoptions->Compatibility = SWMM4;
		else if (strcomp(s2, "5")) aoptions->Compatibility = SWMM5;
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
			aoptions->RouteStep = tStep;
		}
		else aoptions->LengtheningStep = MAX(0.0, tStep);
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
		aoptions->NumThreads = m;
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
		aoptions->MinSurfArea = atof(s2);
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
		aoptions->MaxTrials = m;
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
		aoptions->SysFlowTol /= 100.0;
		break;

		// --- steady state tolerance on nodal lateral inflow
	case LAT_FLOW_TOL:
		if (!getDouble(s2, &LatFlowTol))
		{
			return error_setInpError(ERR_NUMBER, s2);
		}
		aoptions->LatFlowTol /= 100.0;
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
  delete[] _aoptions;

  _gages = NULL;
  _subcatches = NULL;
  _aoptions = NULL;
}

void SWMMLoader::AllocObjArrays()
{
  //make sure any previous values are disposed of 
  ClearObjArrays();

  //() sets all space to zero
  _gages=new TGage[_Nobjects[GAGE]]();
  _subcatches=new TSubcatch[_Nobjects[SUBCATCH]]();
  _aoptions = new AnalysisOptions[sizeof(AnalysisOptions)]();

  //add more as needed

}

int SWMMLoader::ReadData()
{
    char  line[MAXLINE+1];        // line from input data file
    char  wLine[MAXLINE+1];       // working copy of input line
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

    while ( fgets(line, MAXLINE, _inFile) != NULL )
    {
        // --- make copy of line and scan for tokens
        lineCount++;
        strcpy(wLine, line);
        _Ntokens = getTokens(wLine);

        // --- skip blank lines and comments
        if ( _Ntokens == 0 ) continue;
        if ( *_Tok[0] == ';' ) continue;

        // --- check if max. line length exceeded
        lineLength = strlen(line);
        if ( lineLength >= MAXLINE )
        {
            // --- don't count comment if present
            comment = strchr(line, ';');
            if ( comment ) lineLength = comment - line;    // Pointer math here
            if ( lineLength >= MAXLINE )
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
            if ( inperr > 0 )
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

      default: return 0;
    }
}

int SWMMLoader::ReadGageParams(int j, char* tok[], int ntoks)
{
    int      k, err;
    char     *id;
    char     fname[MAXFNAME+1];
    char     staID[MAXMSG+1];
    double   x[7];

    //gage id check skipped

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

    if ( ntoks < 5 )
      {
	SetError(ERR_ITEMS,"");
	return _errCode;
      }

    k = findmatch(tok[4], GageDataWords);
    if      ( k == RAIN_TSERIES )
    {
      //called directly from gage.c
        err = gage_readSeriesFormat(tok, ntoks, x);
    }
    else if ( k == RAIN_FILE    )
    {
        if ( ntoks < 8 )
	  {
	    SetError(ERR_ITEMS, "");
	    return _errCode;
	  }
        strncpy(fname, tok[5], MAXFNAME);
        strncpy(staID, tok[6], MAXMSG);
      //called directly from gage.c
        err = gage_readFileFormat(tok, ntoks, x);
    }
    else 
      {
	SetError(ERR_KEYWORD, tok[4]);
	return _errCode;
      }

    // --- save parameters to rain gage object
    if ( err > 0 ) return err;
 //   _gages[j].ID		   = id;
    _gages[j].tSeries      = (int)x[0];
    _gages[j].rainType     = (int)x[1];
    _gages[j].rainInterval = (int)x[2];
    _gages[j].snowFactor   = x[3];
    _gages[j].rainUnits    = (int)x[6];
    if ( _gages[j].tSeries >= 0 ) _gages[j].dataSource = RAIN_TSERIES;
    else                        _gages[j].dataSource = RAIN_FILE;
    if ( _gages[j].dataSource == RAIN_FILE )
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

int  SWMMLoader::ReadSubcatchParams(int j, char* tok[], int ntoks)
{
    int    i, k, m;
    char*  id;
    double x[9];

    // --- check for enough tokens
    if ( ntoks < 8 ) 
      {
	SetError(ERR_ITEMS, "");
	return _errCode;
      }
    //skip all other checks

    // --- assign input values to subcatch's properties
 //   _subcatches[j].ID = id;
    _subcatches[j].gage        = (int)x[0];
    _subcatches[j].outNode     = (int)x[1];
    _subcatches[j].outSubcatch = (int)x[2];
    _subcatches[j].area        = x[3] / UCF(LANDAREA);
    _subcatches[j].fracImperv  = x[4] / 100.0;
    _subcatches[j].width       = x[5] / UCF(LENGTH);
    _subcatches[j].slope       = x[6] / 100.0;
    _subcatches[j].curbLength  = x[7];

    // --- create the snow pack object if it hasn't already been created
    // if ( x[8] >= 0 )
    // {
    //     if ( !snow_createSnowpack(j, (int)x[8]) )
    //         return error_setInpError(ERR_MEMORY, "");
    // }
    return 0;
}

int  SWMMLoader::getTokens(char *s)
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

