#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <cmath>
#include <ctime>

using namespace std;

struct point
{
	double x,y;
};

typedef vector<int> subset;
const int NUM_OF_NODES = 4;
const int LEAST_SINR_POINT = 1;

//�Ӽ�����
list<subset*> subsets;
//�ڵ��RSSI
int node_pair_rssi[NUM_OF_NODES+1][NUM_OF_NODES+1];
//�ڵ�����λ��
point  node_position[NUM_OF_NODES+1];

struct transmit_sinr
{
	int transmit_node;
	int sinr;
};

struct node_sinr_in_subset
{
	subset* sub;
	vector<transmit_sinr> sinr;
};

//�ڵ��������Ч���Ӽ�����õ�sinr
list<node_sinr_in_subset> node_sinr[NUM_OF_NODES+1];


point GenPosition()
{
	static int tmp = 1;
	point x;
	x.x = tmp;
	x.y = tmp+1;
	tmp++;
	return x;
}

double Distance(point x1, point x2)
{
	double x = x1.x - x2.x;
	double y = x1.y - x2.y;
	return sqrt(x*x+y*y);
}

bool isSamePosition(point x1, point x2)
{
	return x1.x == x2.x && x1.y == x2.y;
}

bool PositionExist(int pos)
{
	for(int i = 1 ; i < pos; ++i)
	{
		if(isSamePosition(node_position[i], node_position[pos]))
		{
			return true;
		}
	}
	return false;
}


int GenRssi(double dis)
{
	return 90-int(dis);
}


void GenNodePosition()
{
	for(int i = 1 ; i <= NUM_OF_NODES; ++i)
	{
		node_position[i] = GenPosition();
		while(PositionExist(i))
		{
			node_position[i] = GenPosition();
		}
	}
}

void GenNodePairRssi()
{
	for(int i = 1; i <= NUM_OF_NODES; ++i)
	{
		node_pair_rssi[i][i] = 0;
		for(int j = i+1; j <= NUM_OF_NODES; ++j)
		{
			node_pair_rssi[i][j] = GenRssi(Distance(node_position[i], node_position[j]));
			node_pair_rssi[j][i] = node_pair_rssi[i][j];
		}
	}
}

void GenSubset(subset* pre)
{
	int next_node;
	subset* s;
	int len;

	if(pre == NULL)
	{
		next_node = 1;
		len = 0;
	}
	else
	{
		len = pre->size();
		next_node = (*pre)[len-1]+1;		
	}

	//������Ӽ��Ѿ���ȫ����
	if(len == NUM_OF_NODES)
	{
		return;
	}

	for(int i = next_node; i <= NUM_OF_NODES; ++i)
	{
		if(pre == NULL)
		{
			s = new subset;
		}
		else
		{
			s = new subset(*pre);
		}
		s->push_back(0);
		(*s)[len] = i;
		//len��ֵ���Ӽ����ȼ�1��������ʽ����Ӽ��Ƿ��Ѿ���ȫ��
		if(len == NUM_OF_NODES-1)
		{
			continue;
		}
		GenSubset(s);
		//�Ӽ���ֻ��һ��Ԫ��
		if(len == 0)
		{
			continue;
		}
		subsets.push_back(s);
	}
}

bool SubsetCompare(subset* s1, subset* s2)
{
	return s1->size() < s2->size();
}

bool isInSubset(subset* s, int node)
{
	int len = s->size();
	
	for(int i = 0 ;i < len; ++i)
	{
		if((*s)[i] == node)
		{
			return true;
		}
	}
	return false;
}

int NodeSubsetSinr(subset* s, int transmit, int node)
{
	int len = s->size();
	int sinr = 0;
	for(int i = 0 ; i < len; ++i)
	{
		if((*s)[i] == transmit)
		{
			sinr += node_pair_rssi[node][transmit];
		}
		else
		{
			sinr -= node_pair_rssi[(*s)[i]][node];
		}
	}
	return sinr;
}

bool isInTransitionArea(int sinr)
{
	return true;
}

void GenSubsetSinr()
{
	list<subset*>::iterator list_iter;
	
	for(list_iter = subsets.begin(); list_iter != subsets.end(); ++list_iter)
	{
		for(int i = 1 ; i <= NUM_OF_NODES; ++i)
		{
			if(isInSubset(*list_iter, i))
			{
				continue;
			}
			node_sinr_in_subset tmp;
			tmp.sub = *list_iter;
			int len = tmp.sub->size();
			
			for(int j = 0 ; j < len; ++j)
			{
				transmit_sinr x;
				x.transmit_node = (*tmp.sub)[j];
				x.sinr = NodeSubsetSinr(tmp.sub, x.transmit_node, i);
				if(isInTransitionArea(x.sinr))
				{
					tmp.sinr.push_back(x);
				}
				
			}
			if(tmp.sinr.size() != 0)
			{
				node_sinr[i].push_back(tmp);
			}
			
		}
	}
}

list<subset*> choosed_subset;


//�������x��һ��ָʾ��
//���x�Ķ����Ƶĵ�kλ��1����ѡȡsubsets����ĵ�k��Ԫ��
void ChooseSubset(int x)
{
	int len = subsets.size();
	choosed_subset.clear();
	int i = 0;
	int pre = 0;
	list<subset*>::iterator it = subsets.begin();

	while(i < len)
	{
		if(x & (1<<i))
		{
			for(; pre < i; pre++)
			{
				it++;
			}
			choosed_subset.push_back(*it);
		}
		i++;
	}
}

//��ѡ�����Ӽ��£������ڵ�ѡ��ķ��ͽڵ��Լ��ﵽ��sinr
struct choosed_sub_struct
{
	subset* sub;
	transmit_sinr s[NUM_OF_NODES+1];
};

typedef list<choosed_sub_struct>  result_type;

result_type current;
result_type best;

bool SinrExist(int node, int sinr)
{
	result_type::iterator it = current.begin();
	while(it != current.end() && it->s[node].sinr != sinr)
	{
		it++;
	}
	if(it == current.end())
	{
		return false;
	}
	return true;
}

//���������
bool ChangePreSinr(int node)
{
	result_type::reverse_iterator it = current.rbegin();
	while(it != current.rend())
	{
		list<node_sinr_in_subset>::iterator node_iter = node_sinr[node].begin();
		while(node_iter!=node_sinr[node].end() && node_iter->sub != it->sub)
		{
			++node_iter;
		}
		if(node_iter != node_sinr[node].end())
		{
			int len = node_iter->sinr.size();
			int i;
			for(i = 0 ; i < len ; ++i)
			{
				if(node_iter->sinr[i].sinr == it->s[node].sinr)
				{
					break;
				}
			}
			if(i < len-1)
			{
				i++;
				while(i < len && SinrExist(node, node_iter->sinr[i].sinr))
				{
					i++;
				}
				if(i < len)
				{
					it->s[node] = node_iter->sinr[i];
					return true;
				}
			}
		}
		it++;
	}
	return false;
}

void GetCurrentResult()
{
	current.clear();
	list<subset*>::iterator it;
	for(it = choosed_subset.begin(); it != choosed_subset.end(); ++it)
	{
		choosed_sub_struct tmp;
		tmp.sub = *it;
		for(int i = 1 ; i <= NUM_OF_NODES; ++i)
		{
			if(isInSubset(tmp.sub, i))
			{
				tmp.s[i].transmit_node = -1;
				continue;
			}
			list<node_sinr_in_subset>::iterator node_iter = node_sinr[i].begin();
			while(node_iter != node_sinr[i].end() && node_iter->sub != tmp.sub)
			{
				++node_iter;
			}
			int len = node_iter->sinr.size();
			int j;
			
			//TODO
			//�ѱ�ź������goto����Ƴ��滻��
repeat:
			for(j = 0 ; j < len ; ++j)
			{
				int cur_sinr = node_iter->sinr[j].sinr;
				//�ҵ�һ��δ���ֹ���sinrֵ
				if( ! SinrExist(i, cur_sinr))
				{
					tmp.s[i] = node_iter->sinr[j];
					break;
				}
			}
			//����sinrֵ�Ѿ����ֹ�
			if(j == len)
			{
				//ǰ���sinrֵ�ܸı䣿
				if(ChangePreSinr(i))
				{
					goto repeat;
				}
				//�ڸ��Ӽ��£��˽ڵ㲻�ܵõ���ͬ��sinrֵ����ô���Ϊ����
				else
				{
					tmp.s[i].transmit_node = -1;
				}
			}
		}
		current.push_back(tmp);
	}
}

//��ǰ�Ӽ����ý������ǰ���Ӽ�����ý������
bool isBetterResult()
{
	if(best.size() == 0)
	{
		return true;
	}
	result_type::iterator it_cur = current.begin();
	result_type::iterator it_best = best.begin();
	int num_of_cur  = 0;
	int num_of_best = 0;

	for(; it_cur != current.end(); ++it_cur)
	{
		num_of_cur += it_cur->sub->size();
	}
	for(; it_best != best.end(); ++it_best)
	{
		num_of_best += it_best->sub->size();
	}
	if(num_of_cur < num_of_best)
	{
		return true;
	}
	return false;
}

//��ǰ�Ӽ����ý����������Ҫ����
bool isQuantified()
{
	for(int i = 1 ; i <= NUM_OF_NODES; ++i)
	{
		int p = 0;
		result_type::iterator it ;
		for(it = current.begin(); it != current.end(); ++it)
		{
			if(it->s[i].transmit_node != -1)
			{
				p++;
			}
		}
		if(p < LEAST_SINR_POINT)
		{
			return false;
		}
	}
	return true;
}


void GetBestResult()
{
	int init_set  = 1;
	int end_set = 1<<(subsets.size());
	for(; init_set < end_set; ++init_set)
	{
		ChooseSubset(init_set);
		GetCurrentResult(); //���ݵõ����Ӽ���������ڵ���Եõ���sinr��
		if(! isQuantified()) //��������������
		{
			continue;
		}
		if(isBetterResult())
		{
			best = current;
		}
	}

}


void PrintVector(subset* v)
{
	int len = v->size();
	for(int i = 0 ; i < len ; ++i)
	{
		cout<<(*v)[i]<<"\t";
	}
	cout<<endl;
}

void BestStatistic()
{
	result_type::iterator it;
	int num_of_nodes_in_choosed_subsets = 0;
	cout<<"number of used subsets:"<<best.size()<<endl;
	for(it = best.begin(); it != best.end(); ++it)
	{
		num_of_nodes_in_choosed_subsets += it->sub->size();
	}
	cout<<"number of nodes in chosen subsets:"<<num_of_nodes_in_choosed_subsets<<endl;
}

void ShowBest()
{
	cout<<"in show"<<endl;
	result_type::iterator it = best.begin();
	for(; it != best.end(); ++it)
	{
		cout<<"subset:";
		PrintVector(it->sub);
		cout<<"nodes:"<<endl;
		int i;
		for(i = 1 ; i <= NUM_OF_NODES; ++i)
		{
			if(it->s[i].transmit_node != -1)
			{
				cout<<i<<"\ttransmit:"<<it->s[i].transmit_node<<"\tsinr:"<<it->s[i].sinr<<endl;
			}
		}
	}
	BestStatistic();
}

int main()
{
	time_t t1 = time(NULL);
	GenNodePosition();
	GenNodePairRssi();
	GenSubset(NULL);
	subsets.sort(SubsetCompare);
	GenSubsetSinr();
	GetBestResult();
	ShowBest();
	time_t t2 = time(NULL);
	cout<<endl;
	cout<<"time:"<<t2-t1<<endl;
	getchar();
	return 0;
}

