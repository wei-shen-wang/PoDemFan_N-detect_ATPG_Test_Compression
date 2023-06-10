#include <cstdlib>
#include <cstring>
#include <fstream>
#include <chrono>

#include "convert.cpp"

int main(int argc, char *argv[])
{
	std::string inputCktFile, patternFile;
	int ndet = 1;
	bool isTDF = false;
	bool useCompression = false, useFsim = false;
	bool remainFormat = false;

	auto t_start = std::chrono::high_resolution_clock::now();

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
	std::string commandFan = "(./FAN_ATPG/bin/opt/fan ";

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
	commandFan += " > fan.pat ";

	// Set command for PODEM
	std::string commandPodem = "";
	if (!useFsim)
	{
		commandPodem += "(./PODEM/src/atpg";
		commandPodem += ((isTDF) ? " -tdfatpg" : "");
		commandPodem += ((ndet > 1) ? " -ndet " + std::to_string(ndet) : "");
		commandPodem += ((useCompression) ? " -dtc -stc" : "");
		commandPodem += " ";
		commandPodem += inputCktFile;
		commandPodem += " > podem.pat ";
	}

	// command for fsim
	std::string commandFanFsim = ";../PODEM/src/atpg -ndet " + std::to_string(ndet) + " -tdfsim ./fan.pat " + inputCktFile + "> fan_sim.rpt ; echo " + "FAN finish >&2 ; wait)";
	commandFan += commandFanFsim;
	std::string commandPodemFsim = ";../PODEM/src/atpg -ndet " + std::to_string(ndet) + " -tdfsim ./podem.pat " + inputCktFile + "> podem_sim.rpt ; echo " + "PODEM finish >&2) &";
	commandPodem += commandPodemFsim;
	// execute the command
	// std::cout << commandPodem << '\n';
	// std::cout << commandFan << '\n';
	std::string commandATPG = "(" + commandPodem + commandFan + ") && echo ATPG finish >&2";
	// std::cerr << commandATPG << '\n';
	std::system(commandATPG.c_str());

	auto t_atpg = std::chrono::high_resolution_clock::now();
	double atpg_time_s = std::chrono::duration<double>(t_atpg - t_start).count();

	// Extract TL and FC and RT
	std::string line;

	int fanTL;
	double fanFC;
	std::ifstream fileFan("./fan_sim.rpt", std::ifstream::in);
	if (!fileFan)
	{
		std::cerr << "Cannot open input file";
	}
	for (int i = 0; i < 12; i++)
	{
		fileFan.ignore(1000, '\n');
	}
	fileFan >> line;
	std::string fanTLstr = line.substr(7, line.size() - 8);
	fanTL = atoi(fanTLstr.c_str()) + 1;

	for (int i = 0; i < fanTL + 5; i++)
	{
		fileFan.ignore(1000, '\n');
	}
	fileFan >> line;
	fileFan >> line;
	fileFan >> line;
	fileFan >> line;
	fanFC = atof(line.c_str());
	std::cout << "# Result of FAN : " << '\n';
	std::cout << "# Test Length : " << fanTL << '\n';
	std::cout << "# Fault Coverage : " << fanFC << "%\n";
	fileFan.close();

	int podemTL;
	double podemFC;
	std::ifstream filePodem("./podem_sim.rpt", std::ifstream::in);
	if (!filePodem)
	{
		std::cerr << "Cannot open input file";
	}
	for (int i = 0; i < 12; i++)
	{
		filePodem.ignore(1000, '\n');
	}
	filePodem >> line;
	std::string podemTLstr = line.substr(7, line.size() - 8);
	podemTL = atoi(podemTLstr.c_str()) + 1;

	for (int i = 0; i < podemTL + 5; i++)
	{
		filePodem.ignore(1000, '\n');
	}
	filePodem >> line;
	filePodem >> line;
	filePodem >> line;
	filePodem >> line;
	podemFC = atof(line.c_str());
	std::cout << "# Result of PODEM : " << '\n';
	std::cout << "# Test Length : " << podemTL << '\n';
	std::cout << "# Fault Coverage : " << podemFC << "%\n";
	filePodem.close();

	double fanRT;
	std::ifstream fileFanRT("./fan.pat", std::ifstream::in);
	if (!fileFanRT)
	{
		std::cerr << "Cannot open input file";
	}
	for (int i = 0; i < 64; i++)
	{
		fileFanRT.ignore(1000, '\n');
	}
	fileFanRT >> line;
	fileFanRT >> line;
	fileFanRT >> line;
	fileFanRT >> line;
	fanRT = atof(line.c_str());
	fileFanRT.close();

	FILE *filePodemRT;
	filePodemRT = fopen("./podem.pat", "r");
	if (filePodemRT == NULL)
	{
		perror("fopen");
	}
	fseek(filePodemRT, 0, SEEK_END);
	int pos = ftell(filePodemRT);
	while (pos)
	{
		fseek(filePodemRT, --pos, SEEK_SET);
		// std::cerr << (char)fgetc(filePodemRT) << '\n';
		if ((char)fgetc(filePodemRT) == ' ')
		{
			break;
		}
	}
	char podemRT[10];
	fgets(podemRT, 10, filePodemRT);

	std::cout << '\n';
	// Choose pattern file
	std::ifstream fileOut;
	if (fanFC > podemFC)
	{
		std::cout << "# Choose FAN pattern...\n\n";
		fileOut.open("./fan.pat", std::ifstream::out);
		std::cout << fileOut.rdbuf();
	}
	else if (podemFC > fanFC)
	{
		std::cout << "# Choose PODEM pattern...\n\n";
		fileOut.open("./podem.pat", std::ifstream::out);
		std::cout << fileOut.rdbuf();
	}
	else
	{
		if (fanTL < podemTL)
		{
			std::cout << "# Choose FAN pattern...\n\n";
			fileOut.open("./fan.pat", std::ifstream::out);
			std::cout << fileOut.rdbuf();
		}
		else if (podemTL < fanTL)
		{
			std::cout << "# Choose PODEM pattern...\n\n";
			fileOut.open("./podem.pat", std::ifstream::out);
			std::cout << fileOut.rdbuf();
		}
		else
		{
			std::cout << "# Choose PODEM pattern (both are good)...\n\n";
			fileOut.open("./podem.pat", std::ifstream::out);
			std::cout << fileOut.rdbuf();
		}
	}
	fileOut.close();

	std::string commandRf;
	commandRf = "rm fan.pat";
	std::system(commandRf.c_str());
	commandRf = "rm podem.pat";
	std::system(commandRf.c_str());
	commandRf = "rm fan_sim.rpt";
	std::system(commandRf.c_str());
	commandRf = "rm podem_sim.rpt";
	std::system(commandRf.c_str());

	auto t_end = std::chrono::high_resolution_clock::now();
	double elapsed_time_s = std::chrono::duration<double>(t_end - t_start).count();
	std::cout << "# FAN ATPG runtime : " << fanRT << " s\n";
	std::cout << "# PODEM ATPG runtime : " << podemRT;
	std::cout << "# ATPG runtime : " << atpg_time_s << " s\n";
	std::cout << "# Total runtime : " << elapsed_time_s << " s\n";

	return 0;
}