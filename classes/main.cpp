#include "text.h"
#include "SWMMRunner.h"

extern void writecon(char *s);

int main(int argc, char* argv[])
{
	// note that the MAXFNAME defined in constants.h as 259
	char blank[] = "";

	char* pathinp = blank;
	char* pathrpt = blank;
	char* pathout = blank;
	char* pathrain = blank;

	int err;

	if (argc < 3) writecon(FMT01);
	else
	{
		// --- extract file names from command line arguments
		pathinp = argv[1];
		pathrpt = argv[2];
		if (argc > 3) pathout = argv[3];
		else          pathout = blank;
		if (argc > 4) pathrain = argv[4];
		else		  pathrain = blank;
		writecon(FMT02);
	}

	SWMMRunner swmmrunner(pathinp, pathrpt, pathout, pathrain);
	err = swmmrunner.Run();
	if (err)
		return 1;

	return 0;

}

