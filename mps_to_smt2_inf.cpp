#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <fstream>
#include <cmath>
using namespace std;

string filename;
ofstream fout;
ifstream fin;
string objsense = "MIN";
int varnum = 0;
bool hasN = false;
const string namechange = "ZRTMRH";				  // to add at end of a name to make something now appeared before
map<string, vector<pair<string, string>>> matrix; // row, variable, coefficient
map<string, string> constraints;				  // row, constraint type
map<string, pair<string, string>> variables;	  // variable, lower bound, upper bound
map<string, string> variable_type;
map<string, vector<pair<string, string>>> indicators; // row,var,num
map<string, string> constraints_rhs;
map<string, string> to_var_name;
map<string, string> back_var_name;

void input_rows();
void input_columns();
void input_bounds();
void input_int_columns();
void input_rhs();
void output_fun();
void input_ranges();
void input_indicators();
// split a line of string into words
vector<string> Stringsplit(string str)
{
	int start_pos = 0;
	int word_lnth = 1;

	vector<string> toreturn;
	if (str.length() == 0)
		return toreturn;
	for (int i = 0; i < str.length(); i++)
	{
		if (i == str.length() - 1 && (str[i] == ' ' || str[i] == '\n' || str[i] == '	' || str[i] == '\0'))
			return toreturn;
		if (str[i] == ' ' || str[i] == '\n' || str[i] == '	' || str[i] == '\0')
			continue;
		start_pos = i;

		break;
	}
	while (true)
	{

		for (; word_lnth + start_pos - 1 < str.length(); word_lnth++)
		{

			if (start_pos + word_lnth == str.length())
			{

				toreturn.push_back(str.substr(start_pos, word_lnth));
				return toreturn;
			}
			if (str[start_pos + word_lnth] == ' ' || str[start_pos + word_lnth] == '\n' || str[start_pos + word_lnth] == '	' || str[start_pos + word_lnth] == '\0')
			{

				toreturn.push_back(str.substr(start_pos, word_lnth));
				start_pos += word_lnth;
				word_lnth = 0;
				while (start_pos < str.length() && (str[start_pos] == ' ' || str[start_pos] == '\n' || str[start_pos] == '	' || str[start_pos] == '\0'))
				{
					start_pos++;
					if (start_pos == str.length())
						return toreturn;
				}
			}
		}
	}
	return toreturn;
}
// delete useless 0 at end of double string
string delete_0(string num)
{
	int lnth = num.length();
	for (int i = 0; i < lnth; i++)
	{
		if (num[i] == '.')
		{
			bool sign = false;
			for (int j = i + 1; j < lnth; j++)
			{
				if (num[j] != '0')
					sign = true;
			}
			if (sign)
				return num;
			else
				return num.substr(0, i);
		}
	}
	return num;
}

// replace () in variable namespace
string remove_paranthesis(string s)
{
	while (s.find('(') != std::string::npos)
	{
		int lnth = s.length();
		int ind = s.find('(');
		s = s.substr(0, ind) + "ZRTMRH" + s.substr(ind + 1, lnth - ind - 1);
	}
	while (s.find(')') != std::string::npos)
	{
		int lnth = s.length();
		int ind = s.find('(');
		s = s.substr(0, ind) + "zrtmrh" + s.substr(ind + 1, lnth - ind - 1);
	}
	return s;
}

// check whether a number is Int or real
bool is_real(string num)
{
	int lnth = num.length();
	bool ans = false;
	for (int i = 0; i < lnth; i++)
	{
		if (num[i] == '.')
			ans = true;
		if (num[i] == 'e')
			ans = true;
		if (num[i] == 'E')
			ans = true;
		if (num[i] == '/')
			ans = true;
	}
	return ans;
}
string to_real2(string num)
{
	if (is_real(num))
	{
		if (num[0] == '.')
			return "0" + num;
		if (num[0] == '-' && num[1] == '.')
			return "(- 0" + num.substr(1, num.length() - 1) + ")";
		if (num[num.length() - 1] == '.')
			return num + "0";
		return num;
	}
	else
		return num + ".0";

	return num;
}
// transfer scientific notation
string drop_Ep(string s)
{
	if (s[0] == '(')
		return s;
	int lnth = s.length();
	for (int i = 0; i < lnth; i++)
		if (s[i] == 'E')
		{
			string s1, s2;
			s1 = s.substr(0, i);
			s2 = s.substr(i + 1, lnth - i - 1);
			int p = stoi(s2);
			string toreturn;
			if (p == 0)
				return s1;
			if (p > 0)
				toreturn = "(* ";
			else
			{
				toreturn = "(/ ";
				p *= -1;
			}
			toreturn += to_real2(s1) + " 1";
			for (int j = 0; j < p; j++)
				toreturn += "0";
			toreturn += ".0)";
			return toreturn;
		}
	for (int i = 0; i < lnth; i++)
		if (s[i] == '/')
			return "(/ " + s.substr(0, i) + " " + s.substr(i + 1, lnth - i - 1) + ")";
	return s;
}

// change a number into standard real form
string to_real(string num)
{
	num = drop_Ep(num);
	if (is_real(num))
	{
		if (num[0] == '.')
			return "0" + num;
		if (num[0] == '-' && num[1] == '.')
			return "-0" + num.substr(1, num.length() - 1);
		if (num[num.length() - 1] == '.')
			return num + "0";
		return num;
	}
	else
		return num + ".0";

	return num;
}

string to_int(string num)
{

	return num;
}

// start read a new section; for now assume there is no lazyconstraints,
// usercuts, non-linear constraints ,piecewise objections, SOS constraints,
// indicator constraints, general constraints, or senarior section.
void new_section(string section_name)
{
	if (section_name == "ROWS")
	{
		input_rows();
	}
	else if (section_name == "LAZYCONS")
	{
		return;
	}
	else if (section_name == "USERCUTS")
	{
		return;
	}
	else if (section_name == "COLUMNS")
	{
		input_columns();
	}
	else if (section_name == "RHS")
	{
		input_rhs();
	}
	else if (section_name == "RANGES")
	{
		input_ranges();
	}
	else if (section_name == "BOUNDS")
	{
		input_bounds();
	}
	else if (section_name == "QUADOBJ")
	{
		return;
	}
	else if (section_name == "QCMATRIX")
	{
		return;
	}
	else if (section_name == "PWLOBJ")
	{
		return;
	}
	else if (section_name == "SOS")
	{
		return;
	}
	else if (section_name == "INDICATORS")
	{
		input_indicators();
	}
	else if (section_name == "GENCONS")
	{
		return;
	}
	else if (section_name == "SCENARIOS")
	{
		return;
	}
	else if (section_name == "PWLOBJ")
	{
		return;
	}
	else if (section_name == "ENDATA")
	{
		output_fun();
		exit(0);
	}
}

// input the ROWS section
void input_rows()
{
	while (true)
	{
		char string_line[10000];
		fin.getline(string_line, 10000);
		vector<string> words_of_line = Stringsplit(string_line);

		if (words_of_line[0].size() != 1)
		{

			new_section(words_of_line[0]);
			return;
		}
		if (words_of_line[0] == "N")
		{
			if (hasN)
				words_of_line[0] = "P";
			hasN = true;
		}
		vector<pair<string, string>> variable_of_row;
		matrix.insert(pair<string, vector<pair<string, string>>>(words_of_line[1], variable_of_row));

		constraints.insert(pair<string, string>(words_of_line[1], words_of_line[0]));
	}
}

// input the integrality COLUMNS
void input_int_columns()
{
	while (true)
	{
		char string_line[10000];
		fin.getline(string_line, 10000);
		string varname;
		vector<string> words_of_line = Stringsplit(string_line);
		if (words_of_line.size() == 1)
		{
			if (to_var_name.find(words_of_line[0]) == to_var_name.end())
			{
				varnum++;
				varname = "x" + to_string(varnum);

				to_var_name.insert(make_pair(words_of_line[0], varname));
				back_var_name.insert(make_pair(varname, words_of_line[0]));
				variables.insert(pair<string, pair<string, string>>(varname, make_pair("0", "1")));
				variable_type.insert(make_pair(varname, "Int"));
				continue;
			}
			continue;
		}
		if (words_of_line[1] == "'MARKER'")
		{
			return;
		}
		if (to_var_name.find(words_of_line[0]) == to_var_name.end())
		{
			varnum++;
			varname = "x" + to_string(varnum);

			to_var_name.insert(make_pair(words_of_line[0], varname));
			back_var_name.insert(make_pair(varname, words_of_line[0]));
			variables.insert(pair<string, pair<string, string>>(varname, make_pair("0", "1")));
			variable_type.insert(make_pair(varname, "Int"));
		}
		varname = "x" + to_string(varnum);
		matrix.at(words_of_line[1]).push_back(make_pair(varname, drop_Ep(words_of_line[2])));
		if (words_of_line.size() > 3)
		{

			matrix.at(words_of_line[3]).push_back(make_pair(varname, drop_Ep(words_of_line[4])));
		}
	}
}

// input the COLUMNS section
void input_columns()
{
	while (true)
	{
		char string_line[10000];
		fin.getline(string_line, 10000);
		string varname;
		vector<string> words_of_line = Stringsplit(string_line);
		if (string_line[0] != ' ' && string_line[0] != '	')
		{
			new_section(words_of_line[0]);
			return;
		}
		if (words_of_line.size() == 1)
		{
			if (to_var_name.find(words_of_line[0]) == to_var_name.end())
			{
				varnum++;
				varname = "x" + to_string(varnum);
				to_var_name.insert(make_pair(words_of_line[0], varname));
				back_var_name.insert(make_pair(varname, words_of_line[0]));
				variables.insert(pair<string, pair<string, string>>(varname, make_pair("0", "inf")));
				variable_type.insert(make_pair(varname, "Real"));
				continue;
			}
			continue;
		}
		if (words_of_line[1] == "'MARKER'")
		{
			input_int_columns();
			continue;
		}

		if (to_var_name.find(words_of_line[0]) == to_var_name.end())
		{
			varnum++;
			varname = "x" + to_string(varnum);
			to_var_name.insert(make_pair(words_of_line[0], varname));
			back_var_name.insert(make_pair(varname, words_of_line[0]));
			variables.insert(pair<string, pair<string, string>>(varname, make_pair("0", "inf")));
			variable_type.insert(make_pair(varname, "Real"));
		}
		varname = "x" + to_string(varnum);
		matrix.at(words_of_line[1]).push_back(make_pair(varname, drop_Ep(words_of_line[2])));
		if (words_of_line.size() > 3)
		{

			matrix.at(words_of_line[3]).push_back(make_pair(varname, drop_Ep(words_of_line[4])));
		}
	}
}

// input the right-hand side values
void input_rhs()
{
	while (true)
	{
		char string_line[10000];
		fin.getline(string_line, 10000);
		vector<string> words_of_line = Stringsplit(string_line);
		if (string_line[0] != ' ' && string_line[0] != '	')
		{
			new_section(words_of_line[0]);
			return;
		}
		constraints_rhs.insert(make_pair(words_of_line[1], drop_Ep(words_of_line[2])));
		if (words_of_line.size() > 3)
		{
			constraints_rhs.insert(make_pair(words_of_line[3], drop_Ep(words_of_line[4])));
		}
	}
}

// input the bounds; for now I assume there is no semi-continuous or semi-integer variable
void input_bounds()
{
	while (true)
	{
		char string_line[10000];
		fin.getline(string_line, 10000);
		vector<string> words_of_line = Stringsplit(string_line);
		if (string_line[0] != ' ' && string_line[0] != '	')
		{
			new_section(words_of_line[0]);
			return;
		}
		string up_b, lw_b;
		string varname = to_var_name.at(words_of_line[2]);
		up_b = variables.at(varname).second;
		lw_b = variables.at(varname).first;
		if (words_of_line[0] == "LO")
		{
			variables.at(varname) = make_pair(drop_Ep(words_of_line[3]), up_b);
			continue;
		}
		else if (words_of_line[0] == "UP")
		{
			variables.at(varname) = make_pair(lw_b, drop_Ep(words_of_line[3]));
			continue;
		}
		else if (words_of_line[0] == "FX")
		{
			variables.at(varname) = make_pair(drop_Ep(words_of_line[3]), drop_Ep(words_of_line[3]));
			continue;
		}
		else if (words_of_line[0] == "FR")
		{
			variables.at(varname) = make_pair("-inf", "inf");
			continue;
		}
		else if (words_of_line[0] == "MI")
		{
			variables.at(varname) = make_pair("-inf", up_b);
			continue;
		}
		else if (words_of_line[0] == "PL")
		{
			variables.at(varname) = make_pair(lw_b, "inf");
			continue;
		}
		else if (words_of_line[0] == "BV")
		{
			variables.at(varname) = make_pair("0", "1");
			variable_type.at(varname) = "Int";
			continue;
		}
		else if (words_of_line[0] == "LI")
		{
			variables.at(varname) = make_pair(drop_Ep(words_of_line[3]), up_b);
			variable_type.at(varname) = "Int";
			continue;
		}
		else if (words_of_line[0] == "UI")
		{
			variables.at(varname) = make_pair(lw_b, drop_Ep(words_of_line[3]));
			variable_type.at(varname) = "Int";
			continue;
		}
	}
}

// input ranges section
void input_ranges()
{
	while (true)
	{
		char string_line[10000];
		fin.getline(string_line, 10000);
		vector<string> words_of_line = Stringsplit(string_line);
		if (string_line[0] != ' ' && string_line[0] != '	')
		{
			new_section(words_of_line[0]);
			return;
		}
		string row_name = words_of_line[1];
		bool sign = true;
		string range_value = words_of_line[2];
		string new_row = row_name + namechange;
		vector<pair<string, string>> variable_of_row;
		for (pair<string, string> pa : matrix.at(row_name))
		{
			variable_of_row.push_back(pa);
		}
		matrix.insert(pair<string, vector<pair<string, string>>>(new_row, variable_of_row));
		if (range_value[0] == '-')
			sign = false;
		if (constraints.at(row_name) == "E")
		{
			if (sign)
			{
				constraints.at(row_name) = "G";
				constraints.insert(pair<string, string>(new_row, "L"));
				constraints_rhs.insert(make_pair(new_row, delete_0(to_string(stod(constraints_rhs.at(row_name)) + stod(range_value)))));
			}
			else
			{
				constraints.at(row_name) = "L";
				constraints.insert(pair<string, string>(new_row, "G"));
				constraints_rhs.insert(make_pair(new_row, delete_0(to_string(stod(constraints_rhs.at(row_name)) + stod(range_value)))));
			}
		}
		else if (constraints.at(row_name) == "G")
		{
			constraints.insert(pair<string, string>(new_row, "L"));
			constraints_rhs.insert(make_pair(new_row, delete_0(to_string(stod(constraints_rhs.at(row_name)) + fabs(stod(range_value))))));
		}
		else if (constraints.at(row_name) == "L")
		{
			constraints.insert(pair<string, string>(new_row, "G"));
			constraints_rhs.insert(make_pair(new_row, delete_0(to_string(stod(constraints_rhs.at(row_name)) - fabs(stod(range_value))))));
		}
		if (words_of_line.size() > 3)
		{
			row_name = words_of_line[3];
			sign = true;
			range_value = words_of_line[4];
			new_row = row_name + namechange;
			vector<pair<string, string>> variable_of_rowb;
			for (pair<string, string> pa : matrix.at(row_name))
			{
				variable_of_rowb.push_back(pa);
			}
			matrix.insert(pair<string, vector<pair<string, string>>>(new_row, variable_of_rowb));
			if (range_value[0] == '-')
				sign = false;
			if (constraints.at(row_name) == "E")
			{
				if (sign)
				{
					constraints.at(row_name) = "G";
					constraints.insert(pair<string, string>(new_row, "L"));
					constraints_rhs.insert(make_pair(new_row, delete_0(to_string(stod(constraints_rhs.at(row_name)) + stod(range_value)))));
				}
				else
				{
					constraints.at(row_name) = "L";
					constraints.insert(pair<string, string>(new_row, "G"));
					constraints_rhs.insert(make_pair(new_row, delete_0(to_string(stod(constraints_rhs.at(row_name)) + stod(range_value)))));
				}
			}
			else if (constraints.at(row_name) == "G")
			{
				constraints.insert(pair<string, string>(new_row, "L"));
				constraints_rhs.insert(make_pair(new_row, delete_0(to_string(stod(constraints_rhs.at(row_name)) + fabs(stod(range_value))))));
			}
			else if (constraints.at(row_name) == "L")
			{
				constraints.insert(pair<string, string>(new_row, "G"));
				constraints_rhs.insert(make_pair(new_row, delete_0(to_string(stod(constraints_rhs.at(row_name)) - fabs(stod(range_value))))));
			}
		}
	}
}

// input indicator section
void input_indicators()
{
	while (true)
	{
		char string_line[10000];
		fin.getline(string_line, 10000);
		vector<string> words_of_line = Stringsplit(string_line);
		if (string_line[0] != ' ' && string_line[0] != '	')
		{
			new_section(words_of_line[0]);
			return;
		}
		if (indicators.find(words_of_line[1]) != indicators.end())
		{
			indicators.at(words_of_line[1]).push_back(make_pair(to_var_name.at(words_of_line[2]), words_of_line[3]));
			continue;
		}
		vector<pair<string, string>> indicator_set;
		indicator_set.push_back(make_pair(to_var_name.at(words_of_line[2]), words_of_line[3]));
		indicators.insert(pair<string, vector<pair<string, string>>>(words_of_line[1], indicator_set));
	}
}

// write a real constraint
string output_real_constraint(string row, vector<pair<string, string>> contents, string row_print)
{

	if (contents.size() == 1)
	{
		row_print = row_print + "(* ";
		row_print = row_print + to_real(contents[0].second) + " ";
		if (variable_type.at(contents[0].first) == "Int")
		{
			row_print = row_print + "(to_real " + contents[0].first + ")) ";
		}
		else
		{
			row_print = row_print + contents[0].first + ") ";
		}
		row_print = row_print + to_real(constraints_rhs.at(row)) + ")";
	}
	else
	{
		row_print = row_print + "(+";
		for (pair<string, string> co_and_var : contents)
		{
			row_print = row_print + " (* ";
			row_print = row_print + to_real(co_and_var.second) + " ";
			if (variable_type.at(co_and_var.first) == "Int")
			{
				row_print = row_print + "(to_real " + co_and_var.first + "))";
			}
			else
			{
				row_print = row_print + co_and_var.first + ")";
			}
		}
		row_print = row_print + ") " + to_real(constraints_rhs.at(row)) + ")";
	}
	return row_print;
}

// write an int constraint
string output_int_constraint(string row, vector<pair<string, string>> contents, string row_print)
{

	if (contents.size() == 1)
	{
		row_print = row_print + "(* ";
		row_print = row_print + to_int(contents[0].second) + " ";
		row_print = row_print + contents[0].first + ") ";
		row_print = row_print + to_int(constraints_rhs.at(row)) + ")";
	}
	else
	{
		row_print = row_print + "(+";
		for (pair<string, string> co_and_var : contents)
		{
			row_print = row_print + " (* ";
			row_print = row_print + to_int(co_and_var.second) + " " + co_and_var.first + ")";
		}
		row_print = row_print + ") " + to_int(constraints_rhs.at(row)) + ")";
	}
	return row_print;
}

// write (maybe more than one)indicators for a row
void output_indicators(string row, string row_print)
{

	for (pair<string, string> pa : indicators.at(row))
	{
		fout << "(assert (or (= " << pa.first << " ";
		if (pa.second == "1")
			fout << "0";
		else
			fout << "1";
		fout << ") " << row_print << "))" << endl;
	}
}

// write into new smt2 file
void output_fun()
{
	fout << "(set-info :smt-lib-version 2.6)" << endl;
	fout << "(set-logic ALL)" << endl;
	fout << "(set-info :sourse |Transformed from an MPS format problem, with only satisfiability info|)" << endl;
	fout << "; --- END HEADER ---" << endl;
	fout << endl;
	for (pair<string, string> pa : variable_type)
	{
		string var = pa.first;
		string type = pa.second;
		fout << "(declare-fun " << var << " () " << type << ")" << endl;
		string up_b, lw_b;
		up_b = variables.at(var).second;
		lw_b = variables.at(var).first;
		if (up_b != "inf")
		{
			if (type == "Real")
			{
				fout << "(assert (<= " << var << " " << to_real(up_b) << "))" << endl;
			}
			else if (is_real(up_b) && type == "Int")
			{
				fout << "(assert (<= (to_real " << var << ") " << to_real(up_b) << "))" << endl;
			}
			else if (!is_real(up_b) && type == "Int")
			{
				fout << "(assert (<= " << var << " " << up_b << "))" << endl;
			}
		}
		if (lw_b != "-inf")
		{
			if (type == "Real")
			{
				fout << "(assert (>= " << var << " " << to_real(lw_b) << "))" << endl;
			}
			else if (is_real(lw_b) && type == "Int")
			{
				fout << "(assert (>= (to_real " << var << ") " << to_real(lw_b) << "))" << endl;
			}
			else if (!is_real(lw_b) && type == "Int")
			{
				fout << "(assert (>= " << var << " " << lw_b << "))" << endl;
			}
		}
	}
	for (pair<string, vector<pair<string, string>>> pa : matrix)
	{
		string row = pa.first;

		vector<pair<string, string>> contents = pa.second;
		if (contents.size() == 0)
			continue;
		if (constraints.at(row) == "P" || constraints.at(row) == "N")
			continue;
		if (constraints_rhs.find(row) == constraints_rhs.end())
		{
			constraints_rhs.insert(make_pair(row, "0"));
		}
		string row_print = "(";

		if (constraints.at(row) == "E")
		{
			row_print = row_print + "= ";
		}

		else if (constraints.at(row) == "L")
		{
			row_print = row_print + "<= ";
		}
		else if (constraints.at(row) == "G")
		{
			row_print = row_print + ">= ";
		}
		bool real_constraint = false;
		if (is_real(constraints_rhs.at(row)))
		{
			real_constraint = true;
		}
		for (pair<string, string> co_and_var : contents)
		{
			if (is_real(co_and_var.second))
				real_constraint = true;
			if (variable_type.at(co_and_var.first) == "Real")
				real_constraint = true;
		}
		if (real_constraint)
		{
			row_print = output_real_constraint(row, contents, row_print);
		}
		else
		{
			row_print = output_int_constraint(row, contents, row_print);
		}
		// write output
		if (indicators.find(row) != indicators.end())
		{
			output_indicators(row, row_print);
			continue;
		}
		fout << "(assert " << row_print << ")" << endl;
	}
	fout << "(check-sat)" << endl;
	fout << "(exit)" << endl;
	fout.close();
	return;
}
int main()
{
	cout << "Please type the filename: " << endl;
	cin >> filename;
	int lnth=filename.length();
	filename=filename.substr(0,lnth-4);
	cout << endl;
	string s1, s2;
	s1 = filename + ".mps";
	s2 = filename + "_mps.smt2";
	fout.open(s2);
	fin.open(s1);

	while (true)
	{
		char string_line[100000];
		fin.getline(string_line, 100000);
		vector<string> words_of_line = Stringsplit(string_line);

		if (words_of_line[0] == "NAME")
			continue;
		if (words_of_line[0] == "OBJSENSE")
		{
			objsense = words_of_line[1];
			continue;
		}
		if (string_line[0] == ' ' || string_line[0] == '	' || string_line[0] == '\0' || string_line[0] == '\n' || string_line[0] == '*')
			continue;
		new_section(words_of_line[0]);
	}

	return 0;
}
