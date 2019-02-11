// Author: Shrinidhi Udupi (shrinidhiku@gmail.com)
// Last updated : 26.07.2017

// This file contains the ternary simulation semantics and the Observability propagation models
// for the the library cells of the 45nm Nangate Open Cell Library

#include <iostream>
#include <design_model.h>
#include <cstdarg>

ostream& operator<<(ostream& os, value a){
    string tmp;
    if(a==one) tmp= "1";
   else  if(a==zero) tmp= "0";
   else  if(a==ex)  tmp= "X";
    else tmp= "U";
    os << tmp ;
   return os;
}
value to_value(string a){
    if(a=="1") return one;
    if(a=="0") return zero;
    if(a=="X") return ex;
    else { cout<< "returning undefined!" << endl ; 
	return un;
    }
}

value operator!(value a){
    if(a==one) return zero;
    if(a==zero) return one;
    if(a==ex) return ex;
    else { cout<< "returning undefined!" << endl ; 
	return un;
    }
}

Net operator&(Net a, Net b){
    Net *tmp= new Net;
    
    (*tmp).NetVal=un;
    if(a.NetVal==un || b.NetVal==un) { cout<< "returning undefined!" << endl ; 
	return *tmp;
    }
    
    if(a.NetVal==zero)  (*tmp).NetVal=zero;
    else if(b.NetVal==zero)  (*tmp).NetVal=zero;
    else if(a.NetVal==ex || b.NetVal==ex)  (*tmp).NetVal=ex;
    else  (*tmp).NetVal=one;
    
    return *tmp;
}

Net operator!(Net a){
    Net *tmp= new Net;
    
    (*tmp).NetVal=un;
    if(a.NetVal==un ) { cout<< "returning undefined!" << endl ; 
	return *tmp;
    }
    
    if(a.NetVal==zero)  (*tmp).NetVal=one;
    else if(a.NetVal==one)  (*tmp).NetVal=zero;
    else  (*tmp).NetVal=ex;
    
        return *tmp;
}

Net operator|(Net a, Net b){
    Net *tmp= new Net;
    (*tmp).NetVal=un;
    if(a.NetVal==un || b.NetVal==un) { cout<< "returning undefined!" << endl ; 
	return *tmp;
    }
    
    if(a.NetVal==one)  (*tmp).NetVal=one;
    else if(b.NetVal==one)  (*tmp).NetVal=one;
    else if(a.NetVal==ex || b.NetVal==ex)  (*tmp).NetVal=ex;
    else  (*tmp).NetVal=zero;
        return *tmp;
}

Net operator^(Net a, Net b){
    Net *tmp= new Net;
    
    (*tmp).NetVal=un;
    if(a.NetVal==un || b.NetVal==un) { cout<< "returning undefined!" << endl ; 
	return *tmp;
    }
    
    if(a.NetVal==ex || b.NetVal==ex) (*tmp).NetVal=ex;
    else if((a.NetVal==one && b.NetVal==zero) || (a.NetVal==zero & b.NetVal==one))  (*tmp).NetVal=one;
    else  (*tmp).NetVal=zero;
        return *tmp;
}

Net &operator<<(Net &b,Net a){
    b.NetVal=a.NetVal;

    cout << "                    "<< b.NetVal << endl;
    
}

vector<Net*> globalQ;

Net* OR(unsigned n,...){
    va_list arguments;    
    va_start(arguments, n);
    
    Net *result=new Net;
    vector<Net*> *tmp = new vector<Net*>;
    
    int one_index=-1;
    result->NetVal=zero;
     for(unsigned i=0; i< n;i++){
	   Net* new_arg= new Net;
	    new_arg=va_arg(arguments, Net*);
	    globalQ.push_back(new_arg);
	//    	 cout << " size = " << tmp->size() << endl;
	    *result= *result|*new_arg;
	    tmp->push_back(new_arg);
	    if(new_arg->NetVal==one)one_index=i;
     }
     
     va_start(arguments, n);
	 
     if(one_index!=-1)
	 for(unsigned i=0;i<n;i++){
	     if(i!=one_index)
		 va_arg(arguments,Net*)->Obs=false;
	 }
     result->InputNets=*tmp;
     globalQ.push_back(result);
     return result;
}


Net* AND(unsigned n,...){
    va_list arguments;
    
    va_start(arguments, n);
    
     Net *result=new Net;
    result->NetVal=one;
	
    vector<Net*> *tmp = new vector<Net*>;
    int zero_index=-1;
     for(unsigned i=0; i< n;i++){	 
	 Net* new_arg= new Net;
	 new_arg=va_arg(arguments, Net*);
// 	// cout << "size = " << tmp->size() << endl;
	 globalQ.push_back(new_arg);
	 tmp->push_back(new_arg);
	 if(new_arg->NetVal==zero)zero_index=i;
	*result= *result&*new_arg;
     }
     
         va_start(arguments, n);
	 
     if(zero_index!=-1)
	 for(unsigned i=0;i<n;i++){
	     if(i!=zero_index)
		 va_arg(arguments,Net*)->Obs=false;
	 }
	 
	 
     result->InputNets=*tmp;
     globalQ.push_back(result);
     return result;
}

Net* XOR(unsigned n,...){
    va_list arguments;    
    va_start(arguments, n);
    
    Net *result=new Net;
    result->NetVal=one;
	
    vector<Net*> *tmp = new vector<Net*>;
    int zero_index=-1;
     for(unsigned i=0; i< n;i++){	 
	 Net* new_arg= new Net;
	 new_arg=va_arg(arguments, Net*);
	 globalQ.push_back(new_arg);
	 tmp->push_back(new_arg);
	*result= *result^*new_arg;
     }
         va_start(arguments, n);
   /*Inputs of XOR gates are always observable*/
     	 for(unsigned i=0;i<n;i++){
		 va_arg(arguments,Net*)->Obs=true;
	 }

     result->InputNets=*tmp;
     globalQ.push_back(result);
     return result;
}

Net* NOT(Net * n){
   
    
    Net *result=new Net;
    *result=!*n;    
     
    /*Inputs of NOT gates are always observable*/
	 
     return result;
}

void assignObservability(Net*){
    /*Read the globalQ vector in reverse and assign Observability based on parent nets*/
    for(unsigned i=0; i< globalQ.size();i++){
	unsigned r=globalQ.size()-i-1;
	if(!globalQ[r]->Obs){
	    for(unsigned j=0; j < globalQ[r]->InputNets.size();j++){
		globalQ[r]->InputNets[j]->Obs=false;
	    }
	}
    }
    globalQ.clear();
}
Net OAI221_X1(vector<Net*> I){//!(((*I[3] | *I[2]) & *I[4]) & (*I[0] | *I[1]))
//         Net tmp=*AND(2,AND(2,OR(2,I[3],I[2]),I[4]),OR(2,I[0],I[1])); 
    Net tmp=*AND(2,AND(2,OR(2,I[1],I[2]),I[0]),OR(2,I[3],I[4])); 
    
    assignObservability(&tmp);
    
    return  !tmp;
}

Net INV_X1(vector<Net*> I){
    Net tmp=*I[0]; 
    
    assignObservability(&tmp);
    
    return  !tmp;
}

Net NAND2_X1(vector<Net*> I){ //!(*I[0] & *I[1])
    Net tmp=*AND(2,I[0],I[1]); 
    
    assignObservability(&tmp);
    
    return  !tmp;
}

Net OAI21_X1(vector<Net*> I){ //!(*I[2] & (*I[0] | *I[1]))
//     Net tmp=*AND(2,I[2],OR(2,I[0],I[1]));
    Net tmp=*AND(2,I[0],OR(2,I[2],I[1])); 
    
    assignObservability(&tmp);
    
    return  !tmp;
}

Net OAI22_X1(vector<Net*> I){ //!((*I[0] | *I[1]) & (*I[2]| *I[3]))
    Net tmp=*AND(2,OR(2,I[0],I[1]),OR(2,I[2],I[3])); 
    
    assignObservability(&tmp);
    
    return  !tmp;
}

Net XOR2_X1(vector<Net*> I){ //(*I[0] ^ *I[1])
    Net tmp=*XOR(2,I[0],I[1]);
    
    //assignObservability(&tmp);
    
    return  tmp;
}

Net AOI21_X1(vector<Net*> I){ //!(*I[2] | (*I[0] & *I[1]))
//     Net tmp=*OR(2,I[2],AND(2,I[0],I[1])); 
    Net tmp=*OR(2,I[0],AND(2,I[2],I[1]));
    
    assignObservability(&tmp);
    
    return  !tmp;
}

Net NOR2_X1(vector<Net*> I){ //!(*I[0] | *I[1])
    Net tmp=*OR(2,I[0],I[1]); 
    
    assignObservability(&tmp);
    
    return  !tmp;
}

Net AND2_X1(vector<Net*> I){ //(*I[0] & *I[1])
    Net tmp=*AND(2,I[0],I[1]); 
    
    assignObservability(&tmp);
    
    return  tmp;
}

Net XNOR2_X1(vector<Net*> I){ //!(*I[0] ^ *I[1])
    Net tmp=*XOR(2,I[0],I[1]);
    
    //assignObservability(&tmp);
    
    return  !tmp;
}

Net NOR3_X1(vector<Net*> I){ //!((*I[0] | *I[1]) | *I[2])
    Net tmp=*OR(3,I[0],I[1],I[2]); 
    
    assignObservability(&tmp);
    
    return  !tmp;
}

Net AOI22_X1(vector<Net*> I){ //!((*I[0] & *I[1]) | (*I[2] & *I[3]))
    Net tmp=*OR(2,AND(2,I[0],I[1]),AND(2,I[2],I[3])); 
    
    assignObservability(&tmp);
    
    return  !tmp;
}

Net OAI211_X1(vector<Net*> I){ //!(((*I[0] | *I[1]) & *I[2]) & *I[3])
//     Net tmp=*AND(3,OR(2,I[0],I[1]),I[2],I[3]);
      Net tmp=*AND(3,OR(2,I[3],I[2]),I[1],I[0]); 
    
    assignObservability(&tmp);
    
    return  !tmp;
}

Net AOI221_X1(vector<Net*> I){ //!(((*I[2] & *I[3]) | *I[4]) | (*I[1] & *I[0]))
//     Net tmp=*OR(2,OR(2,AND(2,I[2],I[3]),I[4]),AND(2,I[1],I[0]));
      Net tmp=*OR(2,OR(2,AND(2,I[2],I[1]),I[0]),AND(2,I[3],I[4])); 
    
    assignObservability(&tmp);
    
    return  !tmp;
}

Net MUX2_X1(vector<Net*> I){ //((*I[2] & *I[1]) | (*I[0] & !*I[2]))
    Net tmp=*OR(2,AND(2,I[2],I[1]),AND(2,I[0],NOT(I[2]))); 
    
    assignObservability(&tmp);
    
    return  tmp;
}


Net OR2_X1(vector<Net*> I){ //(*I[0] | *I[1])
    Net tmp=*OR(2,I[0],I[1]); 
    
    assignObservability(&tmp);
    
    return  tmp;
}

Net NAND3_X1(vector<Net*> I){ //!((*I[0] & *I[1]) & *I[2])
    Net tmp=*AND(3,I[0],I[1],I[2]); 
    
    assignObservability(&tmp);
    
    return  !tmp;
}

Net AOI211_X1(vector<Net*> I){ // !(((*I[0] & *I[1]) | *I[3]) | *I[2])
//     Net tmp=*OR(3,AND(2,I[0],I[1]),I[3],I[2]);
      Net tmp=*OR(3,AND(2,I[3],I[2]),I[0],I[1]); 
    
    assignObservability(&tmp);
    
    return  !tmp;
}

Net AND3_X1(vector<Net*> I){ //((*I[0] & *I[1]) & *I[2])
    Net tmp=*AND(3,I[0],I[1],I[2]); 
    
    assignObservability(&tmp);
    
    return  tmp;
}


Net OAI222_X1(vector<Net*> I){ //  !(((*I[0] | *I[1]) & (*I[2] | *I[3])) & (*I[4] | *I[5]))
    Net tmp=*AND(3,OR(2,I[0],I[1]),OR(2,I[1],I[2]),OR(2,I[4],I[5])); 
    
    assignObservability(&tmp);
    
    return  !tmp;
}

Net NOR4_X1(vector<Net*> I){ // !(((*I[0] | *I[1]) | *I[2]) | *I[3])
    Net tmp=*OR(4,I[0],I[1],I[2],I[3]); 
    
    assignObservability(&tmp);
    
    return  !tmp;
}


Net NAND4_X1(vector<Net*> I){ // !(((*I[0] | *I[1]) | *I[2]) | *I[3])
    Net tmp=*AND(4,I[0],I[1],I[2],I[3]); 
    
    assignObservability(&tmp);
    
    return  !tmp;
}


Net OAI33_X1(vector<Net*> I){ // !(((*I[0] | *I[1]) | *I[2]) & ((*I[3] | *I[4]) | *I[5]))
    Net tmp=*AND(2,OR(3,I[0],I[1],I[2]),OR(3,I[3],I[4],I[5])); 
    
    assignObservability(&tmp);
    
    return  !tmp;
}

Net AOI222_X1(vector<Net*> I){ // !(((*I[0] & *I[1]) | (*I[2] & *I[3])) | (*I[4] & *I[5]))
    Net tmp=*OR(3,AND(2,I[0],I[1]),AND(2,I[2],I[3]),AND(2,I[4],I[5])); 
    
    assignObservability(&tmp);
    
    return  !tmp;
}

Net BUF_X1(vector<Net*> I){
    Net tmp=*I[0]; 
    
    assignObservability(&tmp);
    
    return  tmp;
}

Net OR4_X1(vector<Net*> I){ // (((*I[0] | *I[1]) | *I[2]) | *I[3])
    Net tmp=*OR(4,I[0],I[1],I[2],I[3]); 
    
    assignObservability(&tmp);
    
    return  tmp;
}

Net DLH_X1(vector<Net*> I){ // (((*I[0] | *I[1]) | *I[2]) | *I[3])
    Net tmp=*AND(2,I[0],I[1]); 
    
    assignObservability(&tmp);
    
    return  tmp;
}

vector<Net> FA_X1(vector<Net*> I){ // *O[0] <<  ((*I[0] & *I[1]) | (*I[2] & (*I[0] | *I[1]))) ;    *O[1] <<  (*I[2] ^ (*I[0] ^ *I[1]))

    Net *tmp= new Net;
    tmp=XOR(2,XOR(3,I[0],I[1],I[2]),OR(2,AND(2,I[0],I[1]),AND(2,I[2],OR(2,I[0],I[1])))); 
               // cout << "Here "<< tmp->InputNets.size() << endl;
    vector<Net> *ret= new vector<Net>;
    assignObservability(tmp);
     for(unsigned i=0; i < I.size();i++) I[i]->Obs=true; // ALL inputs of an FA are always observable
    ret->push_back(*(tmp->InputNets[1]));
    ret->push_back(*(tmp->InputNets[0]));
    return  *ret;
}
void printGloblQ(){
    for(unsigned i=0; i< globalQ.size();i++){
	unsigned r=globalQ.size()-i-1;
	cout << " globalq "<< r <<" = " <<globalQ[r]->Obs << " " << globalQ[r]->InputNets.size() << endl;
    }

}
void Cell::simulate(){
    vector<Net*>I=InputNets;
    vector<Net*>O=OutputNets;
    
    if(Type=="OAI221_X1") { *O[0] <<  OAI221_X1(I);} //ZN = "!(((C1 | C2) & A) & (B1 | B2))" B1 1, B2 2, C1 3 , C2 4, A 0
else if(Type=="INV_X1")   { *O[0] <<  INV_X1(I);}
else if(Type=="NAND2_X1") { *O[0] <<  NAND2_X1(I);}
else if(Type=="OAI22_X1") { *O[0] <<  OAI22_X1(I);} //A1, A2, B1, B2 
else if(Type=="OAI21_X1") { *O[0] <<  OAI21_X1(I);}//B1 1, B2 2, A 0
else if(Type=="XOR2_X1")  { *O[0] <<  XOR2_X1(I);}
else if(Type=="AOI21_X1") { *O[0] <<  AOI21_X1(I);} //B1 1 , B2 2 , A 0 
else if(Type=="NOR2_X1")  { *O[0] <<  NOR2_X1(I);}
else if(Type=="AND2_X1")  { *O[0] <<  AND2_X1(I);}
else if(Type=="XNOR2_X1") { *O[0] <<  XNOR2_X1(I);}
else if(Type=="NOR3_X1")  { *O[0] <<  NOR3_X1(I);}
else if(Type=="AOI22_X1") { *O[0] <<  AOI22_X1(I);} //A1, A2, B1, B2 
else if(Type=="OAI211_X1"){ *O[0] <<  OAI211_X1(I);  } //C1 2 , C2 3, A 0, B 1 
else if(Type=="AOI221_X1"){ *O[0] <<  AOI221_X1(I);} //B1 1, B2 2, C1 3 , C2 4, A 0
else if(Type=="MUX2_X1")  { *O[0] <<  MUX2_X1(I);} //A 0, B 1, S 2
else if(Type=="OR2_X1")   { *O[0] <<  OR2_X1(I);}
else if(Type=="NAND3_X1") { *O[0] <<  NAND3_X1(I);}
else if(Type=="AOI211_X1"){ *O[0] <<  AOI211_X1(I);} //C1 2, C2 3, A 0, B 1
else if(Type=="AND3_X1")  { *O[0] <<  AND3_X1(I);}
else if(Type=="OAI222_X1"){ *O[0] <<  OAI222_X1(I);}//A1, A2, B1, B2, C1, C2 
else if(Type=="NOR4_X1")  { *O[0] <<  NOR4_X1(I);}
else if(Type=="NAND4_X1") { *O[0] <<  NAND4_X1(I);}
else if(Type=="OAI33_X1") { *O[0] <<  OAI33_X1(I);}//A1, A2, A3, B1, B2, B3 
else if(Type=="AOI222_X1"){ *O[0] <<  AOI222_X1(I);}//A1, A2, B1, B2, C1, C2
else if(Type=="BUF_X1")   { *O[0] <<  BUF_X1(I);}
else if(Type=="OR4_X1")   { *O[0] <<  OR4_X1(I);}
else if(Type=="DLH_X1")   {cout << Type << "Should not be simulated !" << endl; exit(-1);}
else if(Type=="DFF_X1")   {cout << Type << "Should not be simulated !" << endl; exit(-1);}
else if(Type=="DFFR_X1")  {cout << Type << "Should not be simulated !" << endl; exit(-1);}
else if(Type=="FA_X1")    {vector<Net> t= FA_X1(I); cout << "Warning--Still need to account for reconvergence!" << endl;
			    *O[0] << t[0];
			    *O[1] << t[1];
} 
else if(Type=="SDFF_X1")  {cout << Type << "Should not be simulated !" << endl; exit(-1);}
else if(Type=="CLKGATETST_X1") {}
else if(Type.find("SNPS_CLOCK_GATE_HIGH")!=string::npos){ *O[0] <<  ((*I[1] | *I[2]) & *I[0]); } // Can be two input also!
else { cout << " Type " << Type << " not found " << endl; exit(-1);}


for(unsigned i=0;i < InputNets.size();i++)
    if(Obs.size()!=InputNets.size())
	Obs.push_back(InputNets[i]->Obs);
    else
	Obs[i]=InputNets[i]->Obs;

cout <<"Cell "<< Name << " has " << InputNets.size() << " InputNets and " << Obs.size() <<" Obs values" << endl;  
}
