#include <bits/stdc++.h>

using namespace std;
string filename;
ofstream fout;
ifstream fin;
string objsense;
int varnum,intnum,cons_num,bcons_num,sol_num,der_num;
vector<string>var,var_type;
vector<string>obj;
vector<string>cons_0;
vector<string>cons_type,cons_rhs,cons_name;// -1.0 is <=,0.0 is =, 1.0 is >=, 2.0 is impossible
vector<set<int>>sols;
vector<string>sol_name;
vector<string>der_name;
vector<set<int>>assumps,coefficients;
set<int>objcoe,allasum;
map<int,vector<int>>del_as;
int domind=0;

string s;
int ind;
int num;
bool feas=true;
string lb="-inf";
string ub="inf";

//split a line of string into words (to me the most difficult part)
vector<string> Stringsplit(string str)
{
	int start_pos=0;
	int word_lnth=1;
	

	vector<string> toreturn;
	if(str.length()==0)return toreturn;
	for(int i=0;i<str.length();i++){
		if(i==str.length()-1&&(str[i]==' '||str[i]=='\n'||str[i]=='	'||str[i]=='\0'))return toreturn;
		if(str[i]==' '||str[i]=='\n'||str[i]=='	'||str[i]=='\0')continue;
		start_pos=i;

		break;
	}
	while(true){
		
		for(;word_lnth+start_pos-1<str.length();word_lnth++){

			if(start_pos+word_lnth==str.length()){
				
				toreturn.push_back(str.substr(start_pos,word_lnth));
				return toreturn;
			}
			if(str[start_pos+word_lnth]==' '||str[start_pos+word_lnth]=='\n'||str[start_pos+word_lnth]=='	'||str[start_pos+word_lnth]=='\0'){
				
				toreturn.push_back(str.substr(start_pos,word_lnth));
				start_pos+=word_lnth;
				word_lnth=0;
				while(start_pos<str.length()&&(str[start_pos]==' '||str[start_pos]=='\n'||str[start_pos]=='	'||str[start_pos]=='\0')){
					start_pos++;
					if(start_pos==str.length())return toreturn;
				}
			}
		}
	}
	return toreturn;
}

//check if a number is real type
bool is_real(string num){
	int lnth=num.length();
	bool ans=false;
	for(int i=0;i<lnth;i++){
		if(num[i]=='.')ans=true;
		if(num[i]=='e')ans=true;
		if(num[i]=='E')ans=true;
		if(num[i]=='/')ans=true;
	}
	return ans;
}
string to_real2(string num){
	if(is_real(num)){
		if(num[0]=='.')return "0"+num;
		if(num[0]=='-'&&num[1]=='.')return "-0"+num.substr(1,num.length()-1);
		return num;
	}else return num+".0";
	
	return num;
}
//transfer scientific notation
string drop_Ep(string s){
	if(s[0]=='(')return s;
	int lnth=s.length();
	for(int i=0;i<lnth;i++)if(s[i]=='E'){
		string s1,s2;
		s1=s.substr(0,i);
		s2=s.substr(i+1,lnth-i-1);
		int p=stoi(s2);
		string toreturn;
		if(p==0)return s1;
		if(p>0)toreturn="(* ";
		else {toreturn="(/ ";p*=-1;}
		toreturn+=to_real2(s1)+" 1";
		for(int j=0;j<p;j++)toreturn+="0";
		toreturn+=".0)";
		return toreturn;
	}
	for(int i=0;i<lnth;i++)if(s[i]=='/')return "(/ "+s.substr(0,i)+" "+s.substr(i+1,lnth-i-1)+")";
	return s;
}

//change a number into standard real form
string to_real(string num){
	num=drop_Ep(num);
	if(is_real(num)){
		if(num[0]=='.')return "0"+num;
		if(num[0]=='-'&&num[1]=='.')return "-0"+num.substr(1,num.length()-1);
		return num;
	}else return num+".0";
	
	return num;
}

//cons1 dominate cons2; 
void dom_cons(set<int>a1,string sense1,string b1,string pre1,set<int>a2,string sense2,string b2,string pre2){
	fout<<"(assert (or (and";
	for(int k:a1)fout<<" (= "<<pre1<<k<<" 0.0)";
	fout<<" (=> (= "<<sense1<<" -1.0) (< "<<b1<<" 0.0))";
	fout<<" (=> (= "<<sense1<<" 1.0) (> "<<b1<<" 0.0))";
	fout<<" (=> (= "<<sense1<<" 0.0) (not (= "<<b1<<" 0.0))))";
	fout<<" (and";
	for(int k:a1){if(a2.find(k)==a2.end())fout<<" (= "<<pre1<<k<<" 0.0)";}
	for(int k:a2){if(a1.find(k)==a1.end())fout<<" (= "<<pre2<<k<<" 0.0)";}
	for(int k:a1){if(a2.find(k)!=a2.end())fout<<" (= "<<pre1<<k<<" "<<pre2<<k<<")";}
	fout<<" (or";
	fout<<" (and (= "<<b1<<" "<<b2<<") (= "<<sense1<<" 0.0) (= "<<sense2<<" 0.0))";
	fout<<" (and (<= "<<b1<<" "<<b2<<") (= "<<sense2<<" -1.0) (<= "<<sense1<<" 0.0))";
	fout<<" (and (>= "<<b1<<" "<<b2<<") (= "<<sense2<<" 1.0) (>= "<<sense1<<" 0.0))))))"<<endl;
}

//check last constraint has no assumption; free space for no longer used assumps; add largest index
void delete_assum(int i,string curs){
	if(i==cons_num+der_num-1){
		if(assumps[i].size()!=0){cout<<"error: last constraint has assumption"<<endl;exit(0);}
	}
	if(del_as.find(i)!=del_as.end()){
		for(int k:del_as.at(i)){assumps[k].clear();coefficients[k].clear();}
		del_as.erase(i);
	}
	if(curs!="-1"){
		int j=stoi(curs);
		if(del_as.find(j)==del_as.end()){vector<int>ve;del_as[j]=ve;}
		del_as[j].push_back(i);
	}
}
//input DER section
void input_DER(){
	cout<<"DER"<<endl;
	fin>>s;
	if(s!="DER"){cout<<"error in DER"<<endl;exit(0);}
	fin>>der_num;
	for(int i=cons_num;i<cons_num+der_num;i++){
		fin>>s;
		cons_name.push_back(s);
		fin>>s;
		if(s=="L")cons_type.push_back("-1.0");
		if(s=="E")cons_type.push_back("0.0");
		if(s=="G")cons_type.push_back("1.0");
		fout<<"(declare-fun cs"<<i<<" () Real)"<<endl;
		
		fout<<"(assert (= cs"<<i<<" "<<cons_type[i]<<"))"<<endl;
		fin>>s;
		cons_rhs.push_back(to_real(s));
		fout<<"(declare-fun crhs"<<i<<" () Real)"<<endl;
		fout<<"(assert (= crhs"<<i<<" "<<cons_rhs[i]<<"))"<<endl;
		fin>>s;
		set<int>new_coe;
		coefficients.push_back(new_coe);
		if(s=="OBJ"){
			coefficients[i]=objcoe;
			for(int j:objcoe){
				fout<<"(declare-fun c"<<i<<"x"<<j<<" () Real)"<<endl;
				fout<<"(assert (= c"<<i<<"x"<<j<<" obj"<<j<<"))"<<endl;
			}
		}else{
			num=stoi(s);
			for(int j=0;j<num;j++){
				fin>>ind>>s;
				coefficients[i].insert(ind);
				fout<<"(declare-fun c"<<i<<"x"<<ind<<" () Real)"<<endl;
				fout<<"(assert (= c"<<i<<"x"<<ind<<" "<<to_real(s)<<"))"<<endl;
			}
		}

		
		fin>>s;
		if(s!="{"){cout<<"error in DER"<<endl;exit(0);}
		fin>>s;
		if(i==cons_num+der_num-1){// to be changed
			if(feas){
				if(objsense=="min"){
					dom_cons(coefficients[i],"cs"+to_string(i),"crhs"+to_string(i),"c"+to_string(i)+"x",objcoe,"1.0",to_real(lb),"obj");
				}else dom_cons(coefficients[i],"cs"+to_string(i),"crhs"+to_string(i),"c"+to_string(i)+"x",objcoe,"-1.0",to_real(ub),"obj");
			}else {
				fout<<"(assert (and";
				for(int k:coefficients[i])fout<<" (= "<<"c"+to_string(i)+"x"<<k<<" 0.0)";
				fout<<" (=> (= "<<"cs"+to_string(i)<<" -1.0) (< "<<"crhs"+to_string(i)<<" 0.0))";
				fout<<" (=> (= "<<"cs"+to_string(i)<<" 1.0) (> "<<"crhs"+to_string(i)<<" 0.0))";
				fout<<" (=> (= "<<"cs"+to_string(i)<<" 0.0) (not (= "<<"crhs"+to_string(i)<<" 0.0)))))"<<endl;
			}
			//we may need more things, if obsurdity dominate the bound in feasible circumstance(if exist solution then fine.)
		}
		set<int>new_as;
		if(s=="asm"){
			fin>>s;
			
			new_as.insert(i);
			assumps.push_back(new_as);
			allasum.insert(i);
			fin>>s;
			delete_assum(i,s);
			continue;
		}
		if(s=="lin"||s=="rnd"){
			bool is_rnd=false;
			if(s=="rnd")is_rnd=true;
			set<int>rea_coe;
			map<int,string>mrind;
			map<int,set<int>>coe_map;
			fin>>num;
			if(num<=0){cout<<"error in lin or rnd"<<endl;exit(0);}
			for(int j=0;j<num;j++){
				fin>>ind>>s;
				for(int k:assumps[ind])new_as.insert(k);
				set<int>new_coe1;
				for(int k:coefficients[ind])coe_map.insert(make_pair(k,new_coe1));
				mrind.insert(make_pair(ind,to_real(s)));
				fout<<"(assert (< "<<ind<<".0 "<<i<<".0))"<<endl;//
			}
			for(pair<int,string>pa:mrind)for(int k:coefficients[pa.first])coe_map[k].insert(pa.first);
			fin>>s;
			fin>>s;
			assumps.push_back(new_as);
			
			for(pair<int,set<int>>pa:coe_map){
				set<int>new_coe1=pa.second;
				int j=pa.first;
				rea_coe.insert(j);
				num=new_coe1.size();
				if(num<=0){cout<<"error in lin or rnd"<<endl;exit(0);}
				fout<<"(declare-fun r"<<i<<"x"<<j<<" () Real)"<<endl;
				fout<<"(assert (= r"<<i<<"x"<<j<<" ";
				if(num==1)fout<<"(* "<<mrind[*new_coe1.begin()]<<" c"<<*new_coe1.begin()<<"x"<<j<<")))"<<endl;
				if(num>1){
					fout<<"(+";
					for(int k:new_coe1)fout<<" (* "<<mrind[k]<<" c"<<k<<"x"<<j<<")";
					fout<<")))"<<endl;
				}
				if(is_rnd){
					fout<<"(declare-fun rndr"<<i<<"x"<<j<<" () Int)"<<endl;
					fout<<"(assert (ite is_intx"<<j<<" (= (to_real rndr"<<i<<"x"<<j<<") r"<<i<<"x"<<j<<") (= r"<<i<<"x"<<j<<" 0.0)))"<<endl;
					//do we need to include other constraints to make it faster?
				}
			}
			
			fout<<"(declare-fun beta"<<i<<" () Real)"<<endl;
			fout<<"(assert (= beta"<<i<<" ";
			num=mrind.size();
			if(num<=0){cout<<"error in mrind"<<endl;exit(0);}
			if(num==1)fout<<"(* "<<(*mrind.begin()).second<<" crhs"<<(*mrind.begin()).first<<")";
			if(num>1){
				fout<<"(+";
				for(pair<int,string>pa:mrind)fout<<" (* "<<pa.second<<" crhs"<<pa.first<<")";
				fout<<")";
			}
			fout<<"))"<<endl;
			fout<<"(declare-fun cleq"<<i<<" () Bool)"<<endl;
			fout<<"(declare-fun cgeq"<<i<<" () Bool)"<<endl;
			fout<<"(declare-fun ceq"<<i<<" () Bool)"<<endl;
			if(num==1){
				fout<<"(assert (= ceq"<<i<<" (= (* "<<(*mrind.begin()).second<<" cs"<<(*mrind.begin()).first<<") 0.0)))"<<endl;
				fout<<"(assert (= cleq"<<i<<" (<= (* "<<(*mrind.begin()).second<<" cs"<<(*mrind.begin()).first<<") 0.0)))"<<endl;
				fout<<"(assert (= cgeq"<<i<<" (>= (* "<<(*mrind.begin()).second<<" cs"<<(*mrind.begin()).first<<") 0.0)))"<<endl;
			}
			if(num>1){
				fout<<"(assert (= ceq"<<i<<" (and";
				for(pair<int,string>pa:mrind)fout<<" (= (* "<<pa.second<<" cs"<<pa.first<<") 0.0)";
				fout<<")))"<<endl;
				
				fout<<"(assert (= cleq"<<i<<" (and";
				for(pair<int,string>pa:mrind)fout<<" (<= (* "<<pa.second<<" cs"<<pa.first<<") 0.0)";
				fout<<")))"<<endl;
				
				fout<<"(assert (= cgeq"<<i<<" (and";
				for(pair<int,string>pa:mrind)fout<<" (>= (* "<<pa.second<<" cs"<<pa.first<<") 0.0)";
				fout<<")))"<<endl;
				
			}
			fout<<"(declare-fun rs"<<i<<" () Real)"<<endl;
			fout<<"(assert (ite ceq"<<i<<" (= rs"<<i<<" 0.0) (ite cleq"<<i<<" (= rs"<<i<<" -1.0) (ite cgeq"<<i<<" (= rs"<<i<<" 1.0) xfalse))))"<<endl;
			if(is_rnd){
				fout<<"(assert (or (= rs"<<i<<" -1.0) (= rs"<<i<<" 1.0)))"<<endl;
				fout<<"(declare-fun rndbeta"<<i<<" () Int)"<<endl;
				fout<<"(assert (ite (= rs"<<i<<" -1.0) (and (<= (to_real rndbeta"<<i<<") beta"<<i<<") (> (to_real (+ rndbeta"<<i<<" 1)) beta"<<i<<")) (and (>= (to_real rndbeta"<<i<<") beta"<<i<<") (< (to_real (- rndbeta"<<i<<" 1)) beta"<<i<<"))))"<<endl;
				fout<<"(declare-fun brndbeta"<<i<<" () Real)"<<endl;
				fout<<"(assert (= brndbeta"<<i<<" (to_real rndbeta"<<i<<")))"<<endl;
			}
			if(is_rnd)dom_cons(rea_coe,"rs"+to_string(i),"brndbeta"+to_string(i),"r"+to_string(i)+"x",coefficients[i],"cs"+to_string(i),"crhs"+to_string(i),"c"+to_string(i)+"x");
			else dom_cons(rea_coe,"rs"+to_string(i),"beta"+to_string(i),"r"+to_string(i)+"x",coefficients[i],"cs"+to_string(i),"crhs"+to_string(i),"c"+to_string(i)+"x");
			delete_assum(i,s);
			continue;
		}
		if(s=="uns"){
			int i1,l1,i2,l2;
			fin>>i1>>l1>>i2>>l2;
			fin>>s>>s;
			if(allasum.find(l1)==allasum.end()||allasum.find(l2)==allasum.end()){cout<<"error in uns"<<endl;exit(0);}
			for(int k:assumps[i1]){
				if(k!=l1)new_as.insert(k);
			}
			for(int k:assumps[i2]){
				if(k!=l2)new_as.insert(k);
			}
			assumps.push_back(new_as);
			
			vector<string>a1;
			fout<<"(assert (< "<<i1<<".0 "<<i<<".0))"<<endl;
			fout<<"(assert (< "<<l1<<".0 "<<i<<".0))"<<endl;
			fout<<"(assert (< "<<i2<<".0 "<<i<<".0))"<<endl;
			fout<<"(assert (< "<<l1<<".0 "<<i<<".0))"<<endl;//to avoid strange error
			dom_cons(coefficients[i1],"cs"+to_string(i1),"crhs"+to_string(i1),"c"+to_string(i1)+"x",coefficients[i],"cs"+to_string(i),"crhs"+to_string(i),"c"+to_string(i)+"x");
			dom_cons(coefficients[i2],"cs"+to_string(i2),"crhs"+to_string(i2),"c"+to_string(i2)+"x",coefficients[i],"cs"+to_string(i),"crhs"+to_string(i),"c"+to_string(i)+"x");
			for(int j:coefficients[l1]){
				if(coefficients[l2].find(j)==coefficients[l2].end())fout<<"(assert (= c"<<l1<<"x"<<j<<" 0.0))"<<endl;
				else {
					fout<<"(assert (= c"<<l1<<"x"<<j<<" c"<<l2<<"x"<<j<<"))"<<endl;
					fout<<"(declare-fun rndr"<<i<<"x"<<j<<" () Int)"<<endl;
				}	fout<<"(assert (ite is_intx"<<j<<" (= (to_real rndr"<<i<<"x"<<j<<" ) c"<<l1<<"x"<<j<<") (= c"<<l1<<"x"<<j<<" 0.0)))"<<endl;
			}
			for(int j:coefficients[l2]){if(coefficients[l1].find(j)==coefficients[l1].end())fout<<"(assert (= c"<<l2<<"x"<<j<<" 0.0))"<<endl;}
			fout<<"(declare-fun rndbeta"<<i<<" () Int)"<<endl;
			fout<<"(assert (or (and (= cs"<<l1<<" -1.0) (= cs"<<l2<<" 1.0) (= (to_real rndbeta"<<i<<") crhs"<<l1<<") (= (+ crhs"<<l1<<" 1.0) crhs"<<l2<<")) (and (= cs"<<l2<<" -1.0) (= cs"<<l1<<" 1.0) (= (to_real rndbeta"<<i<<") crhs"<<l2<<") (= (+ crhs"<<l2<<" 1.0) crhs"<<l1<<"))))"<<endl;
			delete_assum(i,s);
		
		
		}
				
	}

	
}
//input SOL section
void input_SOL(){
	cout<<"SOL"<<endl;
	fin>>s;
	if(s!="SOL"){cout<<"error in SOL"<<endl;exit(0);}
	fin>>sol_num;
	for(int i=0;i<sol_num;i++){
		fin>>s;
		sol_name.push_back(s);
		set<int>new_coe;
		fin>>num;
		for(int j=0;j<num;j++){
			fin>>ind>>s;
			new_coe.insert(ind);
			fout<<"(declare-fun sol"<<i<<"x"<<ind<<" () Real)"<<endl;
			fout<<"(assert (= sol"<<i<<"x"<<ind<<" "<<to_real(s)<<"))"<<endl;
		}
		sols.push_back(new_coe);
		for(int j=0;j<cons_num;j++){
			fout<<"(assert (";
			if(cons_type[j]=="0.0")fout<<"= ";
			if(cons_type[j]=="-1.0")fout<<"<= ";
			if(cons_type[j]=="1.0")fout<<">= ";
			set<int>com_coe;
			for(int k:new_coe){if(coefficients[j].find(k)!=coefficients[j].end())com_coe.insert(k);}
			if(com_coe.size()==0)fout<<"0.0 ";
			if(com_coe.size()==1)fout<<"(* c"<<j<<"x"<<*com_coe.begin()<<" sol"<<i<<"x"<<*com_coe.begin()<<") ";
			if(com_coe.size()>1){
				fout<<"(+";
				for(int k:com_coe)fout<<" (* c"<<j<<"x"<<k<<" sol"<<i<<"x"<<k<<")";
				fout<<") ";
			}
			fout<<"crhs"<<j<<"))"<<endl;
		}
	}
	if(feas){
		if(sol_num==0){cout<<"error in SOL"<<endl;exit(0);}
		fout<<"(assert (";
		if(sol_num==1){
			if(objsense=="max")fout<<">= ";
			else fout<<"<= ";
			set<int>com_coe;
			for(int k:sols[0]){if(objcoe.find(k)!=objcoe.end())com_coe.insert(k);}
			if(com_coe.size()==0)fout<<"0.0 ";
			if(com_coe.size()==1)fout<<"(* obj"<<*com_coe.begin()<<" sol"<<0<<"x"<<*com_coe.begin()<<") ";
			if(com_coe.size()>1){
				fout<<"(+";
				for(int k:com_coe)fout<<" (* obj"<<k<<" sol"<<0<<"x"<<k<<")";
				fout<<") ";
			}
			if(objsense=="max")fout<<to_real(lb)<<"))"<<endl;
			else fout<<to_real(ub)<<"))"<<endl;
		}else{
			fout<<"or";
			for(int j=0;j<sol_num;j++){
				fout<<" (";
				if(objsense=="max")fout<<">= ";
				else fout<<"<= ";
				set<int>com_coe;
				for(int k:sols[j]){if(objcoe.find(k)!=objcoe.end())com_coe.insert(k);}
				if(com_coe.size()==0)fout<<"0.0 ";
				if(com_coe.size()==1)fout<<"(* obj"<<*com_coe.begin()<<" sol"<<j<<"x"<<*com_coe.begin()<<") ";
				if(com_coe.size()>1){
					fout<<"(+";
					for(int k:com_coe)fout<<" (* obj"<<k<<" sol"<<j<<"x"<<k<<")";
					fout<<") ";
				}
				if(objsense=="max")fout<<to_real(lb)<<")";
				else fout<<to_real(ub)<<")";
			}
			fout<<"))"<<endl;
		}		

	} else {
		if(sol_num!=0){cout<<"error in SOL"<<endl;exit(0);}
	}
	input_DER();


}
//input RTP section
void input_RTP(){
	cout<<"RTP"<<endl;
	fin>>s;
	if(s!="RTP"){cout<<"error in RTP"<<endl;exit(0);}
	fin>>s;
	if(s=="infeas"){
		feas=false;
		input_SOL();
	}else{
		fin>>lb>>ub;
		if((objsense=="max"&&ub=="inf")||(objsense=="min"&&lb=="-inf")){cout<<"error in rtp"<<endl;exit(0);}
		//need more analysis. what if there is not upperbound? I think the certificate cannot give that. maybe 
		//check the solution set and end?
		input_SOL();
	}	
}
//input CON section
void input_cons(){
	fin>>cons_num>>bcons_num;
	cout<<cons_num<<" "<<varnum<<" "<<bcons_num<<endl;
	for(int i=0;i<cons_num;i++){
		set<int>new_as;
		set<int>new_coe;
		assumps.push_back(new_as);
		coefficients.push_back(new_coe);
		fin>>s;
		cons_name.push_back(s);
		cout<<s<<endl;
		fin>>s;
		if(s=="L")cons_type.push_back("-1.0");
		if(s=="E")cons_type.push_back("0.0");
		if(s=="G")cons_type.push_back("1.0");
		//sign of a constraint
		fout<<"(declare-fun cs"<<i<<" () Real)"<<endl;
		fout<<"(assert (= cs"<<i<<" "<<cons_type[i]<<"))"<<endl;
		fin>>s;
		cons_rhs.push_back(to_real(s));
		fout<<"(declare-fun crhs"<<i<<" () Real)"<<endl;
		fout<<"(assert (= crhs"<<i<<" "<<cons_rhs[i]<<"))"<<endl;
		fin>>s;
		if(s=="OBJ"){
			coefficients[i]=objcoe;
			for(int j:objcoe){
				fout<<"(declare-fun c"<<i<<"x"<<j<<" () Real)"<<endl;
				fout<<"(assert (= c"<<i<<"x"<<j<<" obj"<<j<<"))"<<endl;
			}
		}else{
			num=stoi(s);
			for(int j=0;j<num;j++){
				fin>>ind>>s;
				coefficients[i].insert(ind);
				fout<<"(declare-fun c"<<i<<"x"<<ind<<" () Real)"<<endl;
				fout<<"(assert (= c"<<i<<"x"<<ind<<" "<<to_real(s)<<"))"<<endl;
			}
		}
	}
	input_RTP();
}
int main(){

	cout<<"Please type in the name of certificate file, without extension: ";
	cin>>filename;
	cout<<endl;
	string s1,s2;
	s1=filename+".vipr";
	s2=filename+"_vipr.smt2";
	fout.open(s2);
	fin.open(s1);
	fout<<"(set-info :smt-lib-version 2.6)"<<endl;
	fout<<"(set-logic ALL)"<<endl;
	fout<<"(set-info :sourse |Transformed from an VIPR format problem|)"<<endl;
	fout<<"; --- END HEADER ---"<<endl;
	fout<<endl;
	fout<<"(declare-fun xfalse () Bool)"<<endl;
	fout<<"(assert (not xfalse))"<<endl;
	while(true){
		char string_line[100000];
		char string_line2[100000];
		fin.getline(string_line,100000);
		vector<string>words_of_line=Stringsplit(string_line);
		if(string_line[0]=='%')continue;
		if(words_of_line[0]=="VER")continue;
		if(words_of_line[0]=="VAR"){
			varnum=stoi(words_of_line[1]);
			if(varnum==0){cout<<"no variable"<<endl;exit(0);}
			for(int i=0;i<varnum;i++){
				fin>>s;
				var.push_back(s);
				var_type.push_back("Real");
				cons_0.push_back("0.0");
			}
			obj=cons_0;
			fin>>s;
			if(s!="INT"){
				cout<<"error in INT"<<endl;
				exit(0);
			}
			fin>>intnum;
			for(int i=0;i<intnum;i++){
				
				fin>>ind;
				var_type[ind]="Int";
			}
			for(int i=0;i<varnum;i++){
				fout<<"(declare-fun x"<<i<<" () "<<var_type[i]<<")"<<endl;
				fout<<"(declare-fun is_intx"<<i<<" () Bool)"<<endl;
				if(var_type[i]=="Int")fout<<"(assert is_intx"<<i<<")"<<endl;
				else fout<<"(assert (not is_intx"<<i<<"))"<<endl;
			}
				
			fin>>s;
			if(s!="OBJ"){
				cout<<"error in OBJ"<<endl;
				exit(0);
			}
			fin>>objsense;
			fin>>num;
			for(int i=0;i<num;i++){
				fin>>ind>>s;
				objcoe.insert(ind);
				fout<<"(declare-fun obj"<<ind<<" () Real)"<<endl;
				fout<<"(assert (= obj"<<ind<<" "<<to_real(s)<<"))"<<endl;
			}
			fin>>s;
			if(s!="CON"){
				cout<<"error in CON"<<endl;
				exit(0);
			}
			cout<<"cons"<<endl;
			ios::sync_with_stdio(false);
			input_cons();
			fout<<"(check-sat)"<<endl;
			fout<<"(exit)"<<endl;
			fout.close();
			return 0;
		}
	}
	
	return 0;
}
