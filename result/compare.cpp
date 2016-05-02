#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
using namespace std;
int main(){
	ifstream fin1("now-result.txt");
	ifstream fin2("pre-result.txt");
	ofstream fout("compare.txt");
	vector<string> set1;
	vector<string> set2;
	string r1,r2;
	while(getline(fin1,r1)){
		set1.push_back(r1);
	}
	while(getline(fin2,r2)){
		set2.push_back(r2);
	}
	fin1.close();fin2.close();
	fout<<"now有pre没有:"<<endl;
	for(int i = 0;i<set1.size();i++){
		if(find(set2.begin(),set2.end(),set1[i])==set2.end()){
			fout<<set1[i]<<endl;
		}
	}
	fout<<"pre有now没有:"<<endl;
	for(int i =0;i<set2.size();i++){
		if(find(set1.begin(),set1.end(),set2[i])==set1.end()){
			fout<<set2[i]<<endl;
		}
	}
	fout.close();
	return 0;
}
