#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>

using namespace std;
int lineno = 0;
string targv[100]; /* tokens on current command line */
int targc = 0;		 /* number of args on current command line */
string circuitName;

struct Gate
{
	string name;
	string type;
	vector<string> in;
	string out;
};
vector<string> PI;
vector<string> PO;
vector<Gate> gates;

string idx[9] = {".A", ".B", ".C", ".D", ".E", ".F", ".G", ".H", ".I"};

void error(const string &message)
{
	fprintf(stderr, "%s around line %d\n", message.c_str(), lineno);
	exit(EXIT_FAILURE);
}

void parse_line(const string &line)
{
	for (auto &i : targv)
	{
		i.clear();
	}
	if (!line.empty())
	{
		targc = 1;
		for (char pos : line)
		{
			if (pos <= ' ')
			{
				targc++;
				continue;
			}
			targv[targc - 1].push_back(pos);
		}
	}
	else
	{
		targc = 0;
	}
} /* end of parse_line */

string validName(int pos)
{
	if (targc <= pos)
	{
		cerr << "wrong targc matching!\n";
	}
	string newName = targv[pos];
	std::replace(newName.begin(), newName.end(), '(', '_');
	std::replace(newName.begin(), newName.end(), ')', '_');
	std::replace(newName.begin(), newName.end(), '*', 'n');
	std::replace(newName.begin(), newName.end(), '-', '_');
	return ("G" + newName + "gat");
}

string findType(string gatetype)
{
	string type;
	if (gatetype == "not" || gatetype == "NOT")
	{
		if (targc != 5)
			error("Bad Gate Record");
		return ("INVXL");
	}
	if (gatetype == "buf" || gatetype == "BUF")
	{
		if (targc != 5)
			error("Bad Gate Record");
		return ("BUFX20");
	}
	if (gatetype == "and" || gatetype == "AND")
		type = "AND";
	else if (gatetype == "nand" || gatetype == "NAND")
		type = "NAND";
	else if (gatetype == "or" || gatetype == "OR")
		type = "OR";
	else if (gatetype == "nor" || gatetype == "NOR")
		type = "NOR";

	else if (gatetype == "xor")
	{
		if (targc != 6)
			error("Bad Gate Record");
		type = "XOR";
	}
	else if (gatetype == "eqv")
	{
		if (targc != 6)
			error("Bad Gate Record");
		type = "EQV";
	}
	else
	{
		error("unreconizable gate type");
	}
	int numIn = targc - 4;
	if (numIn == 1)
	{
		if (type == "AND" || type == "OR")
		{
			return "BUFX20";
		}
		if (type == "NAND" || type == "NOR")
		{
			return "INVXL";
		}
		error("wrong input number");
	}
	type = type + to_string(numIn) + "XL";
	return type;
}

void newgate()
{
	if ((targc < 5) || (targv[targc - 2] != ";"))
		error("Bad Gate Record");
	Gate g;
	g.name = "U_" + targv[0];
	g.type = findType(targv[1]);
	int i = 2;
	/* connect the iwire and owire */
	while (i < targc)
	{
		if (targv[i] == ";")
			break;
		g.in.push_back(validName(i));
		i++;
	}
	if (i == 2 || ((i + 2) != targc))
		error("Bad Gate Record");
	i++;
	g.out = validName(i);
	gates.push_back(g);
}

void parse_circuit(string filename)
{
	string line;
	ifstream file(filename, std::ifstream::in); // open the input vectors' file
	if (!file)
	{ // if the ifstream obj does not exist, fail to open the file
		fprintf(stderr, "Cannot open input file %s\n", filename.c_str());
		exit(EXIT_FAILURE);
	}
	while (!file.eof() && !file.bad())
	{
		getline(file, line); // get a line from the file
		lineno++;
		parse_line(line);

		if (targv[0].empty())
			continue;
		if (targv[0] == "name")
		{
			if (targc != 2)
			{
				error("Wrong Input Format!");
			}
			circuitName = targv[1].substr(0, targv[1].find('.'));
			continue;
		}
		switch (targv[0][0])
		{
			case '#':
				break;

			case 'D':
				break;

			case 'g':
				newgate();
				break;

			case 'i':
				PI.push_back(validName(1));
				break;

			case 'o':
				PO.push_back(validName(1));
				break;

			default:
				fprintf(stderr, "Unrecognized command around line %d in file %s\n", lineno, filename.c_str());
				break;
		}
	}
	file.close();
}

void generate_verilog(string filename)
{
	ofstream file(filename, std::ifstream::out); // open the input vectors' file
	if (!file)
	{ // if the ifstream obj does not exist, fail to open the file
		fprintf(stderr, "Cannot open input file %s\n", filename.c_str());
		exit(EXIT_FAILURE);
	}
	file << "module " << circuitName << " ( ";
	for (string pi : PI)
	{
		file << pi << ", ";
	}
	for (int i = 0; i < PO.size() - 1; i++)
	{
		file << PO[i] << ", ";
	}
	file << PO.back() << ");" << endl;
	file << endl;

	for (string pi : PI)
	{
		file << "input " << pi << ";" << endl;
	}
	file << endl;
	for (string po : PO)
	{
		file << "output " << po << ";" << endl;
	}
	file << endl;
	for (Gate gate : gates)
	{
		int inputNum = gate.in.size();
		// if (inputNum > 4)
		// {
		// 	cerr << "Need to split! " << gate.name << " " << gate.type << endl;
		// }
		if (inputNum < 1)
		{
			cerr << "invalid gate\n";
		}
		file << gate.type << " " << gate.name << " (";
		for (int i = 0; i < inputNum; i++)
		{
			file << idx[i] << "(" << gate.in[i] << "), ";
		}
		file << ".Y(" << gate.out << ") );" << endl;
	}
	file << endl
			 << "endmodule" << endl;
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		cout << "Usage: " << argv[0] << " input.ckt output.v" << endl;
		return 1;
	}
	parse_circuit(argv[1]);
	generate_verilog(argv[2]);
	return 0;
}
