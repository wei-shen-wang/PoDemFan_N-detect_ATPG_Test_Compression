#include <cstdlib>
#include <cstring>

#include "convert.cpp"

int main(int argc, char *argv[])
{
	std::string inputCktFile, patternFile;
	int ndet = 1;
	bool isTDF = false;
	bool useCompression = false, useFsim = false;
	bool remainFormat = false;

	int i = 1;
	/* parse the input switches & arguments */
	// Note that default the program will run the atpg
	while (i < argc)
	{
		if (strcmp(argv[i], "-tdfatpg") == 0)
		{
			isTDF = true;
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
		else if (strcmp(argv[i], "-fsim") == 0)
		{
			useFsim = true;
			patternFile = std::string(argv[i + 1]);
			i += 2;
		}
		else if (strcmp(argv[i], "-tdfsim") == 0)
		{
			useFsim = true;
			isTDF = true;
			patternFile = std::string(argv[i + 1]);
			i += 2;
		}
		else if (strcmp(argv[i], "-rpf") == 0)
		{
			remainFormat = true;
			i++;
		}
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
	std::string commandFan = "./bin/opt/fan ";

	commandFan += outputVlogFile;
	commandFan += " ";
	commandFan += std::to_string(ndet);
	commandFan += ((isTDF) ? " 1" : " 0");
	commandFan += ((useCompression) ? " 1" : " 0");
	commandFan += ((useFsim) ? " 1" : " 0");
	if (useFsim)
	{
		commandFan += " ";
		commandFan += patternFile;
	}
	commandFan += ((remainFormat) ? " 1" : " 0");

	// execute the command
	// std::cout << commandFan << '\n';
	std::system(commandFan.c_str());

	return 0;
}