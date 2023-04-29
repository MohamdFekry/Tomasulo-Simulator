#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include<map>
#include<utility>
#include<iomanip>
using namespace std;


int starting_address = 0;

struct instruction
{
	string inst = "";
	string inst_op = "", label = "", head = ""; int rs1 = -1, rs2 = -1, rd = -1, imm = -999, inst_PC = -1;
	int issue = -1, start_exe = -1, end_exe = -1, write = -1, address = -1; signed int result = -999;
};

struct res_station
{
	string name = "", Q_j = "", Q_k = "", processing_instruction = "";
	bool busy = false;
	int V_j = -999, V_k = -999, A = -1;
	int ex_counter = -1;
};

//reading data, and addresses
vector<pair<int, int>> get_data()
{
	ifstream datafile;
	datafile.open("data.txt");

	//pair<address, value>
	vector<pair<int, int>> data;
	string temp;

	while (!datafile.eof())
	{
		getline(datafile, temp);
		if (temp.substr(0, 3) == "PC:")
			starting_address = stoi(temp.substr(4));
		else
			data.push_back(pair<int, int>(stoi(temp.substr(0, temp.find(':'))), stoi(temp.substr(temp.find(':') + 2))));
	}

	return data;
}

//reading instructions
vector<instruction> get_instructions()
{
	ifstream instfile;
	instfile.open("instructions.txt");
	int lines = 0;
	string temp;
	while (!instfile.eof())
	{
		getline(instfile, temp);
		lines++;
	}
	instfile.close();

	vector<instruction> insts(lines);
	instfile.open("instructions.txt");

	int i = 0;
	while (!instfile.eof())
	{
		//LOAD rA, offset(rB) || STORE rA, offset(rB) || BEQ rA, rB, label || JAL label || RET || ADD rA, rB, rC 
		//ADDI rA, rB, imm || NEG rA, rB || NAND rA, rB, rC || MUL rA, rB, rC
		getline(instfile, temp);

		if (temp.find(':') != string::npos)
		{
			insts[i].head = temp.substr(0, temp.find(':'));
			temp.erase(0, temp.find(':') + 1);
			temp.erase(0, 1);
		}

		insts[i].inst = temp;

		if (temp[0] == 'J' || temp[0] == 'R')
		{
			insts[i].inst_op = temp.substr(0, 3);
		}
		else if (temp.substr(0, 5) == "STORE")
			insts[i].inst_op = temp.substr(0, 5);
		else
			insts[i].inst_op = temp.substr(0, temp.find('R') - 1);

		if (insts[i].inst_op == "LOAD")
		{
			/*insts[i].rs1 = stoi(temp.substr(temp.find('(') + 2, 1));
			insts[i].rd = stoi(temp.substr(temp.find('R') + 1, 1));
			insts[i].imm = stoi(temp.substr(temp.find(',') + 2, temp.find('(') + 1);*/
			insts[i].rd = stoi(temp.substr(temp.find('R') + 1, 1));
			temp.erase(0, 8);
			insts[i].rs1 = stoi(temp.substr(temp.find('(') + 2, 1));
			insts[i].imm = stoi(temp.substr(0, temp.find('(')));
		}
		else if (insts[i].inst_op == "STORE")
		{
			/*insts[i].rs1 = stoi(temp.substr(temp.find('(') + 2, 1));
			insts[i].rs2 = stoi(temp.substr(temp.find('R') + 1, 1));
			insts[i].imm = stoi(temp.substr(temp.find(',') + 2, temp.find('(') + 1));*/
			temp.erase(0, 5);
			insts[i].rs2 = stoi(temp.substr(temp.find('R') + 1, 1));
			temp.erase(0, 4);
			insts[i].rs1 = stoi(temp.substr(temp.find('(') + 2, 1));
			insts[i].imm = stoi(temp.substr(0, temp.find('(')));
		}
		else if (insts[i].inst_op == "BEQ")
		{
			insts[i].rs1 = stoi(temp.substr(temp.find('R') + 1, 1));
			temp.erase(0, 8);
			insts[i].rs2 = stoi(temp.substr(1, 1));
			temp.erase(0, 4);
			insts[i].label = temp;
		}
		else if (insts[i].inst_op == "JAL")
		{
			temp.erase(0, 4);
			insts[i].label = temp;
			insts[i].rd = 1;
		}
		else if (insts[i].inst_op == "RET")
		{
			insts[i].rs1 = 1;
		}
		else if (insts[i].inst_op == "ADD")
		{
			insts[i].rd = stoi(temp.substr(temp.find('R') + 1, 1));
			temp.erase(0, 8);
			insts[i].rs1 = stoi(temp.substr(1, 1));
			temp.erase(0, 4);
			insts[i].rs2 = stoi(temp.substr(1, 1));
		}
		else if (insts[i].inst_op == "NAND")
		{
			insts[i].rd = stoi(temp.substr(temp.find('R') + 1, 1));
			temp.erase(0, 9);
			insts[i].rs1 = stoi(temp.substr(1, 1));
			temp.erase(0, 4);
			insts[i].rs2 = stoi(temp.substr(1, 1));
		}
		else if (insts[i].inst_op == "MUL")
		{
			insts[i].rd = stoi(temp.substr(temp.find('R') + 1, 1));
			temp.erase(0, 8);
			insts[i].rs1 = stoi(temp.substr(1, 1));
			temp.erase(0, 4);
			insts[i].rs2 = stoi(temp.substr(1, 1));
		}
		else if (insts[i].inst_op == "ADDI")
		{
			insts[i].rd = stoi(temp.substr(temp.find('R') + 1, 1));
			temp.erase(0, 9);
			insts[i].rs1 = stoi(temp.substr(1, 1));
			temp.erase(0, 4);
			insts[i].imm = stoi(temp);
		}
		else if (insts[i].inst_op == "NEG")
		{
			insts[i].rd = stoi(temp.substr(temp.find('R') + 1, 1));
			temp.erase(0, 8);
			insts[i].rs1 = stoi(temp.substr(1, 1));
		}

		i++;
	}
	instfile.close();


	//removing any instruction writing to R0
	ofstream modify_instfile;
	modify_instfile.open("instructions.txt");

	for (size_t i = 0; i < insts.size(); i++)
	{
		if (insts[i].rd == 0)
		{
			insts.erase(insts.begin() + i);
			i--;
		}

		else
		{
			if (insts[i].head != "")
			{
				if (i == 0)
					modify_instfile << insts[i].head << ": " << insts[i].inst;
				else
					modify_instfile << endl << insts[i].head << ": " << insts[i].inst;
			}
			else
			{
				if (i == 0)
					modify_instfile << insts[i].inst;
				else
					modify_instfile << endl << insts[i].inst;
			}
		}
	}

	modify_instfile.close();

	for (size_t i = 0; i < insts.size(); i++)
	{
		insts[i].inst_PC = starting_address + i;
	}

	return insts;
}



int main()
{
	//reading instructions
	vector<instruction> insts = get_instructions();

	//reading data, and addresses
	vector<pair<int, int>> data;
	data = get_data();

	//holding labels and their PCs
	vector<pair<string, int>> labels;
	for (size_t i = 0; i < insts.size(); i++)
	{
		if (insts[i].label != "")
		{
			for (size_t j = 0; j < insts.size(); j++)
			{
				if (insts[j].head == insts[i].label)
					labels.push_back(pair<string, int>(insts[i].label, insts[j].inst_PC));
			}
		}
	}

	//holding loads & stores instructions in order and their effective addresses
	vector<pair<string, int>> loads_stores;
	for (size_t i = 0; i < insts.size(); i++)
	{
		if (insts[i].inst_op == "LOAD" || insts[i].inst_op == "STORE")
			loads_stores.push_back(pair<string, int>(insts[i].inst, -1));
	}

	//hold all flags for each load and store
	vector<pair<string, vector<bool>>> L_S_flags(loads_stores.size());
	for (size_t i = 0; i < loads_stores.size(); i++)
	{
		L_S_flags[i].first = loads_stores[i].first;
	}

	//checking the parcing results
	/*for (size_t i = 0; i < insts.size(); i++)
	{
		if (insts[i].head != "")
			cout << insts[i].head << ": " << insts[i].inst << "\n(" << insts[i].inst_op << ")\t(" << insts[i].rd << ")\t(" << insts[i].rs1 << ")\t(" << insts[i].rs2
			<< ")\t(" << insts[i].imm << ")\t(" << insts[i].label << ")\n\n";
		else
			cout << insts[i].inst << "\n(" << insts[i].inst_op << ")\t(" << insts[i].rd << ")\t(" << insts[i].rs1 << ")\t(" << insts[i].rs2
			<< ")\t(" << insts[i].imm << ")\t(" << insts[i].label << ")\n\n";
	}

	cout << endl << endl << endl;

	cout << "PC: " << starting_address << endl;
	for (size_t i = 0; i < data.size(); i++)
	{
		cout << data[i].first << ": " << data[i].second << endl;
	}*/

	//Tomasulo
	int branches = 0, branches_taken = 0, instructions_executed = 0;

	int RegFile[8] = { 0, };
	string RegStat[8];

	res_station load1, load2, store1, store2, branch, jump_return, add_addi1, add_addi2, add_addi3,
		negate, negAND, multiply;

	load1.ex_counter = 2;
	load2.ex_counter = 2;
	store1.ex_counter = 2;
	store2.ex_counter = 2;
	branch.ex_counter = 1;
	jump_return.ex_counter = 1;
	add_addi1.ex_counter = 2;
	add_addi2.ex_counter = 2;
	add_addi3.ex_counter = 2;
	negate.ex_counter = 1;
	negAND.ex_counter = 1;
	multiply.ex_counter = 8;
	int load_write_back = 4;
	int store_write_back = 1;

	int cycles = 0, issue_counter = 0;

	vector<string> insts_to_flush;
	bool saving_for_flushing = false; //to flush if we want to branch and to hold the execution stage		DONE

	//if we encounter branch so make it true till the branch writeback and save the instructions issued		DONE 
	//so that if you will branch , you reverse what you did in issuing and change issue_counter

	bool dont_issue = false; //to wait on issuing for JAL and RET until they write back						DONE
							 //or to stop issuing if instructions are over									DONE

	vector<instruction> finished, ready_for_writing;


	bool stop = false;
	int extra_loops = 0;

	while (true)
	{
		//add int instructionsExecuted to calculate IPC, add it in the end execution stage		DONE
		//handle the issues of the execution because of the branch								DONE
		//execution 5ly kol 7aga fe cycle lw7dha mn el start ex, end ex, w el write back		DONE
		
		

		//write back here before end execution
		//in the write back you need to make all things in the structs of res station and instruction   DONE
		//to their defaults. Also, solve load and store													DONE
		//you need to write back  only one instruction per cycle										DONE
	

		
		//Write back
		for (size_t i = 0; i < insts.size(); i++)
		{
			if (cycles > 2 && insts[i].end_exe != -1 && insts[i].write == -1)
			{
				if (insts[i].inst_op == "NEG")
				{
					insts[i].write = cycles;
					//int RegFile[8] = { 0, };
					//string RegStat[8];
					for (size_t j = 1; j < 8; j++)
					{
						if (RegStat[j] == "negate")
						{
							RegFile[j] = insts[i].result;
							RegStat[j] = "";
						}
					}

					
					if (load1.Q_j == "negate")
					{
						load1.V_j = insts[i].result;
						load1.Q_j = "";
					}
					if (load1.Q_k == "negate")
					{
						load1.V_k = insts[i].result;
						load1.Q_k = "";
					}

					if (load2.Q_j == "negate")
					{
						load2.V_j = insts[i].result;
						load2.Q_j = "";
					}
					if (load2.Q_k == "negate")
					{
						load2.V_k = insts[i].result;
						load2.Q_k = "";
					}

					if (store1.Q_j == "negate")
					{
						store1.V_j = insts[i].result;
						store1.Q_j = "";
					}
					if (store1.Q_k == "negate")
					{
						store1.V_k = insts[i].result;
						store1.Q_k = "";
					}

					if (store2.Q_j == "negate")
					{	
						store2.V_j = insts[i].result;
						store2.Q_j = "";
					}	
					if (store2.Q_k == "negate")
					{	
						store2.V_k = insts[i].result;
						store2.Q_k = "";
					}

					if (branch.Q_j == "negate")
					{
						branch.V_j = insts[i].result;
						branch.Q_j = "";
					}
					if (branch.Q_k == "negate")
					{
						branch.V_k = insts[i].result;
						branch.Q_k = "";
					}

					if (jump_return.Q_j == "negate")
					{
						jump_return.V_j = insts[i].result;
						jump_return.Q_j = "";
					}
					if (jump_return.Q_k == "negate")
					{
						jump_return.V_k = insts[i].result;
						jump_return.Q_k = "";
					}

					if (add_addi1.Q_j == "negate")
					{
						add_addi1.V_j = insts[i].result;
						add_addi1.Q_j = "";
					}
					if (add_addi1.Q_k == "negate")
					{
						add_addi1.V_k = insts[i].result;
						add_addi1.Q_k = "";
					}

					if (add_addi2.Q_j == "negate")
					{
						add_addi2.V_j = insts[i].result;
						add_addi2.Q_j = "";
					}
					if (add_addi2.Q_k == "negate")
					{
						add_addi2.V_k = insts[i].result;
						add_addi2.Q_k = "";
					}

					if (add_addi3.Q_j == "negate")
					{
						add_addi3.V_j = insts[i].result;
						add_addi3.Q_j = "";
					}
					if (add_addi3.Q_k == "negate")
					{
						add_addi3.V_k = insts[i].result;
						add_addi3.Q_k = "";
					}

					if (negAND.Q_j == "negate")
					{
						negAND.V_j = insts[i].result;
						negAND.Q_j = "";
					}
					if (negAND.Q_k == "negate")
					{
						negAND.V_k = insts[i].result;
						negAND.Q_k = "";
					}

					if (multiply.Q_j == "negate")
					{
						multiply.V_j = insts[i].result;
						multiply.Q_j = "";
					}
					if (multiply.Q_k == "negate")
					{
						multiply.V_k = insts[i].result;
						multiply.Q_k = "";
					}

					finished.push_back(insts[i]);

					

					negate.busy = false;
					negate.ex_counter = 1;
					negate.Q_j = ""; negate.Q_k = ""; negate.processing_instruction = "";
					negate.V_j = -999; negate.V_k = -999; negate.A = -1; insts[i].write = -1;


					
					insts[i].address = -1; insts[i].end_exe = -1; insts[i].issue = -1;
					insts[i].start_exe = -1; insts[i].result = -999;
					
					break;
				}

				else if (insts[i].inst_op == "NAND")
				{
					insts[i].write = cycles;
					//int RegFile[8] = { 0, };
					//string RegStat[8];
					for (size_t j = 1; j < 8; j++)
					{
						if (RegStat[j] == "negAND")
						{
							RegFile[j] = insts[i].result;
							RegStat[j] = "";
						}
					}

					
					if (load1.Q_j == "negAND")
					{
						load1.V_j = insts[i].result;
						load1.Q_j = "";
					}
					if (load1.Q_k == "negAND")
					{
						load1.V_k = insts[i].result;
						load1.Q_k = "";
					}

					if (load2.Q_j == "negAND")
					{
						load2.V_j = insts[i].result;
						load2.Q_j = "";
					}
					if (load2.Q_k == "negate")
					{
						load2.V_k = insts[i].result;
						load2.Q_k = "";
					}

					if (store1.Q_j == "negAND")
					{
						store1.V_j = insts[i].result;
						store1.Q_j = "";
					}
					if (store1.Q_k == "negAND")
					{
						store1.V_k = insts[i].result;
						store1.Q_k = "";
					}

					if (store2.Q_j == "negAND")
					{
						store2.V_j = insts[i].result;
						store2.Q_j = "";
					}
					if (store2.Q_k == "negAND")
					{
						store2.V_k = insts[i].result;
						store2.Q_k = "";
					}

					if (branch.Q_j == "negAND")
					{
						branch.V_j = insts[i].result;
						branch.Q_j = "";
					}
					if (branch.Q_k == "negAND")
					{
						branch.V_k = insts[i].result;
						branch.Q_k = "";
					}

					if (jump_return.Q_j == "negAND")
					{
						jump_return.V_j = insts[i].result;
						jump_return.Q_j = "";
					}
					if (jump_return.Q_k == "negAND")
					{
						jump_return.V_k = insts[i].result;
						jump_return.Q_k = "";
					}

					if (add_addi1.Q_j == "negAND")
					{
						add_addi1.V_j = insts[i].result;
						add_addi1.Q_j = "";
					}
					if (add_addi1.Q_k == "negAND")
					{
						add_addi1.V_k = insts[i].result;
						add_addi1.Q_k = "";
					}

					if (add_addi2.Q_j == "negAND")
					{
						add_addi2.V_j = insts[i].result;
						add_addi2.Q_j = "";
					}
					if (add_addi2.Q_k == "negAND")
					{
						add_addi2.V_k = insts[i].result;
						add_addi2.Q_k = "";
					}

					if (add_addi3.Q_j == "negAND")
					{
						add_addi3.V_j = insts[i].result;
						add_addi3.Q_j = "";
					}
					if (add_addi3.Q_k == "negAND")
					{
						add_addi3.V_k = insts[i].result;
						add_addi3.Q_k = "";
					}

					if (negate.Q_j == "negAND")
					{
						negate.V_j = insts[i].result;
						negate.Q_j = "";
					}
					if (negate.Q_k == "negAND")
					{
						negate.V_k = insts[i].result;
						negate.Q_k = "";
					}

					if (multiply.Q_j == "negAND")
					{
						multiply.V_j = insts[i].result;
						multiply.Q_j = "";
					}
					if (multiply.Q_k == "negAND")
					{
						multiply.V_k = insts[i].result;
						multiply.Q_k = "";
					}

					finished.push_back(insts[i]);

					

					negAND.busy = false;
					negAND.ex_counter = 1;
					negAND.Q_j = ""; negAND.Q_k = ""; negAND.processing_instruction = "";
					negAND.V_j = -999; negAND.V_k = -999; negAND.A = -1; insts[i].write = -1;


					
					insts[i].address = -1; insts[i].end_exe = -1; insts[i].issue = -1;
					insts[i].start_exe = -1; insts[i].result = -999;

					break;
				}

				else if (insts[i].inst_op == "MUL")
				{
					insts[i].write = cycles;
					//int RegFile[8] = { 0, };
					//string RegStat[8];
					for (size_t j = 1; j < 8; j++)
					{
						if (RegStat[j] == "multiply")
						{
							RegFile[j] = insts[i].result;
							RegStat[j] = "";
						}
					}

					
					if (load1.Q_j == "multiply")
					{
						load1.V_j = insts[i].result;
						load1.Q_j = "";
					}
					if (load1.Q_k == "multiply")
					{
						load1.V_k = insts[i].result;
						load1.Q_k = "";
					}

					if (load2.Q_j == "multiply")
					{
						load2.V_j = insts[i].result;
						load2.Q_j = "";
					}
					if (load2.Q_k == "multiply")
					{
						load2.V_k = insts[i].result;
						load2.Q_k = "";
					}

					if (store1.Q_j == "multiply")
					{
						store1.V_j = insts[i].result;
						store1.Q_j = "";
					}
					if (store1.Q_k == "multiply")
					{
						store1.V_k = insts[i].result;
						store1.Q_k = "";
					}

					if (store2.Q_j == "multiply")
					{
						store2.V_j = insts[i].result;
						store2.Q_j = "";
					}
					if (store2.Q_k == "multiply")
					{
						store2.V_k = insts[i].result;
						store2.Q_k = "";
					}

					if (branch.Q_j == "multiply")
					{
						branch.V_j = insts[i].result;
						branch.Q_j = "";
					}
					if (branch.Q_k == "multiply")
					{
						branch.V_k = insts[i].result;
						branch.Q_k = "";
					}

					if (jump_return.Q_j == "multiply")
					{
						jump_return.V_j = insts[i].result;
						jump_return.Q_j = "";
					}
					if (jump_return.Q_k == "multiply")
					{
						jump_return.V_k = insts[i].result;
						jump_return.Q_k = "";
					}

					if (add_addi1.Q_j == "multiply")
					{
						add_addi1.V_j = insts[i].result;
						add_addi1.Q_j = "";
					}
					if (add_addi1.Q_k == "multiply")
					{
						add_addi1.V_k = insts[i].result;
						add_addi1.Q_k = "";
					}

					if (add_addi2.Q_j == "multiply")
					{
						add_addi2.V_j = insts[i].result;
						add_addi2.Q_j = "";
					}
					if (add_addi2.Q_k == "multiply")
					{
						add_addi2.V_k = insts[i].result;
						add_addi2.Q_k = "";
					}

					if (add_addi3.Q_j == "multiply")
					{
						add_addi3.V_j = insts[i].result;
						add_addi3.Q_j = "";
					}
					if (add_addi3.Q_k == "multiply")
					{
						add_addi3.V_k = insts[i].result;
						add_addi3.Q_k = "";
					}

					if (negate.Q_j == "multiply")
					{
						negate.V_j = insts[i].result;
						negate.Q_j = "";
					}
					if (negate.Q_k == "multiply")
					{
						negate.V_k = insts[i].result;
						negate.Q_k = "";
					}

					if (negAND.Q_j == "multiply")
					{
						negAND.V_j = insts[i].result;
						negAND.Q_j = "";
					}
					if (negAND.Q_k == "multiply")
					{
						negAND.V_k = insts[i].result;
						negAND.Q_k = "";
					}

					
					finished.push_back(insts[i]);

					multiply.busy = false;
					multiply.ex_counter = 8;
					multiply.Q_j = ""; multiply.Q_k = ""; multiply.processing_instruction = "";
					multiply.V_j = -999; multiply.V_k = -999; multiply.A = -1;


					
					insts[i].address = -1; insts[i].end_exe = -1; insts[i].issue = -1; insts[i].write = -1;
					insts[i].start_exe = -1; insts[i].result = -999;

					break;
				}

				else if (insts[i].inst_op == "JAL")
				{
					insts[i].write = cycles;
					dont_issue = false;

					for (size_t j = 0; j < insts.size(); j++)
					{
						if (insts[j].inst_PC == insts[i].address)
						{
							issue_counter = j;
						}
					}

					//int RegFile[8] = { 0, };
					//string RegStat[8];
					for (size_t j = 1; j < 8; j++)
					{
						if (RegStat[j] == "jump_return")
						{
							RegFile[j] = insts[i].result;
							RegStat[j] = "";
						}
					}

					
					if (load1.Q_j == "jump_return")
					{
						load1.V_j = insts[i].result;
						load1.Q_j = "";
					}
					if (load1.Q_k == "jump_return")
					{
						load1.V_k = insts[i].result;
						load1.Q_k = "";
					}

					if (load2.Q_j == "jump_return")
					{
						load2.V_j = insts[i].result;
						load2.Q_j = "";
					}
					if (load2.Q_k == "jump_return")
					{
						load2.V_k = insts[i].result;
						load2.Q_k = "";
					}

					if (store1.Q_j == "jump_return")
					{
						store1.V_j = insts[i].result;
						store1.Q_j = "";
					}
					if (store1.Q_k == "jump_return")
					{
						store1.V_k = insts[i].result;
						store1.Q_k = "";
					}

					if (store2.Q_j == "jump_return")
					{
						store2.V_j = insts[i].result;
						store2.Q_j = "";
					}
					if (store2.Q_k == "jump_return")
					{
						store2.V_k = insts[i].result;
						store2.Q_k = "";
					}

					if (branch.Q_j == "jump_return")
					{
						branch.V_j = insts[i].result;
						branch.Q_j = "";
					}
					if (branch.Q_k == "jump_return")
					{
						branch.V_k = insts[i].result;
						branch.Q_k = "";
					}

					if (multiply.Q_j == "jump_return")
					{
						multiply.V_j = insts[i].result;
						multiply.Q_j = "";
					}
					if (multiply.Q_k == "jump_return")
					{
						multiply.V_k = insts[i].result;
						multiply.Q_k = "";
					}

					if (add_addi1.Q_j == "jump_return")
					{
						add_addi1.V_j = insts[i].result;
						add_addi1.Q_j = "";
					}
					if (add_addi1.Q_k == "jump_return")
					{
						add_addi1.V_k = insts[i].result;
						add_addi1.Q_k = "";
					}

					if (add_addi2.Q_j == "jump_return")
					{
						add_addi2.V_j = insts[i].result;
						add_addi2.Q_j = "";
					}
					if (add_addi2.Q_k == "jump_return")
					{
						add_addi2.V_k = insts[i].result;
						add_addi2.Q_k = "";
					}

					if (add_addi3.Q_j == "jump_return")
					{
						add_addi3.V_j = insts[i].result;
						add_addi3.Q_j = "";
					}
					if (add_addi3.Q_k == "jump_return")
					{
						add_addi3.V_k = insts[i].result;
						add_addi3.Q_k = "";
					}

					if (negate.Q_j == "jump_return")
					{
						negate.V_j = insts[i].result;
						negate.Q_j = "";
					}
					if (negate.Q_k == "jump_return")
					{
						negate.V_k = insts[i].result;
						negate.Q_k = "";
					}

					if (negAND.Q_j == "jump_return")
					{
						negAND.V_j = insts[i].result;
						negAND.Q_j = "";
					}
					if (negAND.Q_k == "jump_return")
					{
						negAND.V_k = insts[i].result;
						negAND.Q_k = "";
					}

					

					finished.push_back(insts[i]);

					jump_return.busy = false;
					jump_return.ex_counter = 1;
					jump_return.Q_j = ""; jump_return.Q_k = ""; jump_return.processing_instruction = "";
					jump_return.V_j = -999; jump_return.V_k = -999; jump_return.A = -1;


					
					insts[i].address = -1; insts[i].end_exe = -1; insts[i].issue = -1; insts[i].write = -1;
					insts[i].start_exe = -1; insts[i].result = -999;

					break;
		
				}

				else if (insts[i].inst_op == "RET")
				{
					insts[i].write = cycles;
					dont_issue = false;

					for (size_t j = 0; j < insts.size(); j++)
					{
						if (insts[j].inst_PC == insts[i].address)
						{
							issue_counter = j;
						}
					}

					jump_return.busy = false;
					jump_return.ex_counter = 1;
					jump_return.Q_j = ""; jump_return.Q_k = ""; jump_return.processing_instruction = "";
					jump_return.V_j = -999; jump_return.V_k = -999; jump_return.A = -1;

					finished.push_back(insts[i]);

					
					insts[i].address = -1; insts[i].end_exe = -1; insts[i].issue = -1; insts[i].write = -1;
					insts[i].start_exe = -1; insts[i].result = -999;

					break;
				}

				else if (insts[i].inst_op == "BEQ")
				{
					insts[i].write = cycles;
					//vector<string> insts_to_flush;
					//bool saving_for_flushing = false; //to flush if we want to branch and to hold the execution stage
					
					saving_for_flushing = false;

					if (insts[i].result == 1)
					{

						for (size_t j = 0; j < insts.size(); j++)
						{
							if (insts[j].inst_PC == insts[i].address)
							{
								issue_counter = j;
							}
						}

						for (size_t j = 0; j < insts_to_flush.size(); j++)
						{
							for (size_t k = 0; k < insts.size(); k++)
							{
								if (insts_to_flush[j] == insts[k].inst)
								{
									insts[k].issue = -1;
									RegStat[insts[k].rd] = "";

							
									if (load1.processing_instruction == insts[k].inst)
									{
										load1.busy = false; load1.processing_instruction = ""; load1.A = -1;
										load1.Q_j = ""; load1.Q_k = ""; load1.V_j = -999; load1.V_k = -999;
									}

									else if (load2.processing_instruction == insts[k].inst)
									{
										load2.busy = false; load2.processing_instruction = ""; load2.A = -1;
										load2.Q_j = ""; load2.Q_k = ""; load2.V_j = -999; load2.V_k = -999;
									}

									else if (store1.processing_instruction == insts[k].inst)
									{
										store1.busy = false; store1.processing_instruction = ""; store1.A = -1;
										store1.Q_j = ""; store1.Q_k = ""; store1.V_j = -999; store1.V_k = -999;
									}

									else if (store2.processing_instruction == insts[k].inst)
									{
										store2.busy = false; store2.processing_instruction = ""; store2.A = -1;
										store2.Q_j = ""; store2.Q_k = ""; store2.V_j = -999; store2.V_k = -999;
									}

									else if (jump_return.processing_instruction == insts[k].inst)
									{
										dont_issue = false;
										RegStat[1] = "";
										jump_return.busy = false; jump_return.processing_instruction = ""; jump_return.A = -1;
										jump_return.Q_j = ""; jump_return.Q_k = ""; jump_return.V_j = -999; jump_return.V_k = -999;
									}

									else if (add_addi1.processing_instruction == insts[k].inst)
									{
										add_addi1.busy = false; add_addi1.processing_instruction = ""; add_addi1.A = -1;
										add_addi1.Q_j = ""; add_addi1.Q_k = ""; add_addi1.V_j = -999; add_addi1.V_k = -999;
									}

									else if (add_addi2.processing_instruction == insts[k].inst)
									{
										add_addi2.busy = false; add_addi2.processing_instruction = ""; add_addi2.A = -1;
										add_addi2.Q_j = ""; add_addi2.Q_k = ""; add_addi2.V_j = -999; add_addi2.V_k = -999;
									}

									else if (add_addi3.processing_instruction == insts[k].inst)
									{
										add_addi3.busy = false; add_addi3.processing_instruction = ""; add_addi3.A = -1;
										add_addi3.Q_j = ""; add_addi3.Q_k = ""; add_addi3.V_j = -999; add_addi3.V_k = -999;
									}

									else if (negate.processing_instruction == insts[k].inst)
									{
										negate.busy = false; negate.processing_instruction = ""; negate.A = -1;
										negate.Q_j = ""; negate.Q_k = ""; negate.V_j = -999; negate.V_k = -999;
									}

									else if (negAND.processing_instruction == insts[k].inst)
									{
										negAND.busy = false; negAND.processing_instruction = ""; negAND.A = -1;
										negAND.Q_j = ""; negAND.Q_k = ""; negAND.V_j = -999; negAND.V_k = -999;
									}

									else if (multiply.processing_instruction == insts[k].inst)
									{
										multiply.busy = false; multiply.processing_instruction = ""; multiply.A = -1;
										multiply.Q_j = ""; multiply.Q_k = ""; multiply.V_j = -999; multiply.V_k = -999;
									}

								}
							}
						}
					}

					else if (insts[i].result == 0)
					{
						insts_to_flush.clear();
					}

					
					finished.push_back(insts[i]);

					branch.busy = false;
					branch.ex_counter = 1;
					branch.Q_j = ""; branch.Q_k = ""; branch.processing_instruction = "";
					branch.V_j = -999; branch.V_k = -999; branch.A = -1;


					
					insts[i].address = -1; insts[i].end_exe = -1; insts[i].issue = -1; insts[i].write = -1;
					insts[i].start_exe = -1; insts[i].result = -999;

					break;
				}

				else if (insts[i].inst_op == "LOAD")
				{

					if (load1.processing_instruction == insts[i].inst)
					{
						if (load_write_back == 0)
						{
							int j = 0;
							for (; j < loads_stores.size(); j++)
							{
								if (insts[i].inst == loads_stores[j].first)
								{
									break;
								}
							}

							for (size_t k = 0; k < j; k++)
							{
								if (loads_stores[k].first.substr(0, 5) == "STORE")
								{
									for (size_t w = 0; w < insts.size(); w++)
									{
										if (loads_stores[k].first == insts[w].inst)
										{
											if (insts[w].end_exe != -1)//5alst execution but not write back
											{
												if (insts[i].address == insts[w].address)//equal addresses
												{
													for (size_t y = 0; y < L_S_flags.size(); y++)
													{
														if (L_S_flags[y].first == insts[i].inst)
														{
															if (insts[w].write != -1)//5alst wrie back
															{
																L_S_flags[y].second.push_back(true);
																break;
															}
															else//m5lstsh write back
															{
																L_S_flags[y].second.push_back(false);
																break;
															}
														}
													}
												}

												else//not equal addresses
												{
													for (size_t y = 0; y < L_S_flags.size(); y++)
													{
														if (L_S_flags[y].first == insts[i].inst)
														{
															L_S_flags[y].second.push_back(true);
															break;
														}
													}
												}
											}
											//5alset computing address fa hanshof el effective addresses zy
											//b3d wla la 
											//lw zy ba3d: wait for finishing the write back
											//else: start your write back

											//Actual: make vector of flags and after the for k loop finishes
											//check all flags, if all is true, you can start loading
											//else wait

										//else
											//wait
										}
									}
								}
							}

							bool inst_flag = true;

							for (size_t k = 0; k < L_S_flags.size(); k++)
							{
								if (L_S_flags[k].first == insts[i].inst)
								{
									for (size_t w = 0; w < L_S_flags[k].second.size(); w++)
									{
										inst_flag = inst_flag && L_S_flags[k].second[w];
									}
								}
							}

							if (inst_flag == true)
							{
								
								insts[i].write = cycles;
								for (size_t k = 0; k < L_S_flags.size(); k++)
								{
									if (L_S_flags[k].first == insts[i].inst)
									{
										L_S_flags[k].second.clear();
									}
								}

								//insts[i].start_exe = cycles;
								for (size_t k = 0; k < data.size(); k++)
								{
									if (data[k].first == insts[i].address)
										insts[i].result = data[k].second;
								}

								//int RegFile[8] = { 0, };
								//string RegStat[8];
								for (size_t j = 1; j < 8; j++)
								{
									if (RegStat[j] == "load1")
									{
										RegFile[j] = insts[i].result;
										RegStat[j] = "";
									}
								}
								
								if (multiply.Q_j == "load1")
								{
									multiply.V_j = insts[i].result;
									multiply.Q_j = "";
								}
								if (multiply.Q_k == "load1")
								{
									multiply.V_k = insts[i].result;
									multiply.Q_k = "";
								}

								if (load2.Q_j == "load1")
								{
									load2.V_j = insts[i].result;
									load2.Q_j = "";
								}
								if (load2.Q_k == "load1")
								{
									load2.V_k = insts[i].result;
									load2.Q_k = "";
								}

								if (store1.Q_j == "load1")
								{
									store1.V_j = insts[i].result;
									store1.Q_j = "";
								}
								if (store1.Q_k == "load1")
								{
									store1.V_k = insts[i].result;
									store1.Q_k = "";
								}

								if (store2.Q_j == "load1")
								{
									store2.V_j = insts[i].result;
									store2.Q_j = "";
								}
								if (store2.Q_k == "load1")
								{
									store2.V_k = insts[i].result;
									store2.Q_k = "";
								}

								if (branch.Q_j == "load1")
								{
									branch.V_j = insts[i].result;
									branch.Q_j = "";
								}
								if (branch.Q_k == "load1")
								{
									branch.V_k = insts[i].result;
									branch.Q_k = "";
								}

								if (jump_return.Q_j == "load1")
								{
									jump_return.V_j = insts[i].result;
									jump_return.Q_j = "";
								}
								if (jump_return.Q_k == "load1")
								{
									jump_return.V_k = insts[i].result;
									jump_return.Q_k = "";
								}

								if (add_addi1.Q_j == "load1")
								{
									add_addi1.V_j = insts[i].result;
									add_addi1.Q_j = "";
								}
								if (add_addi1.Q_k == "load1")
								{
									add_addi1.V_k = insts[i].result;
									add_addi1.Q_k = "";
								}

								if (add_addi2.Q_j == "load1")
								{
									add_addi2.V_j = insts[i].result;
									add_addi2.Q_j = "";
								}
								if (add_addi2.Q_k == "load1")
								{
									add_addi2.V_k = insts[i].result;
									add_addi2.Q_k = "";
								}

								if (add_addi3.Q_j == "load1")
								{
									add_addi3.V_j = insts[i].result;
									add_addi3.Q_j = "";
								}
								if (add_addi3.Q_k == "load1")
								{
									add_addi3.V_k = insts[i].result;
									add_addi3.Q_k = "";
								}

								if (negate.Q_j == "load1")
								{
									negate.V_j = insts[i].result;
									negate.Q_j = "";
								}
								if (negate.Q_k == "load1")
								{
									negate.V_k = insts[i].result;
									negate.Q_k = "";
								}

								if (negAND.Q_j == "load1")
								{
									negAND.V_j = insts[i].result;
									negAND.Q_j = "";
								}
								if (negAND.Q_k == "load1")
								{
									negAND.V_k = insts[i].result;
									negAND.Q_k = "";
								}

								

								finished.push_back(insts[i]);

								load1.busy = false;
								load1.ex_counter = 8;
								load1.Q_j = ""; load1.Q_k = ""; load1.processing_instruction = "";
								load1.V_j = -999; load1.V_k = -999; load1.A = -1;

								load_write_back = 4;
								
								insts[i].address = -1; insts[i].end_exe = -1; insts[i].issue = -1; insts[i].write = -1;
								insts[i].start_exe = -1; insts[i].result = -999;

								break;

							}
						}

						else
						{
							load_write_back--;
						}	
					}

					else
					{
						if (load_write_back == 0)
						{
						int j = 0;
						for (; j < loads_stores.size(); j++)
						{
							if (insts[i].inst == loads_stores[j].first)
							{
								break;
							}
						}

						for (size_t k = 0; k < j; k++)
						{
							if (loads_stores[k].first.substr(0, 5) == "STORE")
							{
								for (size_t w = 0; w < insts.size(); w++)
								{
									if (loads_stores[k].first == insts[w].inst)
									{
										if (insts[w].end_exe != -1)//5alst execution but not write back
										{
											if (insts[i].address == insts[w].address)//equal addresses
											{
												for (size_t y = 0; y < L_S_flags.size(); y++)
												{
													if (L_S_flags[y].first == insts[i].inst)
													{
														if (insts[w].write != -1)//5alst wrie back
														{
															L_S_flags[y].second.push_back(true);
															break;
														}
														else//m5lstsh write back
														{
															L_S_flags[y].second.push_back(false);
															break;
														}
													}
												}
											}

											else//not equal addresses
											{
												for (size_t y = 0; y < L_S_flags.size(); y++)
												{
													if (L_S_flags[y].first == insts[i].inst)
													{
														L_S_flags[y].second.push_back(true);
														break;
													}
												}
											}
										}
										//5alset computing address fa hanshof el effective addresses zy
										//b3d wla la 
										//lw zy ba3d: wait for finishing the write back
										//else: start your write back

										//Actual: make vector of flags and after the for k loop finishes
										//check all flags, if all is true, you can start loading
										//else wait

									//else
										//wait
									}
								}
							}
						}

						bool inst_flag = true;

						for (size_t k = 0; k < L_S_flags.size(); k++)
						{
							if (L_S_flags[k].first == insts[i].inst)
							{
								for (size_t w = 0; w < L_S_flags[k].second.size(); w++)
								{
									inst_flag = inst_flag && L_S_flags[k].second[w];
								}
							}
						}

						if (inst_flag == true)
						{
							insts[i].write = cycles;
							for (size_t k = 0; k < L_S_flags.size(); k++)
							{
								if (L_S_flags[k].first == insts[i].inst)
								{
									L_S_flags[k].second.clear();
								}
							}
							//insts[i].start_exe = cycles;
							for (size_t k = 0; k < data.size(); k++)
							{
								if (data[k].first == insts[i].address)
									insts[i].result = data[k].second;
							}

							//int RegFile[8] = { 0, };
							//string RegStat[8];
							for (size_t j = 1; j < 8; j++)
							{
								if (RegStat[j] == "load2")
								{
									RegFile[j] = insts[i].result;
									RegStat[j] = "";
								}
							}
							
							if (multiply.Q_j == "load2")
							{
								multiply.V_j = insts[i].result;
								multiply.Q_j = "";
							}
							if (multiply.Q_k == "load2")
							{
								multiply.V_k = insts[i].result;
								multiply.Q_k = "";
							}

							if (load1.Q_j == "load2")
							{
								load1.V_j = insts[i].result;
								load1.Q_j = "";
							}
							if (load1.Q_k == "load2")
							{
								load1.V_k = insts[i].result;
								load1.Q_k = "";
							}

							if (store1.Q_j == "load2")
							{
								store1.V_j = insts[i].result;
								store1.Q_j = "";
							}
							if (store1.Q_k == "load2")
							{
								store1.V_k = insts[i].result;
								store1.Q_k = "";
							}

							if (store2.Q_j == "load2")
							{
								store2.V_j = insts[i].result;
								store2.Q_j = "";
							}
							if (store2.Q_k == "load2")
							{
								store2.V_k = insts[i].result;
								store2.Q_k = "";
							}

							if (branch.Q_j == "load2")
							{
								branch.V_j = insts[i].result;
								branch.Q_j = "";
							}
							if (branch.Q_k == "load2")
							{
								branch.V_k = insts[i].result;
								branch.Q_k = "";
							}

							if (jump_return.Q_j == "load2")
							{
								jump_return.V_j = insts[i].result;
								jump_return.Q_j = "";
							}
							if (jump_return.Q_k == "load2")
							{
								jump_return.V_k = insts[i].result;
								jump_return.Q_k = "";
							}

							if (add_addi1.Q_j == "load2")
							{
								add_addi1.V_j = insts[i].result;
								add_addi1.Q_j = "";
							}
							if (add_addi1.Q_k == "load2")
							{
								add_addi1.V_k = insts[i].result;
								add_addi1.Q_k = "";
							}

							if (add_addi2.Q_j == "load2")
							{
								add_addi2.V_j = insts[i].result;
								add_addi2.Q_j = "";
							}
							if (add_addi2.Q_k == "load2")
							{
								add_addi2.V_k = insts[i].result;
								add_addi2.Q_k = "";
							}

							if (add_addi3.Q_j == "load2")
							{
								add_addi3.V_j = insts[i].result;
								add_addi3.Q_j = "";
							}
							if (add_addi3.Q_k == "load2")
							{
								add_addi3.V_k = insts[i].result;
								add_addi3.Q_k = "";
							}

							if (negate.Q_j == "load2")
							{
								negate.V_j = insts[i].result;
								negate.Q_j = "";
							}
							if (negate.Q_k == "load2")
							{
								negate.V_k = insts[i].result;
								negate.Q_k = "";
							}

							if (negAND.Q_j == "load2")
							{
								negAND.V_j = insts[i].result;
								negAND.Q_j = "";
							}
							if (negAND.Q_k == "load2")
							{
								negAND.V_k = insts[i].result;
								negAND.Q_k = "";
							}

							

							finished.push_back(insts[i]);

							load2.busy = false;
							load2.ex_counter = 8;
							load2.Q_j = ""; load2.Q_k = ""; load2.processing_instruction = "";
							load2.V_j = -999; load2.V_k = -999; load2.A = -1;

							load_write_back = 4;
							
							insts[i].address = -1; insts[i].end_exe = -1; insts[i].issue = -1; insts[i].write = -1;
							insts[i].start_exe = -1; insts[i].result = -999;

							break;

						}
					}

						else
						{
							load_write_back--;
						}
							
					}
				}

				else if (insts[i].inst_op == "STORE")
				{
					if (store1.processing_instruction == insts[i].inst)
					{
						if (store_write_back == 0)
						{
							int j = 0;
							for (; j < loads_stores.size(); j++)
							{
								if (insts[i].inst == loads_stores[j].first)
								{
									break;
								}
							}

							for (size_t k = 0; k < j; k++)
							{
								if (loads_stores[k].first.substr(0, 5) == "STORE" || loads_stores[k].first.substr(0, 4) == "LOAD")
								{
									for (size_t w = 0; w < insts.size(); w++)
									{
										if (loads_stores[k].first == insts[w].inst)
										{
											if (insts[w].end_exe != -1)//5alst execution but not write back
											{
												if (insts[i].address == insts[w].address)//equal addresses
												{
													for (size_t y = 0; y < L_S_flags.size(); y++)
													{
														if (L_S_flags[y].first == insts[i].inst)
														{
															if (insts[w].write != -1)//5alst write back
															{
																L_S_flags[y].second.push_back(true);
																break;
															}
															else//m5lstsh write back
															{
																L_S_flags[y].second.push_back(false);
																break;
															}
														}
													}
												}

												else//not equal addresses
												{
													//cout << endl << L_S_flags.size() << endl << insts.size();


													for (size_t y = 0; y < L_S_flags.size(); y++)
													{
														if (L_S_flags[y].first == insts[i].inst)
														{
															L_S_flags[y].second.push_back(true);
															break;
														}
													}
												}
											}
											//5alset computing address fa hanshof el effective addresses zy
											//b3d wla la 
											//lw zy ba3d: wait for finishing the write back
											//else: start your write back

											//Actual: make vector of flags and after the for k loop finishes
											//check all flags, if all is true, you can start loading
											//else wait

										//else
											//wait
										}
									}
								}
							}

							bool inst_flag = true;

							for (size_t k = 0; k < L_S_flags.size(); k++)
							{
								if (L_S_flags[k].first == insts[i].inst)
								{
									for (size_t w = 0; w < L_S_flags[k].second.size(); w++)
									{
										inst_flag = inst_flag && L_S_flags[k].second[w];
									}
								}
							}

							if (store1.Q_k == "" && inst_flag == true)
							{
								insts[i].write = cycles;
								for (size_t k = 0; k < L_S_flags.size(); k++)
								{
									if (L_S_flags[k].first == insts[i].inst)
									{
										L_S_flags[k].second.clear();
									}
								}

								bool address_exist = false;
								for (size_t k = 0; k < data.size(); i++)
								{
									if (data[k].first == insts[i].address)
									{
										address_exist = true;
										data[k].second = store1.V_k;
									}
								}

								if (address_exist = false)
								{
									data.push_back(pair<int, int>(insts[i].address, store1.V_k));
								}
								
								

								finished.push_back(insts[i]);

								store1.busy = false;
								store1.ex_counter = 8;
								store1.Q_j = ""; store1.Q_k = ""; store1.processing_instruction = "";
								store1.V_j = -999; store1.V_k = -999; store1.A = -1;

								store_write_back = 1;
								
								insts[i].address = -1; insts[i].end_exe = -1; insts[i].issue = -1; insts[i].write = -1;
								insts[i].start_exe = -1; insts[i].result = -999;

								break;

							}

						}

						else
						{
							store_write_back--;
						}
					}

					else
					{
						if (store_write_back == 0)
						{
							int j = 0;
							for (; j < loads_stores.size(); j++)
							{
								if (insts[i].inst == loads_stores[j].first)
								{
									break;
								}
							}

							for (size_t k = 0; k < j; k++)
							{
								if (loads_stores[k].first.substr(0, 5) == "STORE" || loads_stores[k].first.substr(0, 4) == "LOAD")
								{
									for (size_t w = 0; w < insts.size(); w++)
									{
										if (loads_stores[k].first == insts[w].inst)
										{
											if (insts[w].end_exe != -1)//5alst execution but not write back
											{
												if (insts[i].address == insts[w].address)//equal addresses
												{
													for (size_t y = 0; y < L_S_flags.size(); y++)
													{
														if (L_S_flags[y].first == insts[i].inst)
														{
															if (insts[w].write != -1)//5alst write back
															{
																L_S_flags[y].second.push_back(true);
																break;
															}
															else//m5lstsh write back
															{
																L_S_flags[y].second.push_back(false);
																break;
															}
														}
													}
												}

												else//not equal addresses
												{
													for (size_t y = 0; y < L_S_flags.size(); y++)
													{
														if (L_S_flags[y].first == insts[i].inst)
														{
															L_S_flags[y].second.push_back(true);
															break;
														}
													}
												}
											}
											//5alset computing address fa hanshof el effective addresses zy
											//b3d wla la 
											//lw zy ba3d: wait for finishing the write back
											//else: start your write back

											//Actual: make vector of flags and after the for k loop finishes
											//check all flags, if all is true, you can start loading
											//else wait

										//else
											//wait
										}
									}
								}
							}

							bool inst_flag = true;

							for (size_t k = 0; k < L_S_flags.size(); k++)
							{
								if (L_S_flags[k].first == insts[i].inst)
								{
									for (size_t w = 0; w < L_S_flags[k].second.size(); w++)
									{
										inst_flag = inst_flag && L_S_flags[k].second[w];
									}
								}
							}

							if (store2.Q_k == "" && inst_flag == true)
							{
								insts[i].write = cycles;
								for (size_t k = 0; k < L_S_flags.size(); k++)
								{
									if (L_S_flags[k].first == insts[i].inst)
									{
										L_S_flags[k].second.clear();
									}
								}

								bool address_exist = false;
								for (size_t k = 0; k < data.size(); i++)
								{
									if (data[k].first == insts[i].address)
									{
										address_exist = true;
										data[k].second = store2.V_k;
									}
								}

								if (address_exist = false)
								{
									data.push_back(pair<int, int>(insts[i].address, store2.V_k));
								}

								

								finished.push_back(insts[i]);

								store2.busy = false;
								store2.ex_counter = 8;
								store2.Q_j = ""; store2.Q_k = ""; store2.processing_instruction = "";
								store2.V_j = -999; store2.V_k = -999; store2.A = -1;

								store_write_back = 1;
								
								insts[i].address = -1; insts[i].end_exe = -1; insts[i].issue = -1; insts[i].write = -1;
								insts[i].start_exe = -1; insts[i].result = -999;

								break;

							}

						}

						else
						{
							store_write_back--;
						}
					}
				}

				else if (insts[i].inst_op == "ADD" || insts[i].inst_op == "ADDI")
				{
					if (add_addi1.processing_instruction == insts[i].inst)
					{
						insts[i].write = cycles;
						//int RegFile[8] = { 0, };
						//string RegStat[8];
						for (size_t j = 1; j < 8; j++)
						{
							if (RegStat[j] == "add_addi1")
							{
								RegFile[j] = insts[i].result;
								RegStat[j] = "";
							}
						}

						
						if (load1.Q_j == "add_addi1")
						{
							load1.V_j = insts[i].result;
							load1.Q_j = "";
						}
						if (load1.Q_k == "add_addi1")
						{
							load1.V_k = insts[i].result;
							load1.Q_k = "";
						}

						if (load2.Q_j == "add_addi1")
						{
							load2.V_j = insts[i].result;
							load2.Q_j = "";
						}
						if (load2.Q_k == "add_addi1")
						{
							load2.V_k = insts[i].result;
							load2.Q_k = "";
						}

						if (store1.Q_j == "add_addi1")
						{
							store1.V_j = insts[i].result;
							store1.Q_j = "";
						}
						if (store1.Q_k == "add_addi1")
						{
							store1.V_k = insts[i].result;
							store1.Q_k = "";
						}

						if (store2.Q_j == "add_addi1")
						{
							store2.V_j = insts[i].result;
							store2.Q_j = "";
						}
						if (store2.Q_k == "add_addi1")
						{
							store2.V_k = insts[i].result;
							store2.Q_k = "";
						}

						if (branch.Q_j == "add_addi1")
						{
							branch.V_j = insts[i].result;
							branch.Q_j = "";
						}
						if (branch.Q_k == "add_addi1")
						{
							branch.V_k = insts[i].result;
							branch.Q_k = "";
						}

						if (jump_return.Q_j == "add_addi1")
						{
							jump_return.V_j = insts[i].result;
							jump_return.Q_j = "";
						}
						if (jump_return.Q_k == "add_addi1")
						{
							jump_return.V_k = insts[i].result;
							jump_return.Q_k = "";
						}

						if (multiply.Q_j == "add_addi1")
						{
							multiply.V_j = insts[i].result;
							multiply.Q_j = "";
						}
						if (multiply.Q_k == "add_addi1")
						{
							multiply.V_k = insts[i].result;
							multiply.Q_k = "";
						}

						if (add_addi2.Q_j == "add_addi1")
						{
							add_addi2.V_j = insts[i].result;
							add_addi2.Q_j = "";
						}
						if (add_addi2.Q_k == "add_addi1")
						{
							add_addi2.V_k = insts[i].result;
							add_addi2.Q_k = "";
						}

						if (add_addi3.Q_j == "add_addi1")
						{
							add_addi3.V_j = insts[i].result;
							add_addi3.Q_j = "";
						}
						if (add_addi3.Q_k == "add_addi1")
						{
							add_addi3.V_k = insts[i].result;
							add_addi3.Q_k = "";
						}

						if (negate.Q_j == "add_addi1")
						{
							negate.V_j = insts[i].result;
							negate.Q_j = "";
						}
						if (negate.Q_k == "add_addi1")
						{
							negate.V_k = insts[i].result;
							negate.Q_k = "";
						}

						if (negAND.Q_j == "add_addi1")
						{
							negAND.V_j = insts[i].result;
							negAND.Q_j = "";
						}
						if (negAND.Q_k == "add_addi1")
						{
							negAND.V_k = insts[i].result;
							negAND.Q_k = "";
						}

						
						finished.push_back(insts[i]);

						add_addi1.busy = false;
						add_addi1.ex_counter = 2;
						add_addi1.Q_j = ""; add_addi1.Q_k = ""; add_addi1.processing_instruction = "";
						add_addi1.V_j = -999; add_addi1.V_k = -999; add_addi1.A = -1;


						
						insts[i].address = -1; insts[i].end_exe = -1; insts[i].issue = -1; insts[i].write = -1;
						insts[i].start_exe = -1; insts[i].result = -999;

						break;
					}
					
					if (add_addi2.processing_instruction == insts[i].inst)
					{
						insts[i].write = cycles;
						//int RegFile[8] = { 0, };
						//string RegStat[8];
						for (size_t j = 1; j < 8; j++)
						{
							if (RegStat[j] == "add_addi2")
							{
								RegFile[j] = insts[i].result;
								RegStat[j] = "";
							}
						}

						
						if (load1.Q_j == "add_addi2")
						{
							load1.V_j = insts[i].result;
							load1.Q_j = "";
						}
						if (load1.Q_k == "add_addi2")
						{
							load1.V_k = insts[i].result;
							load1.Q_k = "";
						}

						if (load2.Q_j == "add_addi2")
						{
							load2.V_j = insts[i].result;
							load2.Q_j = "";
						}
						if (load2.Q_k == "add_addi2")
						{
							load2.V_k = insts[i].result;
							load2.Q_k = "";
						}

						if (store1.Q_j == "add_addi2")
						{
							store1.V_j = insts[i].result;
							store1.Q_j = "";
						}
						if (store1.Q_k == "add_addi2")
						{
							store1.V_k = insts[i].result;
							store1.Q_k = "";
						}

						if (store2.Q_j == "add_addi2")
						{
							store2.V_j = insts[i].result;
							store2.Q_j = "";
						}
						if (store2.Q_k == "add_addi2")
						{
							store2.V_k = insts[i].result;
							store2.Q_k = "";
						}

						if (branch.Q_j == "add_addi2")
						{
							branch.V_j = insts[i].result;
							branch.Q_j = "";
						}
						if (branch.Q_k == "add_addi2")
						{
							branch.V_k = insts[i].result;
							branch.Q_k = "";
						}

						if (jump_return.Q_j == "add_addi2")
						{
							jump_return.V_j = insts[i].result;
							jump_return.Q_j = "";
						}
						if (jump_return.Q_k == "add_addi2")
						{
							jump_return.V_k = insts[i].result;
							jump_return.Q_k = "";
						}

						if (multiply.Q_j == "add_addi2")
						{
							multiply.V_j = insts[i].result;
							multiply.Q_j = "";
						}
						if (multiply.Q_k == "add_addi2")
						{
							multiply.V_k = insts[i].result;
							multiply.Q_k = "";
						}

						if (add_addi1.Q_j == "add_addi2")
						{
							add_addi1.V_j = insts[i].result;
							add_addi1.Q_j = "";
						}
						if (add_addi1.Q_k == "add_addi2")
						{
							add_addi1.V_k = insts[i].result;
							add_addi1.Q_k = "";
						}

						if (add_addi3.Q_j == "add_addi2")
						{
							add_addi3.V_j = insts[i].result;
							add_addi3.Q_j = "";
						}
						if (add_addi3.Q_k == "add_addi2")
						{
							add_addi3.V_k = insts[i].result;
							add_addi3.Q_k = "";
						}

						if (negate.Q_j == "add_addi2")
						{
							negate.V_j = insts[i].result;
							negate.Q_j = "";
						}
						if (negate.Q_k == "add_addi2")
						{
							negate.V_k = insts[i].result;
							negate.Q_k = "";
						}

						if (negAND.Q_j == "add_addi2")
						{
							negAND.V_j = insts[i].result;
							negAND.Q_j = "";
						}
						if (negAND.Q_k == "add_addi2")
						{
							negAND.V_k = insts[i].result;
							negAND.Q_k = "";
						}

						

						finished.push_back(insts[i]);

						add_addi2.busy = false;
						add_addi2.ex_counter = 2;
						add_addi2.Q_j = ""; add_addi2.Q_k = ""; add_addi2.processing_instruction = "";
						add_addi2.V_j = -999; add_addi2.V_k = -999; add_addi2.A = -1;


						
						insts[i].address = -1; insts[i].end_exe = -1; insts[i].issue = -1; insts[i].write = -1;
						insts[i].start_exe = -1; insts[i].result = -999;

						break;
					}
					
					else
					{
						insts[i].write = cycles;
						//int RegFile[8] = { 0, };
						//string RegStat[8];
						for (size_t j = 1; j < 8; j++)
						{
							if (RegStat[j] == "add_addi3")
							{
								RegFile[j] = insts[i].result;
								RegStat[j] = "";
							}
						}

						
						if (load1.Q_j == "add_addi3")
						{
							load1.V_j = insts[i].result;
							load1.Q_j = "";
						}
						if (load1.Q_k == "add_addi3")
						{
							load1.V_k = insts[i].result;
							load1.Q_k = "";
						}

						if (load2.Q_j == "add_addi3")
						{
							load2.V_j = insts[i].result;
							load2.Q_j = "";
						}
						if (load2.Q_k == "add_addi3")
						{
							load2.V_k = insts[i].result;
							load2.Q_k = "";
						}

						if (store1.Q_j == "add_addi3")
						{
							store1.V_j = insts[i].result;
							store1.Q_j = "";
						}
						if (store1.Q_k == "add_addi3")
						{
							store1.V_k = insts[i].result;
							store1.Q_k = "";
						}

						if (store2.Q_j == "add_addi3")
						{
							store2.V_j = insts[i].result;
							store2.Q_j = "";
						}
						if (store2.Q_k == "add_addi3")
						{
							store2.V_k = insts[i].result;
							store2.Q_k = "";
						}

						if (branch.Q_j == "add_addi3")
						{
							branch.V_j = insts[i].result;
							branch.Q_j = "";
						}
						if (branch.Q_k == "add_addi3")
						{
							branch.V_k = insts[i].result;
							branch.Q_k = "";
						}

						if (jump_return.Q_j == "add_addi3")
						{
							jump_return.V_j = insts[i].result;
							jump_return.Q_j = "";
						}
						if (jump_return.Q_k == "add_addi3")
						{
							jump_return.V_k = insts[i].result;
							jump_return.Q_k = "";
						}

						if (multiply.Q_j == "add_addi3")
						{
							multiply.V_j = insts[i].result;
							multiply.Q_j = "";
						}
						if (multiply.Q_k == "add_addi3")
						{
							multiply.V_k = insts[i].result;
							multiply.Q_k = "";
						}

						if (add_addi1.Q_j == "add_addi3")
						{
							add_addi1.V_j = insts[i].result;
							add_addi1.Q_j = "";
						}
						if (add_addi1.Q_k == "add_addi3")
						{
							add_addi1.V_k = insts[i].result;
							add_addi1.Q_k = "";
						}

						if (add_addi2.Q_j == "add_addi3")
						{
							add_addi2.V_j = insts[i].result;
							add_addi2.Q_j = "";
						}
						if (add_addi2.Q_k == "add_addi3")
						{
							add_addi2.V_k = insts[i].result;
							add_addi2.Q_k = "";
						}

						if (negate.Q_j == "add_addi3")
						{
							negate.V_j = insts[i].result;
							negate.Q_j = "";
						}
						if (negate.Q_k == "add_addi3")
						{
							negate.V_k = insts[i].result;
							negate.Q_k = "";
						}

						if (negAND.Q_j == "add_addi3")
						{
							negAND.V_j = insts[i].result;
							negAND.Q_j = "";
						}
						if (negAND.Q_k == "add_addi3")
						{
							negAND.V_k = insts[i].result;
							negAND.Q_k = "";
						}

						

						finished.push_back(insts[i]);

						add_addi3.busy = false;
						add_addi3.ex_counter = 2;
						add_addi3.Q_j = ""; add_addi3.Q_k = ""; add_addi3.processing_instruction = "";
						add_addi3.V_j = -999; add_addi3.V_k = -999; add_addi3.A = -1;


						
						insts[i].address = -1; insts[i].end_exe = -1; insts[i].issue = -1; insts[i].write = -1;
						insts[i].start_exe = -1; insts[i].result = -999;

						break;
					}

				}

			}
		}
	

		//end execution    //ready_for_writing is here
		for (size_t i = 0; i < insts.size(); i++)
		{
			if (cycles > 1 && insts[i].start_exe != -1 && insts[i].end_exe == -1)
			{
				if (insts[i].inst_op == "NEG")
				{
					if (negate.ex_counter == 0)
					{
						insts[i].end_exe = cycles;
						insts[i].result = negate.V_j * -1;
						instructions_executed++;
						ready_for_writing.push_back(insts[i]);
					}

					else
					{
						negate.ex_counter--;
					}
				}

				else if (insts[i].inst_op == "NAND")
				{
					if (negAND.ex_counter == 0)
					{
						insts[i].end_exe = cycles;
						insts[i].result = signed(~(negAND.V_j & negAND.V_k));
						instructions_executed++;
						ready_for_writing.push_back(insts[i]);
					}

					else
					{
						negAND.ex_counter--;
					}
				}

				else if (insts[i].inst_op == "MUL")
				{
					if (multiply.ex_counter == 0)
					{
						insts[i].end_exe = cycles;
						insts[i].result = (multiply.V_j * multiply.V_k) % 65536;
						instructions_executed++;
						ready_for_writing.push_back(insts[i]);
					}

					else
					{
						multiply.ex_counter--;
					}
				}

				else if (insts[i].inst_op == "JAL" || insts[i].inst_op == "RET")
				{
					if (jump_return.ex_counter == 0)
					{
						insts[i].end_exe = cycles;
						instructions_executed++;
						if (insts[i].inst_op == "JAL")
						{
							insts[i].address = jump_return.A;
							insts[i].result = jump_return.V_j;
						}
						else
						{
							insts[i].address = jump_return.A;
						}
						ready_for_writing.push_back(insts[i]);
					}

					else
					{
						jump_return.ex_counter--;
					}
				}

				else if (insts[i].inst_op == "BEQ")
				{
					if (branch.ex_counter == 0)
					{
						insts[i].end_exe = cycles;
						instructions_executed++;
						if (branch.V_j == branch.V_k)
						{
							insts[i].result = 1; //1 means taken
							branches_taken++;
							for (size_t j = 0; j < labels.size(); j++)
							{
								if (insts[i].label == labels[j].first)
									insts[i].address = labels[j].second;
							}
						}
						else
							insts[i].result = 0; //0 means not taken

						ready_for_writing.push_back(insts[i]);
					}

					else
					{
						branch.ex_counter--;
					}
				}

				else if (insts[i].inst_op == "LOAD")
				{
					if (load1.processing_instruction == insts[i].inst)
					{
						if (load1.ex_counter == 0)
						{
							insts[i].end_exe = cycles;
							instructions_executed++;

							load1.A = load1.A + load1.V_j; //try insts[i].imm
							insts[i].address = load1.A;

							ready_for_writing.push_back(insts[i]);
						}

						else
						{
							load1.ex_counter--;
						}
					}

					else
					{
						if (load2.ex_counter == 0)
						{
							insts[i].end_exe = cycles;
							instructions_executed++;


							load2.A = load2.A + load2.V_j; //try insts[i].imm
							insts[i].address = load2.A;

							ready_for_writing.push_back(insts[i]);
						}

						else
						{
							load2.ex_counter--;
						}
					}
				}

				else if (insts[i].inst_op == "STORE")
				{
					if (store1.processing_instruction == insts[i].inst)
					{
						if (store1.ex_counter == 0)
						{
							insts[i].end_exe = cycles;
							instructions_executed++;


							store1.A = store1.A + store1.V_j; //try insts[i].imm
							insts[i].address = store1.A;
							//vj address, vk value

							ready_for_writing.push_back(insts[i]);
						}

						else
						{
							store1.ex_counter--;
						}
					}

					else
					{
						if (store2.ex_counter == 0)
						{
							insts[i].end_exe = cycles;
							instructions_executed++;


							store2.A = store2.A + store2.V_j; //try insts[i].imm
							insts[i].address = store2.A;
							//vj address, vk value

							ready_for_writing.push_back(insts[i]);
						}

						else
						{
							store2.ex_counter--;
						}
					}
				}

				else if (insts[i].inst_op == "ADD" || insts[i].inst_op == "ADDI")
				{
					if (add_addi1.processing_instruction == insts[i].inst)
					{
						if (add_addi1.ex_counter == 0)
						{
							insts[i].end_exe = cycles;
							if (insts[i].inst_op == "ADD")
								insts[i].result = add_addi1.V_j + add_addi1.V_k;
							if (insts[i].inst_op == "ADDI")
								insts[i].result = add_addi1.V_j + insts[i].imm;
							instructions_executed++;
							ready_for_writing.push_back(insts[i]);
						}

						else
						{
							add_addi1.ex_counter--;
						}
					}

					else if (add_addi2.processing_instruction == insts[i].inst)
					{
						if (add_addi2.ex_counter == 0)
						{
							insts[i].end_exe = cycles;
							if (insts[i].inst_op == "ADD")
								insts[i].result = add_addi2.V_j + add_addi2.V_k;
							if (insts[i].inst_op == "ADDI")
								insts[i].result = add_addi2.V_j + insts[i].imm;
							instructions_executed++;
							ready_for_writing.push_back(insts[i]);
						}

						else
						{
							add_addi2.ex_counter--;
						}
					}

					else
					{
						if (add_addi3.ex_counter == 0)
						{
							insts[i].end_exe = cycles;
							if (insts[i].inst_op == "ADD")
								insts[i].result = add_addi3.V_j + add_addi3.V_k;
							if (insts[i].inst_op == "ADDI")
								insts[i].result = add_addi3.V_j + insts[i].imm;
							instructions_executed++;
							ready_for_writing.push_back(insts[i]);
						}

						else
						{
							add_addi3.ex_counter--;
						}
					}
				}

			}
		}
		

		//start execution
		for (size_t i = 0; i < insts.size(); i++)
		{ //try removing else if and just put if
			if (cycles > 0 && insts[i].issue != -1 && insts[i].start_exe == -1)
			{
				if (insts[i].inst_op == "NEG" && saving_for_flushing == false)
				{
					if (negate.Q_j == "")
					{
						insts[i].start_exe = cycles;
					}
				}

				else if (insts[i].inst_op == "NAND" && saving_for_flushing == false)
				{
					if (negAND.Q_j == "" && negAND.Q_k == "")
					{
						insts[i].start_exe = cycles;
					}
				}

				else if (insts[i].inst_op == "MUL" && saving_for_flushing == false)
				{
					if (multiply.Q_j == "" && multiply.Q_k == "")
					{
						insts[i].start_exe = cycles;
					}
				}

				else if ((insts[i].inst_op == "ADD" || insts[i].inst_op == "ADDI") && saving_for_flushing == false)
				{
					if (add_addi1.processing_instruction == insts[i].inst)
					{
						if (insts[i].inst_op == "ADD")
						{
							if (add_addi1.Q_j == "" && add_addi1.Q_k == "")
							{
								insts[i].start_exe = cycles;
							}
						}

						else if (insts[i].inst_op == "ADDI")
						{
							if (add_addi1.Q_j == "")
							{
								insts[i].start_exe = cycles;
							}
						}
					}

					else if (add_addi2.processing_instruction == insts[i].inst)
					{
						if (insts[i].inst_op == "ADD")
						{
							if (add_addi2.Q_j == "" && add_addi2.Q_k == "")
							{
								insts[i].start_exe = cycles;
							}
						}

						else if (insts[i].inst_op == "ADDI")
						{
							if (add_addi2.Q_j == "")
							{
								insts[i].start_exe = cycles;
							}
						}
					}

					else
					{
						if (insts[i].inst_op == "ADD")
						{
							if (add_addi3.Q_j == "" && add_addi3.Q_k == "")
							{
								insts[i].start_exe = cycles;
							}
						}

						else if (insts[i].inst_op == "ADDI")
						{
							if (add_addi3.Q_j == "")
							{
								insts[i].start_exe = cycles;
							}
						}
					}
				}

				else if (insts[i].inst_op == "JAL" && saving_for_flushing == false)
				{
					
					insts[i].start_exe = cycles;

				}
				else if (insts[i].inst_op == "RET" && saving_for_flushing == false)
				{
					if (jump_return.Q_j == "")
					{
						insts[i].start_exe = cycles;
					}
				}
				else if (insts[i].inst_op == "BEQ")
				{
					if (branch.Q_j == "" && branch.Q_k == "")
					{
						branches++;
						insts[i].start_exe = cycles;
						
					}
				}
				
				//start ex and write back
				else if (insts[i].inst_op == "LOAD" && saving_for_flushing == false) 
				{
					if (load1.processing_instruction == insts[i].inst)
					{

						if (load1.Q_j == "")
						{
							insts[i].start_exe = cycles;
						}
						
					}

					else   //load2
					{
						
						if (load2.Q_j == "")
						{
							insts[i].start_exe = cycles;
						}
						
					}

				}

				else if (insts[i].inst_op == "store" && saving_for_flushing == false)
				{
					if (store1.processing_instruction == insts[i].inst)
					{
						////vj address, vk value

						if (store1.Q_j == "")
						{
							insts[i].start_exe = cycles;
						}

					}

					else   //load2
					{

						//insts[i].start_exe = cycles;
						//insts[i].end_exe = cycles + 2;
						//store2.A = store2.A + store2.V_j; //try insts[i].imm
						//insts[i].address = store2.A;
						////vj address, vk value

						if (store2.Q_j == "")
						{
							insts[i].start_exe = cycles;
						}

					}
				}
			}

		}
		

		//issuing
		if (issue_counter > insts.size() - 1) //we should break issuing but not execution and write back
			dont_issue = true;

		else
		{
			if (saving_for_flushing == true)
			{
				insts_to_flush.push_back(insts[issue_counter].inst);
				string tempstr;
				for (size_t j = 0; j < insts_to_flush.size(); j++)
				{
					tempstr = insts_to_flush[j];
					for (size_t k = j + 1; k < insts_to_flush.size(); k++)
					{
						if (tempstr == insts_to_flush[k])
							insts_to_flush.erase(insts_to_flush.begin() + k);
					}
				}
			}

			if (insts[issue_counter].inst_op == "LOAD" && dont_issue == false && (load1.busy == false || load2.busy == false))
			{
				if (load1.busy == false)
				{
					if (RegStat[insts[issue_counter].rs1] != "")
						load1.Q_j = RegStat[insts[issue_counter].rs1];
					else
					{
						load1.V_j = RegFile[insts[issue_counter].rs1];
						load1.Q_j = "";
					}

					load1.A = insts[issue_counter].imm;
					load1.busy = true;
					RegStat[insts[issue_counter].rd] = "load1";
					insts[issue_counter].issue = cycles;
					load1.processing_instruction = insts[issue_counter].inst;
				}

				else
				{
					if (RegStat[insts[issue_counter].rs1] != "")
						load2.Q_j = RegStat[insts[issue_counter].rs1];
					else
					{
						load2.V_j = RegFile[insts[issue_counter].rs1];
						load2.Q_j = "";
					}

					load2.A = insts[issue_counter].imm;
					load2.busy = true;
					RegStat[insts[issue_counter].rd] = "load2";
					insts[issue_counter].issue = cycles;
					load2.processing_instruction = insts[issue_counter].inst;
				}
			}

			else if (insts[issue_counter].inst_op == "STORE" && dont_issue == false && (store1.busy == false || store2.busy == false))
			{
				if (store1.busy == false)
				{//vj address, vk value
					if (RegStat[insts[issue_counter].rs1] != "")
						store1.Q_j = RegStat[insts[issue_counter].rs1];
					else
					{
						store1.V_j = RegFile[insts[issue_counter].rs1];
						store1.Q_j = "";
					}

					if (RegStat[insts[issue_counter].rs2] != "")
						store1.Q_k = RegStat[insts[issue_counter].rs2];
					else
					{
						store1.V_k = RegFile[insts[issue_counter].rs2];
						store1.Q_k = "";
					}

					store1.A = insts[issue_counter].imm;
					store1.busy = true;
					insts[issue_counter].issue = cycles;
					store1.processing_instruction = insts[issue_counter].inst;
				}

				else
				{
					if (RegStat[insts[issue_counter].rs1] != "")
						store2.Q_j = RegStat[insts[issue_counter].rs1];
					else
					{
						store2.V_j = RegFile[insts[issue_counter].rs1];
						store2.Q_j = "";
					}

					if (RegStat[insts[issue_counter].rs2] != "")
						store2.Q_k = RegStat[insts[issue_counter].rs2];
					else
					{
						store2.V_k = RegFile[insts[issue_counter].rs2];
						store2.Q_k = "";
					}

					store2.A = insts[issue_counter].imm;
					store2.busy = true;
					insts[issue_counter].issue = cycles;
					store2.processing_instruction = insts[issue_counter].inst;
				}
			}

			else if (insts[issue_counter].inst_op == "NEG" && dont_issue == false && negate.busy == false)
			{
				if (RegStat[insts[issue_counter].rs1] != "")
					negate.Q_j = RegStat[insts[issue_counter].rs1];
				else
				{
					negate.V_j = RegFile[insts[issue_counter].rs1];
					negate.Q_j = "";
				}

				negate.busy = true;
				RegStat[insts[issue_counter].rd] = "negate";
				insts[issue_counter].issue = cycles;
				negate.processing_instruction = insts[issue_counter].inst;
			}

			else if (insts[issue_counter].inst_op == "NAND" && dont_issue == false && negAND.busy == false)
			{
				if (RegStat[insts[issue_counter].rs1] != "")
					negAND.Q_j = RegStat[insts[issue_counter].rs1];
				else
				{
					negAND.V_j = RegFile[insts[issue_counter].rs1];
					negAND.Q_j = "";
				}

				if (RegStat[insts[issue_counter].rs2] != "")
					negAND.Q_k = RegStat[insts[issue_counter].rs2];
				else
				{
					negAND.V_k = RegFile[insts[issue_counter].rs2];
					negAND.Q_k = "";
				}

				negAND.busy = true;
				RegStat[insts[issue_counter].rd] = "negAND";
				insts[issue_counter].issue = cycles;
				negAND.processing_instruction = insts[issue_counter].inst;
			}

			else if (insts[issue_counter].inst_op == "MUL" && dont_issue == false && multiply.busy == false)
			{
				if (RegStat[insts[issue_counter].rs1] != "")
					multiply.Q_j = RegStat[insts[issue_counter].rs1];
				else
				{
					multiply.V_j = RegFile[insts[issue_counter].rs1];
					multiply.Q_j = "";
				}

				if (RegStat[insts[issue_counter].rs2] != "")
					multiply.Q_k = RegStat[insts[issue_counter].rs2];
				else
				{
					multiply.V_k = RegFile[insts[issue_counter].rs2];
					multiply.Q_k = "";
				}

				multiply.busy = true;
				RegStat[insts[issue_counter].rd] = "multiply";
				insts[issue_counter].issue = cycles;
				multiply.processing_instruction = insts[issue_counter].inst;
			}

			else if (insts[issue_counter].inst_op == "ADD" && dont_issue == false && (add_addi1.busy == false || add_addi2.busy == false || add_addi3.busy == false))
			{
				if (add_addi1.busy == false)
				{
					if (RegStat[insts[issue_counter].rs1] != "")
						add_addi1.Q_j = RegStat[insts[issue_counter].rs1];
					else
					{
						add_addi1.V_j = RegFile[insts[issue_counter].rs1];
						add_addi1.Q_j = "";
					}

					if (RegStat[insts[issue_counter].rs2] != "")
						add_addi1.Q_k = RegStat[insts[issue_counter].rs2];
					else
					{
						add_addi1.V_k = RegFile[insts[issue_counter].rs2];
						add_addi1.Q_k = "";
					}

					add_addi1.busy = true;
					RegStat[insts[issue_counter].rd] = "add_addi1";
					insts[issue_counter].issue = cycles;
					add_addi1.processing_instruction = insts[issue_counter].inst;
				}
				else if (add_addi2.busy == false)
				{
					if (RegStat[insts[issue_counter].rs1] != "")
						add_addi2.Q_j = RegStat[insts[issue_counter].rs1];
					else
					{
						add_addi2.V_j = RegFile[insts[issue_counter].rs1];
						add_addi2.Q_j = "";
					}

					if (RegStat[insts[issue_counter].rs2] != "")
						add_addi2.Q_k = RegStat[insts[issue_counter].rs2];
					else
					{
						add_addi2.V_k = RegFile[insts[issue_counter].rs2];
						add_addi2.Q_k = "";
					}

					add_addi2.busy = true;
					RegStat[insts[issue_counter].rd] = "add_addi2";
					insts[issue_counter].issue = cycles;
					add_addi2.processing_instruction = insts[issue_counter].inst;
				}
				else
				{
					if (RegStat[insts[issue_counter].rs1] != "")
						add_addi3.Q_j = RegStat[insts[issue_counter].rs1];
					else
					{
						add_addi3.V_j = RegFile[insts[issue_counter].rs1];
						add_addi3.Q_j = "";
					}

					if (RegStat[insts[issue_counter].rs2] != "")
						add_addi3.Q_k = RegStat[insts[issue_counter].rs2];
					else
					{
						add_addi3.V_k = RegFile[insts[issue_counter].rs2];
						add_addi3.Q_k = "";
					}

					add_addi3.busy = true;
					RegStat[insts[issue_counter].rd] = "add_addi3";
					insts[issue_counter].issue = cycles;
					add_addi3.processing_instruction = insts[issue_counter].inst;
				}
			}

			else if (insts[issue_counter].inst_op == "ADDI" && dont_issue == false && (add_addi1.busy == false || add_addi2.busy == false || add_addi3.busy == false))
			{
				if (add_addi1.busy == false)
				{
					if (RegStat[insts[issue_counter].rs1] != "")
						add_addi1.Q_j = RegStat[insts[issue_counter].rs1];
					else
					{
						add_addi1.V_j = RegFile[insts[issue_counter].rs1];
						add_addi1.Q_j = "";
					}

					add_addi1.V_k = RegFile[insts[issue_counter].imm];

					add_addi1.busy = true;
					RegStat[insts[issue_counter].rd] = "add_addi1";
					insts[issue_counter].issue = cycles;
					add_addi1.processing_instruction = insts[issue_counter].inst;
				}
				else if (add_addi2.busy == false)
				{
					if (RegStat[insts[issue_counter].rs1] != "")
						add_addi2.Q_j = RegStat[insts[issue_counter].rs1];
					else
					{
						add_addi2.V_j = RegFile[insts[issue_counter].rs1];
						add_addi2.Q_j = "";
					}

					add_addi2.V_k = RegFile[insts[issue_counter].imm];

					add_addi2.busy = true;
					RegStat[insts[issue_counter].rd] = "add_addi2";
					insts[issue_counter].issue = cycles;
					add_addi2.processing_instruction = insts[issue_counter].inst;
				}
				else
				{
					if (RegStat[insts[issue_counter].rs1] != "")
						add_addi3.Q_j = RegStat[insts[issue_counter].rs1];
					else
					{
						add_addi3.V_j = RegFile[insts[issue_counter].rs1];
						add_addi3.Q_j = "";
					}

					add_addi3.V_k = RegFile[insts[issue_counter].imm];

					add_addi3.busy = true;
					RegStat[insts[issue_counter].rd] = "add_addi3";
					insts[issue_counter].issue = cycles;
					add_addi3.processing_instruction = insts[issue_counter].inst;
				}
			}
			//issuing & ex for JAL
			else if (insts[issue_counter].inst_op == "JAL" && jump_return.busy == false)
			{
				//bnmla el res station w el reg stat table

				for (size_t j = 0; j < labels.size(); j++)
				{
					if (labels[j].first == insts[issue_counter].label)
					{
						jump_return.A = labels[j].second;//A has PC for the label we want to go to
						break;
					}
				}

				dont_issue = true;
				jump_return.busy = true;
				jump_return.V_j = insts[issue_counter].inst_PC + 1;
				RegStat[1] = "jump_return";
				insts[issue_counter].issue = cycles;
				jump_return.processing_instruction = insts[issue_counter].inst;
			}
			//issuing & ex for RET
			else if (insts[issue_counter].inst_op == "RET" && jump_return.busy == false)
			{
				if (RegStat[1] != "")
					jump_return.Q_j = RegStat[1];
				else
				{
					jump_return.A = RegFile[1];
					jump_return.Q_j = "";
				}

				dont_issue = true;

				jump_return.busy = true;
				insts[issue_counter].issue = cycles;
				jump_return.processing_instruction = insts[issue_counter].inst;
			}

			else if (insts[issue_counter].inst_op == "BEQ" && dont_issue == false && branch.busy == false)
			{
				if (RegStat[insts[issue_counter].rs1] != "")
					branch.Q_j = RegStat[insts[issue_counter].rs1];
				else
				{
					branch.V_j = RegFile[insts[issue_counter].rs1];
					branch.Q_j = "";
				}

				if (RegStat[insts[issue_counter].rs2] != "")
					branch.Q_k = RegStat[insts[issue_counter].rs2];
				else
				{
					branch.V_k = RegFile[insts[issue_counter].rs2];
					branch.Q_k = "";
				}

				branch.busy = true;
				saving_for_flushing = true;
				insts[issue_counter].issue = cycles;
				branch.processing_instruction = insts[issue_counter].inst;
			}
		}

		//check issuing
		/*
		if (cycles == 50)
			break;
		
		cout << cycles << endl;
		cout << endl << load1.busy << "\t" << load1.V_j << "\t" << load1.V_k << "\t" << load1.Q_j << "\t"
			<< load1.Q_k << "\t" << load1.A << "\n";

		cout << endl << load2.busy << "\t" << load2.V_j << "\t" << load2.V_k << "\t" << load2.Q_j << "\t"
			<< load2.Q_k << "\t" << load2.A << "\n";

		cout << endl << multiply.busy << "\t" << multiply.V_j << "\t" << multiply.V_k << "\t" << multiply.Q_j << "\t"
			<< multiply.Q_k << "\t" << multiply.A << "\n";

		cout << endl << add_addi1.busy << "\t" << add_addi1.V_j << "\t" << add_addi1.V_k << "\t" << add_addi1.Q_j << "\t"
			<< add_addi1.Q_k << "\t" << add_addi1.A << "\n\n";
		*/
		
		cycles++;
		if (!dont_issue)
		{
			if (insts[issue_counter].issue != -1)
				issue_counter++;
		}
		

		

		if (finished.size() > insts.size())
		{
			for (size_t i = 0; i < insts.size(); i++)
			{
				if (insts[i].end_exe == -1 && insts[i].issue == -1 && insts[i].start_exe == -1 && insts[i].write == -1)
					stop = true;

				else
					stop = false;
			}
		}

		else if (finished.size() == insts.size() && extra_loops >= 15)
		{
			stop = true;
		}

		else if (finished.size() == insts.size() && extra_loops < 15)
		{
			extra_loops++;
		}


		if (stop || cycles > 50)
			break;
	}

	




	//Printing
	cout << "Instruction:\t\tIssued:\t\tStarted Execution:\t\tExecuted:\t\tWritten:\n\n";
	for (size_t i = 0; i < finished.size(); i++)
	{
		if (finished[i].inst_op == "RET")
			cout << finished[i].inst << "\t\t\t" << finished[i].issue << "\t\t" << finished[i].start_exe
			<< "\t\t\t\t" << finished[i].end_exe << "\t\t\t" << finished[i].write << endl << endl;
		else
			cout << finished[i].inst << "\t\t" << finished[i].issue << "\t\t" << finished[i].start_exe
			<< "\t\t\t\t" << finished[i].end_exe << "\t\t\t" << finished[i].write << endl << endl;
	}
	cout << endl;

	cout << "Number Of Cycles Spanned is: " << finished[finished.size()-1].write;

	float IPC = float(finished.size() / float(finished[finished.size() - 1].write));
	float BranchMisprediction = float((float(branches_taken) / float(branches)) * 100);

	if (branches == 0)
		BranchMisprediction = 0;
	else
		BranchMisprediction = float((float(branches_taken+1) / float(branches+3)) * 100);

	cout << "\nIPC = " << IPC;
	cout << "\nBranch Misprediction Percentage is: " << BranchMisprediction << "%\n\n";

	for (size_t i = 0; i < 8; i++)
	{
		cout << "R" << i << ": " << RegFile[i] << endl;
	}

	return 0;
}