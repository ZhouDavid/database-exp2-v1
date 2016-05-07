#include "SimJoiner.h"

using namespace std;

SimJoiner::SimJoiner() {
}

SimJoiner::~SimJoiner() {
}

int SimJoiner::joinJaccard(const char *filename1, const char *filename2, double threshold, vector<JaccardJoinResult> &result) {
    result.clear();
    readJacData(string(filename1));
    readJacQuery(string(filename2));
    jac_init(threshold);
    vector<int> candidate;
    for(int i = 0;i<jac_queries.size();i++){
        candidate.clear();
        gen_jac_candidate(jac_queries[i],candidate,threshold);
        verify(i,jac_queries[i],candidate,threshold,result);
    }
    return SUCCESS;
}

int SimJoiner::joinED(const char *filename1, const char *filename2, unsigned threshold, vector<EDJoinResult> &result) {
    result.clear();
    readData(string(filename2),threshold);
    unordered_map<int,LengthGroup>::iterator it;
    for(it = groups.begin();it!=groups.end();it++){
        it->second.gen_lists();
    }
    gen_group_lens();

    readQuery(string(filename1));
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

void SimJoiner::gen_candidate(const string& query,int tau,vector<int>&candidate){
    int len = query.length();
    int llen = len-tau;
    int ulen = len+tau;
    vector<int>::iterator it = lower_bound(group_lens.begin(),group_lens.end(),llen);
    for(;it!=group_lens.end();it++){
        if(*it>ulen) break;
        vector<string> substrs;
        LengthGroup* group = &groups[*it];  //可加速
//        if(switcher1&&*it==5){
//            switcher1 = switcher2 = false;
//        }
        int delta = len - group->len;
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
                }
            }
        }
    }
    sort(candidate.begin(),candidate.end());
    candidate.erase(unique(candidate.begin(),candidate.end()),candidate.end());
}
void SimJoiner::test(string filename1,string filename2){
//    vector<EDJoinResult> resultED;
//    int tau = 3;
//    joinED(filename1.c_str(),filename2.c_str(),tau,resultED);
//    for(int i = 0;i<resultED.size();i++){
//        cout<<resultED[i].id1<<' '<<resultED[i].id2<<' '<<resultED[i].s<<endl;
//    }
//    cout<<"total records number:"<< resultED.size()<<endl;

    vector<JaccardJoinResult> resultJaccard;
    double tau = 0.73;
    joinJaccard(filename1.c_str(),filename2.c_str(),tau,resultJaccard);
    for(int i = 0;i<resultJaccard.size();i++){
        cout<<resultJaccard[i].id1<<' '<<resultJaccard[i].id2<<' '<<resultJaccard[i].s<<endl;
    }
    cout<<"total records number:"<< resultJaccard.size()<<endl;
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

void SimJoiner::gen_substrs(const string& query,int tau,int p,int delta,int id,int len,vector<string>& substrs){
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

void SimJoiner::verify(int id1,const string& query,const vector<int>&candidate,int tau,vector<EDJoinResult>&result){
    for(int i = 0;i<candidate.size();i++){
        int len1 = records[candidate[i]].length();
        int len2 = query.length();
        int ed = -1;
        if(len1<len2){
            ed = calED(records[candidate[i]],query,tau);
        }
        else{
            ed = calED(query,records[candidate[i]],tau);
        }

        if(ed>-1&&ed<=tau){
            EDJoinResult r;
            r.id1 =id1;
            r.id2 =candidate[i];
            r.s = ed;
            result.push_back(r);
        }
    }
}

int SimJoiner::calED(const string& s1,const string& s2,int threshold){
    int row = s1.length();
    int col = s2.length();

    for(int i =0;i<=threshold&&i<=col;i++)d[0][i]=i;
    for(int i = 0;i<=threshold&&i<=row;i++)d[i][0]=i;
    int start;int end;int last_end = threshold<col?threshold:col;
    int count = 0;
    for(int i = 1;i<=row;i++){
        count = 0;
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
        last_end = end;
        if(count == end-start+1) return -1;
    }
    return d[row][col];
}

int SimJoiner::H(const int l,const int s,const double tau){
    return floor((1-tau)/(1+tau)*double(l+s));
}
int SimJoiner::H(const int l,const double tau){
    return floor(((1-tau)/tau)*l);
}

void SimJoiner::readJacQuery(string filename){
    ifstream fin(filename);
    string q;
    vector<string> words;
    while(getline(fin,q)){
        words.clear();
        split(q,' ',words);
        sort(words.begin(),words.end());
        words.erase(unique(words.begin(),words.end()),words.end());
        jac_queries.push_back(words);
        for(int i = 0;i<words.size();i++) U.push_back(words[i]);  //生成universe
    }
    fin.close();
}

void SimJoiner::readJacData(string filename){
    ifstream fin(filename);
    string r;
    vector<string> words;
    while(getline(fin,r)){
        words.clear();
        split(r,' ',words);
        sort(words.begin(),words.end());
        words.erase(unique(words.begin(),words.end()),words.end());
        jac_records.push_back(words);
        for(int i = 0;i<words.size();i++) U.push_back(words[i]); //生成universe
    }
    fin.close();
}

void SimJoiner::split(const string& str,char s,vector<string>&words){
    int len = str.length();
    int start = 0;
    for(int i = 0;i<len;i++){
        if(str[i] == s){
            words.push_back(str.substr(start,i-start));
            while(str[i+1] == s){
                i++;
            }
            start = i+1;
        }
    }
    words.push_back(str.substr(start,len-start));
}

void SimJoiner::jac_init(double tau){
    //U的排序及去重
    sort(U.begin(),U.end());
    U.erase(unique(U.begin(),U.end()),U.end());

    //生成universe序号表
    for(int i = 0;i<U.size();i++){
        universe[U[i]] = i+1;
    }

    //按照长度分组生成groups
    for(int i = 0;i<jac_records.size();i++){
        int size = jac_records[i].size();
        jac_groups[size].size = size;
        double m = jac_groups[size].m = H(size,tau)+1;
        unordered_map<int,string>tmp;
        double wid;
        int gid;
        for(int j = 0;j<jac_records[i].size();j++){
             wid= universe[jac_records[i][j]];
             gid= ceil(wid/m);
             tmp[gid]+=(jac_records[i][j]+" ");
        }
        for(unordered_map<int,string>::iterator it = tmp.begin();it!=tmp.end();it++){
            jac_groups[size].partitions[it->first].invlist[it->second].push_back(i);
        }
    }

    //生成所有的集合大小int数组
    for(unordered_map<int,JacGroup>::iterator it = jac_groups.begin();it!=jac_groups.end();it++){
        group_sizes.push_back(it->first);
    }
    sort(group_sizes.begin(),group_sizes.end());
}

void SimJoiner::gen_jac_candidate(const vector<string>& query,vector<int>& candidate,double tau){
    int lsize = ceil(double(query.size())*tau);  //下界
    int usize = floor(double(query.size())/tau); //上界
    vector<int>::iterator it = lower_bound(group_sizes.begin(),group_sizes.end(),lsize);
    unordered_map<int,string> tmp;
    for(;it!=group_sizes.end();it++){
        tmp.clear();
        if(*it>usize) break;
        double m = H(*it,tau)+1;   //要分成的组数

        for(int j = 0;j<query.size();j++){
            double wid = universe[query[j]];
            int gid = ceil(wid/m);
            tmp[gid]+=query[j]+" ";
        }

        unordered_map<string,vector<int>>* pointer;
        for(unordered_map<int,string>::iterator iit=tmp.begin();iit!=tmp.end();iit++){
            int gid = iit->first;
            pointer = &jac_groups[*it].partitions[gid].invlist;
            if(pointer->find(tmp[gid])!=pointer->end()){
                for(int j = 0;j<(*pointer)[tmp[gid]].size();j++){
                    candidate.push_back((*pointer)[tmp[gid]][j]);
                }
            }
        }
        delete pointer;
    }
    sort(candidate.begin(),candidate.end());
    candidate.erase(unique(candidate.begin(),candidate.end()),candidate.end());
}

void SimJoiner::verify(int id1,vector<string>& query,vector<int>&candidate,double tau,vector<JaccardJoinResult>&result){
    double jac;
    for(int i = 0;i<candidate.size();i++){
        jac = calJaccard(query,jac_records[candidate[i]],tau);
        if(jac>=tau){
            JaccardJoinResult r;
            r.id1 = id1;
            r.id2 = candidate[i];
            r.s = jac;
            result.push_back(r);
        }
    }
}

double SimJoiner::calJaccard(vector<string>& s1,vector<string>&s2,double tau){
    double size1 = s1.size();
    double size2 = s2.size();
    double common = 0;
    for(int i = 0;i<size1;i++){
        vector<string>::iterator found = find(s2.begin(),s2.end(),s1[i]);
        if(found !=s2.end()) common++;
    }
    return common/(size1+size2-common);

}