#include "SWMMRunner.h"
#include <cstring>

SWMMRunner::SWMMRunner()
:_inputFile()
{

}

// consider setting a default rpt / out filename here
SWMMRunner::SWMMRunner(char* inputFile)
: _inputFile()
{
	setInputFile(inputFile);
}

SWMMRunner::SWMMRunner(char* inputFile, char* reportFile, char* binaryFile)
: _inputFile(), _reportFile(), _binaryFile()
{
	setInputFile(inputFile);
	setReportFile(reportFile);
	setBinaryFile(binaryFile);

	for (int i = 0; i < MAXFNAME + 1; ++i)
		_rainFile[i] = '\0';
}

SWMMRunner::SWMMRunner(char* inputFile, char* reportFile, char* binaryFile, char* rainFile)
: _inputFile(), _reportFile(), _binaryFile(), _rainFile()
{
	setInputFile(inputFile);
	setReportFile(reportFile);
	setBinaryFile(binaryFile);
	setRainFile(rainFile);
}

void SWMMRunner::setInputFile(char *inputFile)
{
	for (int i = 0; i < MAXFNAME + 1; ++i)
		_inputFile[i] = '\0';

	strncpy(_inputFile, inputFile, MAXFNAME); 
}

void SWMMRunner::setReportFile(char *reportFile)
{
	for (int i = 0; i < MAXFNAME + 1; ++i)
		_reportFile[i] = '\0';

	strncpy(_reportFile, reportFile, MAXFNAME);
}

void SWMMRunner::setBinaryFile(char *binaryFile)
{
	for (int i = 0; i < MAXFNAME + 1; ++i)
		_binaryFile[i] = '\0';

	strncpy(_binaryFile, binaryFile, MAXFNAME);
}

void SWMMRunner::setRainFile(char *rainFile)
{
	for (int i = 0; i < MAXFNAME + 1; ++i)
		_rainFile[i] = '\0';

	strncpy(_rainFile, rainFile, MAXFNAME);
}

int SWMMRunner::Run()
{
	int err = inp_swmm_start(_inputFile, _reportFile, _binaryFile, _rainFile);
	return err;
}