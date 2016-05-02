#include <fstream>
#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#define RECORD_NUM 100000
using namespace std;
int main(){
	srand((unsigned)time(NULL));
	ofstream o("dataset.txt");
	int width1 = 'z'-'a';
	int width2 = 'Z'-'A';
	for(int i = 0;i<RECORD_NUM;i++){
		string line;
		int len = rand()%256;
		for(int j = 0;j<len;j++){
			int possible = rand()%100;
			if(possible>13&& possible<88) line+='a'+(rand()%width1);
			else if(possible>=88)line+='A'+(rand()%width2);
			else line+=' ';
		}
		o<<line<<endl;
	} 
	o.close();
	return 0;
} 
