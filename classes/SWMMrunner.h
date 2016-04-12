#ifndef SWMMRUNNER_H
#define SWMMRUNNER_H

#include <cstdio>
#include "consts.h"
#include "swmm5.h"

extern int inp_swmm_start(char* f1, char* f2, char* f3, char *f4);

class SWMMRunner
{
public:
	SWMMRunner();
	SWMMRunner(char *inputFile);
	SWMMRunner(char *inputFile, char *reportFile, char *binaryFile, char *rainFile);

	//~SWMMRunner();

	int Run();

	void setInputFile(char *inputFile);
	void setReportFile(char *reportFile);
	void setBinaryFile(char *binaryFile);
	void setRainFile(char *rainFile);

private:
	char _inputFile[MAXFNAME + 1];
	char _reportFile[MAXFNAME + 1];
	char _binaryFile[MAXFNAME + 1];
	char _rainFile[MAXFNAME + 1];

};

#endif