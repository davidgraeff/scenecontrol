#include <QtGui/QApplication>
#include <iostream>
#include <algorithm>
#include <ctype.h>
using namespace std;

string generate_propery_string(string type, string name) {
    string name_upper = name;
    name_upper[0] = toupper(name[0]);
    return "Q_PROPERTY("+type+" "+name+" READ "+name+" WRITE set"+name_upper+");";
}

string generate_getter(string type, string name) {
    return type + " " + name + "() { return m_"+name+"; }";
}

string generate_setter(string type, string name) {
    string name_upper = name;
    name_upper[0] = toupper(name[0]);

    return "void set"+name_upper+"("+type+" "+name+") {m_"+name+" = "+name+";}";
}

string generate_variable(string type, string name) {
    return type + " m_" + name + ";";
}

string generate_init(string type, string name) {
    if (type != "QString" && type != "QStringList")
      return ", m_" + name + "(0)";
    else
      return string();
}

string generate_headerid(string name) {
    return name + "StateTracker_h";
}

string classname(string name) {
    string name_upper = name;
    name_upper[0] = toupper(name[0]);
  return name_upper + "StateTracker";
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    list<string>::iterator it1;
    list<string>::iterator it2;

    cout << "Name:";
    string name;
    cin >> name;

    cout << "--Name:" << name << "\n";

    bool abort = false;
    list<string> propery_names;
    list<string> propery_types;
    int i=0;
    cout << "Now properties. Break with .\n";
    while (!abort) {
        string name;
        string type;
        cout << "Property " << i++ << " (Type Name): ";
        cin >> type >> name;
	if (type=="."||name==".") break;
        cout << "--Property: "<<generate_propery_string(type,name)<<"\n";
        propery_types.push_back(type);
        propery_names.push_back(name);
    }

    cout << "RESULT:\n";
    cout << "#ifndef " << generate_headerid(name) << "\n" << "#define " << generate_headerid(name) << "\n";
    cout << "#include <shared/abstractstatetracker.h>\n\nclass "<<classname(name)<<" : public AbstractStateTracker\n";
    cout << "{\n\tQ_OBJECT\n";
    //props
    for ( it1=propery_types.begin(),it2=propery_names.begin() ; it1 != propery_types.end(); it1++,it2++ ) {
        cout << "\t" << generate_propery_string(*it1,*it2) << "\n";
    }

    cout << "public:\n\t"<<classname(name)<<"(QObject* parent = 0) : AbstractStateTracker(parent)";
    for ( it1=propery_types.begin(),it2=propery_names.begin() ; it1 != propery_types.end(); it1++,it2++ ) {
	cout << generate_init(*it1,*it2);
    }
    cout << "{}\n";
    //getter/setter
    for ( it1=propery_types.begin(),it2=propery_names.begin() ; it1 != propery_types.end(); it1++,it2++ ) {
        cout << "\t" << generate_getter(*it1,*it2) << "\n";
        cout << "\t" << generate_setter(*it1,*it2) << "\n";
    }
    cout << "private:\n";
    // vars
    for ( it1=propery_types.begin(),it2=propery_names.begin() ; it1 != propery_types.end(); it1++,it2++ ) {
        cout << "\t" << generate_variable(*it1,*it2) << "\n";
    }
    cout << "};\n";
    cout << "#endif //" << generate_headerid(name) << "\n";
    return 0;
}
