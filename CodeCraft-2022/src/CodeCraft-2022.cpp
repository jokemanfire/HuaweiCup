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
using namespace std;


//��Ʒ�ύ·��
string file_root = "/data/";
string file_Demand = "demand.csv";
string file_Bandwidth = "site_bandwidth.csv";
string file_qos = "qos.csv";
string file_config = "config.ini";
string file_output = "/output/solution.txt";




//һ�����ݸ�ʽ����
class DatasStruct {
public:
	vector<string> headName;
	vector<vector< string>> Datas;
};
//�û���
class User
{

private:
	//�û����������
	int Need_width;
	//�û����ýڵ������ɱ䣩
	int Usefulsize;
public:
	//����û���ǰʱ���´�������
	int GetNeed_width() { return Need_width; };
	//�����û���ǰʱ���´�������
	void SetNeed_width(int width) { Need_width = width; };
};


//�ڵ���
class Node
{
private:
	//��ǰ������
	int MaxWidth;

public:
	//���������� 
	void SetMaxWidth(int width) { MaxWidth = width; };
	//��õ�ǰ����
	int GetWidth() { return MaxWidth; };
	//�޸ĵ�ǰ���ô���
	void ADDWidth(int width) { MaxWidth -= width; };

};


class UserManage
{
private:
	//�����û���
	vector<string> Usernames;
	//��ͬʱ���û��������
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
	//�����û��ڵ�
	vector<string> Nodenames;
	//�洢Ŀ��ڵ��µĿ����û�����
	unordered_map<string, vector<bool>> UsefulUser_flag;
	//�洢node�Ĵ���
	unordered_map<string, int> Node_Width;
public:
	vector<string>& Get_Nodenames() { return Nodenames; };
	void AddNode_Usefuluser(string nodename, bool flag) { UsefulUser_flag[nodename].push_back(flag); };
	vector<bool>& Get_UsefulUser(string nodename) { return UsefulUser_flag[nodename]; };
	void SetWidth(string name, int width) { Node_Width[name] = width; };
	int GetWidth(string name) {
		return Node_Width[name];
	}
	void PushName(string name) { Nodenames.push_back(name); };
};



//�ַ����и��
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

//ȥ�����з�
void strim(string& str)
{
	int Pos = str.size() - 1;
	if (str[Pos] == '\n' || str[Pos] == '\r')
	{
		str.erase(str.begin() + Pos);
	}
}

//��ȡ�ṹ������
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


//��ȡini�ļ��е�����
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

//���һ���û��ķ���
void NodeAssign(vector<vector<int>>& allc, int user, int node)
{

	for (int j = node; j < allc[user].size(); j++)
	{
		//������ܷ���
		if (allc[user][j] == -1)
			continue;
		else
		{
			//�ڵ�ɷ���Ϊ0
			if (allc[0][j] == 0)
				continue;
			//�û�����С�ڵ��ڿɷ���
			if (allc[user][0] <= allc[0][j])
			{
				allc[user][j] += allc[user][0];
				allc[0][j] -= allc[user][0];
				allc[user][0] -= allc[user][0];

			}
			//�û�������ڿɷ���
			else
			{
				//�÷������д���
				//����������λ��
				allc[user][j] += allc[0][j];
				int temp = allc[user][0];
				//�����û�����
				allc[user][0] -= allc[0][j];
				//���½ڵ����
				allc[0][j] -= allc[0][j];
			}
		}
	}

}
//���·���
void Reset(vector<vector<int>>& allc, int user, int node)
{
	//���ٷ���
	int Rest_width = allc[user][0];
	for (int i = 1; i < user; i++)
	{
		//����ɷ���
		if (allc[i][node] != -1)
		{
			//����ѷ����������
			if (allc[i][node] > Rest_width)
			{
				allc[i][node] -= Rest_width;
				allc[i][0] += Rest_width;
				Rest_width -= Rest_width;

			}
			//����ѷ���С������
			else
			{
				Rest_width -= allc[i][node];
				allc[i][node] -= allc[i][node];
			}
		}
	}
}
//�������� ̰���㷨 user��Ŀ���û����䣬node ��Ŀ��ڵ����
void AverageChoose(vector<vector<int>>& allc, int user, int node)
{
	if (user < allc.size())
		NodeAssign(allc, user, node);
	else
		return;
	//���û�з�����
	if (allc[user][0] != 0)
	{
		for (int j = node; j < allc[user].size(); j++)
		{
			if (allc[user][j] > 0)
			{
				//Reset�����·���
				Reset(allc, user, j);
				//�Ը��û�����
				AverageChoose(allc, user, j);
				if (allc[user][0] == 0)
					break;
			}
		}
		return;
	}
	//�����������һ��
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


//ʱ��ڵ��ϵķ��� 
void DealOneAlg(int Min_time, int Max_time, UserManage* Um, NodeManage* Nm)
{
	//��ʼ���������
	vector<string>Usernames = Um->Get_usersnames();
	vector<string>Nodenames = Nm->Get_Nodenames();
	vector<vector<int>> CountJu(Usernames.size() + 1, vector<int>(Nodenames.size() + 1));
	//��ͷ �û��������
	//��ͷ Node��ӵ�еĴ���

	ofstream outfile(file_output);

	//��ʼ�����������ֵ ���ܷ���Ϊ-1
	for (int i = 0; i < Nodenames.size(); i++)
	{
		vector<bool> flags = Nm->Get_UsefulUser(Nodenames[i]);
		for (int j = 0; j < flags.size(); j++)
		{
			if (flags[j])
				CountJu[j+1][i+1] = 0;
			else //���ܷ���ڵ���Ϊ-1
				CountJu[j+1][i+1] = -1;
		}
	}
	for (int i = Min_time; i < Max_time; i++)
	{
		vector<vector<int>> CountJuTemp = CountJu;
		//��ȡ��ǰʱ���µ�ӳ��

		for (int k = 1; k < Usernames.size() + 1; k++)
		{

			//��ͷ
			CountJuTemp[k][0] = Um->GetWidth(i, Usernames[k - 1]);
		}
		for (int j = 1; j < Nodenames.size() + 1; j++)
		{
			//��ͷ
			CountJuTemp[0][j] = Nm->GetWidth(Nodenames[j - 1]);
		}
		int Reuslt = 1;
		while (Reuslt != -1)
		{
			AverageChoose(CountJuTemp, Reuslt, 1);
			Reuslt = IsEffect(CountJuTemp);
		}
		//��÷�����

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
int main()
{
	//��ȡ�ͻ��������
	DatasStruct UserWidths = GetData(file_root + file_Demand);

	//��ȡ��Ե�ڵ������
	DatasStruct NodeWidths = GetData(file_root + file_Bandwidth);

	//��ȡ�ͻ����Ե�ڵ�����ʱ��
	DatasStruct PeopleQos = GetData(file_root + file_qos);

	//��ȡconfig
	int qos = ReadQos(file_root + file_config);

	UserManage* Um = new UserManage;
	NodeManage* Nm = new NodeManage;


	//�����Ҫ���ȵ����ʱ����
	int Max_times = UserWidths.Datas.size();


	//��ʼ���û� 
	for (int i = 1; i < PeopleQos.headName.size(); i++)
	{

		string username = PeopleQos.headName[i];
		Um->PushName(username);
	}

	//��ʼ��node
	for (int i = 0; i < NodeWidths.Datas.size(); i++)
	{
		string Nodename = NodeWidths.Datas[i][0];
		Nm->PushName(Nodename);
		int NodeWidth = atoi(NodeWidths.Datas[i][1].c_str());
		Nm->SetWidth(Nodename, NodeWidth);
	}


	//��ʼ��Node�����û�
	for (int i = 0; i < PeopleQos.Datas.size(); i++)
	{
		for (int j = 1; j < PeopleQos.Datas[i].size(); j++)
		{
			//�ӳ�С����ֵ
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


	//��ʼ���û���ͬʱ���������
	for (int time = 0; time < UserWidths.Datas.size(); time++)
	{
		for (int j = 1; j < UserWidths.Datas[time].size(); j++)
		{
			string Username = UserWidths.headName[j];
			//��ø�ʱ�����û�
			int width = atoi(UserWidths.Datas[time][j].c_str());
			//���ø�ʱ�����û��������
			Um->SetWidth(time, width, Username);
		}
	}



	//���е����㷨
	int Min_time = 0;

	DealOneAlg(Min_time, Max_times, Um, Nm);
	return 0;
}