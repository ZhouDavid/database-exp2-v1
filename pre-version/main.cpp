#include "SimJoiner.h"
#include <fstream>
#include <iostream>
#include <string>
#include <time.h>
using namespace std;

int main(int argc, char **argv)
{
    clock_t start,end;
    SimJoiner joiner;

//    vector<EDJoinResult> resultED;
//    vector<JaccardJoinResult> resultJaccard;
//
//    unsigned edThreshold = 2;
//    double jaccardThreshold = 0.85;
//
//    joiner.joinJaccard(argv[1], argv[2], jaccardThreshold, resultJaccard);
//    joiner.joinED(argv[1], argv[2], edThreshold, resultED);
    freopen("E:\\semester\\dabase-training\\hw2\\test2\\result\\pre-result.txt","w",stdout);
    start = clock();
    joiner.test(string(argv[1]),string(argv[2]));
    end = clock();
    cout<<"run time:"<<(double)(end-start)/CLOCKS_PER_SEC<<'s'<<endl;
    return 0;
}

