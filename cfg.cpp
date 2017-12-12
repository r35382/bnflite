//

#include <vector>
#include <iostream>
#include "bnflite.h"

using namespace bnf;
using namespace std;

struct client
{
     string key; 
     string mail;
     vector< pair< string,  string> > prop;
     client(const char* key, size_t lkey, const char* mail, size_t lmail) 
         : mail(mail, lmail), key(key, lkey) {}
};


vector <struct client> Cfg;  // client configuration container 

typedef Interface<int> Gen;  


static bool printMsg(const char* lexem, size_t len)
{   // debug function
    printf("Debug: %.*s;\n", len, lexem);
    return true;
}


Gen DoClient(vector<Gen>& res)
{   // save new client, it is 4th lexem in Rule xclient below
    Cfg.push_back(client(res[4].text, res[4].length, res[7].text, res[7].length));
    return Gen(0, res);
}

Gen DoAlert(vector<Gen>& res)
{   // save new client's property: 4th lexem - name and 7th lexem is property value
   Cfg.back().prop.push_back(make_pair(
       string(res[4].text + 1, res[4].length - 2), string(res[7].text + 1, res[7].length - 2)));
   int i = Cfg.back().prop.size();
   return Gen(0, res);
}


const char* xml = // this is example of xml-like configuration file
"<client key=\"xxx\" mail=\"asa@asda.com\">"
"<alert type=\"memory\" limit=\"50%\" />"
"<alert type=\"cpu\" limit=\"20%\" />"
"<alert type=\"processes\" limit=\"50\" />"
"</client>";

int main()
{
    Token digit1_9('1', '9');
    Token DIGIT("0123456789");
    Lexem i_digit = 1*DIGIT; 

    Token string0(1,255); string0.Remove("\"");
    Lexem quotedstring = "\"" + *string0 + "\"";

    Lexem client = Token("c") + "l" + "i" + "e" + "n" + "t";
    Lexem key = Token("k") + "e" + "y";
    Lexem type = Token("t") + "y" + "p" + "e";
    Lexem alert = Token("a") + "l" + "e" + "r" + "t" ;
    Lexem limit = Token("l") + "i" + "m" + "i" + "t" ;
    Lexem mail = Token("m") + "a" + "i" + "l";
    Lexem _client = Token("<") + Token("/") + client  +">";
    Lexem _end = Token("/") +">";

    Rule xclient = Token("<") + client  + key  + "=" + quotedstring 
                                        + mail + "=" + quotedstring + ">";
    Rule xalert = Token("<") + alert  + type + "=" + quotedstring 
                                                + limit + "=" +  quotedstring  + _end;

    Rule xclient1 = xclient + printMsg;
    Rule xalert1 = xalert + printMsg;
   
    Rule root = *(xclient  + *(xalert) + _client);

  
    Bind(xclient, DoClient);
    Bind(xalert, DoAlert);

    
    const char* tail = 0;
    Gen result;

    int tst = Analyze(root, xml, &tail, result);
    if (tst > 0)
        cout << "Clients configured:" << Cfg.size()  << endl;
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


