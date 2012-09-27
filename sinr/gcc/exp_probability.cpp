#include <iostream>
#include <ctime>
#include "sinr.h"

using namespace std;

int main()
{
	Sinr sinr;
	time_t t1,t2;
	time(&t1);
	sinr.MyCenter();
	time(&t2);
	sinr.PrintResult();
	cout<<endl<<endl<<"consuming time:"<<t2 - t1<<endl;
	//getchar();
	return 0;
}