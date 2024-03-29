// **************************************************************************
// File       [ main.cpp ]
// Author     [ littleshamoo ]
// Synopsis   [ ]
// Date       [ 2011/07/05 created ]
// **************************************************************************

#include <cstdlib>
#include <cstring> // For VLSI final

#include "common/sys_cmd.h"
#include "setup_cmd.h"
#include "atpg_cmd.h"
#include "misc_cmd.h"

using namespace CommonNs;
using namespace IntfNs;
using namespace CoreNs;
using namespace FanNs;

void printWelcome();
void initOpt(OptMgr &mgr);
void initCmd(CmdMgr &cmdMgr, FanMgr &fanMgr);
void printGoodbye(TmUsage &tmusg);

int main(int argc, char **argv)
{

	// start calculating resource usage
	// TmUsage tmusg;
	// tmusg.totalStart();

	// initialize option manager
	OptMgr optMgr;
	initOpt(optMgr);

	// optMgr.parse(argc, argv);
	// if (optMgr.isFlagSet("h"))
	// {
	// 	optMgr.usage();
	// 	exit(0);
	// }

	// initialize command manager and FAN manager
	FanMgr fanMgr;
	CmdMgr cmdMgr;
	initCmd(cmdMgr, fanMgr);
	// CmdMgr::Result res = CmdMgr::SUCCESS;

	// welcome message
	// printWelcome();

	// run startup commands
	// if (optMgr.isFlagSet("f"))
	// {
	// 	std::string cmd = "source " + optMgr.getFlagVar("f");
	// 	res = cmdMgr.exec(cmd);
	// }

	// For VLSI final
	std::string inputCktFile, patternFile;
	int i = 1;
	int ndet = 1;
	bool isTDF = false;
	bool useCompression = false, useFsim = false;
	bool remainFormat = false;

	/* parse the input switches & arguments */
	// 1st circuit file
	inputCktFile = argv[i];
	i++;
	// 2nd ndet
	ndet = atoi(argv[i]);
	i++;
	// 3rd isTDF
	isTDF = atoi(argv[i]);
	i++;
	// 4th useCompression
	useCompression = atoi(argv[i]);
	i++;
	// 5th useFsim (6th pattern file)
	useFsim = atoi(argv[i]);
	if (useFsim)
	{
		i++;
		patternFile = argv[i];
	}
	i++;
	// Last remainFormat
	remainFormat = atoi(argv[i]);

	// Check parse content
	std::cout << "#  Mode : " << ((useFsim) ? "fsim" : "atpg") << "\n";
	std::cout << "#  Fault type : " << ((isTDF) ? "TDF" : "SAF") << "\n";
	std::cout << "#  Input file name : " << inputCktFile << "\n";
	std::cout << "#  Pattern file name : " << patternFile << "\n";
	std::cout << "#  N-det : " << ndet << "\n";
	std::cout << "#  Test compression : " << ((useCompression) ? "on" : "off") << "\n";
	std::cout << "#  Pattern format : " << ((remainFormat) ? "FAN" : "PODEM") << "\n";

	// Main function
	// Determine the content in the script
	std::vector<std::string> scriptStr;
	if (!useFsim)
	{
		if (isTDF)
		{
			// Do TDF ATPG
			if (useCompression)
			{
				if (!remainFormat)
				{
					scriptStr = {
							"read_lib ./FAN_ATPG/techlib/mod_nangate45.mdt",
							"read_netlist " + inputCktFile,
							"report_netlist",
							"build_circuit --frame 1",
							"report_circuit",
							"set_fault_type tdf",
							"add_fault --all",
							"set_static_compression on",
							"set_dynamic_compression on",
							"set_X-Fill on",
							"set_pattern_type LOC",
							"run_atpg -n " + std::to_string(ndet),
							"report_statistics"};
				}
				else
				{
					// This is used for debugging, TDFATPG, remainFormat on, report fault
					scriptStr = {
							"read_lib ./FAN_ATPG/techlib/mod_nangate45.mdt",
							"read_netlist " + inputCktFile,
							"report_netlist",
							"build_circuit --frame 1",
							"report_circuit",
							"set_fault_type tdf",
							"add_fault --all",
							"set_static_compression on",
							"set_dynamic_compression on",
							"set_X-Fill on",
							"set_pattern_type LOC",
							"run_atpg -n " + std::to_string(ndet),
							"report_statistics",
							"report_fault -s au",
							"report_fault -s ab",
							"write_pattern ./tdf_test.pat"};
				}
			}
			else
			{
				if (!remainFormat)
				{
					scriptStr = {
							"read_lib ./FAN_ATPG/techlib/mod_nangate45.mdt",
							"read_netlist " + inputCktFile,
							"report_netlist",
							"build_circuit --frame 1",
							"report_circuit",
							"set_fault_type tdf",
							"add_fault --all",
							"set_static_compression off",
							"set_dynamic_compression off",
							"set_X-Fill on",
							"set_pattern_type LOC",
							"run_atpg -n " + std::to_string(ndet),
							"report_statistics"};
				}
				else
				{
					// This is used for debugging, TDFATPG, remainFormat on, report fault
					scriptStr = {
							"read_lib ./FAN_ATPG/techlib/mod_nangate45.mdt",
							"read_netlist " + inputCktFile,
							"report_netlist",
							"build_circuit --frame 1",
							"report_circuit",
							"set_fault_type tdf",
							"add_fault --all",
							"set_static_compression off",
							"set_dynamic_compression off",
							"set_X-Fill on",
							"set_pattern_type LOC",
							"run_atpg -n " + std::to_string(ndet),
							"report_statistics",
							"report_fault -s au",
							"report_fault -s ab",
							"write_pattern ./tdf_test.pat"};
				}
			}
		}
		else
		{
			// Do SAF ATPG
			if (useCompression)
			{
				scriptStr = {
						"read_lib ./FAN_ATPG/techlib/mod_nangate45.mdt",
						"read_netlist " + inputCktFile,
						"report_netlist",
						"build_circuit --frame 1",
						"report_circuit",
						"set_fault_type saf",
						"add_fault --all",
						"set_static_compression on",
						"set_dynamic_compression on",
						"set_X-Fill on",
						"run_atpg -n " + std::to_string(ndet),
						"report_statistics"};
			}
			else
			{
				scriptStr = {
						"read_lib ./FAN_ATPG/techlib/mod_nangate45.mdt",
						"read_netlist " + inputCktFile,
						"report_netlist",
						"build_circuit --frame 1",
						"report_circuit",
						"set_fault_type saf",
						"add_fault --all",
						"set_static_compression off",
						"set_dynamic_compression off",
						"set_X-Fill on",
						"run_atpg -n " + std::to_string(ndet),
						"report_statistics"};
			}
		}
	}
	else
	{
		// Now we just use tdfsim to test the generated TDF pattern
		if (isTDF)
		{
			scriptStr = {
					"read_lib ./FAN_ATPG/techlib/mod_nangate45.mdt",
					"read_netlist " + inputCktFile,
					"report_netlist",
					"build_circuit --frame 1",
					"report_circuit",
					"set_pattern_type LOC",
					"read_pattern " + patternFile,
					"set_fault_type tdf",
					"add_fault --all",
					"run_fault_sim -m pf -n " + std::to_string(ndet),
					"report_fault -s ud",
					"report_statistics"};
		}
		else
		{
			scriptStr = {
					"read_lib ./FAN_ATPG/techlib/mod_nangate45.mdt",
					"read_netlist " + inputCktFile,
					"report_netlist",
					"build_circuit --frame 1",
					"report_circuit",
					"read_pattern " + patternFile,
					"report_pattern",
					"set_fault_type saf",
					"add_fault --all",
					"run_fault_sim -m pf -n " + std::to_string(ndet),
					"report_statistics"};
		}
	}

	// Run commands
	for (i = 0; i < (int)scriptStr.size(); i++)
	{
		cmdMgr.exec(scriptStr[i]);
	}
	// Write(print) pattern
	if (!useFsim)
	{
		std::cout << "\n";
		std::cout << "#  Generated Patterns : \n";
		for (int i = 0; i < (int)fanMgr.pcoll->patternVector_.size(); ++i)
		{
			if (!fanMgr.pcoll->patternVector_[i].PI1_.empty())
			{
				std::cout << "T'";
				for (int j = 0; j < fanMgr.pcoll->numPI_; ++j)
				{
					if (fanMgr.pcoll->patternVector_[i].PI1_[j] == L)
						std::cout << '0';
					else if (fanMgr.pcoll->patternVector_[i].PI1_[j] == H)
						std::cout << '1';
					else
						std::cout << 'X';
				}
				if (!fanMgr.pcoll->patternVector_[i].PI2_.empty())
				{
					std::cout << ' ';
					if (fanMgr.pcoll->patternVector_[i].PI2_[0] == L)
						std::cout << '0';
					else if (fanMgr.pcoll->patternVector_[i].PI2_[0] == H)
						std::cout << '1';
					else
						std::cout << 'X';
				}
			}
			std::cout << "'\n";
		}
		std::cout << "\n";
	}
	return 0;

	/*
	// enter user interface
	while (res != CmdMgr::EXIT)
	{
		res = cmdMgr.read();
		if (res == CmdMgr::NOT_EXIST)
		{
			std::cerr << "**ERROR main(): command `" << cmdMgr.getErrorStr();
			std::cerr << "' not found" << "\n";
			continue;
		}
	}
	*/

	// goodbye
	// printGoodbye(tmusg);
	// return 0;
}

void printWelcome()
{
	std::cout << "#  =========================================================================="
						<< "\n";
	std::cout << "#"
						<< "\n";
	std::cout << "#                                   FAN ATPG"
						<< "\n";
	std::cout << "#"
						<< "\n";
	std::cout << "#              Copyright(c) Laboratory of Dependable Systems(II),"
						<< "\n";
	std::cout << "#                Graduate Institute of Electronics Engineering,"
						<< "\n";
	std::cout << "#                          National Taiwan University"
						<< "\n";
	std::cout << "#                             All Rights Reserved."
						<< "\n";
	std::cout << "#"
						<< "\n";
	std::cout << "#  =========================================================================="
						<< "\n";
	std::cout << "#"
						<< "\n";

	// system information
	// OS kernel
	FILE *systemOutput;
	systemOutput = popen("uname -s 2> /dev/null", "r");
	char buf[128];
	std::cout << "#  Kernel:   ";
	if (!systemOutput)
		std::cout << "UNKNOWN"
							<< "\n";
	else
	{
		if (fgets(buf, sizeof(buf), systemOutput))
			std::cout << buf;
		else
			std::cout << "UNKNOWN"
								<< "\n";
		pclose(systemOutput);
	}

	// platform
	systemOutput = popen("uname -i 2> /dev/null", "r");
	std::cout << "#  Platform: ";
	if (!systemOutput)
		std::cout << "UNKNOWN"
							<< "\n";
	else
	{
		if (fgets(buf, sizeof(buf), systemOutput))
			std::cout << buf;
		else
			std::cout << "UNKNOWN"
								<< "\n";
		pclose(systemOutput);
	}

	// memory
	FILE *meminfo = fopen("/proc/meminfo", "r");
	std::cout << "#  Memory:   ";
	if (!meminfo)
		std::cout << "UNKNOWN"
							<< "\n";
	else
	{
		while (fgets(buf, 128, meminfo))
		{
			char *ch;
			if ((ch = strstr(buf, "MemTotal:")))
			{
				std::cout << (double)atol(ch + 9) / 1024.0 << " MB"
									<< "\n";
				break;
			}
		}
		fclose(meminfo);
	}

	std::cout << "#"
						<< "\n";
}

void printGoodbye(TmUsage &tmusg)
{
	TmStat stat;
	tmusg.getTotalUsage(stat);
	std::cout << "#  Goodbye"
						<< "\n";
	std::cout << "#  Runtime        ";
	std::cout << "real " << (double)stat.rTime / 1000000.0 << " s        ";
	std::cout << "user " << (double)stat.uTime / 1000000.0 << " s        ";
	std::cout << "sys " << (double)stat.sTime / 1000000.0 << " s"
						<< "\n";
	std::cout << "#  Memory         ";
	std::cout << "peak " << (double)stat.vmPeak / 1024.0 << " MB"
						<< "\n";
}

void initOpt(OptMgr &mgr)
{
	// set program information
	mgr.setName("fan");
	mgr.setShortDes("FAN based ATPG");
	mgr.setDes("FAN based ATPG");

	// register options
	Opt *opt = new Opt(Opt::BOOL, "print this usage", "");
	opt->addFlag("h");
	opt->addFlag("help");
	mgr.regOpt(opt);

	opt = new Opt(Opt::STR_REQ, "execute command file at startup", "file");
	opt->addFlag("f");
	mgr.regOpt(opt);
}

void initCmd(CmdMgr &cmdMgr, FanMgr &fanMgr)
{
	// system commands
	Cmd *listCmd = new SysListCmd("ls");
	Cmd *cdCmd = new SysCdCmd("cd");
	Cmd *catCmd = new SysCatCmd("cat");
	Cmd *pwdCmd = new SysPwdCmd("pwd");
	Cmd *setCmd = new SysSetCmd("set", &cmdMgr);
	Cmd *exitCmd = new SysExitCmd("exit", &cmdMgr);
	Cmd *quitCmd = new SysExitCmd("quit", &cmdMgr);
	Cmd *sourceCmd = new SysSourceCmd("source", &cmdMgr);
	Cmd *helpCmd = new SysHelpCmd("help", &cmdMgr);
	cmdMgr.regCmd("SYSTEM", listCmd);
	cmdMgr.regCmd("SYSTEM", cdCmd);
	cmdMgr.regCmd("SYSTEM", catCmd);
	cmdMgr.regCmd("SYSTEM", pwdCmd);
	cmdMgr.regCmd("SYSTEM", setCmd);
	cmdMgr.regCmd("SYSTEM", exitCmd);
	cmdMgr.regCmd("SYSTEM", quitCmd);
	cmdMgr.regCmd("SYSTEM", sourceCmd);
	cmdMgr.regCmd("SYSTEM", helpCmd);

	// setup commands
	Cmd *readLibCmd = new ReadLibCmd("read_lib", &fanMgr);
	Cmd *readNlCmd = new ReadNlCmd("read_netlist", &fanMgr);
	Cmd *setFaultTypeCmd = new SetFaultTypeCmd("set_fault_type", &fanMgr);
	Cmd *buildCirCmd = new BuildCircuitCmd("build_circuit", &fanMgr);
	Cmd *reportNlCmd = new ReportNetlistCmd("report_netlist", &fanMgr);
	Cmd *reportCellCmd = new ReportCellCmd("report_cell", &fanMgr);
	Cmd *reportLibCmd = new ReportLibCmd("report_lib", &fanMgr);
	Cmd *setPatternTypeCmd = new SetPatternTypeCmd("set_pattern_type", &fanMgr);
	Cmd *setStaticCompressionCmd = new SetStaticCompressionCmd("set_static_compression", &fanMgr);
	Cmd *setDynamicCompressionCmd = new SetDynamicCompressionCmd("set_dynamic_compression", &fanMgr);
	Cmd *setXFillCmd = new SetXFillCmd("set_X-Fill", &fanMgr);
	cmdMgr.regCmd("SETUP", readLibCmd);
	cmdMgr.regCmd("SETUP", readNlCmd);
	cmdMgr.regCmd("SETUP", setFaultTypeCmd);
	cmdMgr.regCmd("SETUP", buildCirCmd);
	cmdMgr.regCmd("SETUP", reportNlCmd);
	cmdMgr.regCmd("SETUP", reportCellCmd);
	cmdMgr.regCmd("SETUP", reportLibCmd);
	cmdMgr.regCmd("SETUP", setPatternTypeCmd);
	cmdMgr.regCmd("SETUP", setStaticCompressionCmd);
	cmdMgr.regCmd("SETUP", setDynamicCompressionCmd);
	cmdMgr.regCmd("SETUP", setXFillCmd);

	// ATPG commands
	Cmd *readPatCmd = new ReadPatCmd("read_pattern", &fanMgr);
	Cmd *reportPatCmd = new ReportPatCmd("report_pattern", &fanMgr);
	Cmd *addFaultCmd = new AddFaultCmd("add_fault", &fanMgr);
	Cmd *reportFaultCmd = new ReportFaultCmd("report_fault", &fanMgr);
	Cmd *addPinConsCmd = new AddPinConsCmd("add_pin_constraint", &fanMgr);
	Cmd *runLogicSimCmd = new RunLogicSimCmd("run_logic_sim", &fanMgr);
	Cmd *runFaultSimCmd = new RunFaultSimCmd("run_fault_sim", &fanMgr);
	Cmd *runAtpgCmd = new RunAtpgCmd("run_atpg", &fanMgr);
	Cmd *reportCircuitCmd = new ReportCircuitCmd("report_circuit", &fanMgr);
	Cmd *reportGateCmd = new ReportGateCmd("report_gate", &fanMgr);
	Cmd *reportValueCmd = new ReportValueCmd("report_value", &fanMgr);
	Cmd *reportStatsCmd = new ReportStatsCmd("report_statistics", &fanMgr);
	Cmd *writePatCmd = new WritePatCmd("write_pattern", &fanMgr);
	Cmd *writeStilCmd = new WriteStilCmd("write_to_STIL", &fanMgr);
	Cmd *writeProcCmd = new WriteProcCmd("write_test_procedure_file", &fanMgr);
	Cmd *addScanChainsCmd = new AddScanChainsCmd("add_scan_chains", &fanMgr);
	Cmd *tdfAtpgCmd = new TdfAtpgCmd("test", &fanMgr); // For VLSI final, not used
	cmdMgr.regCmd("ATPG", readPatCmd);
	cmdMgr.regCmd("ATPG", reportPatCmd);
	cmdMgr.regCmd("ATPG", addFaultCmd);
	cmdMgr.regCmd("ATPG", reportFaultCmd);
	cmdMgr.regCmd("ATPG", addPinConsCmd);
	cmdMgr.regCmd("ATPG", runLogicSimCmd);
	cmdMgr.regCmd("ATPG", runFaultSimCmd);
	cmdMgr.regCmd("ATPG", runAtpgCmd);
	cmdMgr.regCmd("ATPG", reportCircuitCmd);
	cmdMgr.regCmd("ATPG", reportGateCmd);
	cmdMgr.regCmd("ATPG", reportValueCmd);
	cmdMgr.regCmd("ATPG", reportStatsCmd);
	cmdMgr.regCmd("ATPG", writePatCmd);
	cmdMgr.regCmd("ATPG", writeStilCmd);
	cmdMgr.regCmd("ATPG", writeProcCmd);
	cmdMgr.regCmd("ATPG", addScanChainsCmd);
	cmdMgr.regCmd("ATPG", tdfAtpgCmd); // For VLSI final, not used

	// misc commands
	Cmd *reportPatFormatCmd = new ReportPatFormatCmd("report_pattern_format");
	Cmd *reportMemUsgCmd = new ReportMemUsgCmd("report_memory_usage");
	cmdMgr.regCmd("MISC", reportPatFormatCmd);
	cmdMgr.regCmd("MISC", reportMemUsgCmd);

	// user interface
	cmdMgr.setComment('#');
	cmdMgr.setPrompt("fan> ");
	cmdMgr.setColor(CmdMgr::YELLOW);
}
