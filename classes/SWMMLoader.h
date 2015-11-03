/*
Working input API -- set up to load from input file
*/

#pragma once

#include <cstdio>

extern int gage_readSeriesFormat(char* tok[], int ntoks, double x[]);
extern int gage_readFileFormat(char* tok[], int ntoks, double x[]);

extern int findmatch(char *s, char *keyword[]);

extern double UCF(int u);

#include "headers.h"

class SWMMLoader
{
 public:
  SWMMLoader();
  SWMMLoader(const char* path);
  ~SWMMLoader();

  bool OpenFile(const char* path);

  //error handling
  inline int GetErr() const { return _errCode;}
  inline char* GetErrString() { return (char*)_errString;}
  void ClearErr();

  //access gage info
  TGage* GetGages();
  TGage* GetGage(const int & i);
  int GetGageCount() const;

  //access subcatch info
  TSubcatch* GetSubcatches();
  TSubcatch* GetSubcatch(const int & i);
  int GetSubcatchCount() const;

  //GetCounts for all types
  int* GetAllCounts();

  
 protected:
 
  int _Nobjects[MAX_OBJ_TYPES];  // Number of each object type
  int _Mobjects[MAX_OBJ_TYPES];	 // working array
  /* int _Nnodes[MAX_NODE_TYPES];   // Number of each node sub-type */
  /* int _Nlinks[MAX_LINK_TYPES]; */
  int _errCode;
  char _errString[512];
  int _Ntokens;
  char *_Tok[MAXTOKS];             // String tokens from line of input

  //-----------------------------------------------------------------------------
  static const int MAXERRS;        // Max. input errors reported

  FILE* _inFile;

  //containers for objects; add more as needed
  TGage* _gages;
  TSubcatch* _subcatches;
  AnalysisOptions* _aoptions; // struct to store analysis options, defined in objects.c

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
  int getTokens(char *s);
  int addObject(int objType, char* id);

  //utility functions - scraped from project.c
  int ProjectReadOption(char* s1, char* s2, AnalysisOptions *aoptions); 

  //utility functions - scraped from gage.c
  int ReadGageParams(int j, char* tok[], int ntoks);
  int ReadSubcatchParams(int j, char* tok[], int ntoks);

};

