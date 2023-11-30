#include <bits/stdc++.h>
using namespace std;

string filename;
ofstream fout;
ifstream fin;

int main(){
	cout<<"filename without extension: ";
	cin>>filename;
	string s1,s2;
	s1=filename+".smt2";
	s2=filename+"_norm.smt2";
	fout.open(s2);
	fin.open(s1);
	
	int lines=0;
	//change minus sign into minus operator
	while(true){
		string s;
		fin>>s;
		if(s.length()>1&&s[0]=='-'&&s[1]>='0'&&s[1]<='9'){
			int lnth=s.length();
			fout<<"(- ";
			fout<<s.substr(1,lnth-1);
			fout<<")";
		}else fout<<s;
		if(s.substr(0,6)=="(exit)"){
			fin.close();
			fout.close();
			exit(0);
		}
		char k;
		k=fin.get();
		//change line may not be same in different systems
		if(k=='\n'||k=='r')fout<<endl;
		else fout<<k;
	}
	
	return 0;
}