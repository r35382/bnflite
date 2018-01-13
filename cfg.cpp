//
#pragma warning(disable: 4786)

#include <vector>
#include <iostream>
#include "bnflite.h"

using namespace bnf;
using namespace std;


const char* xml = // this is example of xml-like configuration file
"<client key=\"xxx\" mail=\"asa@asda.com\">"
"<alert type=\"memory\" limit=\"50%\" />"
"<alert type=\"cpu\" limit=\"20%\" />"
"<alert type=\"processes\" limit=\"50\" />"
"</client>";


struct client
{
     string key, mail; 
     vector< pair< string,  string> > prop;
};

vector <struct client> Cfg;  // client configuration container 
string tmpstr;


static bool printMsg(const char* lexem, size_t len)
{   // debug function
    printf("Debug: %.*s;\n", len, lexem);
    return true; // should retuirn true to continue parsing
}

static bool addkey(const char* lexem, size_t len)
{   
    Cfg.resize(Cfg.size() + 1);
    Cfg.back().key = string(lexem + 1, len - 2);
    return true;
}

static bool addmail(const char* lexem, size_t len)
{   
    Cfg.back().mail = string(lexem + 1, len - 2);
    return true;
}

static bool addtype(const char* lexem, size_t len)
{   
    tmpstr = string(lexem + 1, len - 2);
    return true;
}

static bool addlimit(const char* lexem, size_t len)
{   
    Cfg.back().prop.push_back(make_pair(tmpstr, string(lexem + 1, len - 2)));
    return true;
}


int main()
{
    Token value(1,255); value.Remove("\""); // assume the value can contain any character

    Lexem client("client"); // literals
    Lexem key("key");
    Lexem type("type");
    Lexem alert("alert");
    Lexem limit("limit");
    Lexem mail("mail");

    Lexem quotedvalue = "\"" + *value + "\"";
    Lexem _client = Token("<") + Token("/") + client  +">";
    Lexem _end = Token("/") +">";

    Rule xclient = Token("<") + client  + key  + "=" + quotedvalue + addkey
                                        + mail + "=" + quotedvalue + addmail + ">";
    Rule xalert = Token("<") + alert  + type + "=" + quotedvalue + addtype
                                                + limit + "=" +  quotedvalue + addlimit + _end;

    Rule xclient1 = xclient + printMsg;
    Rule xalert1 = xalert + printMsg;
   
    Rule root = *(xclient  + *(xalert) + _client);
    
    const char* tail = 0;
    int tst = Analyze(root, xml, &tail);
    if (tst > 0)
        cout << "Clients configured: " << Cfg.size()  << endl;
    else
        cout << "Parsing errors detected, status = " << hex << tst << endl
         << "stopped at: " << tail << endl;


    for (vector<struct client>::iterator j = Cfg.begin(); j != Cfg.end(); ++j) {
        cout << "Client " << j->key << " has " << (*j).prop.size() << " properties: "; 
        for (vector<pair<string, string> >::iterator i = j->prop.begin(); i != j->prop.end(); ++i) {
            cout << i->first << "="  << i->second <<"; ";
        }
    }

	return  0; 
}
