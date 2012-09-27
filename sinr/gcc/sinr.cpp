#include "sinr.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <ctime>
#include <functional>
#include <iterator>
#include <iostream>

using namespace std;

bool PerformanceCompare(const performance_struct& a, const performance_struct& b)
{
	return a.performance >  b.performance;
}

struct PerformanceCount:public unary_function<performance_struct, bool>
{
	int best;
	PerformanceCount(int b):best(b){}
	bool operator()(performance_struct& tmp)
	{
		return tmp.performance == best;
	}
};

Sinr::Sinr()
{
	GenNodePosition();
	GenAllRssi();
	cout<<"start:"<<endl;
#ifdef __TEST__
	_warnning_count = 0;
#endif
}

double Sinr::Distance( node a, node b )
{
	double x = a.x - b.x;
	double y = a.y - b.y;
	return sqrt(x*x + y*y);
}

double Sinr::GenRssi()
{
	return 0.0;
}

void Sinr::GenAllRssi()
{
	for(int i = 0 ; i < NUM_NODES; ++i)
	{
		for(int j = 0 ; j < NUM_NODES ; ++j)
		{
			if(i == j)
			{
				node_rssi[i][i] = -1000;
				continue;
			}
			node_rssi[i][j] = NodePairRssi(nodes[i].power, Distance(nodes[i],nodes[j]));
		}
	}
}

double Sinr::NodePairRssi(double power, double distance )
{
	return power/pow(distance, ATTENUATION);
}

bool Sinr::isSamePosition( node a ,node b)
{
	return a.x == b.x && a.y == b.y;
}

bool Sinr::PositionExist( node a , int pos)
{
	if(pos == 0)
	{
		return false;
	}
	for (int i = 0 ; i < pos ; ++i)
	{
		if(isSamePosition(a,nodes[i]))
		{
			return true;
		}
	}
	return false;
}

void Sinr::Center()
{
	vector<int> point_count(NUM_NODES);
	bool used[NUM_NODES];
	
	cout<<"in center:"<<endl;

	srand(time(NULL));

	fill(point_count.begin(), point_count.end(), 0);
	
	while(!isSatisfied(point_count))
	{
		vector<int> chosen_set;
		fill(&used[0], &used[NUM_NODES], false);

		int first_node = rand()%NUM_NODES;
		chosen_set.push_back(first_node);
		used[first_node] = true;

		int pre_best = 0;

		while(true)
		{
			int next_node = -1;
			int best = -1;
			vector<performance_struct> node_performance(NUM_NODES);
			vector<int> new_set(chosen_set.size()+1);

			copy(chosen_set.begin(), chosen_set.end(), new_set.begin());

			for(int i = 0 ; i < NUM_NODES; ++i)
			{
				node_performance[i].performance = -1;
				node_performance[i].node_id = i;
				if( !used[i])
				{
					node_performance[i].performance = 0;
					new_set.back() = i;
					
					for(int j = 0 ; j < NUM_NODES; ++j)
					{
						if(j != i && !used[j])
						{
								if(SubsetSatisfyNode(j, new_set))
								{
									node_performance[i].performance++;
								}
						}
					}
				}
			}
			sort(node_performance.begin(), node_performance.end(), PerformanceCompare);
			best = node_performance[0].performance;
			int n_best = count_if(node_performance.begin(), node_performance.end(), 
				PerformanceCount(best));
			next_node = node_performance[rand()%n_best].node_id;
			if(next_node != -1 && best > pre_best)
			{
				chosen_set.push_back(next_node);
				pre_best = best;
				used[next_node] = true;
			}
			else
			{
				break;
			}
		}

		if (chosen_set.size() > 1)
		{
			vector<int>* vp = new vector<int>(chosen_set);
			sort(vp->begin(), vp->end());
			int n_sets = subsets.size();
			bool used_set = false;
			for(int i = 0 ; i < n_sets; ++i)
			{
				if(vp->size() == subsets[i].subset->size() &&
					equal(vp->begin(), vp->end(), subsets[i].subset->begin()))
				{
					used_set = true;
					break;
				}
			}
			

			if(!used_set)
			{
				//cout<<"add set"<<endl;
				//copy(vp->begin(), vp->end(), ostream_iterator<int>(cout, "\t"));
				//cout<<endl;
				SubsetNode tmp_subset;
				tmp_subset.subset = vp;
				ComputeSubsetSinr(tmp_subset, used);
				subsets.push_back(tmp_subset);
				for(int k = 0 ; k < NUM_NODES; ++k)
				{
					if(tmp_subset.node_sinr[k].transmit_node != -1)
					{
						point_count[k]++;
					}
				}
			}

		}
		
	}
}

void Sinr::MyCenter()
{
	vector<int> point_count(NUM_NODES);
	bool used[NUM_NODES];

	cout<<"My Center:"<<endl;

	fill(point_count.begin(), point_count.end(), 0);

	while(! isSatisfied(point_count))
	{

		for(int i = 0 ; i < NUM_NODES; ++i)
		{
			vector<int> chosen_set;
			chosen_set.push_back(i);

			fill(&used[0], &used[NUM_NODES], false);
			used[i] = true;
			double pre_max_ratio = 0;

			for(int k = 2 ; k < NUM_NODES-1; ++k)
			{
				int index = -1;
				int max = 0;
				chosen_set.push_back(-1); // 占一个元素的位子，使后面可以通过索引赋值。
				for(int j = 1 ; j < NUM_NODES ; ++j)
				{
					int cc = 0;
					if(!used[j])
					{
						chosen_set[k] = j;
						for(int x = 1 ; x < NUM_NODES ; ++x)
						{
							if(!used[x] && x != j)
							{
								if(SubsetSatisfyNode(x, chosen_set))
								{
									cc++;
								}
							}
						}
					}
					if(cc > max)
					{
						max = cc;
						index = j;
					}
				}
				if(max / (double)k > pre_max_ratio)
				{
					pre_max_ratio = (double)max / k;
					chosen_set[k] = index;
					used[index] = true;
				}
				else if(chosen_set.size() - 1 > 1)
				{
					chosen_set.erase(chosen_set.end()-1);
					SubsetNode tmp_node;
					tmp_node.subset = &chosen_set;
					ComputeSubsetSinr(tmp_node, used);
					subsets.push_back(tmp_node);
					for(int x = 0 ; x < NUM_NODES ; ++x)
					{
						if(tmp_node.node_sinr[i].transmit_node != -1)
						{
							point_count[i]++;
						}
					}
					break;
				}
				else
				{
					break;
				}
			}


		}
	}
}



void Sinr::PrintResult()
{
	int n_set = subsets.size();
	for(int i = 0 ; i < n_set ; ++i)
	{
		copy(subsets[i].subset->begin(), subsets[i].subset->end(),
			ostream_iterator<int>(cout,"\t"));
		cout<<endl;
		int efficient_node_count = 0;
		for(int j = 0 ; j < NUM_NODES; ++j)
		{
			
			if(subsets[i].node_sinr[j].transmit_node != -1)
			{
				efficient_node_count++;
				cout<<"node:"<<j<<" transmit node "<<subsets[i].node_sinr[j].transmit_node
					<<" SINR:"<<subsets[i].node_sinr[j].SINR<<endl;
			}		
		}
		if(efficient_node_count == 0)
		{
			cout<<"WARNNING"<<endl;
#ifdef __TEST__
			_warnning_count++;
#endif
		}
	}
	cout<<"sum of nodes:"<<SumOfNodes()<<endl;
#ifdef __TEST__
	cout<<"warnnings:"<<_warnning_count<<endl;
#endif
}

bool Sinr::isSatisfied(const vector<int>& point_count)
{
	int n = point_count.size();
	for(int i = 0 ; i < n ; ++i)
	{
		if(point_count[i] < NUM_POINTS)
		{
			return false;
		}
	}
	return true;
}



//未来需要考虑如果此子集不能给节点j新的sinr，那么改变此节点在先前的子集中的sinr，
//能否使此次获得不重复的sinr
bool Sinr::SubsetSatisfyNode( int j, vector<int>& new_set )
{
	int n = new_set.size();
	double sum = 0.0;
	for(int i = 0 ; i < n ; ++i)
	{
		sum += node_rssi[i][j];
	}
	for(int i = 0 ; i < n ; ++i)
	{
		double trans = node_rssi[i][j];
		double interference = sum - trans;
		SINR_type SINR = 10*log10(trans/(interference + noise));
		if(!SINRExist(j, SINR))
		{
			return true;
		}
	}
	return false;
}


//对给定的subset，各个节点计算能否在此subset下获得一个没有用到过的sinr
void Sinr::ComputeSubsetSinr( SubsetNode& sub, bool used[])
{
	vector<int>* subset = sub.subset;
	int n = subset->size();
	for(int i = 0 ; i < NUM_NODES ; ++i)
	{
		if(used[i])
		{
			sub.node_sinr[i].transmit_node = -1;
		}
		else
		{
			sub.node_sinr[i] = GenSinr(i, subset);
		}
	}
}

NodeSINR Sinr::GenSinr( int node_id, vector<int>* subset )
{
	int n = subset->size();
	double sum = 0.0;
	
	for(int i = 0 ; i < n ; ++i)
	{
		sum += node_rssi[(*subset)[i]][node_id];
	}
	NodeSINR tmp;
	for(int i = 0 ; i < n ; ++i)
	{
		double trans = node_rssi[(*subset)[i]][node_id];
		double interference = sum - trans;
		double SINR = 10*log10(trans/(interference + noise));
		
		if(!SINRExist(node_id, SINR))
		{
			if(isInTransition(SINR))
			{
				tmp.SINR = SINR;
				tmp.transmit_node = (*subset)[i];
				return tmp;
			}
			
		}
	}
	tmp.transmit_node = -1;
	return tmp;
}


bool Sinr::SINRExist( int node_id, SINR_type SINR )
{
	int n_set = subsets.size();
	for(int i = 0 ; i < n_set; ++i)
	{
		if(isSameSinr(subsets[i].node_sinr[node_id].SINR, SINR))
		{
			return true;
		}
	}
	return false;
	
}

bool Sinr::isSameSinr( SINR_type SINR1, SINR_type SINR2 )
{
	return SINR1 == SINR2;
}

void Sinr::GenNodePosition()
{
	srand(time(NULL));
	for(int i = 0 ; i < NUM_NODES ; ++i)
	{
		node tmp;
		do 
		{
			tmp.x = rand()%GRIDSIZE;
			tmp.y = rand()%GRIDSIZE;
		} while (PositionExist(tmp, i));
		nodes[i] = tmp;
		nodes[i].power = 1;
	}
}

bool Sinr::isInTransition( SINR_type SINR )
{
	return MIN_SINR <= SINR && SINR<= MAX_SINR;
}

int Sinr::SumOfNodes()
{
	int n_set = subsets.size();
	int sum = 0;
	for(int i = 0 ; i < n_set; ++i)
	{
		sum += subsets[i].subset->size();
	}
	return sum;
}
