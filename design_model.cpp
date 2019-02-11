// Author: Shrinidhi Udupi (shrinidhiku@gmail.com)
// Last updated : 26.07.2017

// This file contains the ternary simulation semantics and the Observability propagation models
// for the the library cells of the 45nm Nangate Open Cell Library and 65nm ST Micro library.

// In addition, the ternary simulation is performed symbolically using the CUDD package.

#include <iostream>
#ifndef DESIGN_MODEL
#define DESIGN_MODEL
#include <design_model.h>
#endif
#include <cstdarg>
#define BDD 
#define CUDD_MAX_SIZE 1000
 DdManager *manager;
void Net::setObs(bool b){
    Obs=b;
}

bool Net::getObs(){
return Obs;
}


void bddNetInitialize(value v, Net* n, bool inv){
    n->NetVal=v;
   // if(n->bddNetNode!=NULL) Cudd_RecursiveDeref(manager,n->bddNetNode);  // Dereference the node if any that already exists
    if(v==one){
	n->bddNetNode=Cudd_ReadOne(manager);
    }else if(v==zero){
	n->bddNetNode=Cudd_ReadLogicZero(manager);
    } else {
        if(!inv){
        n->bddNetNode=Cudd_bddNewVar(manager);
        }else{
            n->bddNetNode=Cudd_Not( n->InputCell->OutputNets[0]->bddNetNode);
        }
    }
    Cudd_Ref(n->bddNetNode);
}

value getVal(DdNode* n, DdManager *m){
    DdNode * test;
//    Cudd_Ref(test);
      cout << "** \tDAGSIZE1 = " << Cudd_DagSize(n) << endl;
    if(Cudd_IsConstant(n)==1){
	if(Cudd_IsConstant(Cudd_bddAnd(m,n,test))==1){
	    cout << "CUDD: Evaluated to 0" << endl;
	    return zero;
	}
	else {
	    cout << "CUDD: Evaluated to 1" << endl;
	    return one;
	}
    }else{
	cout << "CUDD: Evaluated to X" << endl;
	return ex;
    }
//     Cudd_RecursiveDeref(manager,test);
}


Net * Cell::gAct( string formal){
    string Net="";
    for(unsigned i= 0; i< associationList.size();i++){
	
	if(associationList[i].first==formal) {Net = associationList[i].second; break;}
    }
    if(Net=="") { cout << "ERROR: "<< formal << " not found in association list of cell "<< Name; exit(-1); }
    for(unsigned i=0; i< InputNets.size();i++){
	if(InputNets[i]->Name==Net) return InputNets[i];
    }
    
}

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
    if(a=="X" || a == "x") return ex;
    else { cout<< "returning undefined! to_value" << endl ; exit(-1);
	return un;
    }
}

value operator!(value a){
    if(a==one) return zero;
    if(a==zero) return one;
    if(a==ex) return ex;
    else { cout<< "returning undefined! operator!" << endl ;  exit(-1);
	return un;
    }
}

Net operator^(Net a, Net b){
    Net *tmp= new Net;
    cout << "Simulating XOR" << endl;
    (*tmp).NetVal=un;
    if(a.NetVal==un || b.NetVal==un) { cout<< "returning undefined! operator^" << endl ;  exit(-1);
	return *tmp;
    }
    
    if(a.NetVal==one){
        (*tmp).NetVal=ex;
        (*tmp).bddNetNode= Cudd_Not(b.bddNetNode);        
    }else if(b.NetVal==one){
        (*tmp).NetVal=ex;
        (*tmp).bddNetNode= Cudd_Not(a.bddNetNode);        
    }else if(a.NetVal==zero){
        (*tmp).NetVal=ex;
        (*tmp).bddNetNode= b.bddNetNode;        
    }else if(b.NetVal==zero){
        (*tmp).NetVal=ex;
        (*tmp).bddNetNode= a.bddNetNode;        
    }else{
        (*tmp).bddNetNode= Cudd_bddXor(manager,a.bddNetNode,b.bddNetNode);
            if(Cudd_DagSize(tmp->bddNetNode)>CUDD_MAX_SIZE){ // Delete node if size greater than a fixed value to reduce processing load
        Cudd_RecursiveDeref(manager,tmp->bddNetNode); 
                tmp->bddNetNode=Cudd_bddNewVar(manager);
    }
        (*tmp).NetVal=getVal((*tmp).bddNetNode,manager);
    }
        Cudd_Ref(tmp->bddNetNode);
        return *tmp;
}

Net operator!(Net a){
    Net *tmp= new Net;
        cout << "Simulating NOT" << endl;
    (*tmp).NetVal=un;
    if(a.NetVal==un ) { cout<< "returning undefined! operator!" << endl ;  exit(-1);
	return *tmp;
    }
    
    if(a.NetVal==zero)  {
	(*tmp).NetVal=one;
	(*tmp).bddNetNode=Cudd_ReadOne(manager);
    }
    else if(a.NetVal==one) {
	(*tmp).NetVal=zero;
	(*tmp).bddNetNode=Cudd_Not(Cudd_ReadOne(manager));
    }
    else {
	(*tmp).NetVal=ex;
#ifdef BDD

	(*tmp).bddNetNode=Cudd_Not(a.bddNetNode);

#endif
    }
    Cudd_Ref(tmp->bddNetNode);
    
        return *tmp;
}

Net operator|(Net a, Net b){
    Net *tmp= new Net;
    (*tmp).NetVal=un;
    
    if(a.NetVal==un || b.NetVal==un) { cout<< "returning undefined! operator|" << endl ;  exit(-1);
	return *tmp;
    }
    if(a.NetVal==un || b.NetVal==un) { cout<< "returning undefined! operator|" << endl ;  exit(-1);
	return *tmp;
    }
        cout << "Simulating OR" << endl;
    if(a.NetVal==one){
	(*tmp).NetVal=one;
	(*tmp).bddNetNode=Cudd_ReadOne(manager);
    }
    else if(b.NetVal==one){
	(*tmp).NetVal=one;
	(*tmp).bddNetNode=Cudd_ReadOne(manager);
    }
    else if(a.NetVal==ex && b.NetVal==zero){
	(*tmp).NetVal=ex;
	(*tmp).bddNetNode=a.bddNetNode;
    }
    else if(a.NetVal==zero && b.NetVal==ex){
	(*tmp).NetVal=ex;
	(*tmp).bddNetNode=b.bddNetNode;
    }
    else if(a.NetVal==ex && b.NetVal==ex){
#ifdef BDD
// 	
	(*tmp).bddNetNode=Cudd_bddOr(manager,a.bddNetNode, b.bddNetNode);
    if(Cudd_DagSize(tmp->bddNetNode)>CUDD_MAX_SIZE){ // Delete node if size greater than a fixed value to reduce processing load
        Cudd_RecursiveDeref(manager,tmp->bddNetNode); 
                tmp->bddNetNode=Cudd_bddNewVar(manager);
    }
	tmp->NetVal= getVal((*tmp).bddNetNode,manager);
#else
    (*tmp).NetVal=ex;
#endif
    }
    else  {
	(*tmp).NetVal=zero;
	(*tmp).bddNetNode=Cudd_ReadLogicZero(manager);
    }
    
    Cudd_Ref(tmp->bddNetNode);
        return *tmp;
}

Net operator&(Net a, Net b){
    Net *tmp= new Net;
    (*tmp).NetVal=un;
    if(a.NetVal==un || b.NetVal==un) { cout<< "returning undefined! operator^" << endl ;  exit(-1);
	return *tmp;
    }
        cout << "Simulating AND" << endl;
 if(a.NetVal==zero){
	(*tmp).NetVal=zero;
	(*tmp).bddNetNode=Cudd_ReadLogicZero(manager);
    }
    else if(b.NetVal==zero){
	(*tmp).NetVal=zero;
	(*tmp).bddNetNode=Cudd_ReadLogicZero(manager);
    }
    else if(a.NetVal==ex && b.NetVal==one){
	(*tmp).NetVal=ex;
	(*tmp).bddNetNode=a.bddNetNode;
    }
    else if(a.NetVal==one && b.NetVal==ex){
	(*tmp).NetVal=ex;
	(*tmp).bddNetNode=b.bddNetNode;
    }
    else if(a.NetVal==ex && b.NetVal==ex){
#ifdef BDD
	 (*tmp).bddNetNode=Cudd_bddAnd(manager,a.bddNetNode, b.bddNetNode); 
        if(Cudd_DagSize(tmp->bddNetNode)>CUDD_MAX_SIZE){ // Delete node if size greater than a fixed value to reduce processing load
        Cudd_RecursiveDeref(manager,tmp->bddNetNode); 
                tmp->bddNetNode=Cudd_bddNewVar(manager);
    }
	tmp->NetVal= getVal((*tmp).bddNetNode,manager);
#else
    (*tmp).NetVal=ex;
#endif
    }
    else  {
	(*tmp).NetVal=one;
	(*tmp).bddNetNode=Cudd_ReadOne(manager);
    }
    
    Cudd_Ref(tmp->bddNetNode);
        return *tmp;
}

Net &operator<<(Net &b,Net a){
    b.NetVal=a.NetVal;
    b.bddNetNode=a.bddNetNode;
    Cudd_Ref(b.bddNetNode);
    Cudd_RecursiveDeref(manager,a.bddNetNode);                 /* Explicit Dereference */
    

    //cout << "                    "<< b.NetVal << endl;
    
}

vector<Net*> globalQ;

void printGloblQ(){
//     cout << " Was here printGloblQ "<< endl;
    for(unsigned i=0; i< globalQ.size();i++){
	unsigned r=globalQ.size()-i-1;
// 	cout << " globalq "<< globalQ[r]->Name <<"'s Observability=" << globalQ[r]->getObs() << " Inputs size is " << globalQ[r]->InputNets.size() << endl;
    }

}

Net* OR(unsigned n,...){
    va_list arguments;    
    va_start(arguments, n);
    
    Net *result=new Net;
    vector<Net*> *tmp = new vector<Net*>; // Temporary pointer to the vector of inputs for this OR operation
    
    int one_index=-1;
    result->NetVal=zero;
//     	 unsigned k=0;
     for(unsigned i=0; i< n;i++){ // Loop through arguments
	 
	   Net* new_arg= new Net;
	    new_arg=va_arg(arguments, Net*);
	    globalQ.push_back(new_arg); // Push each new Net to a global vector
// 	cout<< "GQ OR "  << new_arg<< " "<< new_arg->Name << endl;
	//    	 cout << " size = " << tmp->size() << endl;
	    *result= *result|*new_arg;
	    tmp->push_back(new_arg);
	    if(new_arg->NetVal==one){one_index=i; } 
     }
     
     va_start(arguments, n);
//      cout << " one_index " << one_index << endl;
     if(one_index!=-1)
	 for(unsigned i=0;i<n;i++){
	     Net* x= va_arg(arguments,Net*);
	     if(i!=one_index){
		 x->setObs(false); 
// 		 cout << " set " << x->Name << " obs to false" << endl;
	     }
	     //cout << "Here OR " << x->Name<< " " << x << endl;
	 }
     result->InputNets=*tmp;
     globalQ.push_back(result);//cout << "GQ OR " << result << " "<< result->Name  << endl;
     return result;
}


Net* AND(unsigned n,...){
    va_list arguments;
    
    va_start(arguments, n);
    
     Net *result=new Net;
    result->NetVal=one;
	
    vector<Net*> *tmp = new vector<Net*>;
    int zero_index=-1;
//     	 unsigned k=0;
     for(unsigned i=0; i< n;i++){	 
	 Net* new_arg= new Net;
	 new_arg=va_arg(arguments, Net*);
// 	// cout << "size = " << tmp->size() << endl;
	 globalQ.push_back(new_arg);//cout << "GQ AND " << new_arg << " "<< new_arg->Name << endl;
	 tmp->push_back(new_arg);

	 if(new_arg->NetVal==zero){zero_index=i; } 
        *result= *result&*new_arg;
     }
     
         va_start(arguments, n);
	// cout << " zero index is " << zero_index << endl;
     if(zero_index!=-1){
	 for(unsigned i=0;i<n;i++){
	     Net* x= va_arg(arguments,Net*);
	     if(i!=zero_index){
		x->setObs(false); 
		 
	    }//cout << "Here AND " << x->Name<< " " << x << endl;
	 }
     }
	 
	 
     result->InputNets=*tmp;
     globalQ.push_back(result);//cout << "GQ AND " << result << " "<< result->Name << endl;
     return result;
}

Net* XOR(unsigned n,...){
    va_list arguments;    
    va_start(arguments, n);
    
    Net *result=new Net;
    result->NetVal=zero;
	
    vector<Net*> *tmp = new vector<Net*>;
    int zero_index=-1;
//     cout << "XOR inputs are ";
     for(unsigned i=0; i< n;i++){
	 Net* new_arg= new Net;
	 new_arg=va_arg(arguments, Net*);
	 globalQ.push_back(new_arg);
	 tmp->push_back(new_arg);
	
	*result= *result^*new_arg;
// 	 cout << new_arg->NetVal << " result=" << result->NetVal << " " ;
     }
         va_start(arguments, n);
   /*Inputs of XOR gates are always observable*/
     	 for(unsigned i=0;i<n;i++){
		 va_arg(arguments,Net*)->setObs(true);
	 }
//     cout << " XOR output is " << result->NetVal << endl;
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

void setCellNetObservability(Cell *c,string Name,bool b){

    if(c->Obs.size()==0){ // Initializing the observability vector (Property of cell)
	for(unsigned i=0; i < c->InputNets.size();i++) c->Obs.push_back(true);
       
    }else{
	if(c->Obs.size()!=c->InputNets.size()){cout<< "Something is wrong. The cell input observability was not set correctly "<< endl; exit(-1);}
    }
    unsigned index=0;
    for(index=0;index< c->InputNets.size();index++)
	if(c->InputNets[index]->Name==Name)break;
	
    if(index>c->InputNets.size()) {cout << Name <<" Not found in "<< c->Name << endl; exit(-1);}
    else if(index< c->InputNets.size()){	c->Obs[index]=b; //cout << " Setting "<< c->InputNets[index]->Name << "'s observability to " << b << endl;
	
    }
}
void assignObservability(Cell*c){ 

    /*Read the globalQ vector in reverse and assign Observability based on parent nets*/
    setCellNetObservability(c,"",true);  // Initialze the observability vector Obs for the cell
//     cout << " assigning " << c->Name << " input " << " true " << endl;
    for(unsigned i=0; i< globalQ.size();i++){
	
	unsigned r=globalQ.size()-i-1; // Going from output towards inputs
	
	if(!globalQ[r]->getObs()){ // If net is unobservables all its inputs are unobservable in the global vector
	    
	    for(unsigned j=0; j < globalQ[r]->InputNets.size();j++){
		if(globalQ[r]->InputNets[j]->Name=="") // If the globalQ net input nets are unnamed, implying that they are internal to the cell
		    globalQ[r]->InputNets[j]->setObs(false);
 		else{    
		    setCellNetObservability(c,globalQ[r]->InputNets[j]->Name,false);
		    }
	    }
	}else{
	    for(unsigned j=0; j < globalQ[r]->InputNets.size();j++){
		if(globalQ[r]->InputNets[j]->Name!="") {// If the globalQ net input nets are not unnamed, implying that they are internal to the cell    
		    Net * namedNet=globalQ[r]->InputNets[j];
		    setCellNetObservability(c,namedNet->Name,namedNet->getObs()); // Assign precalculated observability
		    //cout<< globalQ[r]->InputNets[j]->Name << " has observability " << globalQ[r]->InputNets[j]->getObs() << endl;
		}
		    
	    }
	}
// 	cout << "Assigned " << globalQ[r]->Name << " Obs =" << globalQ[r]->getObs()<<  endl; 
    }
    
    // Reset net observablities ( different from Cell net observabilities)
    for(unsigned i=0; i < c->InputNets.size();i++)
	c->InputNets[i]->setObs(true);
    printGloblQ();
    globalQ.clear();
}

Net OAI221_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
//!(((*I[3] | *I[2]) & *I[4]) & (*I[0] | *I[1]))
   // Net tmp=*AND(2,AND(2,OR(2,I[3],I[2]),I[4]),OR(2,I[0],I[1]));
 Net tmp=*AND(2,AND(2,OR(2,c->gAct("C1"),c->gAct("C2")),c->gAct("A")),OR(2,c->gAct("B1"),c->gAct("B2")));  
    
    assignObservability(c);

    return  !tmp;
}

Net INV_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 

    Net tmp=*I[0]; 
    
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LS_IVX7(Cell*c){ 
 vector<Net*>I= c->InputNets; 

    Net tmp=*I[0]; 
    
    assignObservability(c);
    
    return  !tmp;
}

Net NAND2_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!(*I[0] & *I[1])
    Net tmp=*AND(2,I[0],I[1]); 
    
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LS_NAND2X5(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 
    Net tmp=*AND(2,I[0],I[1]); 
    
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LS_NAND2AX7(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 
    Net tmp=*AND(2,NOT(I[0]),I[1]); 
    
    assignObservability(c);
    
    return  !tmp;
}

Net OAI21_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!(*I[2] & (*I[0] | *I[1]))
    //Net tmp=*AND(2,I[2],OR(2,I[0],I[1])); 
    Net tmp=*AND(2,c->gAct("A"),OR(2,c->gAct("B1"),c->gAct("B2"))); 
    assignObservability(c);
    
    return  !tmp;
}

Net OAI22_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!((*I[0] | *I[1]) & (*I[2]| *I[3]))
    Net tmp=*AND(2,OR(2,I[0],I[1]),OR(2,I[2],I[3])); 
    
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LS_OA22X9(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!((*I[0] | *I[1]) & (*I[2]| *I[3]))
    Net tmp=*AND(2,OR(2,I[0],I[1]),OR(2,I[2],I[3])); 
    
    assignObservability(c);
    
    return  tmp;
}

Net HS65_LS_AOI32X5(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!((*I[0] | *I[1]) & (*I[2]| *I[3]))
    Net tmp=*OR(2,AND(2,I[3],I[4]),AND(3,I[0],I[1],I[2])); 
    
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LS_AOI33X(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!((*I[0] | *I[1]) & (*I[2]| *I[3]))
    Net tmp=*OR(2,AND(3,I[3],I[4],I[5]),AND(3,I[0],I[1],I[2])); 
    
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LS_AO33X(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!((*I[0] | *I[1]) & (*I[2]| *I[3]))
    Net tmp=*OR(2,AND(3,I[3],I[4],I[5]),AND(3,I[0],I[1],I[2])); 
    
    assignObservability(c);
    
    return  tmp;
}


Net HS65_LS_AO312X9(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!((*I[0] | *I[1]) & (*I[2]| *I[3]))
    Net tmp=*OR(3,I[5],AND(2,I[3],I[4]),AND(3,I[0],I[1],I[2])); 
    
    assignObservability(c);
    
    return  tmp;
}

Net HS65_LS_AO212X9(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!((*I[0] | *I[1]) & (*I[2]| *I[3]))
    Net tmp=*OR(3,AND(2,I[3],I[2]),AND(2,I[0],I[1]),I[4]); 
    
    assignObservability(c);
    
    return  tmp;
}

Net HS65_LS_OA212X9(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!((*I[0] | *I[1]) & (*I[2]| *I[3]))
    Net tmp=*AND(3,OR(2,I[3],I[2]),OR(2,I[0],I[1]),I[4]); 
    
    assignObservability(c);
    
    return  !tmp;
}


Net XOR2_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //(*I[0] ^ *I[1])
    Net tmp=*XOR(2,I[0],I[1]);
    
    assignObservability(c);
    
    return  tmp;
}

Net HS65_LSS_XOR2X6(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //(*I[0] ^ *I[1])
    Net tmp=*XOR(2,I[0],I[1]);
    
    assignObservability(c);
    
    return  tmp;
}


Net AOI21_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!(*I[0] | (*I[2] & *I[1]))
   // Net tmp=*OR(2,I[0],AND(2,I[2],I[1]));
 Net tmp=*OR(2,c->gAct("A"),AND(2,c->gAct("B1"),c->gAct("B2")));  
    
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LS_AOI21X12(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!(*I[0] | (*I[2] & *I[1]))
    Net tmp=*OR(2,I[2],AND(2,I[0],I[1])); 
    
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LS_OA12X9(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!(*I[0] | (*I[2] & *I[1]))
    Net tmp=*AND(2,I[2],OR(2,I[0],I[1])); 
    
    assignObservability(c);
    
    return  tmp;
}

Net HS65_LS_OAI12X5(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 
    Net tmp=*AND(2,I[2],OR(2,I[0],I[1])); 
    
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LS_OAI13X5(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //
    Net tmp=*AND(2,I[3],OR(3,I[2],I[1],I[0])); 
    
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LS_OA31X9(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //
    Net tmp=*AND(2,I[3],OR(3,I[2],I[1],I[0])); 
    
    assignObservability(c);
    
    return  tmp;
}

Net HS65_LS_OA33X(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //
    Net tmp=*AND(2,OR(3,I[3],I[4],I[5]),OR(3,I[2],I[1],I[0])); 
    
    assignObservability(c);
    
    return  tmp;
}

Net HS65_LS_OAI31X(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //
    Net tmp=*AND(2,I[3],OR(3,I[2],I[1],I[0])); 
    
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LS_AOI12X6(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!(*I[2] | (*I[0] & *I[1]))
    Net tmp=*OR(2,I[2],AND(2,I[0],I[1])); 
    
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LS_AO12X9(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!(*I[2] | (*I[0] & *I[1]))
    Net tmp=*OR(2,I[2],AND(2,I[0],I[1])); 
    
    assignObservability(c);
    
    return  tmp;
}

Net HS65_LS_AOI13X5(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!(*I[2] | (*I[0] & *I[1]))
    Net tmp=*OR(2,I[3],AND(3,I[0],I[1],I[2])); 
    
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LS_AOI112X7(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 
    Net tmp=*OR(3,I[3],I[2],AND(2,I[0],I[1])); 
    
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LS_AO112X9(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 
    Net tmp=*OR(3,I[3],I[2],AND(2,I[0],I[1])); 
    
    assignObservability(c);
    
    return  tmp;
}

Net HS65_LS_OAI112X5(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 
    Net tmp=*AND(3,I[3],I[2],OR(2,I[0],I[1])); 
    
    assignObservability(c);
    
    return  !tmp;
}


Net HS65_LS_OA112X9(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 
    Net tmp=*AND(3,I[3],I[2],OR(2,I[0],I[1])); 
    
    assignObservability(c);
    
    return  tmp;
}

Net HS65_LS_MUX21I1X6(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 
    Net tmp=*OR(2,AND(2,NOT(I[2]),NOT(I[0])),AND(2,I[2],I[1])); 
    
    assignObservability(c);
    
    return  tmp;
}

Net HS65_LS_MUXI21X(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 
    Net tmp=*OR(2,AND(2,NOT(I[2]),NOT(I[0])),AND(2,I[2],I[1])); 
    
    assignObservability(c);
    
    return  !tmp;
}
Net HS65_LS_MUX21X9(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 
    Net tmp=*OR(2,AND(2,NOT(I[2]),I[0]),AND(2,I[2],I[1])); 
    
    assignObservability(c);
    
    return  tmp;
}


Net HS65_LS_MX41X7(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 
    Net tmp=*OR(4,AND(2,I[0],I[1]),AND(2,I[2],I[3]),AND(2,I[4],I[5]),AND(2,I[6],I[7])); 
    
    assignObservability(c);
    
    return  tmp;
}

Net HS65_LS_MUX31X9(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 
    Net tmp=*OR(3,AND(3,NOT(I[3]),NOT(I[4]),I[0]),AND(3,I[3],NOT(I[4]),I[1]),AND(2,I[4],I[2])); 
    
    assignObservability(c);
    
    return  tmp;
}

Net HS65_LS_AO31X9(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 
    Net tmp=*OR(2,AND(3,I[0],I[1],I[2]),I[3]); 
    
    assignObservability(c);
    
    return  tmp;
}


Net HS65_LS_AO32X9(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 
    Net tmp=*OR(2,AND(3,I[0],I[1],I[2]),AND(2,I[3],I[4])); 
    
    assignObservability(c);
    
    return  tmp;
}


Net NOR2_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!(*I[0] | *I[1])
    Net tmp=*OR(2,I[0],I[1]); 
    
    assignObservability(c);
//        printGloblQ(); 
    return  !tmp;
}

Net HS65_LS_NOR2AX6(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!(*I[0] | *I[1])
    Net tmp=*OR(2,NOT(I[0]),I[1]); 
    
    assignObservability(c);
    
    return  !tmp;
}

Net NOR2_X2(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!(*I[0] | *I[1])
    Net tmp=*OR(2,I[0],I[1]); 
    
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LS_NOR2X5(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!(*I[0] | *I[1])
    Net tmp=*OR(2,I[0],I[1]); 
    
    assignObservability(c);

    return  !tmp;
}


Net AND2_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //(*I[0] & *I[1])
    Net tmp=*AND(2,I[0],I[1]); 
    
    assignObservability(c);
    
    return  tmp;
}

Net XNOR2_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!(*I[0] ^ *I[1])
    Net tmp=*XOR(2,I[0],I[1]);
    
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LSS_XNOR2X6(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 
    Net tmp=*XOR(2,I[0],I[1]);
    
    assignObservability(c);
    
    return  !tmp;
}


Net NOR3_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!((*I[0] | *I[1]) | *I[2])
    Net tmp=*OR(3,I[0],I[1],I[2]); 
    
    assignObservability(c);
    
    return  !tmp;
}


Net NOR3A_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!((*I[0] | *I[1]) | *I[2])
    Net tmp=*OR(3,NOT(I[0]),I[1],I[2]); 
    
    assignObservability(c);
    
    return  !tmp;
}

Net AOI22_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!((*I[0] & *I[1]) | (*I[2] & *I[3]))
    Net tmp=*OR(2,AND(2,I[0],I[1]),AND(2,I[2],I[3])); 
    
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LS_AO22X9(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!((*I[0] & *I[1]) | (*I[2] & *I[3]))
    Net tmp=*OR(2,AND(2,I[0],I[1]),AND(2,I[2],I[3])); 
    
    assignObservability(c);
    
    return  tmp;
}

Net OAI211_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!(((*I[0] | *I[1]) & *I[2]) & *I[3])
    //Net tmp=*AND(3,OR(2,I[0],I[1]),I[2],I[3]);
	Net tmp=*AND(3,OR(2,c->gAct("C1"),c->gAct("C2")),c->gAct("A"),c->gAct("B"));  
    
    assignObservability(c);
    
    return  !tmp;
}


Net AOI221_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!(((*I[2] & *I[3]) | *I[4]) | (*I[1] & *I[0])) Input order based on component instantiation
   // Net tmp=*OR(2,OR(2,AND(2,I[2],I[3]),I[4]),AND(2,I[1],I[0]));
 Net tmp=*OR(2,OR(2,AND(2,c->gAct("C1"),c->gAct("C2")),c->gAct("A")),AND(2,c->gAct("B1"),c->gAct("B2")));  
    
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LS_AO222(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!(((*I[2] & *I[3]) | *I[4]) | (*I[1] & *I[0])) Input order based on component instantiation
    Net tmp=*OR(3,AND(2,I[2],I[3]),AND(2,I[1],I[0]),AND(2,I[4],I[5])); 
    
    assignObservability(c);
    
    return  tmp;
}

Net AOI221_X2(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!(((*I[2] & *I[3]) | *I[4]) | (*I[1] & *I[0]))
   // Net tmp=*OR(2,OR(2,AND(2,I[2],I[3]),I[4]),AND(2,I[1],I[0]));
 Net tmp=*OR(2,OR(2,AND(2,c->gAct("C1"),c->gAct("C2")),c->gAct("A")),AND(2,c->gAct("B1"),c->gAct("B2")));  
    
    assignObservability(c);
    
    return  !tmp;
}

Net MUX2_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //((*I[2] & *I[1]) | (*I[0] & !*I[2]))
    Net tmp=*OR(2,AND(2,I[2],I[1]),AND(2,I[0],NOT(I[2]))); 
    
    assignObservability(c);
    
    return  tmp;
}


Net OR2_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //(*I[0] | *I[1])
    Net tmp=*OR(2,I[0],I[1]); 
    
    assignObservability(c);
    
    return  tmp;
}

Net NAND3_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!((*I[0] & *I[1]) & *I[2])
    Net tmp=*AND(3,I[0],I[1],I[2]); 
    
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LS_NAND3AX6(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //!((*I[0] & *I[1]) & *I[2])
    Net tmp=*AND(3,NOT(I[0]),I[1],I[2]); 
    
    assignObservability(c);
    
    return  !tmp;
}

Net AOI211_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 // !(((*I[0] & *I[1]) | *I[3]) | *I[2])
 //   Net tmp=*OR(3,AND(2,I[0],I[1]),I[3],I[2]);
   Net tmp=*OR(3,AND(2,c->gAct("C1"),c->gAct("C2")),c->gAct("B"),c->gAct("A"));  
    
    assignObservability(c);
    
    return  !tmp;
}

Net AND3_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //((*I[0] & *I[1]) & *I[2])
    Net tmp=*AND(3,I[0],I[1],I[2]); 
    
    assignObservability(c);
    
    return  tmp;
}


Net OAI222_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //  !(((*I[0] | *I[1]) & (*I[2] | *I[3])) & (*I[4] | *I[5]))
//    cout << I[0]->Obs << "-"<<I[1]->Obs <<  "  " << I[2]->Obs << " -*- "<<I[3]->Obs << " " << I[4]->Obs <<   " " << I[5]->Obs << endl;
    Net tmp=*AND(3,OR(2,I[0],I[1]),OR(2,I[2],I[3]),OR(2,I[4],I[5])); 
        
    assignObservability(c);
    
    return  !tmp;
}

Net OA222_X(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 //  !(((*I[0] | *I[1]) & (*I[2] | *I[3])) & (*I[4] | *I[5]))
//    cout << I[0]->Obs << "-"<<I[1]->Obs <<  "  " << I[2]->Obs << " -*- "<<I[3]->Obs << " " << I[4]->Obs <<   " " << I[5]->Obs << endl;
    Net tmp=*AND(3,OR(2,I[0],I[1]),OR(2,I[2],I[3]),OR(2,I[4],I[5])); 
        
    assignObservability(c);
    
    return  tmp;
}

Net HS65_LS_OAI212X5(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 
    Net tmp=*AND(3,OR(2,I[0],I[1]),OR(2,I[2],I[3]),I[4]); 
        
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LS_OAI311X(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 
    Net tmp=*AND(3,OR(2,I[0],I[1],I[2]),I[3],I[4]); 
        
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LS_OAI32X(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 
    Net tmp=*AND(2,OR(2,I[0],I[1],I[2]),OR(2,I[3],I[4])); 
        
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LS_OAI211X5(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 
    Net tmp=*AND(3,OR(2,I[0],I[1]),I[2],I[3]); 
        
    assignObservability(c);
    
    return  !tmp;
}

Net NOR4ABX(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 // !(((*I[0] | *I[1]) | *I[2]) | *I[3])
    Net tmp=*OR(4,NOT(I[0]),NOT(I[1]),I[2],I[3]); 
    
    assignObservability(c);
    
    return  !tmp;
}

Net NOR4_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 // !(((*I[0] | *I[1]) | *I[2]) | *I[3])
    Net tmp=*OR(4,I[0],I[1],I[2],I[3]); 
    
    assignObservability(c);
    
    return  !tmp;
}


Net NAND4_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 // !(((*I[0] | *I[1]) | *I[2]) | *I[3])
    Net tmp=*AND(4,I[0],I[1],I[2],I[3]); 
    
    assignObservability(c);
    
    return  !tmp;
}

Net AND4_X(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 // !(((*I[0] | *I[1]) | *I[2]) | *I[3])
    Net tmp=*AND(4,I[0],I[1],I[2],I[3]); 
    
    assignObservability(c);
    
    return  tmp;
}

Net HS65_LS_NAND4AB(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 // !(((*I[0] | *I[1]) | *I[2]) | *I[3])
    Net tmp=*AND(4,NOT(I[0]),NOT(I[1]),I[2],I[3]); 
    
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LS_AND4X6(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 // !(((*I[0] | *I[1]) | *I[2]) | *I[3])
    Net tmp=*AND(4,I[0],I[1],I[2],I[3]); 
    
    assignObservability(c);
    
    return  tmp;
}

Net OAI33_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 // !(((*I[0] | *I[1]) | *I[2]) & ((*I[3] | *I[4]) | *I[5]))
    Net tmp=*AND(2,OR(3,I[0],I[1],I[2]),OR(3,I[3],I[4],I[5])); 
    
    assignObservability(c);
    
    return  !tmp;
}

Net AOI222_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 // !(((*I[0] & *I[1]) | (*I[2] & *I[3])) | (*I[4] & *I[5]))
    Net tmp=*OR(3,AND(2,I[0],I[1]),AND(2,I[2],I[3]),AND(2,I[4],I[5])); 
    
    assignObservability(c);
    
    return  !tmp;
}

Net BUF_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 

    Net tmp=*I[0]; 
    
    assignObservability(c);
    
    return  tmp;
}

Net OR3_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 // (((*I[0] | *I[1]) | *I[2]) )
    Net tmp=*OR(3,I[0],I[1],I[2]); 
    
    assignObservability(c);
    
    return  tmp;
}

Net OR4_X(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 // (((*I[0] | *I[1]) | *I[2]) )
    Net tmp=*OR(4,I[0],I[1],I[2],I[3]); 
    
    assignObservability(c);
    
    return  tmp;
}

Net OR4_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 // (((*I[0] | *I[1]) | *I[2]) | *I[3])
    Net tmp=*OR(4,I[0],I[1],I[2],I[3]); 
    
    assignObservability(c);
    
    return  tmp;
}


Net HS65_LS_PAOI2X6(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 // !((I[0] & I[1] & NOT(I[2]) | I[2] & (I[0] | I[1])))
    Net tmp=*OR(2,AND(3,I[0],I[1],NOT(I[2])),AND(2,I[2],OR(2,I[0],I[1])));
    
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LS_PAO2X9(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 // !((I[0] & I[1] & NOT(I[2]) | I[2] & (I[0] | I[1])))
    Net tmp=*OR(2,AND(3,I[0],I[1],NOT(I[2])),AND(2,I[2],OR(2,I[0],I[1])));
    
    assignObservability(c);
    
    return  tmp;
}

Net HS65_LS_CBI4I1X5(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 
    Net tmp=*AND(2,OR(2,AND(2,I[0],I[1]),I[2]),I[3]);
    
    assignObservability(c);
    
    return  !tmp;
}


Net HS65_LS_CBI4I6X5(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 
    Net tmp=*OR(2,AND(2,OR(2,I[0],I[1]),I[2]),I[3]);
    
    assignObservability(c);
    
    return  !tmp;
}

Net HS65_LS_CB4I6X9(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 
    Net tmp=*OR(2,AND(2,OR(2,I[0],I[1]),I[2]),I[3]);
    
    assignObservability(c);
    
    return  tmp;
}

Net HS65_LS_CB4I1X9(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 
    Net tmp=*AND(2,OR(2,AND(2,I[0],I[1]),I[2]),I[3]);
    
    assignObservability(c);
    
    return  tmp;
}

Net DLH_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 

    Net tmp=*AND(2,I[0],I[1]); 
    
    assignObservability(c);
    
    return  tmp;
}

Net SDFF_X(Cell*c){ 
 vector<Net*>I= c->InputNets; 

    Net tmp=*OR(2,AND(2,I[0],NOT(I[2])),AND(2,I[2],I[1])); 
    
    assignObservability(c);
    
    return  tmp;
}

//Q = "(!(!RN)*((D)+(!SN)))"
Net DFF_X(Cell*c){ 
 vector<Net*>I= c->InputNets; 

    Net tmp;//=*AND(2,I[0],I[1]); 
//     if(I[1]->Name!="ap_clk")
        tmp=*AND(2,I[0],I[1]);
//     else
//         tmp=*I[0];
    assignObservability(c);
    
    return  tmp;
}
vector<Net> FA_X1(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 // *O[0] <<  ((*I[0] & *I[1]) | (*I[2] & (*I[0] | *I[1]))) ;    *O[1] <<  (*I[2] ^ (*I[0] ^ *I[1]))

    Net *tmp= new Net;
    tmp=XOR(2,XOR(3,I[0],I[1],I[2]),OR(2,AND(2,I[0],I[1]),AND(2,I[2],OR(2,I[0],I[1])))); 
               // cout << "Here "<< tmp->InputNets.size() << endl;
    vector<Net> *ret= new vector<Net>;
    assignObservability(c);
     setCellNetObservability(c,"",true); // ALL inputs of an FA are always observable
    ret->push_back(*(tmp->InputNets[1]));
    ret->push_back(*(tmp->InputNets[0]));
    return  *ret;
}
vector<Net> HS65_LS_HA1X9(Cell*c){ 
 vector<Net*>I= c->InputNets; 
 
    Net *tmp= new Net;
    tmp=XOR(2,XOR(2,I[0],I[1]),AND(2,I[0],I[1])); 
               // cout << "Here "<< tmp->InputNets.size() << endl;
    vector<Net> *ret= new vector<Net>;
    assignObservability(c);
     setCellNetObservability(c,"",true); // ALL inputs of an FA are always observable
    ret->push_back(*(tmp->InputNets[1]));
    ret->push_back(*(tmp->InputNets[0]));
    return  *ret;
}


string simplified(string s){
 while(true){
     if(isdigit(s[s.length()-1])){
	 s.erase(s.length()-1);
     }else{
	 break;
     }
	 
 }
return s;
}
value Cell::simulate(){
    vector<Net*>I=InputNets;
    vector<Net*>O=OutputNets;
    
    if(simplified(Type)=="OAI221_X") { *O[0] <<  OAI221_X1(this);} //ZN = "!(((C1 | C2) & A) & (B1 | B2))" B1 1, B2 2, C1 3 , C2 4, A 0
else if(simplified(Type)=="INV_X" || simplified(Type)=="HS65_LS_CNIVX")   { *O[0] <<  INV_X1(this);}
else if(simplified(Type)=="HS65_LS_IVX" ) {*O[0] <<  HS65_LS_IVX7(this); }
else if(simplified(Type)=="NAND2_X" || simplified(Type)=="HS65_LS_NAND2X" || simplified(Type)=="HS65_LS_CNNAND2X" ) { *O[0] <<  NAND2_X1(this);}
else if(simplified(Type)=="HS65_LS_NAND2AX") { *O[0] <<  HS65_LS_NAND2AX7(this);}
else if(simplified(Type)=="OAI22_X" || simplified(Type)=="HS65_LS_OAI22X") { *O[0] <<  OAI22_X1(this);} //A1, A2, B1, B2 
else if(simplified(Type)=="OAI21_X" || simplified(Type)=="HS65_LS_OAI21X") { *O[0] <<  OAI21_X1(this);}//B1 1, B2 2, A 0
else if(simplified(Type)=="XOR2_X")  { *O[0] <<  XOR2_X1(this);}
else if(simplified(Type)=="HS65_LSS_XOR2X")  { *O[0] <<  HS65_LSS_XOR2X6(this);}
else if(simplified(Type)=="HS65_LS_OA22X")  { *O[0] <<  HS65_LS_OA22X9(this);}
else if(simplified(Type)=="HS65_LS_OAI212X")  { *O[0] <<  HS65_LS_OAI212X5(this);}
else if(simplified(Type)=="AOI21_X") { *O[0] <<  AOI21_X1(this);} //B1 1 , B2 2 , A 0 
else if(simplified(Type)=="HS65_LS_AOI12X" || simplified(Type)=="HS65_LS_AOI12X") { *O[0] <<  HS65_LS_AOI12X6(this);}
else if(simplified(Type)=="HS65_LS_AOI112X") { *O[0] <<  HS65_LS_AOI112X7(this);}
else if(simplified(Type)=="HS65_LS_AO112X") { *O[0] <<  HS65_LS_AO112X9(this);}
else if(simplified(Type)=="HS65_LS_OAI12X") { *O[0] <<  HS65_LS_OAI12X5(this);}
else if(simplified(Type)=="HS65_LS_OAI112X") { *O[0] <<  HS65_LS_OAI112X5(this);}
else if(simplified(Type)=="HS65_LS_OAI13X") { *O[0] <<  HS65_LS_OAI13X5(this);}
else if(simplified(Type)=="NOR2_X"|| simplified(Type)=="HS65_LS_NOR2X")  { *O[0] <<  NOR2_X1(this);/*cout << " pglobalq" << endl;*/}
else if(simplified(Type)=="HS65_LS_NOR2AX")  { *O[0] <<  HS65_LS_NOR2AX6(this);}
else if(simplified(Type)=="HS65_LS_PAO2X")  { *O[0] <<  HS65_LS_PAO2X9(this);}
else if(simplified(Type)=="NOR2_X")  { *O[0] <<  NOR2_X2(this);}
// else if(simplified(Type)=="HS65_LS_NOR2X")  { *O[0] <<  HS65_LS_NOR2X5(this); cout << " pglobalq" << endl;}
else if(simplified(Type)=="HS65_LS_CBI4I1X")  { *O[0] <<  HS65_LS_CBI4I1X5(this);}
else if(simplified(Type)=="HS65_LS_CBI4I6X" )  { *O[0] <<  HS65_LS_CBI4I6X5(this);}
else if(simplified(Type)=="HS65_LS_AOI13X")  { *O[0] <<  HS65_LS_AOI13X5(this);}
else if(simplified(Type)=="AND2_X" || simplified(Type)=="HS65_LS_AND2X" )  { *O[0] <<  AND2_X1(this);}
else if(simplified(Type)=="XNOR2_X") { *O[0] <<  XNOR2_X1(this);}
else if(simplified(Type)=="HS65_LSS_XNOR2X") { *O[0] <<  HS65_LSS_XNOR2X6(this);}
else if(simplified(Type)=="NOR3_X" || simplified(Type)=="HS65_LS_NOR3X")  { *O[0] <<  NOR3_X1(this);}
else if(simplified(Type)=="HS65_LS_NOR3AX")  { *O[0] <<  NOR3A_X1(this);}
else if(simplified(Type)=="AOI22_X" || simplified(Type)=="HS65_LS_AOI22X") { *O[0] <<  AOI22_X1(this);} //A1, A2, B1, B2 
else if(simplified(Type)=="HS65_LS_AO22X") { *O[0] <<  HS65_LS_AO22X9(this);} 
else if(simplified(Type)=="OAI211_X"){ *O[0] <<  OAI211_X1(this);  } //C1 2 , C2 3, A 0, B 1 
else if(simplified(Type)=="AOI221_X"){ *O[0] <<  AOI221_X1(this);} //B1 1, B2 2, C1 3 , C2 4, A 0
else if(simplified(Type)=="AOI221_X"){ *O[0] <<  AOI221_X2(this);}
else if(simplified(Type)=="MUX2_X")  { *O[0] <<  MUX2_X1(this);} //A 0, B 1, S 2
else if(simplified(Type)=="OR2_X" || simplified(Type)=="HS65_LS_OR2X")   { *O[0] <<  OR2_X1(this);}
else if(simplified(Type)=="OR3_X" || simplified(Type) == "HS65_LS_OR3X")   { *O[0] <<  OR3_X1(this);}
else if(simplified(Type)=="OR4_X" || simplified(Type) == "HS65_LS_OR4X")   { *O[0] <<  OR4_X(this);}
else if(simplified(Type)=="NAND3_X"|| simplified(Type)=="HS65_LS_NAND3X" ) { *O[0] <<  NAND3_X1(this);}
else if( simplified(Type)=="HS65_LS_NAND3AX") { *O[0] <<  HS65_LS_NAND3AX6(this);}
else if(simplified(Type)=="HS65_LS_PAOI2X") { *O[0] <<  HS65_LS_PAOI2X6(this);}
else if(simplified(Type)=="HS65_LS_AOI21X" ) { *O[0] <<  HS65_LS_AOI21X12(this);}
else if(simplified(Type)=="HS65_LS_AOI32X") { *O[0] <<  HS65_LS_AOI32X5(this);}
else if(simplified(Type)=="HS65_LS_AOI33X") { *O[0] <<  HS65_LS_AOI33X(this);}
else if(simplified(Type)=="HS65_LS_AO33X") { *O[0] <<  HS65_LS_AO33X(this);}
else if(simplified(Type)=="HS65_LS_AO312X") { *O[0] <<  HS65_LS_AO312X9(this);}
else if(simplified(Type)=="HS65_LS_OA112X") { *O[0] <<  HS65_LS_OA112X9(this);}
else if(simplified(Type)=="HS65_LS_CB4I6X") { *O[0] <<  HS65_LS_CB4I6X9(this);}
else if(simplified(Type)=="HS65_LS_AO212X") { *O[0] <<  HS65_LS_AO212X9(this);}
else if(simplified(Type)=="HS65_LS_AO12X") { *O[0] <<  HS65_LS_AO12X9(this);}
else if(simplified(Type)=="HS65_LS_AO31X") { *O[0] <<  HS65_LS_AO31X9(this);}
else if(simplified(Type)=="HS65_LS_AO32X") { *O[0] <<  HS65_LS_AO32X9(this);}
else if(simplified(Type)=="HS65_LS_OAI211X") { *O[0] <<  HS65_LS_OAI211X5(this);}
else if(simplified(Type)=="HS65_LS_OA31X") { *O[0] <<  HS65_LS_OA31X9(this);}
else if(simplified(Type)=="HS65_LS_OA33X") { *O[0] <<  HS65_LS_OA33X(this);}
else if(simplified(Type)=="HS65_LS_OAI31X") { *O[0] <<  HS65_LS_OAI31X(this);}
else if(simplified(Type)=="HS65_LS_MUX21I1X") { *O[0] <<  HS65_LS_MUX21I1X6(this);}
else if(simplified(Type)=="HS65_LS_MUXI21X") { *O[0] <<  HS65_LS_MUXI21X(this);}
else if(simplified(Type)=="HS65_LS_MUX21X") { *O[0] <<  HS65_LS_MUX21X9(this);}
else if(simplified(Type)=="HS65_LS_MUX31X") { *O[0] <<  HS65_LS_MUX31X9(this);}
else if(simplified(Type)=="HS65_LS_MX41X") { *O[0] <<  HS65_LS_MX41X7(this);}
else if(simplified(Type)=="HS65_LS_OA212X") { *O[0] <<  HS65_LS_OA212X9(this);}
else if(simplified(Type)=="AOI211_X"){ *O[0] <<  AOI211_X1(this);} //C1 2, C2 3, A 0, B 1
else if(simplified(Type)=="AND3_X" || simplified(Type)=="HS65_LS_AND3X")  { *O[0] <<  AND3_X1(this);}
else if(simplified(Type)=="OAI222_X" || simplified(Type)=="HS65_LS_OAI222X"){ *O[0] <<  OAI222_X1(this);}//A1, A2, B1, B2, C1, C2 
else if(simplified(Type)=="OA222_X" || simplified(Type)=="HS65_LS_OA222X"){ *O[0] <<  OA222_X(this);}//A1, A2, B1, B2, C1, C2 
else if(simplified(Type)=="NOR4_X" || simplified(Type)=="HS65_LS_NOR4X")  { *O[0] <<  NOR4_X1(this);}
else if(simplified(Type)=="HS65_LS_NOR4ABX")  { *O[0] <<  NOR4ABX(this);}
else if(simplified(Type)=="NAND4_X" || simplified(Type)=="HS65_LS_NAND4X") { *O[0] <<  NAND4_X1(this);}
else if(simplified(Type)=="AND4_X" || simplified(Type)=="HS65_LS_NAND4X") { *O[0] <<  AND4_X(this);}
else if( simplified(Type)=="HS65_LS_NAND4ABX" ) { *O[0] <<  HS65_LS_NAND4AB(this);}
else if(simplified(Type)=="HS65_LS_AND4X") { *O[0] <<  HS65_LS_AND4X6(this);}
else if(simplified(Type)=="HS65_LS_OA12X") { *O[0] <<  HS65_LS_OA12X9(this);}
else if(simplified(Type)=="HS65_LS_OAI311X") { *O[0] <<  HS65_LS_OAI311X(this);}
else if(simplified(Type)=="HS65_LS_OAI32X") { *O[0] <<  HS65_LS_OAI32X(this);}
else if(simplified(Type)=="HS65_LS_CB4I1X") { *O[0] <<  HS65_LS_CB4I1X9(this);}
else if(simplified(Type)=="HS65_LS_AO222X") { *O[0] <<  HS65_LS_AO222(this);}
else if(simplified(Type)=="OAI33_X") { *O[0] <<  OAI33_X1(this);}//A1, A2, A3, B1, B2, B3 
else if(simplified(Type)=="AOI222_X"){ *O[0] <<  AOI222_X1(this);}//A1, A2, B1, B2, C1, C2
else if(simplified(Type)=="BUF_X" || simplified(Type)=="HS65_LS_BFX" || simplified(Type)=="CLKBUF_X")   { *O[0] <<  BUF_X1(this);}
else if(simplified(Type)=="OR4_X")   { *O[0] <<  OR4_X1(this);}
// else if(simplified(Type)=="DLH_X")   {cout << Type << "Should not be simulated !" << endl; exit(-1);}
// else if(simplified(Type)=="DFF_X")   {return DFF_X(this).NetVal;}//cout << Type << "Should not be simulated !" << endl; exit(-1);}
// else if(simplified(Type)=="DFFR_X")  {cout << Type << "Should not be simulated !" << endl; exit(-1);}
else if(simplified(Type)=="FA_X" || simplified(Type)=="HS65_LS_FA1X" || simplified(Type)=="HS65_LSS1_FA1X")    {vector<Net> t= FA_X1(this); /*cout << "Warning--Still need to account for reconvergence!" << endl;*/
			    *O[0] << t[0];
			    *O[1] << t[1];

} 
else if(simplified(Type)=="HS65_LS_HA1X" || simplified(Type)=="HS65_LSS1_HA1X")    {vector<Net> t= HS65_LS_HA1X9(this); /*cout << "Warning--Still need to account for reconvergence!" << endl;*/
			    *O[0] << t[0];
			    *O[1] << t[1];

} 
// else if(simplified(Type)=="SDFF_X")  {return  SDFF_X(this).NetVal;}//cout << Type << "Should not be simulated !" << endl; exit(-1);}
else if(simplified(Type)=="CLKGATETST_X") {}
else if(BB.find(simplified(Type))!=BB.end()) { Net *dummy= new Net; bddNetInitialize(ex,dummy);//dummy->NetVal =ex; 
    Net *Zero= new Net;
    //Zero->NetVal = zero;
    bddNetInitialize(zero,Zero);
    for(unsigned x=0; x < O.size();x++){
//     cout << O[x]->Name << endl; 
	if(O[x]->Name=="ffrmmo")
	    *O[x]<< *Zero;
	else
    *O[x]<< *dummy;

    }
    for(unsigned y=0; y < I.size();y++){
        Obs.push_back(true);
    }
    
}
else if(Type.find("SNPS_CLOCK_GATE_HIGH")!=string::npos){
    if(I.size()==3)
    *O[0] <<  ((*I[1] | *I[2]) & *I[0]);
    else
       *O[0] <<  ((*I[1]) & *I[0]);
    assignObservability(this);} // Can be two input also!
else if(SynopsysLatches.find(simplified(Type))==SynopsysLatches.end() && (BB.find(Type)==BB.end())){ cout << " Type " << Type << " not found " << endl;
       cout << " Add to the library above this code!" << endl;
       cout << " The port list is in the order in which it appears in the output file."<< endl;
       cout << " i.e if port map( B =>, C => , A=>) then I[0]=B, I[1]=C, I[2]=A"<< endl;
       exit(-1);}

 
}

