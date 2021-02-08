#include<stdio.h>
#include<string>
#include<iostream>
#include<fstream>
#include<map>
#include<algorithm>
#include<vector>
#include<set>
#include<queue>
#include<time.h>
#include <Windows.h>
#include <Psapi.h>
#include <random>
#include <numeric>

using namespace std;

#define EdgeList map<string, Edge>
#define Graph map<int, Node>
#define DDVQ set<Pri_Q>
#define DDV map<int, double>
#define TSN 0
#define ASN 1

double theta = 0.00; int type = TSN;
//double theta = 0.00; int type = ASN;
//double theta = 0.16; int type = ASN;
//double theta = 0.20; int type = ASN;

int R = 20;
int K = 100;


set<int> s;

map<int, int> cconnect;
map<int, int> connect_count;

int total_nodes, total_edges;

struct Edge
{
	int from, to, time;
	double value;
	int weight;
	Edge(int _from, int _to, int _time)
	{
		from = _from;
		to = _to;
		time = _time;
		value = 1.0;
		weight = 1;
	}
};

EdgeList edgeList;

struct Node
{
	int id;
	vector<int> edges;
	vector<double> sp;
	vector<boolean> deleted;
	vector<int> weight;
};

Graph graph;

struct Pri_Q
{
	int id;
	double value;
	Pri_Q(int _id, double _value)
	{
		id = _id;
		value = _value;
	}
	bool operator <(const Pri_Q& a)const
	{
		if (value != a.value)
		{
			return value > a.value;
		}
		return id < a.id;
	}
};

PROCESS_MEMORY_COUNTERS showMemoryInfo()
{
	//ofstream fout(fileName, ios::app);
	HANDLE handle = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
	/*cout << "page file " << 1.0 * pmc.PagefileUsage / (1024 * 1024) << "MB" << endl;
	cout << "peak page file " << 1.0 * pmc.PeakPagefileUsage / (1024 * 1024) << "MB" << endl;*/
	//fout << "page file " << 1.0 * pmc.PagefileUsage / (1024 * 1024) << "MB" << endl;
	//fout << "peak page file " << 1.0 * pmc.PeakPagefileUsage / (1024 * 1024) << "MB" << endl;
	return pmc;
}

Edge update_edge(int nowt, Edge edge)
{
	double lastv = edge.value;
	int lastt = edge.time;
	double lastvt = pow((1.0 - lastv) / 0.56, 1 / 0.06);
	double nowv = 1 - 0.28 * pow(1.0 * lastvt + nowt - lastt, 0.06);
	edge.time = nowt;
	edge.value = nowv;
	edge.weight++;
	//edge.value = 1;
	return edge;
}

double calculate_today(int nowt, Edge edge)
{
	double lastv = edge.value;
	int lastt = edge.time;
	double lastvt = pow((1.0 - lastv) / 0.56, 1 / 0.06);
	lastvt = 0;
	double nowv = 1 - 0.56 * pow(1.0 * lastvt + nowt - lastt, 0.06);
	return nowv;
}

map<double, int> static_value;

void build_graph(int day, double theta)
{
	graph.clear();
	for (EdgeList::iterator it = edgeList.begin(); it != edgeList.end(); it++)
	{
		double nowv = calculate_today(day * 24, it->second);
		if (nowv < theta)
		{
			continue;
		}
		int from = it->second.from, to = it->second.to;
		Graph::iterator it_g_from = graph.find(from);
		Graph::iterator it_g_to = graph.find(to);
		if (it_g_from == graph.end())
		{
			Node node;
			node.id = from;
			node.edges.push_back(to);
			node.deleted.push_back(false);
			node.sp.push_back(nowv);
			node.weight.push_back(it->second.weight);
			graph.insert(make_pair(from, node));
		}
		else
		{
			it_g_from->second.edges.push_back(to);
			it_g_from->second.deleted.push_back(false);
			it_g_from->second.sp.push_back(nowv);
			it_g_from->second.weight.push_back(it->second.weight);
		}

		if (it_g_to == graph.end())
		{
			Node node;
			node.id = to;
			node.edges.push_back(from);
			node.deleted.push_back(false);
			node.sp.push_back(nowv);
			node.weight.push_back(it->second.weight);
			graph.insert(make_pair(to, node));
		}
		else
		{
			it_g_to->second.edges.push_back(from);
			it_g_to->second.deleted.push_back(false);
			it_g_to->second.sp.push_back(nowv);
			it_g_to->second.weight.push_back(it->second.weight);
		}
	}
	if (type == TSN)
	{
		for (Graph::iterator it = graph.begin(); it != graph.end(); it++)
		{
			double sum = accumulate(it->second.weight.begin(), it->second.weight.end(), 0);

			for (int i = 0; i < it->second.sp.size(); i++)
			{
				it->second.sp[i] = it->second.weight[i] / sum; 
				map<double, int>::iterator iitt = static_value.find(it->second.sp[i]);
				if (iitt == static_value.end())
				{
					static_value.insert(make_pair(it->second.sp[i], 1));
				}
				else
				{
					iitt->second++;
				}
			}
		}
	}
}

int  count_edges()
{
	int sum = 0;
	for (Graph::iterator it_g = graph.begin(); it_g != graph.end(); it_g++)
	{
		sum += it_g->second.edges.size();
	}
	return sum;
}

int alone_node()
{
	int num_alone_node = total_nodes - graph.size();
	return num_alone_node;
}

int connect_compment()
{
	int count = 0;
	cconnect.clear();
	queue<int> q;
	for (Graph::iterator it_g = graph.begin(); it_g != graph.end(); it_g++)
	{
		if (cconnect.find(it_g->first) == cconnect.end())
		{
			count++;
			q.push(it_g->first);
			cconnect.insert(make_pair(it_g->first, count));
			while (!q.empty())
			{
				int head = q.front();
				q.pop();
				Graph::iterator it_head = graph.find(head);
				for (int i = 0; i < it_head->second.edges.size(); i++)
				{
					if (cconnect.find(it_head->second.edges[i]) != cconnect.end())
					{
						continue;
					}
					q.push(it_head->second.edges[i]);
					cconnect.insert(make_pair(it_head->second.edges[i], count));
				}
			}
		}
	}
	return count;
}

void connect_compment_count(int count)
{
	vector<int> sum(count + 5, 0);
	for (map<int, int>::iterator it = cconnect.begin(); it != cconnect.end(); it++)
	{
		sum[it->second] ++;
	}
	for (int i = 1; i <= count; i++)
	{
		if (sum[i] == 0)
		{
			continue;
		}
		map<int, int>::iterator it = connect_count.find(sum[i]);
		if (it == connect_count.end())
		{
			cconnect.insert(make_pair(sum[i], 1));
		}
		else
		{
			it->second++;
		}
	}
}

double cal_average_degree()
{
	double sum = 0;
	for (Graph::iterator it_g = graph.begin(); it_g != graph.end(); it_g++)
	{
		sum += it_g->second.edges.size();
	}

	return sum / graph.size();

}

double cal_variance_degree(double avg)
{
	double variance = 0;
	for (Graph::iterator it_g = graph.begin(); it_g != graph.end(); it_g++)
	{
		variance += (it_g->second.edges.size() - avg) * (it_g->second.edges.size() - avg);
	}
	variance /= graph.size();
	return sqrt(variance);
}

set<int> seed;
set<int> T;
queue<int> target;
double problity = 0.1;

set<int> rgs;
map<int, set<int> > rgv;
int num_edge;

void new_greedy_IC_RGS()
{
	rgs.clear();

	queue<int> q;
	for (set<int>::iterator it = seed.begin(); it != seed.end(); it++)
	{
		if (rgs.find(*it) != rgs.end())
		{
			continue;
		}
		//cout << "a1:" << *it << " ";
		Graph::iterator it_g = graph.find(*it);
		q.push(it_g->first);
		rgs.insert(it_g->first);
		//cout << "a2" << " ";
		while (!q.empty())
		{
			int head = q.front();
			q.pop();
			Graph::iterator it_head = graph.find(head);
			//cout << "a3" << " ";
			for (int i = 0; i < it_head->second.edges.size(); i++)
			{
				if (it_head->second.deleted[i] || rgs.find(it_head->second.edges[i]) != rgs.end())
				{
					continue;
				}
				q.push(it_head->second.edges[i]);
				rgs.insert(it_head->second.edges[i]);
			}
		}

	}
}

double new_greedy_IC_RGV(int v)
{
	int sum = 0;
	set<int> vis;

	queue<int> q;
	Graph::iterator it_g = graph.find(v);
	q.push(it_g->first);
	vis.insert(it_g->first);
	while (!q.empty())
	{
		int head = q.front();
		q.pop();
		Graph::iterator it_head = graph.find(head);
		for (int i = 0; i < it_head->second.edges.size(); i++)
		{
			if (it_head->second.deleted[i] || vis.find(it_head->second.edges[i]) != vis.end() || rgs.find(it_head->second.edges[i]) != rgs.end()) //如果边已经被删  or  点已经在rgs
			{
				continue;
			}
			q.push(it_head->second.edges[i]);
			vis.insert(it_head->second.edges[i]);
		}
	}
	return vis.size();
}

void new_greedy_IC(int K, int R)
{
	seed.clear();
	for (int k = 0; k < K; k++)
	{
		cout << k << "/" << K << "\r";
		if (seed.size() == graph.size())
		{
			break;
		}
		int max_sv = -1;
		int index_v = -1;
		map<int, double > sv;
		for (Graph::iterator it = graph.begin(); it != graph.end(); it++)
		{
			sv.insert(make_pair(it->first, 0));
		}
		for (int r = 0; r < R; r++)
		{
			//cout << r << "/" << R << "\r";
			num_edge = 0;
			std::random_device e;
			std::uniform_real_distribution<double> rand_uniform(0, 1);
			for (Graph::iterator it = graph.begin(); it != graph.end(); it++)
			{
				for (int i = 0; i < it->second.sp.size(); i++)
				{
					it->second.deleted[i] = false;
					it->second.deleted[i] = (rand_uniform(e) <= 1.0 - it->second.sp[i]);
					if (it->second.deleted[i] == false)
					{
						num_edge++;
					}
				}
			}
			//cout << "a" << " ";
			new_greedy_IC_RGS();
			//cout << "b" << " ";
			if (rgs.size() == graph.size())
			{
				break;
			}
			int kk = 0;
			for (Graph::iterator it = graph.begin(); it != graph.end(); it++)
			{
				//cout << "\r" << kk++ << "/" << graph.size();

				if (rgs.find(it->first) != rgs.end())
				{
					continue;
				}
				map<int, double>::iterator it_sv = sv.find(it->first);
				//cout << "c" << " ";
				it_sv->second += new_greedy_IC_RGV(it->first) / R;
				//cout << "d" << " ";
				if (max_sv <= it_sv->second)
				{
					max_sv = it_sv->second;
					index_v = it->first;
				}
				if (index_v == -1)
				{
					continue;
				}
			}
		}
		if (index_v == -1)
		{
			break;
		}
		//cout << endl << "index_v = " << index_v << " " << endl;
		seed.insert(index_v);

	}
}

double new_greedy_run_IC()
{
	std::random_device e;
	std::uniform_real_distribution<double> rand_uniform(0, 1);
	target = queue<int>();
	T.clear();
	T = seed;
	for (set<int>::iterator it = T.begin(); it != T.end(); it++)
	{
		target.push(*it);
	}
	while (!target.empty())
	{
		int u = target.front();
		target.pop();
		Graph::iterator it_g = graph.find(u);
		for (int j = 0; j < it_g->second.edges.size(); j++)
		{
			int v = it_g->second.edges[j];
			if (T.find(v) != T.end())
			{
				continue;
			}
			if (rand_uniform(e) <= it_g->second.sp[j])
			{
				T.insert(v);
				target.push(v);
			}
		}
	}
	return T.size();
}

clock_t start1, end1;
void execute_algorithm(int nowt)
{

	clock_t start2, end2;
	clock_t start_IC, end_IC;
	clock_t start_SIM, end_SIM;
	clock_t start_CC, end_CC;
	ofstream fout("0206_4_weibo_" + to_string(theta) + ".txt", ios::app);
	//ofstream fout("0206_4_stack_" + to_string(theta) + ".txt", ios::app);
	//ofstream fout("0206_dept1" + to_string(theta) + ".txt", ios::app);
	//ofstream fout("0206_dept2" + to_string(theta) + ".txt", ios::app);
	//ofstream fout("0206_dept3" + to_string(theta) + ".txt", ios::app);
	//ofstream fout("0206_dept4" + to_string(theta) + ".txt", ios::app);

	int num_alone_node = alone_node();
	start_CC = clock();
	int count = connect_compment();
	end_CC = clock();
	int count_edge = count_edges();
	//connect_compment_count(count);
	double average_degree = cal_average_degree();
	double variance_degree = cal_variance_degree(average_degree);
	start_IC = clock();
	new_greedy_IC(K, R);
	end_IC = clock();
	cout << "finish" << endl;
	end1 = clock();

	start2 = clock();
	start_SIM = clock();
	double difusse_rate = 0;
	int rounds = 20;
	for (int i = 0; i < rounds; i++)
	{
		//difusse_rate += degreee_discount_run_IC()/rounds;
		difusse_rate += 1.0 * new_greedy_run_IC() / rounds;
	}
	end_SIM = clock();
	end2 = clock();
	double total_time = (double)(end1 - start1) / CLOCKS_PER_SEC + (double)(end2 - start2) / CLOCKS_PER_SEC;
	double time_IC = (double)(end_IC - start_IC) / CLOCKS_PER_SEC;
	double time_SIM = (double)(end_SIM - start_SIM) / CLOCKS_PER_SEC;
	double time_CC = (double)(end_CC - start_CC) / CLOCKS_PER_SEC;


	PROCESS_MEMORY_COUNTERS pmc = showMemoryInfo();
	/*cout << "page file " << 1.0 * pmc.PagefileUsage / (1024 * 1024) << "MB" << endl;
	cout << "peak page file " << 1.0 * pmc.PeakPagefileUsage / (1024 * 1024) << "MB" << endl;
	cout << "time = " << (double)(end1 - start1) / CLOCKS_PER_SEC  + (double)(end2 - start2) / CLOCKS_PER_SEC / rounds<< endl;*/
	/*cout << "day = " << nowt << ",active_nodes = " << total_nodes - num_alone_node << "/" << total_nodes << ",avg_degree = " << average_degree << ",CC_num = " << count << ",difusse_rate = " << difusse_rate <<
		",total_time = " << total_time << ",avg_total_time = " << total_time/rounds << ",page_file = " << 1.0 * pmc.PagefileUsage / (1024 * 1024) << ",peak_page_file = " << 1.0 * pmc.PeakPagefileUsage / (1024 * 1024) << endl;
	fout << "day = " << nowt << ",active_nodes = " << total_nodes - num_alone_node << "/" << total_nodes << ",avg_degree = " << average_degree << ",CC_num = " << count << ",difusse_rate = " << difusse_rate <<
		",total_time = " << total_time << ",avg_total_time = " << total_time / rounds << ",page_file = " << 1.0 * pmc.PagefileUsage / (1024 * 1024) << ",peak_page_file = " << 1.0 * pmc.PeakPagefileUsage / (1024 * 1024) << endl;*/
		//cout << "day = " << nowt << ",active_nodes = " << total_nodes - num_alone_node << "/" << total_nodes << ",difusse_rate = " << seed.size() << "/" << difusse_rate <<
		//	",time = " << time_IC << "/" << time_SIM << ",peak_page_file = " << 1.0 * pmc.PeakPagefileUsage / (1024 * 1024) << endl;
		//fout << "day = " << nowt << ",active_nodes = " << total_nodes - num_alone_node << "/" << total_nodes << ",difusse_rate = " << seed.size() << "/" << difusse_rate <<
		//	",time = " << time_IC << "/" << time_SIM << ",peak_page_file = " << 1.0 * pmc.PeakPagefileUsage / (1024 * 1024) << endl;

		//cout << "day = " << nowt << ",active_nodes = " << total_nodes - num_alone_node << "/" << total_nodes << ",EDGES = " << count_edge << ",CC_TIME = " << time_CC << endl;
		//fout << "day = " << nowt << ",active_nodes = " << total_nodes - num_alone_node << "/" << total_nodes << ",EDGES = " << count_edge << ",CC_TIME = " << time_CC << endl;
	cout << nowt << " " << variance_degree << endl;
	fout << nowt << " " << variance_degree << endl;
	fout.close();
}


int main()
{
	ifstream fin("weibo_fix_sort1.txt");
	//ifstream fin("sx-stackoverflow_800days.txt");
	//ifstream fin("dept1.txt");
	//ifstream fin("dept2.txt");
	//ifstream fin("dept3.txt");
	//ifstream fin("dept4.txt");
	int from, to, nowt;
	int day = 1;
	int max_day;
	fin >> total_nodes >> max_day;
	int x = 1;
	start1 = clock();
	while (fin >> from >> to >> nowt)
	{
		//cout << x++ << endl;
		while (nowt > day * 24 && day <= max_day)
		{
			//cout << day << endl;
			if (day < 10000)
			{
				//cout << s.size() << endl;
				build_graph(day, theta);
				//cout << "day :" << nowt << " " << total_nodes - s.size() << endl;
				execute_algorithm(day);
			}

			day++;
			start1 = clock();
		}
		if (nowt > max_day * 24)
		{
			break;
		}

		if (from == to)
		{
			continue;
		}
		s.insert(from);
		s.insert(to);
		if (from > to)
		{
			swap(from, to);
		}
		EdgeList::iterator it = edgeList.find(to_string(from) + "_" + to_string(to));
		if (it == edgeList.end())
		{
			Edge edge(from, to, nowt);
			edgeList.insert(make_pair(to_string(from) + "_" + to_string(to), edge));
		}
		else
		{
			it->second = update_edge(nowt, it->second);
		}

	}

	while (day <= max_day)
	{
		build_graph(day, theta);
		day++;
		start1 = clock();
	}
	return 0;
}