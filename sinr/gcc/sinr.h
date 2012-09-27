#ifndef _SINR_H_
#define _SINR_H_

#define __TEST__

#include <vector>

using namespace std;

const int NUM_NODES = 30;
const int NUM_POINTS = 4;
const double POWER = 1;
const int ATTENUATION = 2;
const double RSSI_THRESHOLD = 0.001;
const int GRIDSIZE = 20;
const int MIN_SINR = -3;
const int MAX_SINR = 5;
const double noise = 0;

typedef  double SINR_type;

struct node
{
	double x,y;
	double power;
};

struct performance_struct
{
	int node_id;
	int performance;
};

struct NodeSINR
{
	int transmit_node;
	SINR_type SINR;
};

struct SubsetNode
{
	vector<int>* subset;
	NodeSINR node_sinr[NUM_NODES];
};

class Sinr
{
public:
	Sinr();
	void Center();
	void MyCenter();
	void PrintResult();
	double NodePairRssi(double power, double distance);
	void GenAllRssi();
	void GenNodePosition();
	double Distance(node a, node b);
	bool isSamePosition(node a, node b);
	bool PositionExist(node a, int pos);
	double GenRssi();
	bool isSatisfied(const vector<int>&);
	bool SubsetSatisfyNode( int j, vector<int>& new_set );
	void ComputeSubsetSinr(SubsetNode&, bool used[]);
	NodeSINR GenSinr(int node_id, vector<int>* subset);
	bool SINRExist(int node_id, SINR_type SINR);
	bool isSameSinr(SINR_type SINR1, SINR_type SINR2 );
	bool isInTransition( SINR_type SINR );
	int SumOfNodes();
private:
	node nodes[NUM_NODES];
	//记录节点对之间的rssi，node_rssi[i][j] 代表节点i作为发送者时，j收到的rssi
	SINR_type node_rssi[NUM_NODES][NUM_NODES];
	vector<SubsetNode> subsets;
#ifdef __TEST__
	int _warnning_count;
#endif
};

#endif