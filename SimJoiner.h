#ifndef __EXP2_SIMJOINER_H__
#define __EXP2_SIMJOINER_H__

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
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
class Partition{
public:
   // int id;//组号
    unordered_map<string,vector<int>> invlist;
    unordered_map<string,vector<int>> neighbor_invlist; //1-deletion neighborhood
};
class JacGroup{
public:
    int size;  //该group元素数目
    int m;
    vector<int> V;//allocation 数组
    unordered_map<int,Partition> partitions;  //所有分组 key代表分组

    void gen_partitions(const vector<string>& set,int id,unordered_map<string,int>& universe){
        int group_id = 0;
        for(int i = 0;i<set.size();i++){
            group_id = ceil(double(universe[set[i]])/double(m));
            partitions[group_id].invlist[set[i]].push_back(id);
        }
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

public : // for compute ed
    int joinJaccard(const char *filename1, const char *filename2, double threshold, std::vector<JaccardJoinResult> &result);
    int joinED(const char *filename1, const char *filename2, unsigned threshold, std::vector<EDJoinResult> &result);
    void readData(string filename,int tau);
    void readQuery(string filename);
    void gen_group_lens();
    void gen_candidate(const string& query,int tau,vector<int>&candidate);
    void gen_substrs(const string&,int,int,int,int,int,vector<string>&substrs);
    void verify(int id1,const string& query,const vector<int>&candidate,int tau,vector<EDJoinResult>&result);
    void test(string filename1,string filename2);
    int calED(const string& s1,const string& s2,int tau);
    static int min3(int a,int b,int c){int t = a<b?a:b;return t<c?t:c;}
    static bool sort_result(const EDJoinResult& r1,const EDJoinResult& r2);

public:  //for compute jaccard
    vector<vector<string>>jac_records;
    vector<vector<string>>jac_queries;
    unordered_map<int,JacGroup> jac_groups;
    unordered_map<string,int> universe;
    vector<int>group_sizes;
    int wid = 0;
    bool there = false;
    static int H(int l,int s,double tau);
    static int H(int l,double tau);
    static void split(const string& str,char s,vector<string>&words);
    static double calJaccard(vector<string>& s1,vector<string>&s2,double tau);


    void readJacQuery(string filename);
    void readJacData(string filename);
    void jac_init(double tau);
    void gen_jac_candidate(const vector<string>& query,vector<int>& candidate,double tau);
    void jac_verify(int id1,vector<string>& query,vector<int>&candidate,double tau,vector<JaccardJoinResult>&result);
};
#endif
