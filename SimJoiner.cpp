#include "SimJoiner.h"

using namespace std;

SimJoiner::SimJoiner() {
}

SimJoiner::~SimJoiner() {
}

int SimJoiner::joinJaccard(const char *filename1, const char *filename2, double threshold, vector<JaccardJoinResult> &result) {
    result.clear();
    return SUCCESS;
}

int SimJoiner::joinED(const char *filename1, const char *filename2, unsigned threshold, vector<EDJoinResult> &result) {
    result.clear();
    readData(string(filename1),threshold);
    unordered_map<int,LengthGroup>::iterator it;
    for(it = groups.begin();it!=groups.end();it++){
        it->second.gen_lists();
    }
    gen_group_lens();

    readQuery(string(filename2));
    vector<int>candidate;
    for(int i = 0;i<queries.size();i++){
        string query = queries[i].str;
        candidate.clear();
        gen_candidate(query,threshold,candidate);
        verify(i,query,candidate,threshold,result);
    }
    return SUCCESS;
}
void SimJoiner::readData(string filename,int tau){
    ifstream fin(filename);
    if(!fin){cout<<"open file failed!"<<endl; return;}
    string r;
    int i = 0;
    while(getline(fin,r)){
        this->records.push_back(r);
        int len = r.length();
        groups[len].len = len;
        groups[len].tau = tau;
        groups[len].records.push_back(Record(i++,r));
    }
    fin.close();
}

void SimJoiner::gen_candidate(string query,int tau,vector<int>&candidate){
    int len = query.length();
    int llen = len-tau;
    int ulen = len+tau;
    vector<int>::iterator it = lower_bound(group_lens.begin(),group_lens.end(),llen);
    for(;it!=group_lens.end();it++){
        if(*it>ulen) break;
        vector<string> substrs;
        LengthGroup* group = &groups[*it];  //可加速
        int delta = abs(len - group->len);
        for(int i = 0;i<group->lists.size();i++){
            int p = group->lists[i].start_pos;
            int id = group->lists[i].id;
            gen_substrs(query,tau,p,delta,id,group->s[i],substrs);
        }
        for(int j = 0;j<substrs.size();j++){
            for(int i = 0;i<group->lists.size();i++){
                if(group->lists[i].invlist.find(substrs[j])!=group->lists[i].invlist.end()){
                    vector<int>::iterator pointer = group->lists[i].invlist[substrs[j]].begin();
                    for(;pointer!=group->lists[i].invlist[substrs[j]].end();pointer++){
                        candidate.push_back(*pointer);
                    }
                    break;
                }
            }
        }
    }
    sort(candidate.begin(),candidate.end());
    candidate.erase(unique(candidate.begin(),candidate.end()),candidate.end());
}
void SimJoiner::test(string filename1,string filename2){
    vector<EDJoinResult> resultED;
    int tau = 3;
    joinED(filename1.c_str(),filename2.c_str(),tau,resultED);
//    for(int i = 0;i<resultED.size();i++){
//        cout<<resultED[i].id1<<' '<<resultED[i].id2<<' '<<resultED[i].s<<endl;
//    }
}

void SimJoiner::gen_group_lens() {
    for(unordered_map<int,LengthGroup>::iterator m = groups.begin();m!=groups.end();m++) {
        group_lens.push_back(m->first);
    }
    sort(group_lens.begin(),group_lens.end());
}

void SimJoiner::readQuery(string filename){
    string q;
    int i = 0;
    ifstream fin(filename);
    if(!fin){cout<<"cannot open query file!"<<endl;return;}
    while(getline(fin,q)){
        queries.push_back(Record(i++,q));
    }
}

void SimJoiner::gen_substrs(string& query,int tau,int p,int delta,int id,int len,vector<string>& substrs){
    //采用multi-match策略
    int t1= id-1;
    int t2 = tau-t1;
    int t3 = p+delta;
    int start = max(p-t1,t3-t2);
    int end = min(p+t1,t3+t2);
    if(start<0)start = 0;
    for(int i = start;i<=end;i++){
        if(i+len>query.size()) break;
        substrs.push_back(query.substr(i,len));
    }
}

void SimJoiner::verify(int id2,const string query,const vector<int>&candidate,int tau,vector<EDJoinResult>&result){
    for(int i = 0;i<candidate.size();i++){
        int len1 = records[candidate[i]].length();
        int len2 = query.length();
        int ed = len1<len2? calED(records[candidate[i]],query,tau):calED(query,records[candidate[i]],tau);
        if(ed>-1&&ed<=tau){
            EDJoinResult r;
            r.id1 =candidate[i];
            r.id2 = id2;
            r.s = ed;
            result.push_back(r);
        }
    }
}

int SimJoiner::calED(const string s1,const string s2,int threshold){
    int row = s1.length();
    int col = s2.length();

    for(int i =0;i<=threshold&&i<=col;i++)d[0][i]=i;
    for(int i = 0;i<=threshold&&i<=row;i++)d[i][0]=i;
    int start;int end;int last_end;
    int count = 0;
    for(int i = 1;i<=row;i++){
        count = 0;
        last_end = end;
        start = (i-threshold>1)?(i-threshold):1;
        end = (i+threshold)<col?(i+threshold):col;
        for(int j = start;j<=end;j++){
            if(j == start){
                int cost = (s1[i-1]==s2[j-1])? 0:1;
                int insertion = d[i-1][j]+1;
                int match = d[i-1][j-1]+cost;
                d[i][j] = min(insertion,match);
                if(d[i][j]>threshold) count++;
            }
            else if(j == end){
                if(last_end == col){
                    int cost = s1[i-1]==s2[j-1]? 0:1;
                    int insertion = d[i-1][j]+1;
                    int deletion = d[i][j-1]+1;
                    int match = d[i-1][j-1]+cost;
                    d[i][j] = min3(deletion,insertion,match);
                    if(d[i][j]>threshold) count++;
                }
                else{
                    int cost = s1[i-1]==s2[j-1]? 0:1;
                    int deletion = d[i][j-1]+1;
                    int match = d[i-1][j-1]+cost;
                    d[i][j] = min(deletion,match);
                    if(d[i][j]>threshold) count++;
                }

            }
            else{
                int cost = s1[i-1]==s2[j-1]? 0:1;
                int insertion = d[i-1][j]+1;
                int deletion = d[i][j-1]+1;
                int match = d[i-1][j-1]+cost;
                d[i][j] = min3(deletion,insertion,match);
                if(d[i][j]>threshold) count++;
            }
        }
        if(count == end-start+1) return -1;
    }
    return d[row][col];
}