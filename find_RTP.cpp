#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <fstream>
#include <cmath>
using namespace std;

string filename;
ifstream fin;

int main(){
	string s;
	cout<<"filename without extension: "<<endl;
	cin>>s;
	s+=".vipr";
	fin.open(s);
	while(true){
		fin>>s;
		if(s=="OBJ"){
			fin>>s;
			cout<<"problem type: "<<s<<endl;
			break;
		}
	}
	while(true){
		fin>>s;
		if(s=="RTP"){
			fin>>s;
			cout<<s<<endl;
			if(s!="range")return 0;
			fin>>s;
			cout<<"lower bound: "<<s<<endl;
			fin>>s;
			cout<<"upper bound: "<<s<<endl;
			break;
		}
	}
	return 0;
}
