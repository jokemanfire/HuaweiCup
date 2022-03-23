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

int ALLMax_times = 1;
void InitAverCount(vector<vector<int>>& CountJu);

//一般数据格式内容
class DatasStruct {
public:
	vector<string> headName;
	vector<vector< string>> Datas;
};
//用户类
class User
{

private:
	//用户的需求带宽
	int Need_width;
	//用户可用节点数（可变）
	int Usefulsize;
public:
	//获得用户当前时刻下带宽需求
	int GetNeed_width() { return Need_width; };
	//设置用户当前时刻下带宽需求
	void SetNeed_width(int width) { Need_width = width; };
};


//节点类
class Node
{
private:
	//当前最大带宽
	int MaxWidth;

public:
	//设置最大带宽 
	void SetMaxWidth(int width) { MaxWidth = width; };
	//获得当前带宽
	int GetWidth() { return MaxWidth; };
	//修改当前可用带宽
	void ADDWidth(int width) { MaxWidth -= width; };

};

struct MinHeapCmp
{
	inline bool operator()(map<int, int>& x, map<int, int>& y)
	{
		return x.begin()->second > y.begin()->second;
	}
};

class MyHeap
{
private:
	vector<map<int, int>> Indexs;
	int max_size = 5;
	map<int, int> Drop_index;
public:
	MyHeap() { make_heap(Indexs.begin(), Indexs.end(), MinHeapCmp()); };
	bool Pushdata(map<int, int> data);
	map<int, int> GetDrop_index() { return Drop_index; };
	vector<map<int, int>>& GetIndex() {
		return Indexs;
	};
	void SetMax_size(int size) { max_size = size; };
};

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
	void PushName(string name) { Usernames.push_back(name); };
};

class NodeManage
{
private:
	//所有用户节点
	vector<string> Nodenames;
	//存储目标节点下的可用用户名称
	unordered_map<string, vector<bool>> UsefulUser_flag;
	//存储node的带宽
	unordered_map<string, int> Node_Width;
	//存储时刻下最大使用节点
	vector<vector<int>>Time_node;
public:
	vector<string>& Get_Nodenames() { return Nodenames; };
	void AddNode_Usefuluser(string nodename, bool flag) { UsefulUser_flag[nodename].push_back(flag); };
	vector<bool>& Get_UsefulUser(string nodename) { return UsefulUser_flag[nodename]; };
	void SetWidth(string name, int width) { Node_Width[name] = width; };
	int GetWidth(string name) {
		return Node_Width[name];
	}
	void PushName(string name) { Nodenames.push_back(name); };
	vector<vector<int>>& GetTimeNode() {
		return Time_node;
	};
	//存储优先分配节点次数
	vector<int> Node_count;
	//存储分配次数
	vector<float> Node_float;
	vector<vector<int>>Cost_time;
	vector<MyHeap*>Node_heap;
	vector < vector < bool>> UnconNodes;
};



bool MyHeap::Pushdata(map<int, int> data)
{
	if (Indexs.size() == max_size)
	{
		/*sort_heap(Indexs.begin(), Indexs.end(), MinHeapCmp());*/
		map<int, int> First = Indexs[0];
		//如果传入数据比堆数据小
		if (data.begin()->second <= First.begin()->second)
		{
			Drop_index = data;
			//make_heap(Indexs.begin(), Indexs.end());
			return false;
		}
		else
		{
			pop_heap(Indexs.begin(), Indexs.end());
			map<int, int> temp = Indexs.back();
			Indexs.pop_back();
			Indexs.push_back(data);
			push_heap(Indexs.begin(), Indexs.end(), MinHeapCmp());
			Drop_index = temp;
			//make_heap(Indexs.begin(), Indexs.end());
			return false;
		}
	}
	else
	{
		//如果堆的大小比最大小 直接放入
		if (Indexs.size() < max_size)
		{
			Indexs.push_back(data);
			push_heap(Indexs.begin(), Indexs.end(), MinHeapCmp());
		}
		return true;

	}
}

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

//超级贪心算法 user对目标用户分配，node 对目标节点分配
int ChooseNode(vector<vector<int>>& allc,int user, vector<bool> &UnconNode)
{
	vector<int> allNodecount(allc[0].size()-1, 0);
	int index = -1;
	for (int i = 1; i < allc[0].size(); i++)
	{
		int NodeWidt = 0;
		if (allc[user][i] == -1 || allc[0][i] == 0)
		{
			allNodecount[i - 1] = -1;
			continue;
		}
			
		for (int j = 1; j < allc.size(); j++)
		{
			int this_need = allc[j][0];
			if(this_need != -1  )
			NodeWidt += this_need;
		}
		allNodecount[i - 1] = abs(allc[0][i] - NodeWidt);
	}

	//获取最小坐标
	for (int i = 0; i < allNodecount.size(); i++)
	{
		if (UnconNode[i] == true || allNodecount[i] == -1)
			continue;
		if (index != -1 && allNodecount[i] < allNodecount[index])
			index = i;
		else if (index == -1)
			index = i;
	}
	//如果实在没有选择
	if (index == -1)
		return -1;
	return index + 1;
}

//针对一个用户的分配
void NodeAssign(vector<vector<int>>& allc, int user, int node)
{
	int j = node;
	// if(av)
	// 	j = rand()% (allc[user].size()-1)+1;
	int Max_Count = allc[user].size();
	int Count = 0;
	while (Count < Max_Count)
	{
		//每次选择占比最大节点
		int s;
		s = (j + Count) % ((allc[user].size() - 1) + 1);
		//如果不能分配
		if (allc[user][s] == -1)
		{
			Count++;
			continue;

		}
		else
		{
			//节点可分配为0
			if (allc[0][s] == 0)
			{
				Count++;
				continue;
			}

			//用户需求小于等于可分配
			if (allc[user][0] <= allc[0][s])
			{
				allc[user][s] += allc[user][0];
				allc[0][s] -= allc[user][0];
				allc[user][0] -= allc[user][0];
			}
			//用户需求大于可分配
			else
			{
				//该分配所有带宽
				//更新所分配位置
				allc[user][s] += allc[0][s];
				int temp = allc[user][0];
				//更新用户需求
				allc[user][0] -= allc[0][s];
				//更新节点带宽
				allc[0][s] -= allc[0][s];
			}
		}
		Count++;
	}
}
//重新分配
void Reset(vector<vector<int>>& allc, int user, int node)
{
	//减少分配
	int Rest_width = allc[user][0];
	int i = node;
	// if(av)
	// 	i = rand()% (user-1)+1;
	int Max_Count = user;
	int Count = 1;
	while (Count < Max_Count)
	{
		//如果可分配
		if (allc[Count][node] != -1)
		{
			//如果已分配大于需求
			if (allc[Count][node] >= Rest_width)
			{
				allc[Count][node] -= Rest_width;
				allc[Count][0] += Rest_width;
				allc[0][node] += Rest_width;
				Rest_width -= Rest_width;

			}
			//如果已分配小于需求
			else
			{
				Rest_width -= allc[Count][node];
				allc[0][node] += allc[Count][node];
				allc[Count][node] -= allc[Count][node];
			}
		}
		Count++;
	}
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
		for (int j = 1; j < allc[user].size(); j++)
		{
			if (allc[user][j] >= 0)
			{
				//Reset后重新分配
				Reset(allc, user, j);
				//对该用户分配
				AverageChoose(allc, user, j);
			}
		}
	

		return;
	}
	//分配完继续下一个
	else
	{
		AverageChoose(allc, user + 1, 1);
		return;
	}
}

int IsEffect(vector<vector<int>>& CountJu)
{
	for (int i = 0; i < CountJu.size(); i++)
	{
		if (CountJu[i][0] != 0)
			return i;
	}
	return -1;
}

void InitAverCount(vector<vector<int>>& CountJu)
{
	for (int i = 1; i < CountJu.size(); i++)
	{
		int SumCount = 0;
		for (int j = 1; j < CountJu[i].size(); j++)
		{
			if (CountJu[i][j] == 0 && CountJu[0][j] > 0)
				SumCount++;
		}
		if (SumCount == 0)
			return;
		int AvergCount = CountJu[i][0] / SumCount;
		for (int j = 1; j < CountJu[i].size(); j++)
		{
			if (CountJu[0][j] > AvergCount && CountJu[i][j] == 0)
			{
				CountJu[0][j] -= AvergCount;
				CountJu[i][j] += AvergCount;
				CountJu[i][0] -= AvergCount;
			}
		}
	}
}

void InitMaxData(vector<vector<int>>& CountJu, int node)
{
	for (int i = 1; i < CountJu.size(); i++)
	{
		//如果节点可分配带宽大于0 && 节点为联通
		if (CountJu[0][node] > 0 && CountJu[i][node] != -1)
		{

			//如果能全分配
			if (CountJu[0][node] >= CountJu[i][0])
			{
				CountJu[0][node] -= CountJu[i][0];
				CountJu[i][node] += CountJu[i][0];
				CountJu[i][0] -= CountJu[i][0];
			}
			//不能全分配
			else
			{
				CountJu[i][0] -= CountJu[0][node]; //该用户
				CountJu[i][node] += CountJu[0][node]; //该用户的该节点
				CountJu[0][node] -= CountJu[0][node]; //该节点
			}
		}
	}
}

//对节点最大值初始化
bool InitMaxCount(vector<vector<int>>& CountJu, vector<bool>& UnconNode)
{
	for (int i = 1; i < CountJu.size(); i++)
	{
		int node = ChooseNode(CountJu, i, UnconNode);
		//该用户没有适配节点跳过
		if (node == -1)
			continue;
		//如果节点可分配带宽大于0 && 节点为联通
		if (CountJu[0][node] > 0 && CountJu[i][node] != -1)
		{

			//如果能全分配
			if (CountJu[0][node] >= CountJu[i][0])
			{
				CountJu[0][node] -= CountJu[i][0];
				CountJu[i][node] += CountJu[i][0];
				CountJu[i][0] -= CountJu[i][0];
			}
			//不能全分配
			else
			{
				CountJu[i][0] -= CountJu[0][node]; //该用户
				CountJu[i][node] += CountJu[0][node]; //该用户的该节点
				CountJu[0][node] -= CountJu[0][node]; //该节点
			}
		}
	}
	return true;
}

//时间节点上的分配 
void DealOneAlg(int Min_time, int Max_time, UserManage* Um, NodeManage* Nm, bool av, ofstream& outfile, int node_number = 1)
{
	//初始化计算矩阵
	vector<string>Usernames = Um->Get_usersnames();
	vector<string>Nodenames = Nm->Get_Nodenames();
	vector<vector<int>> CountJu(Usernames.size() + 1, vector<int>(Nodenames.size() + 1));


	//初始化矩阵分配数值 不能分配为-1
	for (int i = 0; i < Nodenames.size(); i++)
	{
		vector<bool> flags = Nm->Get_UsefulUser(Nodenames[i]);
		for (int j = 0; j < flags.size(); j++)
		{
			if (flags[j])
				CountJu[j + 1][i + 1] = 0;
			else //不能分配节点置为-1
				CountJu[j + 1][i + 1] = -1;
		}
	}

	for (int i = Min_time; i < Max_time; i++)
	{
		vector<vector<int>> CountJuTemp = CountJu;
		//获取当前时刻下的映射

		for (int k = 1; k < Usernames.size() + 1; k++)
		{
			//列头
			CountJuTemp[k][0] = Um->GetWidth(i, Usernames[k - 1]);
		}
		for (int j = 1; j < Nodenames.size() + 1; j++)
		{
			//行头
			CountJuTemp[0][j] = Nm->GetWidth(Nodenames[j - 1]);
		}
		if (av)
		{
			InitAverCount(CountJuTemp);
		}
		else
			InitMaxData(CountJuTemp, node_number);
		int Reuslt = 1;
		while (Reuslt != -1)
		{
			AverageChoose(CountJuTemp, Reuslt, node_number);
			Reuslt = IsEffect(CountJuTemp);
		}
		//获得分配结果
		for (int i = 1; i < CountJuTemp.size(); i++)
		{
			outfile << Usernames[i - 1] << ":";
			string tempdata;
			for (int j = 1; j < CountJuTemp[i].size(); j++)
				if (CountJuTemp[i][j] > 0)
				{
					tempdata = tempdata + "<" + Nodenames[j - 1] + "," + to_string(CountJuTemp[i][j]) + ">" + ",";
				}
			if (tempdata.size() > 0)
				tempdata.pop_back();
			outfile << tempdata;
			outfile << endl;
		}
	}
	return;
}

int GetNode(vector<vector<int>>& allc, int user, vector<float>& NodeCount)
{
	vector<int> allNodecount(allc[0].size() - 1, 0);
	int index = 0;
	for (int i = 1; i < allc[0].size(); i++)
	{
		int NodeWidt = 0;
		if (allc[user][i] == -1 || allc[0][i] == 0)
		{
			allNodecount[i - 1] = -1;
			continue;
		}

		for (int j = 1; j < allc.size(); j++)
		{
			int this_need = allc[j][0];
			if (this_need != -1)
				NodeWidt += this_need;
		}
		allNodecount[i - 1] = abs(allc[0][i] - NodeWidt);
		index = i - 1;
	}

	//获取最小坐标
	for (int i = 0; i < allNodecount.size(); i++)
	{

		if (allNodecount[i] >= 0 && allNodecount[i] < allNodecount[index])
			index = i;
	}
	return index + 1;
}

void InitMax(int time, UserManage* Um, NodeManage* Nm)
{
	
	vector<string>Usernames = Um->Get_usersnames();
	vector<string>Nodenames = Nm->Get_Nodenames();
	vector<vector<int>> CountJu(Usernames.size() + 1, vector<int>(Nodenames.size() + 1));
	//行头 用户所需带宽
	//列头 Node所拥有的带宽



	//初始化矩阵分配数值 不能分配为-1
	for (int i = 0; i < Nodenames.size(); i++)
	{
		vector<bool> flags = Nm->Get_UsefulUser(Nodenames[i]);
		for (int j = 0; j < flags.size(); j++)
		{
			if (flags[j])
				CountJu[j + 1][i + 1] = 0;
			else //不能分配节点置为-1
				CountJu[j + 1][i + 1] = -1;
		}
	}



	//获取当前时刻下的映射

	for (int k = 1; k < Usernames.size() + 1; k++)
	{
		//列头
		CountJu[k][0] = Um->GetWidth(time, Usernames[k - 1]);
	}
	for (int j = 1; j < Nodenames.size() + 1; j++)
	{
		//行头
		CountJu[0][j] = Nm->GetWidth(Nodenames[j - 1]);
	}
	
	int Reuslt = 1;
	/*while (Reuslt != -1)
	{
		AverageChoose(CountJu, Reuslt, 1);
		Reuslt = IsEffect(CountJu);
	}*/
	InitMaxCount(CountJu, Nm->UnconNodes[time]);
	
	//取消耗最大
	
	vector<int> Customers;
	Customers.resize(CountJu[0].size() - 1);
	for (int i = 1; i < CountJu[0].size(); i++)
	{
		int UsedWidth = Nm->GetWidth(Nodenames[i - 1]) - CountJu[0][i];
		Customers[i - 1] = UsedWidth;
	}
	vector<int>::iterator max_index =max_element(Customers.begin(), Customers.end());

	int Max_index =  std::distance(Customers.begin(), max_index);
	if (Customers[Max_index] != 0)
	{
		map<int, int> temp;
		temp[time] = Customers[Max_index];
		bool status = Nm->Node_heap[Max_index]->Pushdata(temp);
		//没能放入堆里 
		if (!status)
		{

			//针对该时间节点重新分配
			map<int, int> tm = Nm->Node_heap[Max_index]->GetDrop_index();
			//cout << "time " << tm.begin()->first << " 节点" << Nodenames[Max_index] << endl;
			Nm->UnconNodes[tm.begin()->first][Max_index] = true;
			InitMax(tm.begin()->first, Um, Nm);

		}
	}
	//全局最大 时间复杂度太高
	//for (int i = 1; i < CountJu[0].size(); i++)
	//{

	//	int UsedWidth = Nm->GetWidth(Nodenames[i - 1]) - CountJu[0][i];
	//	if (UsedWidth != 0)
	//	{
	//		map<int, int> temp;
	//		temp[time] = UsedWidth;
	//		bool status = Nm->Node_heap[i - 1]->Pushdata(temp);
	//		//没能放入堆里 
	//		if (!status)
	//		{
	//			//针对该时间节点重新分配
	//			map<int, int> tm = Nm->Node_heap[i - 1]->GetDrop_index();
	//			Nm->UnconNodes[tm.begin()->first][i-1] = true;
	//			InitMax(tm.begin()->first, Um, Nm);				
	//		}
	//	}
	//}
	

	return;
}

int main()
{
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
	ALLMax_times = Max_times;

	//初始化Node可用用户
	for (int i = 0; i < PeopleQos.Datas.size(); i++)
	{
		for (int j = 1; j < PeopleQos.Datas[i].size(); j++)
		{
			//延迟小于阈值
			int this_Qos = atoi(PeopleQos.Datas[i][j].c_str());
			string Nodename = PeopleQos.Datas[i][0];
			if (this_Qos < qos)
			{
				Nm->AddNode_Usefuluser(Nodename, true);
			}
			else
				Nm->AddNode_Usefuluser(Nodename, false);
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

	vector<vector<int>>& Time_node = Nm->GetTimeNode();
	Time_node.resize(Max_times);


	ofstream outfile(file_output);
	Nm->Cost_time.resize(Nm->Get_Nodenames().size());
	for (int i = 0; i < Nm->Cost_time.size(); i++)
		Nm->Cost_time[i].resize(Max_times);
	


	//求所有节点最爽的时间点
	for (int i = 0; i < Nm->Get_Nodenames().size(); i++)
	{
		MyHeap * new_heap = new MyHeap;
		int threld = Max_times * 0.05;
		if ((Max_times * 100) % 5 == 0)
			threld -= 1;
		new_heap->SetMax_size(threld);
		Nm->Node_heap.push_back(new_heap);
	}

	

	//获得时间下最适配带宽
	Nm->UnconNodes.resize(Max_times);
	for (int i = 0; i < Max_times; i++)
	{
		Nm->UnconNodes[i].resize(Nm->Get_Nodenames().size());
	}
	for (int time = 0; time < Max_times; time++)
	{
		InitMax(time, Um, Nm);
	
	}
	map<int, int> TimeNode;
	for (int i = 0; i < Nm->Node_heap.size(); i++)
	{
		vector<map<int, int>>& temp = Nm->Node_heap[i]->GetIndex();
		for (int j = 0; j < temp.size(); j++)
		{
			int time = temp[j].begin()->first;
			TimeNode[time] = i;
		}
		
	}

	for (int time = 0; time < Max_times; time++)
	{
		int node = 0;
		if (TimeNode.count(time) != 0)
		{
			node = TimeNode[time];
			DealOneAlg(time, time + 1, Um, Nm, false, outfile, node + 1);
		}
		else
			DealOneAlg(time, time + 1, Um, Nm, true, outfile, node + 1);
	}



	return 0;
}