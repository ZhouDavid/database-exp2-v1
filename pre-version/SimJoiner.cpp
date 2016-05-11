#include "SimJoiner.h"

using namespace std;

SimSearcher::SimSearcher()
{
}

SimSearcher::~SimSearcher()
{
}

int SimSearcher::edCreateIndex(const char *filename, unsigned q)
{
    this->qq = q;
    ifstream fin(filename);
    string line;
    int id = 0;
    if(!fin){cout<<"could not open file!";return FAILURE;}
    //invertList.rehash(BUCKET_COUNT);
    //smin = 1023;
    while(getline(fin,line)){
        edRecords.push_back(line);
        updateList(line,id);
        //int word_size = updateWordList(line,id);
        //word_counter.push_back(word_size);
        //if(smin > word_size)smin = word_size;
        id++;
    }

    edRecordSize = edRecords.size();
//    for(unordered_map<string,vector<int> >::iterator it=invertList.begin();it!=invertList.end();it++){
//        listSize.push_back(pair<string,int>(it->first,it->second.size()));
//    }
//
//    sort(listSize.begin(),listSize.end(),asdf);

//    for(int i = 0;i<listSize.size();i++){
//        position[listSize[i].first] = i;
//    }
    fin.close();
    return SUCCESS;
}

int SimSearcher::jaccardCreateIndex(const char *filename)
{
    ifstream fin(filename);
    string line;
    int id = 0;
    if(!fin){cout<<"could not open file!";return FAILURE;}
    //invertList.rehash(BUCKET_COUNT);
    smin = 1023;
    while(getline(fin,line)){
        jaccardRecords.push_back(line);
        int word_size = updateWordList(line,id);
        word_counter.push_back(word_size);
        if(smin > word_size)smin = word_size;
        id++;
    }

    jaccardRecordSize = jaccardRecords.size();
    fin.close();
    return SUCCESS;
}

int SimSearcher::searchJaccard(const char *query, double threshold, vector<pair<unsigned, double> > &result)
{
    result.clear();
    string q = query;
    vector<string> words;
    split(q,' ',words);
    //words 去重
    sort(words.begin(),words.end());
    words.erase(unique(words.begin(),words.end()),words.end());
    vector<int> candidate;
    int word_size = words.size();
    int T = my_max(threshold*word_size,(smin+word_size)*threshold/(threshold+1));
    scanCount(jaccardRecordSize,this->word_invertList,words,T,candidate);
    sort(candidate.begin(),candidate.end());
    if(T>=1){
        int size = candidate.size();
        for(int i = 0;i<size;i++){
            int id = candidate[i];
            double jaccard = double(counters[id])/double(word_counter[id]+word_size-counters[id]);
            if(jaccard>=threshold) {
                result.push_back(pair<unsigned,double>(id,jaccard));
            }
        }
    }
    else{
        for(int i = 0;i<jaccardRecordSize;i++){
            double jaccard = double(counters[i])/double(word_counter[i]+word_size-counters[i]);
            if(jaccard>=threshold) result.push_back(pair<unsigned,double>(i,jaccard));
        }
    }
    return SUCCESS;
}

int SimSearcher::searchED(const char *query, unsigned threshold, vector<pair<unsigned, unsigned> > &result)
{
    result.clear();
    int thrs = threshold;
    string q = query;

    vector<string> grams;
    splitIntoGram(q,qq,grams);

    int len1 = q.length();
    int T = len1-qq+1-thrs*qq;
    if(T>=1){
        vector<int>candidate;
        scanCount(edRecordSize,this->gram_invertList,grams,T,candidate);
        int size = candidate.size();
        sort(candidate.begin(),candidate.end());
        for(int i = 0;i<size;i++){
            int id = candidate[i];
            int len2 = edRecords[id].length();
            if(abs(len1-len2)>thrs) continue;
            int distance = 0;
            if(len1<len2){
                distance = calED(q,edRecords[id],thrs);
            }
            else{
                distance = calED(edRecords[id],q,thrs);
            }
            if(distance == -1) continue;
            if(distance<=thrs) result.push_back(pair<unsigned,unsigned>(id,distance));
        }
    }
    else{
        for(int i = 0;i<edRecordSize;i++){
            int len2 = edRecords[i].length();
            if(abs(len1-len2)>thrs) continue;
            int distance = 0;
            if(len1<len2){
                distance = calED(q,edRecords[i],thrs);
            }
            else{
                distance = calED(edRecords[i],q,thrs);
            }
            if(distance == -1) continue;
            if(distance<=thrs) result.push_back(pair<unsigned,unsigned>(i,distance));
        }
   }

    return SUCCESS;
}

int SimSearcher::calED(const string& s1,const string& s2,int threshold) {   //s2 must be longer thant s1!
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


int SimSearcher::splitIntoGram(const string& s,int q,vector<string>& result){
    int len = s.length();
    if(len<q) return FAILURE;
    for(int i = 0;i<=len-q;i++){
        result.push_back(string(s,i,q));
    }
    return SUCCESS;
}
void SimSearcher::updateList(const string& s,const int id){
    vector<string>grams;
    if(splitIntoGram(s,qq,grams)==SUCCESS){
        sort(grams.begin(),grams.end());
        grams.erase(unique(grams.begin(),grams.end()),grams.end());

        int gsize = grams.size();
        for(int i = 0;i<gsize;i++){
            gram_invertList[grams[i]].push_back(id);
        }
    }
}

int SimSearcher::updateWordList(const string& s,int id){
    vector<string> words;
    split(s,' ',words);
    sort(words.begin(),words.end());
    words.erase(unique(words.begin(),words.end()),words.end());
    int size = words.size();
    for(int i = 0;i<size;i++){
        word_invertList[words[i]].push_back(id);
    }
    return size;
}


void SimSearcher::scanCount(int size,unordered_map<string,vector<int> >&invertList,const vector<string>& grams,int T,vector<int>& candidate){
    int gsize = grams.size();
    for(int i = 0;i<size;i++) counters[i] = 0;
    unordered_map<string,vector<int> >::iterator it;
    for(int i = 0;i<gsize;i++){
        it = invertList.find(grams[i]);
        if(it!=invertList.end()){
            for(int j =0;j<it->second.size();j++){
                counters[it->second[j]]++;
                if(counters[it->second[j]] == T)
                    candidate.push_back(it->second[j]);
            }
        }
    }
}

void SimSearcher::divideSkip(unordered_map<string,vector<int> >&invertList,const vector<string>& grams,int T,vector<int>& candidate){
    vector<vector<int> > selected;
    for(int i = 0;i<grams.size();i++){
        if(invertList.find(grams[i])!=invertList.end())
            selected.push_back(invertList[grams[i]]);
    }
    sort(selected.begin(),selected.end(),comp);
    vector<vector<int> >::iterator tmp =selected.begin();
    int L = T/2;
    tmp=tmp+L;
    vector<vector<int> >::iterator border = tmp;
    vector<vector<int> >shortLists(border,selected.end());
    unordered_map<int,int> records;  //第一个int 是id，第二个是出现的次数
    mergeSkip(T-L,shortLists,records);
    for(unordered_map<int,int>::iterator it = records.begin();it!=records.end();it++){
        int count = 0;
        for(tmp = selected.begin();tmp!=border;tmp++){
            if(binary_search(tmp->begin(),tmp->end(),it->first)){
                count++;
            }
        }
        if((it->second+count)>=T){candidate.push_back(it->first);}
    }

}

void SimSearcher::mergeSkip(int T,vector<vector<int>>&lists,unordered_map<int,int>&result){
   // cout<<"start"<<endl;
    int len = lists.size();
    vector<vector<int>::iterator >pointers;
    vector<int>popedList;
    vector<pair<int,int> > heap;  // 第一个int代表list的编号，第二个int代表当前首元素id
    for(int i  =0;i<len;i++){
        heap.push_back(pair<int,int>(i,lists[i][0]));
        pointers.push_back(lists[i].begin());    // 初始化 pointer
    }
    make_heap(heap.begin(),heap.end(),heap_comp);//建立最小堆
    while(!heap.empty()){
        popedList.clear();
        int n = 0;
        int t = heap.front().second;
        while(!heap.empty()&&t == heap.front().second ){
            n++;
            popedList.push_back(heap.front().first);
            pop_heap(heap.begin(),heap.end(),heap_comp);
            heap.pop_back();
        }
        if(n>=T){
            result[t] = n;
            for(int j = 0;j<popedList.size();j++){
                int listId = popedList[j];
                pointers[listId]++;
                if(pointers[listId]!=lists[listId].end()){
                    heap.push_back(pair<int,int>(listId,*pointers[listId]));
                    push_heap(heap.begin(),heap.end(),heap_comp);
                }
            }
        }
        else{
            for(int j = 0;!heap.empty()&&j<T-1-n;j++){
                popedList.push_back(heap.front().first);
                pop_heap(heap.begin(),heap.end(),heap_comp);
                heap.pop_back();
            }
            if(!heap.empty()){    //pop之后heap 没空
                int tp = heap.front().second;
                for(int i = 0;i<popedList.size();i++){
                    int listId = popedList[i];
                    pointers[listId] = lower_bound(lists[listId].begin(),lists[listId].end(),tp);
                    if(pointers[listId]!=lists[listId].end()){
                        heap.push_back(pair<int,int>(listId,*pointers[listId]));
                        push_heap(heap.begin(),heap.end(),heap_comp);
                    }
                }
            }
            else{   //pop空了
                for(int i =0;i<popedList.size();i++){
                    int listId = popedList[i];
                    pointers[listId]++;
                    if(pointers[listId]!=lists[listId].end()){
                        heap.push_back(pair<int,int>(listId,*pointers[listId]));
                        push_heap(heap.begin(),heap.end(),heap_comp);
                    }
                }
            }
        }
    }
   // cout<<"end"<<endl;
}

void SimSearcher::split(const string& str,char s,vector<string>&words){
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

void SimSearcher::mergeSkip2(unordered_map<string,vector<int> >&invertList,const vector<string>& grams,int T,vector<int>& candidate){

    vector<vector<int> >lists;
    unordered_map<string,vector<int> >::iterator it;
    for(int i = 0;i<grams.size();i++){
        it=invertList.find(grams[i]);
        if(it!=invertList.end()){
            lists.push_back(it->second);
        }
    }
    sort(lists.begin(),lists.end(),comp);
    int len = lists.size();
    vector<vector<int>::iterator >pointers;
    vector<int>popedList;
    vector<pair<int,int> > heap;  // 第一个int代表list的编号，第二个int代表当前首元素id
    for(int i  =0;i<len;i++){
        pointers.push_back(lists[i].begin());    // 初始化 pointer
        heap.push_back(pair<int,int>(i,*(lists[i].begin())));
    }
    make_heap(heap.begin(),heap.end(),heap_comp);//建立最小堆
    while(!heap.empty()){
        popedList.clear();
        int n = 0;
        int t = heap.front().second;
        while(!heap.empty()&&t == heap.front().second ){
            n++;
            popedList.push_back(heap.front().first);
            pop_heap(heap.begin(),heap.end(),heap_comp);
            heap.pop_back();
        }
        if(n>=T){
            candidate.push_back(t);
            for(int j = 0;j<popedList.size();j++){
                int listId = popedList[j];
                pointers[listId]++;
                if(pointers[listId]!=lists[listId].end()){
                    heap.push_back(pair<int,int>(listId,*pointers[listId]));
                    push_heap(heap.begin(),heap.end(),heap_comp);
                }
            }
        }
        else{
            for(int j = 0;!heap.empty()&&j<T-1-n;j++){
                popedList.push_back(heap.front().first);
                pop_heap(heap.begin(),heap.end(),heap_comp);
                heap.pop_back();
            }
            if(!heap.empty()){    //pop之后heap 没空
                int tp = heap.front().second;
                for(int i = 0;i<popedList.size();i++){
                    int listId = popedList[i];
                    pointers[listId] = lower_bound(lists[listId].begin(),lists[listId].end(),tp);
                    if(pointers[listId]!=lists[listId].end()){
                        heap.push_back(pair<int,int>(listId,*pointers[listId]));
                        push_heap(heap.begin(),heap.end(),heap_comp);
                    }
                }
            }
            else{   //pop空了
                for(int i =0;i<popedList.size();i++){
                    int listId = popedList[i];
                    pointers[listId]++;
                    if(pointers[listId]!=lists[listId].end()){
                        heap.push_back(pair<int,int>(listId,*pointers[listId]));
                        push_heap(heap.begin(),heap.end(),heap_comp);
                    }
                }
            }
        }
    }
}

SimJoiner::SimJoiner(){

}
SimJoiner::~SimJoiner(){

}

int SimJoiner::readEdQuery(const string filename){
    ifstream fin(filename);
    if(!fin){return FAILURE;}
    string query;
    while(getline(fin,query)){
        ed_queries.push_back(query);
    }
    fin.close();
    return SUCCESS;

}

int SimJoiner::readJaccardQuery(const string filename){
    ifstream fin(filename);
    if(!fin){return FAILURE;}
    string query;
    while(getline(fin,query)){
        jaccard_queries.push_back(query);
    }
    fin.close();
    return SUCCESS;
}

int SimJoiner::joinED(const char *filename1, const char *filename2, unsigned threshold, std::vector<EDJoinResult> &result){
    searcher.edCreateIndex(filename2,3);
    readEdQuery(filename1);
    vector<pair<unsigned ,unsigned> > res;
    for(int i  =0;i<ed_queries.size();i++){
        string query = ed_queries[i];
        res.clear();
        searcher.searchED(query.c_str(),threshold,res);
        for(int j = 0;j<res.size();j++){
            EDJoinResult r;
            r.id1 = i;r.id2 = res[j].first;r.s = res[j].second;
            result.push_back(r);
        }
    }

    return SUCCESS;
}

int SimJoiner::joinJaccard(const char *filename1, const char *filename2, double threshold, std::vector<JaccardJoinResult> &result){
    result.clear();
    readJaccardQuery(filename2);
    searcher.jaccardCreateIndex(filename1);
    vector<pair<unsigned,double>>res;
    for(int i = 0;i<jaccard_queries.size();i++){
        searcher.searchJaccard(jaccard_queries[i].c_str(),threshold,res);
        for(int j =0 ;j<res.size();j++){
            JaccardJoinResult r;
            r.id1 = i;r.id2 = res[j].first;r.s = res[j].second;
            result.push_back(r);
        }

    }
    return SUCCESS;
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
    for(int i=0;i<resultJaccard.size();i++){
        cout<<resultJaccard[i].id1<<' '<<resultJaccard[i].id2<<' '<<resultJaccard[i].s<<endl;
    }
    cout<<"total jaccard records number:"<<resultJaccard.size()<<endl;
}