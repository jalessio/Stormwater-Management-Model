

#ifndef SWMMLOADER_H
#define SWMMLOADER_H

#include <cstdio>
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

extern char** SubcatchGetRunoffRoutingWords();


class SWMMLoader
{
public:
	SWMMLoader();
	SWMMLoader(const char* path);
	~SWMMLoader();

	bool OpenFile(const char* path);

	// error handling
	inline int GetErr() const { return _errCode; }
	inline char* GetErrString() { return (char*)_errString; }
	void ClearErr();

	// access gage info
	TGage* GetGages();
	TGage* GetGage(const int & i);
	int GetGageCount() const;

	// access subcatch info
	TSubcatch* GetSubcatches();
	TSubcatch* GetSubcatch(const int & i);
	int GetSubcatchCount() const;

	// access node info
	TNode* GetNodes();
	TNode* GetNode(const int & i);
	int GetNodeCount() const;

	TOutfall* GetOutfalls();
	int GetOutfallCount() const;
	
	// access timeseries info
	TTable* GetTSeries();
	int GetTSeriesCount() const;

	// access infiltration info
	THorton* GetHortInfil(); 
	TGrnAmpt* GetGAInfil();
	TCurveNum* GetCNInfil();

	// access analysis options info
	AnalysisOptions GetAnalysisOptions();
	
	// access timelist info
	TimeList GetTimeList();

	// access hashtable info
	HTtable** GetHtable(); // Hash tables for object ID names
	//char  _MemPoolAllocated;        // TRUE if memory pool allocated

	// access evap info
	TEvap GetEvap();

	// access report flags
	TRptFlags GetRptFlags() const;

	// access lid info
	int GetLidCount() const;
	TLidGroup* GetLidGroups();
	TLidProc* GetLidProcs();

	// access landuse info
	int GetLanduseCount() const;
	TLanduse* GetLanduse();

	// GetCounts for all types
	int* GetAllCounts();

	// setters
	void SetGageCount(const int n);
	int SetSubcatch(int index, double fracimperv);  // just impervious fraction for now

protected:

	int _Nobjects[MAX_OBJ_TYPES];    // number of each object type
	int _Mobjects[MAX_OBJ_TYPES];	 // working array
	int _Nnodes[MAX_NODE_TYPES];     // number of each node sub-type 
	int _Mnodes[MAX_NODE_TYPES];	 // working array
	/* int _Nlinks[MAX_LINK_TYPES]; */
	int _LidCount;					 
	int _GroupCount;				 

	int _status;					 // for first pass at clearobjarrays, _status = 0, then status set to 1
	int _errCode;					 
	char _errString[512];
	int _Ntokens;
	char *_Tok[MAXTOKS];             // String tokens from line of input

	HTtable* _Htable[MAX_OBJ_TYPES]; // Hash tables for object ID names
	char  _MemPoolAllocated;         // TRUE if memory pool allocated

	//-----------------------------------------------------------------------------
	static const int MAXERRS;        // Max. input errors reported

	FILE* _inFile;

	// containers for objects; add more as needed
	TGage* _gages;
	TSubcatch* _subcatches;
	TNode* _nodes;
	TOutfall*  _outfalls;
	TTable* _tseries;
	THorton* _hortinfil;
	TGrnAmpt* _gainfil;
	TCurveNum* _cninfil;
	TLidProc*  _lidProcs;            // array of LID processes
	TLidGroup* _lidGroups;           // array of LID process groups (one for each subcatchment)
	TLanduse* _landuse;
	
	TRptFlags _rptFlags;
	TEvap _evap;

	// structs
	AnalysisOptions _aoptions; 
	TimeList _timelist; 

	// utility functions
	void ClearCounts();
	void SetError(const int & errcode, const char* s);
	void ClearObjArrays();
	void AllocObjArrays();

	// utility functions - scraped from input.c
	//bool CountObjects();
	int CountObjects();
	int ReadOption(char* line);
	int ReadData();
	int ParseLine(int sect, char *line);
	int GetTokens(char *s);
	int ReadNode(int type);

	// utility functions - scraped from project.c
	void CreateHashTables();
	int ProjectReadOption(char* s1, char* s2);
	int ProjectFindObject(int type, char *id);
	int ProjectAddObject(int type, char *id, int n);
	char *ProjectFindID(int type, char *id);
	void SetDefaults();

	// utility functions - scraped from gage.c
	int ReadGageParams(int j, char* tok[], int ntoks);
	int GageReadSeriesFormat(char* tok[], int ntoks, double x[]);

	// utility functions - scraped from subcatch.c
	int ReadSubcatchParams(int j, char* tok[], int ntoks);
	int ReadSubareaParams(char* tok[], int ntoks);
	
	// utility functions - scraped from node.c
	int ReadNodeParams(int j, int type, int k, char* tok[], int ntoks);
	int OutfallReadParams(int j, int k, char* tok[], int ntoks);
	int JuncReadParams(int j, int k, char* tok[], int ntoks);
	void NodeSetParams(int j, int type, int k, double x[]);

	// utility functions - scraped from table.c
	int TableReadTimeseries(char* tok[], int ntoks);

	// utility functions - scraped from infil.c
	void InfilCreate(int subcatchCount, int model);
	int InfilReadParams(int m, char* tok[], int ntoks);
	int HortonSetParams(THorton *infil, double p[]);
	int GrnamptSetParams(TGrnAmpt *infil, double p[]);
	int CurvenumSetParams(TCurveNum *infil, double p[]);

	// utility functions - scraped from report.c
	int ReportReadOptions(char* tok[], int ntoks);

	// utility functions - scraped from lid.c
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

	// utility functions - scraped from climate.c
	int ClimateReadEvapParams(char* tok[], int ntoks);

	// utility functions - scraped from landuse.c
	int LanduseReadParams(int j, char* tok[], int ntoks);
};

#endif