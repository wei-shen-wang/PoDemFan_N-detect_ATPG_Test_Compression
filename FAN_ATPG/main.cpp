#include <cstdlib>
#include <cstring>

#include "convert.cpp"

int main(int argc, char *argv[])
{
	std::string inputCktFile;
	int ndet = 1;
	bool useTdfAtpg = false, useCompression = false;

	int i = 1;
	/* parse the input switches & arguments */
	while (i < argc)
	{
		if (strcmp(argv[i], "-tdfatpg") == 0)
		{
			useTdfAtpg = true;
			i++;
		}
		else if (strcmp(argv[i], "-ndet") == 0)
		{
			ndet = atoi(argv[i + 1]);
			i += 2;
		}
		else if (strcmp(argv[i], "-compression") == 0)
		{
			useCompression = true;
			i++;
		}
		// else if (strcmp(argv[i], "-fsim") == 0)
		// {
		// 	vectorFile = string(argv[i + 1]);
		// 	i += 2;
		// }
		// else if (strcmp(argv[i], "-tdfsim") == 0)
		// {
		// 	vectorFile = string(argv[i + 1]);
		// 	i += 2;
		// }
		else
		{
			inputCktFile = std::string(argv[i]);
			i++;
		}
	}
	// Convert the input ckt file to verilog file
	std::string outputVlogFile = inputCktFile.substr(0, inputCktFile.size() - 3) + "v";
	convertCkt2Vlog(inputCktFile, outputVlogFile);

	// Set command for FAN ATPG
	std::string commandFan = "./bin/opt/fan";
	if (useTdfAtpg)
	{
		commandFan += " -tdfatpg";
	}
	if (ndet != 1)
	{
		commandFan += " -ndet " + std::to_string(ndet);
	}
	if (useCompression)
	{
		commandFan += " -compression";
	}
	commandFan += " ";
	commandFan += outputVlogFile;

	// execute the command
	// std::cout << commandFan << '\n';
	std::system(commandFan.c_str());

	return 0;
}