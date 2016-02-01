#include "SWMMRunner.h"
#include <cstring>

SWMMRunner::SWMMRunner()
:_inputFile()
{

}

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

int SWMMRunner::Run()
{
	int err = inp_swmm_start(_inputFile, _reportFile, _binaryFile);
	return err;
}