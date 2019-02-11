// Author: Shrinidhi Udupi (shrinidhiku@gmail.com)
// Last updated : 26.07.2017


#ifndef DESIGN_MODEL_H
#define DESIGN_MODEL_H
#include <vector>
#include <map>
#include <set>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include "cudd.h"
using namespace std;


 /************************************************************************/
  /* Comparator for case-insensitive comparison in STL assos. containers  */
  /************************************************************************/
  struct ci_less : std::binary_function<std::string, std::string, bool>
  {
    // case-independent (ci) compare_less binary function
    struct nocase_compare : public std::binary_function<unsigned char,unsigned char,bool> 
    {
      bool operator() (const unsigned char& c1, const unsigned char& c2) const {
          return tolower (c1) < tolower (c2); 
      }
    };
    bool operator() (const std::string & s1, const std::string & s2) const {
      return std::lexicographical_compare 
        (s1.begin (), s1.end (),   // source range
        s2.begin (), s2.end (),   // dest range
        nocase_compare ());  // comparison
    }
  };
enum value{
    zero='0',one='1',ex='X', un='U'
};


class Cell;

class Net;

class Port;

const string latchTypes[]={ "DFF_X" ,
                            "SDFF_X" ,
                            "DFFR_X",
                            "DLH_X", 
                            "DLL_X",
                            "SDFFR_X",
                            "DFFS_X",
                            "DFFRS_X",
                            "SDFFRS_X",
                            "SDFFS_X",
                            "HS65_LS_DFPRQX", 
                            "HS65_LS_LDHQX", 
                            "HS65_LS_DFPQX", 
                            "HS65_LS_DFPQNX", 
                            "HS65_LS_DFPRQNX", 
                            "HS65_LSS_DFPQX",
                            "HS65_LS_DFPHQX",
                            "HS65_LS_DFPSQX",
                            "HS65_LS_DFPSQNX",
                            "HS65_LS_LDHQNX",
                            "HS65_LS_LDLRQX"
}; // Increment count below
const std:: set<string> SynopsysLatches(latchTypes,latchTypes+20);

const string nonFF[]={	"DLH_X","HS65_LS_LDHQX","DLL_X" };
const std:: set<string> nonFFs(nonFF,nonFF+3);
const string blackboxes[]={"osa93_reg_wrap","quicksort_s_a"};
const std:: set<string> BB(blackboxes,blackboxes+2);
void bddNetInitialize(value v, Net* n, bool inv=false);
    //DdManager *manager;
class Netlist{
public:

    friend class boost::serialization::access;


    map<string, Port*,ci_less> Ports;
    map<string, Cell*,ci_less> Cells;
    map<string, Net*,ci_less> Nets;
    string Name;  
    unsigned endingLine;

        template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & Ports;
        ar & Cells;
        ar & Nets;
	ar & Name;
	ar & endingLine;

    }
};

class Cell{
public:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
//         ar & InputNets;
//         ar & OutputNets;
        ar & Obs;
	ar & IsIrrelevant;
	ar & isInitialized;
	ar & Type;
	ar & Name;
	ar & associationList;
    }

    vector<Net*> InputNets; // Incoming nets, can be input ports too.
    vector<Net*> OutputNets;// Outgoing nets, can be output ports too.
    vector<bool> Obs;       // stores irrelevance in vector with positional mapping to InputNets
    string Name;
    vector<pair<string,string> > associationList;
    bool IsIrrelevant=false;
    bool isInitialized=false;
    string Type;
    value simulate();
    void doObservability();
    void printGloblQ();
    Net * gAct( string formal);
};

class Net{
public:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & InputCell;
//         ar & OutputCells;
//         ar & OutputPorts;
	ar & NetVal;
	ar & Name;
// 	ar & InputNets;

    }

    Cell * InputCell=NULL; 
    vector<Cell*> OutputCells;
    vector<Port*> OutputPorts; // Assuming input ports are always connected to Cells. Hence, no InputPorts
    value NetVal=un; 
    string Name; 
    vector<Net*> InputNets; 
    void setObs(bool );
    bool getObs( );
    DdNode *bddNetNode;
    Net(){
	//cout << "Initializing net bdd for " << Name << endl;
	bddNetInitialize(ex,this);
    }
    
private:
    bool Obs=true;
};

class Port:public Net{
public:
    friend class boost::serialization::access;
    

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
	ar & boost::serialization::base_object<Net>(*this);
        ar & Obs;
        ar & IsIrrelevant;
        ar & isInitialized;
	ar & InOut;
	ar & Type;
	ar & determinationEx;
    }

    bool Obs=true;// stores irrelevance in vector with positional mapping to InputNets
    bool IsIrrelevant=false;
   // vector<Net*> OutputNets;
    bool isInitialized=false;
    bool InOut;  // true for input
    string Type; 
    string determinationEx="";


    
};



string simplified(string s);


value to_value(string a);

value operator!(value a);

ostream& operator<<(ostream &, value );

#endif
