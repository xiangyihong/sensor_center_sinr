#include <iostream>
#include <ctime>
#include "sinr.h"
#include <fstream>

using namespace std;

int main()
{
	Sinr sinr;
	time_t t1,t2;
	time(&t1);
	//sinr.MyCenter();
	//sinr.Center();
	sinr.AnotherCenter();
	time(&t2);
	ofstream out("data.txt");
	sinr.PrintResult(out);
	cout<<endl<<endl<<"consuming time:"<<t2 - t1<<endl;
	getchar();
	return 0;
}