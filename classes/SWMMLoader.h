/*
Working input API -- loads data from input file

*/

#ifndef SWMMLOADER_H
#define SWMMLOADER_H


#include <cstdio>

//extern int gage_readSeriesFormat(char* tok[], int ntoks, double x[]);
//extern int gage_readFileFormat(char* tok[], int ntoks, double x[]);

#include <stdio.h>
#include "consts.h"
#include "macros.h"
#include "enums.h"
#include "error.h"
#include "datetime.h"
#include "objects.h"
#include "funcs.h"
#include "text.h"
#include "keywords.h"
#include "hash.h"
#include "mempool.h"
#include "lid.h"
#include <math.h>

extern int findmatch(char *s, char *keyword[]);

extern double UCF(int u);


extern void ProjectCreateHashTables();
extern HTtable** ProjectGetHTable();

extern TLidGroup* GetLidGroups();

extern char** SubcatchGetRunoffRoutingWords();

class SWMMLoader
{
public:
	SWMMLoader();
	SWMMLoader(const char* path);
	~SWMMLoader();

	bool OpenFile(const char* path);

	//error handling
	inline int GetErr() const { return _errCode; }
	inline char* GetErrString() { return (char*)_errString; }
	void ClearErr();

	//access gage info
	TGage* GetGages();
	TGage* GetGage(const int & i);
	int GetGageCount() const;

	//access subcatch info
	TSubcatch* GetSubcatches();
	TSubcatch* GetSubcatch(const int & i);
	int GetSubcatchCount() const;

	//access node info -- should this reflect that there are different kind of nodes?
	TNode* GetNodes();
	TNode* GetNode(const int & i);
	int GetNodeCount() const;

	TOutfall* GetOutfalls();
	int GetOutfallCount() const;

	void SetGageCount(int n);
		
	//access timeseries info
	TTable* GetTSeries();
	int GetTSeriesCount() const;

	//access infiltration info
	//only worry about horton for now
	THorton* GetHortInfil(); // probably just overload, but then need an input param
	TGrnAmpt* GetGAInfil();

	//access analysis options info
	AnalysisOptions GetAnalysisOptions();
	
	// access timelist info
	TimeList GetTimeList();

	// access hashtable info
	HTtable** GetHtable(); // Hash tables for object ID names
	//char  _MemPoolAllocated;        // TRUE if memory pool allocated

	// access evap info
	TEvap GetEvap();

	// access report flags
	TRptFlags GetRptFlags();

	// access lid info
	int GetLidCount() const;
	TLidGroup* GetLidGroups();
	TLidProc* GetLidProcs();

	// access landuse info
	int GetLanduseCount() const;
	TLanduse* GetLanduse();

	//GetCounts for all types
	int* GetAllCounts();

	//set subcatch info
	int SetSubcatch(int index, double fracimperv);  // just impervious fraction for now

protected:

	int _Nobjects[MAX_OBJ_TYPES];    // Number of each object type
	int _Mobjects[MAX_OBJ_TYPES];	 // working array
	int _Nnodes[MAX_NODE_TYPES];     // Number of each node sub-type 
	int _Mnodes[MAX_NODE_TYPES];	 // working array
	int _LidCount;
	int _GroupCount;

	/* int _Nlinks[MAX_LINK_TYPES]; */
	int _errCode;
	char _errString[512];
	int _Ntokens;
	char *_Tok[MAXTOKS];             // String tokens from line of input

	// variables that were defined in project.c
	HTtable* _Htable[MAX_OBJ_TYPES]; // Hash tables for object ID names
	char  _MemPoolAllocated;        // TRUE if memory pool allocated

	//-----------------------------------------------------------------------------
	static const int MAXERRS;        // Max. input errors reported

	FILE* _inFile;

	//containers for objects; add more as needed
	TGage* _gages;
	TSubcatch* _subcatches;
	TNode* _nodes;
	TOutfall*  _outfalls;
	TTable* _tseries;
	THorton* _hortinfil;
	TGrnAmpt* _gainfil;
	TLidProc*  _lidProcs;            // array of LID processes
	TLidGroup* _lidGroups;           // array of LID process groups -- defined in lid.c in original swmm
	TLanduse* _landuse;
	
	TRptFlags _rptFlags;

	//structs
	AnalysisOptions _aoptions; 
	TimeList _timelist; 
	TEvap _evap; 
	
	//utility functions
	void ClearCounts();
	void SetError(const int & errcode, const char* s);
	void ClearObjArrays();
	void AllocObjArrays();

	//utility functions - scraped from input.c
	//bool CountObjects();
	int CountObjects();
	int ReadOption(char* line);
	int ReadData();
	int ParseLine(int sect, char *line);
	int GetTokens(char *s);
	int addObject(int objType, char* id);
	int ReadNode(int type);

	//utility functions - scraped from project.c
	void CreateHashTables();
	int ProjectReadOption(char* s1, char* s2);
	int ProjectFindObject(int type, char *id);
	int ProjectAddObject(int type, char *id, int n);
	char *ProjectFindID(int type, char *id);
	void SetDefaults();

	//utility functions - scraped from gage.c
	int ReadGageParams(int j, char* tok[], int ntoks);
	int GageReadSeriesFormat(char* tok[], int ntoks, double x[]);

	//utility functions - scraped from subcatch.c
	int ReadSubcatchParams(int j, char* tok[], int ntoks);
	int ReadSubareaParams(char* tok[], int ntoks);
	
	//utility functions - scraped from node.c
	int ReadNodeParams(int j, int type, int k, char* tok[], int ntoks);
	int OutfallReadParams(int j, int k, char* tok[], int ntoks);
	int JuncReadParams(int j, int k, char* tok[], int ntoks);
	void NodeSetParams(int j, int type, int k, double x[]);

	//utility functions - scraped from table.c
	int TableReadTimeseries(char* tok[], int ntoks);

	//utility functions - scraped from infil.c
	void InfilCreate(int subcatchCount, int model);
	int InfilReadParams(int m, char* tok[], int ntoks);
	int HortonSetParams(THorton *infil, double p[]);
	int GrnamptSetParams(TGrnAmpt *infil, double p[]);

	//utility functions - scraped from report.c
	int ReportReadOptions(char* tok[], int ntoks);

	//utility functions - scraped from lid.c
	void LidCreate(int lidCount, int subcatchCount);
	int LidReadProcParams(char* tok[], int ntoks);
	int AddLidUnit(int j, int k, int n, double x[], char* fname, int drainSubcatch, int drainNode);
	void DeleteLidGroup(int j);
	int LidReadGroupParams(char* tok[], int ntoks);
	int ReadSurfaceData(int j, char* toks[], int ntoks);
	int ReadPavementData(int j, char* toks[], int ntoks);
	int ReadSoilData(int j, char* toks[], int ntoks);
	int ReadStorageData(int j, char* toks[], int ntoks);
	int ReadDrainData(int j, char* toks[], int ntoks);
	int ReadDrainMatData(int j, char* toks[], int ntoks);

	//utility functions - scraped from climate.c
	int ClimateReadEvapParams(char* tok[], int ntoks);

	//utility functions - scraped from landuse.c
	int LanduseReadParams(int j, char* tok[], int ntoks);
};

#endif