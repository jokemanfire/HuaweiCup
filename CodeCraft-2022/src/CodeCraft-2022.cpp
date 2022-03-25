#include<random>
#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<sstream>
#include<map>
#include<set>
#include<algorithm>
#include<numeric>
#include<unordered_map>
#include<thread>
#include<climits>
using namespace std;


//作品提交路径
string file_root = "/data/";
string file_Demand = "demand.csv";
string file_Bandwidth = "site_bandwidth.csv";
string file_qos = "qos.csv";
string file_config = "config.ini";
string file_output = "/output/solution.txt";

//一般数据格式内容
class DatasStruct {
public:
	vector<string> headName;
	vector<vector< string>> Datas;
};

vector<vector<bool>>Node_User;

class UserManage
{
private:
	//所有用户名
	vector<string> Usernames;
	//不同时刻用户所需带宽
	vector<unordered_map<string, int>> WidthNeed;

public:
	vector<string>& Get_usersnames() { return Usernames; };
	void SetWidth(int time, int width, string name) { WidthNeed.resize(time + 1); WidthNeed[time][name] = width; }
	int GetWidth(int time, string name) {
		return WidthNeed[time][name];
	}
	unordered_map<string, int> GetWidths(int time) { return WidthNeed[time]; };
	void PushName(string name) { Usernames.push_back(name); };

};

class NodeManage
{
private:
	//所有用户节点
	vector<string> Nodenames;
	//存储目标节点下的可用用户名称
	//unordered_map<string, vector<bool>> UsefulUser_flag;
	//存储node的带宽
	unordered_map<string, int> Node_Width;


public:
	vector<string>& Get_Nodenames() { return Nodenames; };
	void SetWidth(string name, int width) { Node_Width[name] = width; };
	int GetWidth(string name) {
		return Node_Width[name];
	}
	void PushName(string name) { Nodenames.push_back(name); };

	/*void AddNode_Usefuluser(string nodename, bool flag) { UsefulUser_flag[nodename].push_back(flag); };
	vector<bool>& Get_UsefulUser(string nodename) { return UsefulUser_flag[nodename]; };*/
};



//字符串切割方法
std::vector<std::string> stringSplit(const std::string& str, char delim) {
	std::stringstream ss(str);
	std::string item;
	std::vector<std::string> elems;
	while (std::getline(ss, item, delim)) {
		if (!item.empty()) {
			elems.push_back(item);
		}
	}
	return elems;
}

//去除换行符
void strim(string& str)
{
	int Pos = str.size() - 1;
	if (str[Pos] == '\n' || str[Pos] == '\r')
	{
		str.erase(str.begin() + Pos);
	}
}

//获取结构体数据
DatasStruct GetData(string filepath)
{
	DatasStruct tempData;
	ifstream infile(filepath);
	vector<string> data;
	bool Head = true;
	while (infile.good())
	{

		string value;
		getline(infile, value);
		data = stringSplit(value, ',');
		if (Head)
		{
			strim(data[data.size() - 1]);
			tempData.headName = data;
			Head = false;
		}
		else
		{
			if (data.size() != 0)
				tempData.Datas.push_back(data);
		}

		//cout << value << endl;
	}
	return tempData;
}



//读取ini文件中的配置
int ReadQos(string filepath)
{
	DatasStruct iniData = GetData(filepath);
	string Qos = iniData.Datas[0][0];
	int position = Qos.find("=");
	string Result;
	if (position != Qos.npos)
	{
		Result = Qos.substr(position + 1, Qos.size());
	}
	return atoi(Result.c_str());
}

//针对一个用户的分配
void NodeAssign(vector<vector<int>>& allc, int user, int node)
{
	int j = 1;
	while (j < allc[0].size())
	{

		//如果不能分配
		if (allc[user][j] == -1)
		{
			j++;
			continue;
		}
		else
		{
			//节点可分配为0
			if (allc[0][j] == 0)
			{
				j++;
				continue;
			}
			int Width_use = 0;
			//用户需求小于等于可分配
			if (allc[user][0] <= allc[0][j])
			{
				Width_use = allc[user][0];

			}
			//用户需求大于可分配
			else
			{
				Width_use = allc[0][j];
			}
			allc[user][j] += Width_use;
			allc[0][j] -= Width_use;
			allc[user][0] -= Width_use;
		}
		j++;
	}
	return;
}
//重新分配
int  Reset(vector<vector<int>>& allc, int user, int node)
{
	//减少分配
	int Rest_width = allc[user][0];
	int i = node;

	int Max_Count = allc.size();
	int Count = 1;
	while (Count < Max_Count)
	{
		//如果可分配
		if (allc[Count][node] != -1 && allc[Count][node] != 0)
		{
			int Width_use = Rest_width;
			allc[Count][node] -= Width_use;
			allc[Count][0] += Width_use;
			allc[0][node] += Width_use;
			//Rest_width -= Width_use;
		}
		/*	if (Rest_width == 0)
				break;*/
		Count++;
	}
	return Rest_width;
}

void AverageChoose(vector<vector<int>>& allc, int user, int node)
{
	if (user < allc.size())
	{
		NodeAssign(allc, user, node);
	}
	else
		return;
	//如果没有分配完
	if (allc[user][0] != 0)
	{	
		//Reset后重新分配 前面全部重分
		int result = Reset(allc, user, node);			
		//对该用户分配
		AverageChoose(allc, user, node+1);
		return;
		
	}
	//分配完继续下一个
	else
	{
		AverageChoose(allc, user + 1, 1);
		return;
	}
}
//是否为有效解
int IsEffect(vector<vector<int>>& CountJu)
{
	for (int i = 0; i < CountJu.size(); i++)
	{
		if (CountJu[i][0] != 0)
			return i;
	}
	return -1;
}

//把分配节点的交集去掉
int  Rellcumer(int node , int cost ,vector<int>&UserMax,vector<int>& UserNeed, vector<vector<int>>& UserCost)
{
	//vector<int>& UserNeedTemp = UserNeed;
	////寻找使用该节点的用户
	//for (int user = 0; user < Node_User[0].size(); user++)
	//{
	//	//如果使用了该节点 从交集中剔除
	//	if (Node_User[node][user] == true)
	//	{
	//		int Width_cs=0;
	//		if (UserNeedTemp[user] <= cost)
	//			Width_cs = UserNeedTemp[user];
	//		else
	//			Width_cs = cost;
	//		cost -= Width_cs;
	//		UserNeedTemp[user] -= Width_cs;
	//		//从交集剔除
	//		for (int j = 0; j < Node_User.size(); j++)
	//		{
	//			if (Node_User[j][user] == true)
	//				UserMax[j] -= Width_cs;
	//		}
	//	}
	//}
	//针对该节点对用户进行分配
	for (int user = 0; user < UserNeed.size(); user++)
	{
		//如果使用了该节点
		if (Node_User[node][user] == true)
		{
			int Width_cs = 0;
			if (UserNeed[user] <= cost)
				Width_cs = UserNeed[user];
			else
				Width_cs = cost;
			cost -= Width_cs;
			UserNeed[user] -= Width_cs;
			UserCost[user][node] += Width_cs;
			//从交集剔除
			for (int j = 0; j < Node_User.size(); j++)
			{
				if (Node_User[j][user] == true)
					UserMax[j] -= Width_cs;
			}
		}
	}
	if (cost != 0)
		cout << endl;
	return accumulate(UserNeed.begin(), UserNeed.end(), 0);

	//return;
}

void UserCustomer(vector<int>& Node_Useful, vector<int>& Userneeded, NodeManage* Nm, UserManage* Um, ofstream& outfile);
//1、不同时刻用户需求带宽  2.不同时刻下节点使用带宽 3、不同时刻下 的不同节点最大可用带宽 4、不同时刻下节点剩余带宽

void Diaodu(vector<int> & Time_UserNeed, vector<vector<int>> & Time_NodeUsed,vector<vector<int>> &Time_UserMax,vector<vector<int>> & Time_NodeRest,UserManage * Um,NodeManage * Nm, ofstream& outfile)
{
	
	vector<string> Usernames = Um->Get_usersnames();
	vector<string> Nodenames = Nm->Get_Nodenames();
	//每次分配尽量去选择没被分配的节点
	//尽量做到负载均衡
	vector<int> Nodes_index;
	for (int i = 0; i < Time_NodeUsed[0].size(); i++)
		Nodes_index.push_back(i);

	int rand_point = Nodes_index.size()-1 ;
	for (int time = 0; time < Time_UserNeed.size(); time++)
	{
		string result;
		vector<int> User_Needed;
		//获取该时刻下用户需求
		for (int i = 0; i < Usernames.size(); i++)
			User_Needed.push_back(Um->GetWidth(time, Usernames[i]));
		vector<int> Const_User_Needed = User_Needed;
		//获得用户分配
		vector<vector<int>> CountJu(Usernames.size() , vector<int>(Nodenames.size()));
		while (Time_UserNeed[time] != 0)
		{
			if (rand_point >=0)
			{
				//随机选择一名幸运节点
				int choose = Nodes_index[rand() % (rand_point+1)];
				if (choose != rand_point)
				{
					int temp = Nodes_index[rand_point];
					Nodes_index[rand_point] = Nodes_index[choose];
					Nodes_index[choose] = temp;
				}
				rand_point--;
				//如果取了该节点 意味着 在该节点交集下的用户需求断开
				 
				
				int cost_width = 0;
				//如果用户需求带宽大于可用带宽
				if (Time_UserNeed[time] > Time_UserMax[time][choose])
				{
					
					//如果用户需求大于剩余带宽
					if (Time_UserNeed[time] > Time_NodeRest[time][choose])
					{
						cost_width = Time_NodeRest[time][choose]> Time_UserMax[time][choose]? Time_UserMax[time][choose]: Time_NodeRest[time][choose];
					}
					//如果用户带宽小于剩余带宽
					else{
						cost_width = Time_UserMax[time][choose];
					}
					//将使用该节点的用户需求减少
					Time_UserNeed[time] = Rellcumer(choose,cost_width,Time_UserMax[time],User_Needed, CountJu);
					//Time_UserNeed[time] -= cost_width;
					Time_NodeRest[time][choose] -= cost_width;
					//Time_NodeUsed[time][choose] += Nm->GetWidth(Nm->Get_Nodenames()[choose]);
				}
				//如果用户需求带宽小于等于可用带宽
				else
				{
					//如果用户需求大于剩余带宽
					if (Time_UserNeed[time] > Time_NodeRest[time][choose])
					{
						cost_width = Time_NodeRest[time][choose];
					}
					//如果用户带宽小于剩余带宽
					else{
						cost_width = Time_UserNeed[time];
					}
					//将使用该节点的用户需求减少
					Time_UserNeed[time] = Rellcumer(choose, cost_width, Time_UserMax[time], User_Needed,CountJu);
					//Time_UserNeed[time] -= cost_width;
					Time_NodeRest[time][choose] -= cost_width;
					//Time_NodeUsed[time][choose] += Nm->GetWidth(Nm->Get_Nodenames()[choose]);
				}
	
			}
			//随机节点使用完毕 重新初始化
			else
			{
				rand_point = Nodes_index.size() - 1;
			}
		}
		//将结果输出
	/*	int a = accumulate(Time_NodeUsed[time].begin(), Time_NodeUsed[time].end(), 0);
		int b = accumulate(Const_User_Needed.begin(), Const_User_Needed.end(), 0);
		UserCustomer(Time_NodeUsed[time], Const_User_Needed, Nm, Um, outfile);
		std::cout << time<<endl;*/
		//结果输出
		for (int user = 0; user < CountJu.size(); user++)
		{

			string username = Usernames[user];
			result = result + Usernames[user] + ":";
			bool Cath = false;
			for (int node = 0; node < CountJu[user].size(); node++)
			{
				if (CountJu[user][node] == 0 || CountJu[user][node] == -1)
					continue;
				else
				{
					result = result + "<" + Nodenames[node] + "," + to_string(CountJu[user][node]) + ">" + ",";
					Cath = true;
				}

			}
			if (Cath)
				result.pop_back();
			result += "\n";
		}
		outfile << result;
		
	}
	return;
}

//将结果输出 节点可用的大小
void UserCustomer(vector<int> &Node_Useful,vector<int> &Userneeded, NodeManage * Nm, UserManage * Um, ofstream & outfile)
{
	vector<string> Usernames = Um->Get_usersnames();
	vector<string> Nodenames = Nm->Get_Nodenames();
	string result;

	//用回溯法计算用户分配

	vector<vector<int>> CountJu(Usernames.size() + 1, vector<int>(Nodenames.size() + 1));
	//行头 用户所需带宽
	//列头 Node所拥有的带宽

	//初始化矩阵分配数值 不能分配为-1
	for (int i = 0; i < Node_User.size(); i++)
	{
		for (int j = 0; j < Node_User[i].size(); j++)
		{
			if (Node_User[i][j])
				CountJu[j + 1][i + 1] = 0;
			else //不能分配节点置为-1
				CountJu[j + 1][i + 1] = -1;
		}
	}

	//获取当前时刻下的映射
	for (int k = 1; k < Usernames.size() + 1; k++)
	{
		//列头
		CountJu[k][0] = Userneeded[k-1];
	}
	for (int j = 1; j < Nodenames.size() + 1; j++)
	{
		//行头
		CountJu[0][j] = Node_Useful[j-1];
	}
	int Reuslt = 1;
	while (Reuslt != -1)
	{
		AverageChoose(CountJu, Reuslt, 1);
		Reuslt = IsEffect(CountJu);
	}

	//结果输出
	for (int user = 1; user < CountJu.size(); user++)
	{	
	
		string username = Usernames[user-1];
		result = result +Usernames[user-1] + ":";
		bool Cath=false;
		for (int node = 1; node < CountJu[user].size(); node++)
		{
			if (CountJu[user][node] == 0 || CountJu[user][node] ==-1)
				continue;
			else
			{
				result = result + "<" + Nodenames[node-1] + "," + to_string(CountJu[user][node]) + ">" + ",";
				Cath = true;
			}

		}
		if (Cath)
			result.pop_back();
		result += "\n";
	}
	outfile << result;
	return;
}

int main()
{
	srand(time(NULL));
	//获取客户宽带需求
	DatasStruct UserWidths = GetData(file_root + file_Demand);

	//获取边缘节点带宽数
	DatasStruct NodeWidths = GetData(file_root + file_Bandwidth);

	//获取客户与边缘节点网络时延
	DatasStruct PeopleQos = GetData(file_root + file_qos);

	//获取config
	int qos = ReadQos(file_root + file_config);

	UserManage* Um = new UserManage;
	NodeManage* Nm = new NodeManage;


	//获得需要调度的最大时刻数
	int Max_times = UserWidths.Datas.size();


	//初始化用户 
	for (int i = 1; i < PeopleQos.headName.size(); i++)
	{

		string username = PeopleQos.headName[i];
		Um->PushName(username);
	}

	//初始化node
	for (int i = 0; i < NodeWidths.Datas.size(); i++)
	{
		string Nodename = NodeWidths.Datas[i][0];
		Nm->PushName(Nodename);
		int NodeWidth = atoi(NodeWidths.Datas[i][1].c_str());
		Nm->SetWidth(Nodename, NodeWidth);
	}

	Node_User.resize(PeopleQos.Datas.size());
	//初始化Node可用用户
	for (int i = 0; i < PeopleQos.Datas.size(); i++)
	{
		Node_User[i].resize(PeopleQos.Datas[i].size()-1);
		for (int j = 1; j < PeopleQos.Datas[i].size(); j++)
		{
			//延迟小于阈值
			int this_Qos = atoi(PeopleQos.Datas[i][j].c_str());
			string Nodename = PeopleQos.Datas[i][0];
			string Username = PeopleQos.Datas[0][j];
			if (this_Qos < qos)
			{
				Node_User[i][j - 1] = true;
			}
			else
			{
				Node_User[i][j - 1] = false;
			}
		}
	}


	//初始化用户不同时刻所需带宽
	for (int time = 0; time < UserWidths.Datas.size(); time++)
	{
		for (int j = 1; j < UserWidths.Datas[time].size(); j++)
		{
			string Username = UserWidths.headName[j];
			//获得该时刻下用户
			int width = atoi(UserWidths.Datas[time][j].c_str());
			//设置该时刻下用户所需带宽
			Um->SetWidth(time, width, Username);
		}
	}
	//输出结果
	ofstream outfile(file_output);



	vector<vector<int>> Time_node;
	Time_node.resize(Max_times);
	for (int i = 0; i < Time_node.size(); i++)
		Time_node[i].resize(Nm->Get_Nodenames().size());

	vector<string>NodeNames = Nm->Get_Nodenames();
	vector<string>Usernames = Um->Get_usersnames();
	//获得不同时刻的用户总需求带宽
	vector<int> Time_UserNeed;
	for (int time = 0; time < Max_times; time++)
	{
		int NeedWidth = 0;
		for (int user = 0; user < Usernames.size(); user++)
		{
			NeedWidth += Um->GetWidth(time, Usernames[user]);
		}
		Time_UserNeed.push_back(NeedWidth );
	}


	//获得不同时刻节点的最大带宽
	for (int time = 0; time < Time_node.size(); time++)
	{
		for (int node = 0; node < Nm->Get_Nodenames().size();node++)
		{
			vector<bool> Users = Node_User[node];
			int Node_NeedWidth = 0;
			for (int k = 0; k < Users.size(); k++)
			{
				if (Users[k])
					Node_NeedWidth += Um->GetWidth(time, Usernames[k]);
			}
			Time_node[time][node] = Node_NeedWidth;
		}

	}
	//初始化 节点的剩余带宽
	vector<vector<int>> Time_nodeRest;
	Time_nodeRest.resize(Max_times);
	for (int i = 0; i < Time_nodeRest.size(); i++)
		for(int node =0;node<NodeNames.size();node++)
			Time_nodeRest[i].push_back(Nm->GetWidth(NodeNames[node]));

	//不同时刻下 节点使用带宽
	vector<vector<int>> Time_NodeUsed;
	Time_NodeUsed.resize(Max_times);
	for (int i = 0; i < Time_NodeUsed.size(); i++)
		Time_NodeUsed[i].resize(NodeNames.size());

	//节点调度算法
	Diaodu(Time_UserNeed, Time_NodeUsed, Time_node, Time_nodeRest,Um,Nm,outfile);


	return 0;
}