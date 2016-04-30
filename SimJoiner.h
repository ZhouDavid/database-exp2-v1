#ifndef __EXP2_SIMJOINER_H__
#define __EXP2_SIMJOINER_H__

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <cmath>
#include <algorithm>
using namespace std;

template <typename IDType, typename SimType>
struct JoinResult {
    IDType id1;
    IDType id2;
    SimType s;
};

typedef JoinResult<unsigned, double> JaccardJoinResult;
typedef JoinResult<unsigned, unsigned> EDJoinResult;

const int SUCCESS = 0;
const int FAILURE = 1;

class LengthGroup;
class InvList{
public:
    int id;
    int start_pos;
    unordered_map<string,vector<int>>invlist;
    InvList(int i,int s):id(i),start_pos(s){}
};
class Record{
public:
    int id;
    string str;
    Record(int i,string s):id(i),str(s){}
};
class LengthGroup{
public:
    int len;
    int tau;
    vector<int>s;   //切分数组 例10：2 2 3 3   tau=3
    vector<int>p;   //
    vector<Record> records;
    vector<InvList> lists;

    void partition(){
        for(int i = 0;i<records.size();i++){
            int start = 0;
            for(int j = 0;j<s.size();j++){
                string seg = records[i].str.substr(p[j],s[j]);
                lists[j].invlist[seg].push_back(records[i].id);
            }
        }
    }
    void genS(){
        int last_amount = len%(tau+1);      //  后面切割数量
        int pre_amount = tau+1-last_amount;  //前面切割的数量
        double t = double(len)/double(tau+1);
        int pre = floor(t);
        int last = ceil(t);

        for(int i  =0;i<pre_amount;i++){ s.push_back(pre); }
        for(int i = 0;i<last_amount;i++){ s.push_back(last);}
    }
    void genP(){
        int start = 0;
        for(int i = 0;i<s.size();i++){
            p.push_back(start);
            start+=s[i];
        }
    }

    void init_list(){
        for(int i = 0;i<s.size();i++){
            InvList l(i+1,p[i]);
            lists.push_back(l);
        }
    }

    void gen_lists(){
        genS();
        genP();
        init_list();
        partition();
    }

};

class SimJoiner {
public:
    vector<string> records;
    unordered_map<int,LengthGroup> groups;
    vector<Record> queries;
    vector<int> group_lens;
    int d[256][256];
public:
    SimJoiner();
    ~SimJoiner();

    int joinJaccard(const char *filename1, const char *filename2, double threshold, std::vector<JaccardJoinResult> &result);
    int joinED(const char *filename1, const char *filename2, unsigned threshold, std::vector<EDJoinResult> &result);
    void readData(string filename,int tau);
    void readQuery(string filename);
    void gen_group_lens();
    void gen_candidate(string query,int tau,vector<int>&candidate);
    void gen_substrs(string&,int,int,int,int,int,vector<string>&substrs);
    void verify(int id2,const string query,const vector<int>&candidate,int tau,vector<EDJoinResult>&result);
    void test(string filename1,string filename2);
    int calED(const string s1,const string s2,int tau);
    static int min3(int a,int b,int c){int t = a<b?a:b;return t<c?t:c;}
};
#endif
