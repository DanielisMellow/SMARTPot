#include <iostream>
#include <string>
#include <complex>
#include <cmath>
#include <math.h>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>

//testing commit function

#define M_PI 3.14159265358979323846 /* pi */

using namespace std;

class component
{
public:
    virtual ~component() {}
    virtual std::string type() = 0; // Component name
    virtual char getConn() = 0;     // get connection type
    virtual std::string compInfo(double) = 0;
    virtual bool getNested() = 0;                        // get nested yes/no
    virtual complex<double> getComponentImp(double) = 0; // get component impedance
    virtual double getComponentPhi() = 0;                // get component phase shift
    virtual double getVal() = 0;                         // get the defining value

    //virtual void compInfo(double)=0; // Component info
    //virtual bool getNested()=0; // get nested bool
};


//CONCRETE RESISTOR CLASS
class resistor : public component
{
    double R;  // the resistance, R
    bool n;    // nested boolean
    char conn; // series or parallel choice
public:
    resistor();                   // default constructor
    resistor(double, char, bool); // parameterised constructor for nested circuits
    std::string type();
    double getR();                           // get the resistance
    void setR(double);                       // set the resistance
    double getVal();                         // get value
    bool getNested();                        // get nested yes/no
    char getConn();                          // get connection type
    complex<double> getComponentImp(double); // get component impedance
    double getComponentPhi();                // get component phase shift
    std::string compInfo(double);
    ~resistor() {} // destructor

    //void setZ(double );// set the resistance
    //resistor( double, char); // parameterised constructor
    //void compInfo(double);
};

//CONCRETE INDUCTOR CLASS
class inductor : public component
{
    double L;  // the inductance, L
    bool n;    // nested bool
    char conn; // series or parallel choice
public:
    inductor();                              // default constructor
    inductor(double, char, bool);            // parameterised constructor
    std::string type();                      // component name
    double getL();                           // get the inductance
    void setL(double);                       // set the inductance
    double getVal();                         // return value
    bool getNested();                        //get nested yes/no
    char getConn();                          // get connection type
    complex<double> getComponentImp(double); // get component impedance
    double getComponentPhi();                // get component phase shift
    std::string compInfo(double);
    ~inductor() {} // destructor

    //inductor(double, char); // parameterised constructor
};

//CONCRETE CAPACITOR CLASS
class capacitor : public component
{
    double C;  // the capacitance, C
    bool n;    // nested yes/no
    char conn; // series or parallel choice
public:
    capacitor();                   // default constructor
    capacitor(double, char, bool); // parameterised constructor nested
    std::string type();
    double getC();     // get the resistance
    void setC(double); // set the resistance
    double getVal();
    bool getNested();                        // nested yes/no
    char getConn();                          // get connection type
    complex<double> getComponentImp(double); // get component impedance
    double getComponentPhi();                // get component phase shift
    std::string compInfo(double);
    ~capacitor() {} // destructor

    //capacitor( double, char ); // parameterised constructor
};

//RESISTOR FUNCTIONS
resistor::resistor() // default constructor with L=0
{
    R = 0.;
}

resistor::resistor(double r_in, char c_in, bool nested_in)
{
    R = r_in;
    conn = c_in;
    n = nested_in;
}

std::string resistor::type()
{
    return "resistor";
}

double resistor::getR() // get the inductance
{
    return R;
}

void resistor::setR(double r_in) // set the inductance
{
    R = r_in;
}

double resistor::getVal() // return value
{
    return getR();
}

char resistor::getConn() // get connection type
{
    return conn;
}

bool resistor::getNested() // get nested yes/no
{
    return n;
}

std::string resistor::compInfo(double w) // cout component info
{
    string info = "Resistor \tR = " + to_string(this->getR()) + "\t(Ohms)\tin " + (getConn() == 's' ? "ser." : "par.") + ",\tZ = " + to_string(abs(this->getComponentImp(w))) + " (Ohms),\t(phi) = " + to_string(this->getComponentPhi()) + "\n";

    return info;
}

complex<double> resistor::getComponentImp(double w) // get component impedance
{
    return complex<double>(R, 0. * w);
}

double resistor::getComponentPhi() // get component phase shift
{
    return 0.;
}

//INDUCTOR FUNCTIONS
inductor::inductor() // default constructor with L=0
{
    L = 0.;
}

inductor::inductor(double l_in, char c_in, bool nested_in) // parameterised constructor
{
    L = l_in;
    conn = c_in;
    n = nested_in;
}

std::string inductor::type()
{
    return "inductor";
}

double inductor::getL() // get the inductance
{
    return L;
}

void inductor::setL(double l_in) // get the inductance
{
    L = l_in;
}

double inductor::getVal() // return value
{
    return getL();
}

bool inductor::getNested() // return value
{
    return n;
}

char inductor::getConn() // get connection type
{
    return conn;
}

std::string inductor::compInfo(double w) // cout component info
{
    string info = "Inductor \tL = " + to_string(this->getL()) + "\t(H)\tin " + (getConn() == 's' ? "ser." : "par.") + ",\tZ = " + to_string(abs(this->getComponentImp(w))) + " (Ohms),\t(phi) = " + to_string(this->getComponentPhi()) + "\n";

    return info;
}

complex<double> inductor::getComponentImp(double w) // get component impedance
{
    return complex<double>(0., w * L);
}

double inductor::getComponentPhi() // get component phase shift
{
    return M_PI / 2.;
}

// CAPACITOR FUNCTIONS
capacitor::capacitor() // default constructor with L=0
{
    C = 0.;
}

capacitor::capacitor(double C_in, char c_in, bool nested_in)
{
    C = C_in;
    conn = c_in;
    n = nested_in;
}

std::string capacitor::type()
{
    return "capacitor";
}

double capacitor::getC() // get the inductance
{
    return C;
}

void capacitor::setC(double c_in) // set the inductance
{
    C = c_in;
}

double capacitor::getVal() // return value
{
    return getC();
}

char capacitor::getConn() // get connection type
{
    return conn;
}

bool capacitor::getNested() // get nested boolean
{
    return n;
}

std::string capacitor::compInfo(double w) // cout component info
{
    /*  std::cout.precision(3); // fixed precision
    std::cout.setf( std::ios::fixed, std:: ios::floatfield ); // floatfield set to fixed
    std::cout<<"capacitor with\tC = "<<(this->getC())<<"\tF\tin "<<(getConn()=='s' ? "ser." : "par." )<<
    ",\tZ = "<<this->getComponentImp(w)<<" (Ohms),\t(phi) = "<<this->getComponentPhi()/M_PI<<(this->getComponentPhi()==0 ? "" : " pi" ) << (this->getNested() ? ", nested" :"")<<std::endl; */

    string info = "Capacitor \tF = " + to_string(this->getC()) + "\t(F)\tin " + (getConn() == 's' ? "ser." : "par.") + ",\tZ = " + to_string(abs(this->getComponentImp(w))) + " (Ohms),\t(phi) = " + to_string(this->getComponentPhi()) + "\n";

    return info;
}

complex<double> capacitor::getComponentImp(double w) // get component impedance
{
    return complex<double>(0., -1 / (w * C));
}

double capacitor::getComponentPhi() // get component phase shift
{
    return -M_PI / 2.;
}

//CIRCUIT CLASS
class circuit
{
private:
    complex<double> Z;        // impedance of circuit
    double w;                 // angular of AC driving force
    vector<component *> comp; // vector of pointers to components
    int nodeCount;

public:
    circuit();                                                      // default constructor with w = 2𝜋f and f = 60 Hz
    circuit(double);                                                // parameterised constructor with w = 2𝜋f_in
    void computeZ(bool doNested = true);                            // function to compute the impedance and store it in member data
    complex<double> componentSZ(vector<component *> const &) const; // function to compute impedance of a serial circuit
    complex<double> componentPZ(vector<component *> const &) const; // function to compute impedance of a parallel circuit
    complex<double> getZ() const;                                   // function to retrieve the impedance of the circuit
    double magZ() const;
    void setF(double);                                // set frequency
    void setOmega(double);                            // set angular frequency
    double getF() const;                              // get frequency
    double getOmega() const;                          // get omega
    double getPhi() const;                            // get phase difference
    void addComp(const string &, double, char, bool); // add component
    void removeComp(int);                             // function to remove component by #
    unsigned long int Ncomp() const;                  // number of components

    void computeNodes();
    int geNodeCount();
    vector<string> nodeList(); // generate and print circuit info

    vector<string> info(); // generate and print circuit info



    ~circuit() {} // circuit destructor
};

//CIRCUIT FUNCTIONS
circuit::circuit() // default constructor with w = 2𝜋f and f = 60 Hz
{
    w = 2. * M_PI * 60.;
}

circuit::circuit(double f_in) // parameterised constructor with w = 2𝜋f_in, default is 60
{
    w = 2. * M_PI * f_in;
}

void circuit::computeZ(bool doNested) // compute the impedance of the circuit
{
    // Count subsequent components in series and submit them into the componentZ function
    shared_ptr<vector<component *>> serialOut(new vector<component *>);   // create  a circuit of components in series to send off to the componentZ function
    shared_ptr<vector<component *>> parallelOut(new vector<component *>); // create a circuit of components in parallel to send off to the componentPZ function
    unique_ptr<vector<complex<double>>> nestedImpCircuitImport(new vector<complex<double>>);
    complex<double> totalImpe;

    if (doNested)
    {
        // make local copy
        circuit *circSub = new circuit(*this);                // make local copy for the operations
        shared_ptr<vector<complex<double>>> nestedImpCircuit; // (new vector<complex<double>>);
        shared_ptr<circuit> nestedOut;                        // (new circuit()); // create sub-circuit for nested

        bool doneNested = false;
        for (auto d = circSub->comp.begin(); d != circSub->comp.end(); ++d)
        {
            if ((*d)->getNested() == true)
            {
                nestedOut->addComp((*d)->type(), (*d)->getVal(), (*d)->getConn(), false);
                doneNested = true;
            }
        }
        if (doneNested)
        {
            nestedOut->setF(this->getF()); // call this function on the nestedOut circuit, give it the same frequency
            nestedOut->computeZ(false);
            nestedImpCircuit->push_back(nestedOut->getZ());
            *nestedImpCircuitImport = *nestedImpCircuit;
        }
    }

    for (auto c = comp.begin(); c != comp.end(); ++c)
    {
        if ((*c)->getConn() == 's')
        {
            serialOut->push_back(*c);
            totalImpe += componentSZ(*serialOut);
            serialOut->clear();
        }
        else if ((*c)->getConn() == 'p')
        { // if component is in parallel, also send of to the componentZ function (with one component)

            parallelOut->push_back(*c);
            if (abs(totalImpe) == 0)
            { //Avoid Impedance error if the first RLC segment is in parallel
                totalImpe = componentPZ(*parallelOut);
            }
            else
            {
                totalImpe = (componentPZ(*parallelOut) * totalImpe) / (componentPZ(*parallelOut) + totalImpe); // Otherwise add the segment impedance to total impedance
            }
            parallelOut->clear();
        }
    }

    // return the calculated impedance
    Z = totalImpe;
}

complex<double> circuit::componentSZ(vector<component *> const &in) const // compute the impedance of a (circuittor of) component(s)
{
    //Determine the serial impedance of one component
    double R = 0., X_L = 0., X_C = 0.; // define doubles for the reactances
    auto c = in.begin();

    if ((*c)->type() == "resistor")
    {
        R = (*c)->getVal(); // for resistor: R = R
        complex<double> ZR(R, 0);
        return ZR;
    }
    else if ((*c)->type() == "capacitor")
    {
        X_C = -1. / (w * (*c)->getVal()); // for capacitor: X_C = -1/wc
        complex<double> ZC(0, X_C);
        return ZC;
    }
    else
    {
        X_L = w * (*c)->getVal(); // for inductor X_L = wL
        complex<double> ZL(0, X_L);
        return ZL;
    }
}

complex<double> circuit::componentPZ(vector<component *> const &in) const // compute the impedance of a (circuittor of) component(s)
{
    //Determine the parallel Impedance
    double R = 0., X_L = 0., X_C = 0.; // define doubles for the reactances
    auto c = in.begin();

    if ((*c)->type() == "resistor")
    {
        R = (*c)->getVal(); // for resistor: R = R
        complex<double> ZR(R, 0);
        return pow((1.0 / ZR), -1);
    }
    else if ((*c)->type() == "capacitor")
    {
        X_C = -1. / (w * (*c)->getVal()); // for capacitor: X_C = -1/wc
        complex<double> ZC(0, X_C);
        return pow((1.0 / ZC), -1);
    }
    else
    {
        X_L = w * (*c)->getVal(); // for inductor X_L = wL
        complex<double> ZL(0, X_L);
        return pow((1.0 / ZL), -1);
    }
}

complex<double> circuit::getZ() const // get the complex impedance
{
    return Z;
}

double circuit::magZ() const // get magnitude
{
    return abs(getZ());
}

void circuit::setF(double f_in)
{ // set frequency
    w = 2. * M_PI * f_in;
}

void circuit::setOmega(double w_in)
{ // set angular frequency
    w = w_in;
}

double circuit::getF() const // get frequency
{
    return w / (2. * M_PI);
}

double circuit::getOmega() const // get omega
{
    return w;
}

double circuit::getPhi() const // get phi
{
    return arg(Z);
}

unsigned long int circuit::Ncomp() const // return the number of components
{
    return comp.size();
}

void circuit::addComp(const string &type, double a, char d, bool n) // create nested component
{
    if (type == "resistor")
    {
        comp.push_back(new resistor(a, d, n));
    }
    else if (type == "capacitor")
    {
        comp.push_back(new capacitor(a, d, n));
    }
    else if (type == "inductor")
    {
        comp.push_back(new inductor(a, d, n));
    }
}

void circuit::removeComp(int n) // remove component
{
    delete comp[n];
}

void circuit::computeNodes()
{
    //calculate the number of nodes by iterating through the circuit components

    int nodes;
    for (int i = 0; i < comp.size(); i++)
    {
        if (i + 1 != comp.size())
        {
            if (comp.at(i)->getConn() == 's')
            {
                nodes++;
            }
        }

        if (comp.at(i)->getConn() == 'p')
        {
            if (comp.at(i + 1)->getConn() == 's')
            {
                nodes++;
            }
        }
    }
    nodeCount = nodes;
}

int circuit::geNodeCount()
{
    return nodeCount;
}

vector<string> circuit::nodeList()
{
    vector<string> nodeTree;
    int node = geNodeCount();
    vector<int> location;

    int nodes = 0;
    for (int i = 0; i < comp.size(); i++)
    {
        if (i + 1 != comp.size())
        {
            if (comp.at(i)->getConn() == 's')
            {
                location.push_back(i);
                nodes++;
            }
        }

        if (comp.at(i)->getConn() == 'p')
        {
            if (comp.at(i + 1)->getConn() == 's')
            {
                location.push_back(i);
                nodes++;
            }
        }
    }

    for (int i = 0; i < nodes; i++)
    {
        if (i == 0)
        {
            for (int j = 0; j < location.at(i+1) + 1; j++)
            {
                //cout << "Node: " << nodes-i << " " << comp.at(j)->compInfo(w) << endl;
                nodeTree.push_back("Node: " + to_string(nodes-i) + " " + comp.at(j)->compInfo(w));

            }
        }
        else if (i > 0)
        {
            for (int j = location.at(i); j < location.at(i) + 2; j++)
            {
                //cout << "Node: " << nodes-i << " " << comp.at(j)->compInfo(w) << endl;
                nodeTree.push_back("Node: " + to_string(nodes-i) + " " + comp.at(j)->compInfo(w));
            }
        }
    }

    return nodeTree;
}

vector<string> circuit::info()
{

    vector<string> elements;
    int counter = comp.size();
    for (auto c = comp.begin(); c != comp.end(); ++c)
    {
        elements.push_back("Component " + to_string(counter) + " " + (*c)->compInfo(w));
        //(*c)->compInfo(w);
        --counter;
    }
    return elements;
}




// USER INTERFACE
int main()
{

    /*resistor r1;
    r1.setR(100);
    string hello = r1.compInfo(60);
    cout << hello; */

    cout << "You will now create your own circuit: \n";

    shared_ptr<circuit> userCircuit(new circuit());

    cout << "Enter the circuit's AC frequency in Hz:\n"; // ask for AC Frequency
    string inputFreq;                                    // pointer to char to store input frequency
    bool freqIn = true;
    while (freqIn)
    {
        getline(std::cin, inputFreq); // cin user answer
        if (std::atof(inputFreq.c_str()) > 0)
        {
            cout << "Your frequency was stored.\n";
            freqIn = false;
        }
        else
        {
            cerr << "Input invalid, must be a double > 0. Try again.\n";
        }
    }
    userCircuit->setF(std::atof(inputFreq.c_str())); // set the circuit's frequency

    do
    {
        cin.clear();                                                                       // flush the cin buffer
        cout << "Component Type: Resistor, Inductor, Capacitor (r/l/c).\t\t\t quit(q).\n"; // prompt user
        string inputChoice;
        string compType = ""; // string to hold type of component
        string compVal = "";  // string to hold the component's defining value
        double compValDouble = 0;
        bool good = true; // define a Boolean that keeps the loop alive
        while (good)
        {
            // cin >> inputChoice; // let user input value
            getline(cin, inputChoice);
            if (inputChoice == "l")
            {
                cout << "Enter the inductance in H:\n";
                getline(cin, compVal);
                if (atof(compVal.c_str()) <= 0)
                {
                    cout << "You must enter a non-negative value. Try again.\n";
                    cout << "What component would you like to add? Type resistor, capacitor or inductor. Type q if you want to finish.\n"; // prompt answer
                    good = true;
                }
                else
                    good = false;
                compType = "inductor";
                compValDouble = std::atof(compVal.c_str());
            }
            else if (inputChoice == "r")
            {
                cout << "Enter the resistance in (Ohms):\n";
                getline(cin, compVal);
                if (atof(compVal.c_str()) <= 0)
                {
                    cout << "You must enter a non-negative value. Try again.\n";
                    cout << "What component would you like to add? Type resistor, capacitor or inductor. Type q if you want to finish.\n"; // prompt answer
                    good = true;
                }
                else
                    good = false;
                compType = "resistor";
                compValDouble = std::atof(compVal.c_str());
            }
            else if (inputChoice == "c")
            {
                cout << "Enter the capacitance in F:\n";
                ;
                getline(cin, compVal);
                if (atof(compVal.c_str()) <= 0)
                {
                    cout << "You must enter a non-negative value. Try again.\n";
                    cout << "What component would you like to add? Type resistor, capacitor or inductor. Type q if you want to finish.\n"; // prompt answer
                    good = true;
                }
                good = false;
                compType = "capacitor";
                compValDouble = std::atof(compVal.c_str());
            }
            else if (inputChoice == "q")
                break; // Exit if user types q
            else
            {
                cerr << "Input invalid. You must type (r/l/c). Try again.\n";
            }
        }
        if (inputChoice == "q")
            break;
        cout << "The Components is in Series or Parallel: (s/p)\n"; // ask for s or p connection
        string connectionChoice;                                    // char for user answer
        bool connIn = true;
        while (connIn)
        {
            getline(std::cin, connectionChoice); // cin user answer
            if (connectionChoice.size() == 1 && (connectionChoice[0] == 's' || connectionChoice[0] == 'p'))
            {
                cout << "Your connection type was accepted.\n";
                connIn = false;
            }
            else
            {
                cerr << "Input invalid, you must enter s or p. Try again.\n";
            }
        }
        bool nestBool = false;
        userCircuit->addComp(compType, compValDouble, connectionChoice[0], nestBool); // create component using the addComp function
    } while (true);

    cout << "We'll compute your circuit's impedance now: userCircuit->computeZ()\n";
    userCircuit->computeZ(); // compute the circuit's impedance
    cout << userCircuit->getZ() << endl;
    cout << "|Zeq| = " << userCircuit->magZ() << "(Ohms)\n " << endl;

    vector<string> componentList = userCircuit->info();

    for (int i = 0; i != componentList.size(); ++i)
    {
        cout << componentList.at(i) << endl;
    }

    userCircuit->computeNodes();
    cout << "Amount of Node(s): " << userCircuit->geNodeCount() << endl
         << endl;

    vector<string> NODEList = userCircuit->nodeList();

     for (int i = 0; i != NODEList.size(); ++i)
    {
        cout << NODEList.at(i) << endl;
    }

    return 0;
}
