// Author: Shrinidhi Udupi (shrinidhiku@gmail.com)
// Last updated : 26.07.2017


#include <vector>
#include <map>
using namespace std;


enum value{
    zero='0',one='1',ex='X', un='U'
};


class Cell;

class Net;

class Port;

class Netlist{
public:
    map<string, Port*> Ports;
    map<string, Cell*> Cells;
    map<string, Net*> Nets;
    string Name;  

};

class Cell{
public:
    vector<Net*> InputNets;
    vector<Net*> OutputNets; 
    vector<bool> Obs;// stores irrelevance in vector with positional mapping to InputNets
    string Name;
    bool IsIrrelevant=false;
    string Type;
    void simulate();
    void doObservability();
};

class Net{
public:
    Cell * InputCell=NULL; 
    vector<Cell*> OutputCells;
    vector<Port*> OutputPorts; // Assuming input ports are always connected to Cells. Hence, no InputPorts
    value NetVal=un; 
    string Name; 
    vector<Net*> InputNets; 
    bool Obs=true;
};

class Port:public Net{
public:

    bool Obs=true;// stores irrelevance in vector with positional mapping to InputNets
    bool IsIrrelevant=false;
   // vector<Net*> OutputNets;
    bool InOut; 
    string Type; 
    
};



value to_value(string a);

value operator!(value a);

ostream& operator<<(ostream &, value );
