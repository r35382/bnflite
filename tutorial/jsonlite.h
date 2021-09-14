/****************************************************************************\
*   h-header of JSON lite parser and repository (based on BNFLite)                *
*   Copyright (c) 2021  Alexander A. Semjonov <alexander.as0@mail.ru>        *
*                                                                            *
*   Permission to use, copy, modify, and distribute this software for any    *
*   purpose with or without fee is hereby granted, provided that the above   *
*   copyright notice and this permission notice appear in all copies.        *
*                                                                            *
*   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES *
*   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF         *
*   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR  *
*   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES   *
*   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN    *
*   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF  *
*   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.           *
\****************************************************************************/
#ifndef JSONLITE_H
#define JSONLITE_H

#include <utility>
#include <iostream>
#include <list>
#include <stack>
#include <vector>
#include <map>
#include <memory>
#include <cmath>
#include "bnflite.h"

using namespace std;
using namespace bnf;

//  repo_t - JSON  Repository Structure
//	first.first - primary key, name of memebr/object/array
//	first.second  - secondary key, the int value of parent object
//  second.first - string value of member or unic string id for object/array
//  second.second - auxiliary field
typedef std::map <std::pair<std::string, int>, std::pair<std::string, int>> repo_t;

class Repo : public  std::shared_ptr<repo_t>
{
private:
    repo_t::const_iterator curitr;
	void SetRoot() { curitr = (*this)->find(make_pair("-1", -1)); }
    Repo(const Repo& sp, repo_t::const_iterator f) : std::shared_ptr<repo_t>(sp), curitr(f) {};
    Repo(repo_t* r) : std::shared_ptr<repo_t>(r) 
        {   curitr = (*this)->find(make_pair("-1", -1)); };

public:
    Repo() : std::shared_ptr<repo_t>( new repo_t() ), curitr(this->get()->begin()) 
        {};
    Repo(const Repo& sp) : std::shared_ptr<repo_t>(sp), curitr(sp.curitr)
        {}
    bool IsError() const
        {   return curitr == (*this)->end(); }
    std::string operator*() const 
        {   return curitr == (*this)->end()? "ERROR" : curitr->second.first; }
	operator std::string() const 
        {   return curitr == (*this)->end()? "ERROR" : curitr->second.first; }
    std::string ToString()  
        {   return curitr == (*this)->end()? "ERROR" :
                                curitr->second.first[0] != '\"' ? curitr->second.first :
                                    string(curitr->second.first.c_str() + 1, curitr->second.first.size() - 2); }
    double ToDouble()  
        {   return curitr == (*this)->end()? 0 : stod(curitr->second.first); }
    int Size()  
        {   return curitr == (*this)->end()? 0 : curitr->second.second; }
	Repo operator() (const char* key) 
        {   return  Repo( *this, curitr == (*this)->end()? (*this)->end() :
                                           (*this)->find(make_pair(key, std::stoi(curitr->second.first)))); }
    Repo operator[](size_t i) 
        {   return  Repo( *this, curitr == (*this)->end()? (*this)->end() :
                                            (*this)->find(make_pair(to_string(i), std::stoi(curitr->second.first)))); }
    Repo operator()(const char* key, size_t i) 
        {    return (this->operator()(key)).operator[](i); }
    void dumptree(std::ostream& out = std::cout)
        {   out << std::endl << "\\" << std::endl;
            string str0(" ");
            if (curitr != this->get()->end()) _dumptree(*curitr, str0, out);  }
    static Repo ParseJSON(const char* text, int* status, const char** pstop = 0)
        {   Rule root, element;
            JSONGramma(root, element);
            gramma_callback(eStart, 0, 0);
            int tst = Analyze(root, text, pstop);
	        if (status) *status = tst;
	        return tst >= 0 ? Repo(gramma_callback(eGetRepo, 0, 0)) : Repo();   }

protected:
    void _dumptree(const std::pair<std::pair<std::string, int>, std::pair<std::string, int>>& lroot, string& str, std::ostream& out )
    {
	    size_t len = str.size(); 
        int lvl = std::stoi(lroot.second.first); // next level
        int cnt = std::abs(lroot.second.second);
        for (const std::pair<std::pair<std::string, int>, std::pair<std::string, int>>& obj: *this->get()) {
            if (lvl != obj.first.second) continue;
 		    str += --cnt? "\xC3\xC4\xC4" : "\xC0\xC4\xC4";
            str += obj.first.first;
		    if( obj.second.second == 0 ) { /* subobjects? array? */
                str +=  " = ";
                str +=  obj.second.first;
                out << str << std::endl;
			    str.resize(len);
		    } else {
                out << str << std::endl;
			    str.resize(len);
			    str += cnt ? "\xB3  " :  "   ";
			    _dumptree(obj, str, out);
			    str.resize(len);
		    }
        }
    }

    enum ParserEvent { eStart, eSaveVariable, ePutPalint, ePushKey, ePopKey, eGetRepo };
    static bool SetLastMember(const char* lexem, size_t len) 
        {   gramma_callback(eSaveVariable, lexem, len); return 1; }
    static bool PopKey(const char* lexem, size_t len) 
        {   gramma_callback(ePopKey, lexem, len); return 1; }
    static bool PutPlain(const char* lexem, size_t len) 
        {   gramma_callback(ePutPalint, lexem, len); return 1; }
    static bool PushKey(const char* lexem, size_t len) 
        {   gramma_callback(ePushKey, lexem, len); return 1; } 

    static repo_t* gramma_callback(enum ParserEvent event, const char* lexem, size_t len)
    {
        static int num = 0;
        static string last_member;
        static stack<repo_t::iterator> level;
        static repo_t* repo;

        switch (event) {
        case eStart:
            repo = new repo_t;
            num = 0;
            last_member.erase();
            while (!level.empty()) {
                level.pop();
            }
            break;
        case eSaveVariable:
            if (lexem[0] =='\"' && lexem[len - 1] =='\"') last_member.assign(lexem + 1, len - 2);
            else last_member.assign(lexem, len);
            break;
        case ePutPalint:
            if (last_member.empty()) {
                last_member = to_string(level.top()->second.second++);
            } else {
                level.top()->second.second--;
            }
            repo->insert(make_pair(make_pair(last_member, 
                stoi(level.top()->second.first)), make_pair(string(lexem, len), 0)));
            last_member.erase();
            break;
        case ePushKey:
            if (last_member.empty()) {
                last_member = level.empty()? "-1" : to_string(level.top()->second.second++);
            } else if (!level.empty()){
                level.top()->second.second--;
            }
            level.push(repo->insert(make_pair(
                make_pair(last_member.empty()? string(1, *lexem) : last_member, 
                        level.empty()? -1 : stoi(level.top()->second.first)), 
                make_pair(to_string(num++), 0))).first);
            last_member.erase();
            break;
       case ePopKey:
            level.pop();
            break;
        case eGetRepo:
            return repo;
        }
        return 0;
    }
    static void JSONGramma(Rule& root, Rule& element)
    {
	    Lexem ws = *Token("\x20\x0A\x0D\x09"); // will be not used due to standard pre-parsing in this implementation
	    Lexem sign =  !Token("+-");
	    Token onenine('1', '9');
	    Lexem digit =  "0" |  onenine;
	    Lexem digits = *digit;

	    Lexem integer =    digit |  (onenine  + digits) |  ("-" + digit) | ("-" + onenine + digits);
	    Lexem fraction = !("." + digits);
	    Lexem exponent = !("Ee" + sign + digits);
	    Lexem number = integer + fraction + exponent;

	    Lexem hex =  digit | Token('A', 'F') | Token('a', 'f');
	    Lexem escape = Token("\"\\/bfnrt") | ("u" + 4 * hex);

        Token any(0x20, 255);
        any.Remove("\"\\");
	    Lexem character = any | ("\\" + escape);
        Lexem characters = *character;
        Lexem string = "\"" + characters + "\"";

        Rule elements =  element + *("," + element);
        Rule member = string + SetLastMember + ":" + element;
        Rule members =  member + *("," + member);
        Rule array = Token("[") + PushKey + !elements + "]" + PopKey;
        Rule object = Token("{") + PushKey + !members + "}" + PopKey;
        Rule plain =  string | number | Lexem("true") | Lexem("false") | Lexem("null");
        Rule value =  object | array | (plain + PutPlain);
 
        element = value; 
        root =  element;
    }
};
#endif // JSONLITE_H
