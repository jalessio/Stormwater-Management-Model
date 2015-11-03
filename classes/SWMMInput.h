/*
Working input API -- does not require .inp file, asks for user input

Create hash tables (can be reused)
Set number of objects (user input, as in the EPANET input API)
Set options (needs to be added)
Add objects to hash tables (can be reused)
Create objects (can be reused)
Set object parameters (new functions, similar to those in the EPANET input API)
Save objects in the class instead of as global variables

*/

#pragma once

#include <cstdio>

//extern void project_createHashTables(); 
//extern void project_deleteHashTables();

// project_open() will need to be slightly modified so that it doesn't open an input file
// project_open() calls setDefaults()

extern double UCF(int u);
extern int project_addObject(int type, char* id, int n);

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

class SWMMInput
{
public:
	SWMMInput();
	SWMMInput(const char* path);
	~SWMMInput();

	//error handling
	inline int GetErr() const { return _errCode; }
	inline char* GetErrString() { return (char*)_errString; }
	void ClearErr();

	// set number of objects
	void SetNObjects(int ngage, int nsubcatch, int ntseries);

	// get number of objects
	int GetGageCount() const;
	int GetSubcatchCount() const;
	int GetTSeriesCount() const;
	int* GetAllCounts();

	// set project options
	void SetOptions();

	// set values
	int SetSubcatch(char* sID, int gIndex, int outIndex,
		double area, double fracimperv, double width, double slope, double clength);

	int SetGage(int index, char* rID, int rainfalltype, float raininterval,
		char* tsID, double times[], double rainseries[], int nitems);

	//access gage info
	TGage* GetGages();
	TGage* GetGage(const int & i);

	//access subcatch info
	TSubcatch* GetSubcatches();
	TSubcatch* GetSubcatch(const int & i);

	//access timeseries info (could make this part of gageinfo if wanted)
	TTable* GetTSeries(const int & i);

protected:

	int _Nobjects[MAX_OBJ_TYPES];    // Number of each object type
	int _Mobjects[MAX_OBJ_TYPES];	 // working array
	/* int _Nnodes[MAX_NODE_TYPES];   // Number of each node sub-type */
	/* int _Nlinks[MAX_LINK_TYPES]; */
	int _errCode;
	char _errString[512];
	//int _Ntokens;
	//char *_Tok[MAXTOKS];             // String tokens from line of input
	//HTtable* _Htable[MAX_OBJ_TYPES]; // Hash tables for object ID names
	//char    _MemPoolAllocated;       // TRUE if memory pool allocated

	//-----------------------------------------------------------------------------
	static const int MAXERRS;        // Max. input errors reported

	FILE* _inFile;

	//containers for objects; add more as needed
	TGage* _gages;
	TSubcatch* _subcatches;
	TTable* _tseries;
	TTimeList* _tlist;
	AnalysisOptions* _aoptions;
	
	// time series set as part of SetGage
	int SetTSeries(char* tsID, double times[], double rainseries[], int nitems);

	//utility functions
	void ClearCounts();
	void SetError(const int & errcode, const char* s);
	void ClearObjArrays();
	void AllocObjArrays();

	int  AddObject(int objType, char* id); // adds object to project (including hash table)
	
	void SetTimeDefaults(void);
	void SetAnalysisDefaults(void);

	////utility functions - scraped from input.c
	////bool CountObjects();
	//int CountObjects();
	//int ReadOption(char* line);
	//int ReadData();
	//int ParseLine(int sect, char *line);
	//int getTokens(char *s);
	//int addObject(int objType, char* id);

	//utility functions - scraped from project.c
	//int ReadOption(char* s1, char* s2);
	//void createObjects();
	
	////utility functions - scraped from gage.c
	//int ReadGageParams(int j, char* tok[], int ntoks);
	//int ReadSubcatchParams(int j, char* tok[], int ntoks);



};


