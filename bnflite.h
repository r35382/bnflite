
/*************************************************************************\
*   BNF Lite is a C++ template library for lightweight grammar parsers    *
*   Copyright (c) 2017 by Alexander A. Semjonov.  ALL RIGHTS RESERVED.    *
*                                                                         *
*   Permission is hereby granted, free of charge, to any person           *
*   obtaining  a copy of this software and associated documentation       *
*   files (the "Software"), to deal in the Software without restriction,  *
*   including without limitation the rights to use, copy, modify, merge,  *
*   publish, distribute, sublicense, and/or sell copies of the Software,  *
*   and to permit persons to whom the Software is furnished to do so,     *
*   subject to the following conditions:                                  *
*                                                                         *
*   The above copyright notice and this permission notice shall be        *
*   included in all copies or substantial portions of the Software.       *
*                                                                         *
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       *
*   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    *
*   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*
*   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  *
*   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  *
*   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH         *
*   THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.            *
\*************************************************************************/

#ifndef BNFLITE_H
#define BNFLITE_H

#include <string.h>
#include <string>
#include <list>
#include <vector>
#include <bitset>
#include <map>
#include <algorithm>
#include <typeinfo>

namespace bnf
{
// BNF (Backus-Naur form) is a notation for describing syntax of computer languages
// BNF Lite is the source code template library implementing the way to support BNF specifications
// BNF Terms:
// * Production rule is formal BNF expression which is a conjunction of a series
//   of more concrete rules:
//      production_rule ::= <rule_1>...<rule_n> | <rule_n_1>...<rule_m>;
// * e.g.
//      <digit> ::= <0> | <1> | <2> | <3> | <4> | <5> | <6> | <7> | <8> | <9>
//      <number> ::= <digit> | <digit> <number>
//      where the number is just a digit or another number with one more digit;
// Now this example can be presented in C++ friendly notation:
//      Lexem Digit = Token("0") | "1"  | "2" | "4" | "5" | "6" | "7" | "8" | "9";
//      RULE(Number) = Digit | Digit + Number;
// where:
//      * Token is a terminal production;
//      * Lexem (or LEXEM) is a lexical production;
//      * Rule (or RULE) is used here as synonym of syntax production
// To parse any number (e.g. 532) it is just enough to call the bnf::Analyze(Number, "532")

enum Limits {   maxCharNum = 256, maxLexemLength = 1024, maxRepeate = 4096, maxEmptyStack = 16
            };
enum Status {   eNone = 0, eOk = 1,
                eRet = 0x8, e1st = 0x10, eSkip = 0x20, eTry = 0x40, eNull = 0x80,
                eRest = 0x0100, eNoData = 0x0200, eOver = 0x0400, eEof = 0x0800,
                eBadRule = 0x1000, eBadLexem = 0x2000, eSyntax = 0x4000,
                eError = ((~(unsigned int)0) >> 1) + 1
            };

class _Tie; class _And; class _Or; class _Cycle;

/* context class to support the first kind of callback */
class _Base // base parser class
{
public:
    std::vector<const char*> cntxV; // public for internal extensions
protected:  friend class Token; friend class Lexem; friend class Rule;
            friend class _And;  friend class _Or;   friend class _Cycle;
    int level;
    const char* pstop;
    int _chk_stack()
        {   static const char* org; static int cnt;
            if (org != cntxV.back()) { org = cntxV.back(); cnt = 0; }
            else if (++cnt > maxEmptyStack) return  eOver|eError;
            return 0; }
    const char* (*zero_parse)(const char*);
    int catch_error(const char* ptr) // attempt to catch general syntax error
        { return eSyntax|eError; }
    virtual void _erase(int low, int up = 0)
        {   cntxV.erase(cntxV.begin() + low,  up? cntxV.begin() + up : cntxV.end() ); }
    virtual std::pair<void*, int> _pre_call(void* callback)
        {   return std::make_pair((void*)0, 0); }
    virtual void _post_call(std::pair<void*, int> up)
        {};
    virtual void _do_call(std::pair<void*, int> up, void* callback, size_t org, const char* name)
        {};
    virtual void _stub_call(size_t org, const char* name)
        {};
public:
    int _analyze(_Tie& root, const char* text, size_t*);
    _Base(const char* (*pre)(const char*)) : level(1), pstop(0), zero_parse(pre?pre:base_parser)
        {};
    virtual ~_Base()
        {};
    // default pre-parser procedure to skip special symbols
    static const char* base_parser(const char* ptr)
        {   for (char cc = *ptr; cc != 0; cc = *++ptr) {
                if (cc != ' ' && cc !='\t' && cc != '\n' && cc != '\r') {
                    break; } }
            return ptr; }
};

#if !defined(_MSC_VER)
#define _NAME_OFF 0
#else
#define _NAME_OFF 6
#endif

/* internal base class to support multiform relationships between different BNFlite elements */
class _Tie
{
    bool _is_compound();
protected:              friend class _Base; friend class ExtParser;
    friend class _And;  friend class _Or;   friend class _Cycle;
    friend class Token; friend class Lexem; friend class Rule;

    bool inner;
    mutable std::vector<const _Tie*> use;
    mutable std::list<const _Tie*> usage;
    std::string name;
    template<class T> static void _setname(T* t, const char * name = 0)
        {   static int cnt = 0;
            if (name) { t->name = name; }
            else { t->name = typeid(*t).name() + _NAME_OFF;
                   for (int i = ++cnt; i != 0; i /= 10) {
                       t->name += '0' + i - (i/10)*10; } } }
    void _clone(const _Tie* lnk)
        {   usage.swap(lnk->usage);
            for (std::list<const _Tie*>::const_iterator usg = usage.begin(); usg != usage.end(); ++usg) {
                for (size_t i = 0; i < (*usg)->use.size(); i++) {
                   if ((*usg)->use[i] == lnk) {
                        (*usg)->use[i] = this; } } }
            use.swap(lnk->use);
            for (size_t i = 0; i < use.size(); i++) {
                if (!use[i]) continue;
                std::list<const _Tie*>::iterator itr =
                    std::find(use[i]->usage.begin(), use[i]->usage.end(), lnk);
                *itr = this; }
            if(lnk->inner) {
                delete lnk; } }
    _Tie(std::string nm = "") :inner(false), name(nm)
        {};
    explicit _Tie(const _Tie* lnk) : inner(true), name(lnk->name)
        {   _clone(lnk); }
    _Tie(const _Tie& link) : inner(link.inner), name(link.name)
        {   _clone(&link); }
    virtual ~_Tie()
        {   for (size_t i = 0; i < use.size(); i++) {
                const _Tie* lnk = use[i];
                if (lnk) {
                    lnk->usage.remove(this);
                    for (size_t j = 0; j < use.size(); j++) {
                        if ( use[j] == lnk) {
                            use[j] = 0; } }
                    if (lnk->inner && lnk->usage.size() == 0) {
                        delete lnk; } } } }
    static int call_1st(const _Tie* lnk, _Base* parser)
        {   return lnk->_parse(parser); }
    void _clue(const _Tie& link)
        {   if (!use.size() || _is_compound()) {
                use.push_back(&link);
            } else {
                if (use[0]) {
                    use[0]->usage.remove(this);
                    if (use[0]->inner && use[0]->usage.size() == 0) {
                        delete use[0]; } }
                use[0] = &link; }
            link.usage.push_back(this); }
    template<class T> static T* _safe_delete(T* t)
        {   if (t->usage.size() != 0)  {
                if (!t->inner)    {
                    return new T(t); } }
            return 0; }
    virtual int _parse(_Base* parser) const throw() = 0;
public:
    void setName(const char * name)
        {   this->name = name; }
    const char *getName()
        {   return name.c_str(); }
    _And operator+(const _Tie& link);
    _And operator+(const char* s);
    _And operator+(bool (*f)(const char*, size_t));
    friend _And operator+(const char* s, const _Tie& lnk);
    friend _And operator+(bool (*f)(const char*, size_t),const _Tie& lnk);
    _Or operator|(const _Tie& link);
    _Or operator|(const char* s);
    _Or operator|(bool (*f)(const char*, size_t));
    friend _Or operator|(const char* s, const _Tie& lnk);
    friend _Or operator|(bool (*f)(const char*, size_t), const _Tie& lnk);

    // Support Augmented BNF constructions like "<a>*<b><element>" to implement repetition;
    // In ABNF <a> and <b> imply at least <a> and at most <b> occurrences of the element;
    // e.g *<element> allows any number(from 0 to infinity, 1*<element> requires at least one;
    // 3*3<element> allows exactly 3 and 1*2<element> allows one or two.
    _Cycle operator()(int at_least, int total); // ABNF case <a>.<b>*<element> as element(a,b)
    _Cycle operator*(); // ABNF case *<element> (from 0 to infinity)
    _Cycle operator!(); // ABNF case <0>.<1>*<element> or <1><element> (at least one)
};

/* implementation of parsing control rules */
template <const unsigned int flg, const char cc> class _Ctrl: public _Tie
{
protected:  friend class _Tie;
    virtual int _parse(_Base* parser) const throw()
        {   return flg; }
    explicit _Ctrl(const _Ctrl* ctrl) :_Tie(ctrl)
        {};
    _Ctrl(const _Ctrl& control) :_Tie(control)
        {};
public:
     explicit _Ctrl(): _Tie(std::string(1, cc))
        {};
    ~_Ctrl()
        {   _safe_delete(this); }
};

 /* Null operation, immediate successful return */
typedef _Ctrl<eOk, 'N'> Null;  // stub for some constructions (e.g. "zero-or-one")

/* Force Return, immediate return from conjunction rule to impact disjunction rule */
typedef _Ctrl<eOk|eRet, 'R'> Return;

/* Switch to use "Accept First" strategy for disjunction rule instead "Accept Best" */
typedef _Ctrl<e1st, '1'> AcceptFirst;

/* Try to catch syntax error in current conjunction rule */
typedef _Ctrl<eOk|eTry, 'T'> Try;

/* Check but do not accept next statement for conjunction rule */
typedef _Ctrl<eOk|eSkip, 'S'> Skip;

/* Force syntax error */
typedef _Ctrl<eError|eSyntax, 'E'> Catch;


/* interface class for tokens */
class Token: public _Tie
{
    Token& operator=(const _Tie&);
    explicit Token(const _Tie&);
public:
    class interval_set : protected std::map<wchar_t,bool>
    {
    public:
	    interval_set()
	        {   insert(std::make_pair(0, false)); insert(std::make_pair(WCHAR_MAX, false)); }
        bool test(wchar_t key) const
            {   return (--upper_bound(key))->second; }
        void reset(wchar_t key)
            {   set(key, 0, false); }
        void set(wchar_t key, size_t rep = 0, bool val = true)
            {   wchar_t key_end = key + rep + 1;
                if (key == 0 || key_end == WCHAR_MAX) return;
                std::map<wchar_t,bool>::iterator right_begin = lower_bound(key);
                std::map<wchar_t,bool>::iterator left_begin = right_begin; --left_begin;
                std::map<wchar_t,bool>::iterator right_end = upper_bound(key_end);
                std::map<wchar_t,bool>::iterator left_end = right_end; --left_end;
                if (left_end->second == val)
                    if (left_end->first >= key_end && right_begin == left_end) erase(right_begin);
                    else  erase(right_begin, right_end);
                else {
                    std::map<wchar_t,bool>::iterator itr = insert(std::make_pair(key_end, left_end->second)).first;
                    if(right_begin->first < itr->first)
                            erase(right_begin, itr);  }
                if (left_begin->second != val)
                    insert(std::make_pair(key, val));  }
        void flip()
            {   for (std::map<wchar_t, bool>::iterator itr = begin(); itr != end(); ++itr)
                       itr->second = !itr->second;  }
    };

 protected: friend class _Tie;
#if defined(BNFLITE_WIDE)
    interval_set match;
#else
    std::bitset<bnf::maxCharNum> match;
#endif
    explicit Token(const Token* tkn) :_Tie(tkn), match(tkn->match)
        {};
    virtual int _parse(_Base* parser) const throw()
        {   const char* cc = parser->cntxV.back();
            if (parser->level)
                cc = parser->zero_parse(cc);
            char c = *((unsigned char*)cc);
            if (match.test(c)) {
                if (parser->level) {
                    parser->cntxV.push_back(cc);
                    parser->_stub_call(parser->cntxV.size() - 1, name.c_str()); }
                parser->cntxV.push_back(++cc);
                return  c ? eOk : eOk|eEof; }
            return c ? eNone : eEof; }
public:
    Token(const char c) :_Tie(std::string(1, c))
        {   Add(c, 0); };    // create single char token
    Token(int fst, int lst) :_Tie(std::string(1, fst).append("-") += lst)
        {   Add(fst, lst); };    // create token by ASCII charactes in range
    Token(const char *s) :_Tie(std::string(s))
        {   Add(s); }; // create token by C string sample
    Token(const char *s, const Token& token) :_Tie(std::string(s)), match(token.match)
        {   Add(s); }; // create token by both C string sample and another token set
    Token(const Token& token) :_Tie(token), match(token.match)
        {};
    virtual ~Token()
        {   _safe_delete(this); }
    void Add(int fst, int lst = 0, const char *sample = "")  // add characters in range fst...lst exept mentioned in sample;
        {   switch (lst) { // lst == 0|1: add single | upper&lower case character(s)
            case 1: if (fst >= 'A' && fst <= 'Z') match.set(fst - 'A' + 'a');
                    else if (fst >= 'a' && fst <= 'z') match.set(fst - 'a' + 'A');
            case 0: match.set((unsigned char)fst); break;
            default: for (int i = fst; i <= lst; i++) {
                        match.set((unsigned char)i); }
                     Remove(sample); } }
    void Add(const char *sample)
        {   while (*sample) {
                match.set((unsigned char)*sample++); } }
    void Remove(int fst, int lst = 0)
        {   for (int i = fst; i <= (lst?lst:fst); i++) {
                match.reset((unsigned char)i); } }
    void Remove(const char *sample)
        {   while (*sample) {
                match.reset((unsigned char)*sample++); } }
    int GetSymbol(int next = 1) // get first short symbol
        {   for (unsigned int i = next; i < maxCharNum; i++) {
                if (match.test(i)) return i; }
            return 0; }
    Token& Invert() // invert token to build construction to not match
        {   match.flip(); return *this; }

};
#if __cplusplus > 199711L
inline Token operator""_T(const char* sample, size_t len)
    {   return  Token(std::string(sample, len).c_str());    }
#endif

/*  standalone callback wrapper class */
class Action: public _Tie
{
    bool (*action)(const char* lexem, size_t len);
    Action(_Tie&);
protected:  friend class _Tie;
    explicit Action(const Action* a) :_Tie(a), action(a->action)
        {};
    int _parse(_Base* parser) const throw()
        {   std::vector<const char*>::reverse_iterator itr = parser->cntxV.rbegin() + 1;
            return (*action)(*itr, parser->cntxV.back() - *itr); }
public:
    Action(bool (*action)(const char* lexem, size_t len), const char *name = "")
        :_Tie(name), action(action) {};
    virtual ~Action()
        {   _safe_delete(this); }
};

/* internal class to support conjunction constructions of BNFlite elements */
class _And: public _Tie
{
protected: friend class _Tie; friend class Lexem;
    _And(const _Tie& b1, const _Tie& b2):_Tie("")
        {   (name = b1.name).append("+") += b2.name; _clue(b1); _clue(b2); }
    explicit _And(const _And* rl) :_Tie(rl)
        {};
    virtual int _parse(_Base* parser) const throw()
        {   int stat = 0; size_t save = 0; size_t size = parser->cntxV.size();
            for (unsigned i = 0; i < use.size(); i++, stat &= ~(eSkip|eOk)) {
                stat |= use[i]->_parse(parser);
                if (!(stat & eOk) || (stat & eError) || ((stat & eEof) && (parser->cntxV.back() == parser->cntxV[size - 1]))) {
                    if (parser->level && (stat & eTry) && !(stat & eError) && !save) {
                        stat |= parser->catch_error(parser->cntxV.back()); }
                    parser->_erase(size);
                    return stat & ~(eTry|eSkip|eOk); }
                else {
                    if (save) {
                        parser->cntxV.resize(save);
                        save = 0; }
                    if (stat & eSkip) {
                        save = parser->cntxV.size(); } } }
            return  eOk | (stat & ~(eTry|eSkip)); }
public:
    ~_And()
        {   _safe_delete(this); }
    _And& operator+(const _Tie& rule2)
        {   name.append("+") += rule2.name; _clue(rule2); return *this; }
    _And& operator+(const char* s)
        {   name.append("+") += s; _clue(Token(s)); return *this; }
    _And& operator+(bool (*f)(const char*, size_t))
        {   name += "+()"; _clue(Action(f)); return *this; }
    friend _And operator+(const char* s, const _Tie& link);
    friend _And operator+(bool (*f)(const char*, size_t), const _Tie& link);
};
inline _And _Tie::operator+(const _Tie& rule2)
    {   return _And(*this, rule2); }
inline _And _Tie::operator+(const char* s)
    {   return _And(*this, Token(s)); }
inline _And _Tie::operator+(bool (*f)(const char*, size_t))
    {   return _And(*this, Action(f)); }
inline _And operator+(const char* s, const _Tie& link)
    {   return _And(Token(s), link); }
inline _And operator+(bool (*f)(const char*, size_t), const _Tie& link)
    {   return _And(Action(f), link); }

/* internal class to support disjunction constructions of BNFlite elements */
class _Or: public _Tie
{
protected: friend class _Tie;
    _Or(const _Tie& b1, const _Tie& b2):_Tie("")
        {   (name = b1.name).append("|") += b2.name; _clue(b1); _clue(b2); }
    explicit _Or(const _Or* rl) :_Tie(rl)
        {};
    virtual int _parse(_Base* parser) const throw()
        {   int stat = 0; int tstat = 0; int max = 0; int tmp = -1;
            size_t size = parser->cntxV.size();
            for (unsigned i = 0; i < use.size(); i++, stat &= ~(eOk|eRet|eEof|eError)) {
                size_t msize = parser->cntxV.size();
                if (msize > size) {
                    parser->cntxV.push_back(parser->cntxV[size - 1]); }
                stat |= use[i]->_parse(parser);
                if (stat & (eOk|eError)) {
                    tmp = parser->cntxV.back() - parser->cntxV[size - 1];
                    if ((tmp > max) || (tmp > 0 && (stat & (eRet|e1st))) || (tmp >= 0 && (stat & eError))) {
                        max = tmp;
                        tstat = stat;
                        if (msize > size) {
                            parser->_erase(size, msize + 1); }
                        if (stat & (eRet|e1st|eError)) {
                            break; }
                        continue; } }
                if (parser->cntxV.size() > msize) {
                    parser->_erase(msize); } }
            return (max || tmp >= 0 ? tstat | eOk: tstat & ~eOk) & ~(e1st|eRet); }
public:
    ~_Or()
        {   _safe_delete(this); }
    _Or& operator|(const _Tie& rule2)
        {   name.append("|") += rule2.name; _clue(rule2); return *this; }
    _Or& operator|(const char* s)
        {   name.append("|") += s; _clue(Token(s)); return *this; }
    _Or& operator|(bool (*f)(const char*, size_t))
        {   name += "|()"; _clue(Action(f)); return *this; }
    friend _Or operator|(const char* s, const _Tie& link);
    friend _Or operator|(bool (*f)(const char*, size_t), const _Tie& link);
};
inline _Or _Tie::operator|(const _Tie& rule2)
    {   return _Or(*this, rule2); }
inline _Or _Tie::operator|(const char* s)
    {   return _Or(*this, Token(s)); }
inline _Or _Tie::operator|(bool (*f)(const char*, size_t))
    {   return _Or(*this, Action(f)); }
inline _Or operator|(const char* s, const _Tie& link)
    {   return _Or(Token(s), link); }
inline _Or operator|(bool (*f)(const char*, size_t), const _Tie& link)
    {   return _Or(Action(f), link); }
inline bool _Tie::_is_compound()
    {   return dynamic_cast<_And*>(this) || dynamic_cast<_Or*>(this); }


/* interface class for lexem */
class Lexem: public _Tie
{
    Lexem& operator=(const class Rule&);
    Lexem(const Rule& rule);
protected: friend class _Tie;
    explicit Lexem(Lexem* lxm) :_Tie(lxm)
        {};
    virtual int _parse(_Base* parser) const throw()
        {   if (!use.size())
                return eError|eBadLexem;
            if (!parser->level || dynamic_cast<const Action*>(use[0]))
                return use[0]->_parse(parser);
            size_t size = parser->cntxV.size();
            parser->cntxV.push_back(parser->zero_parse(parser->cntxV.back()));
            parser->level--;
            int stat = use[0]->_parse(parser);
            parser->level++;
            if ((stat & eOk) && parser->cntxV.size() - size > 1) {
                parser->_stub_call(size - 1, name.c_str());
                if (parser->cntxV.back() > parser->pstop) parser->pstop = parser->cntxV.back();
                parser->cntxV[(++size)++] = parser->cntxV.back(); }
            parser->cntxV.resize(size);
            return stat; }
public:
    Lexem(const char *literal, bool cs = 0) :_Tie()
        {   int size = strlen(literal);
            switch (size) {
            case 1: this->operator=(Token(literal[0], cs));
            case 0: break;
            default: {
                _And _and(Token(literal[0], cs), Token(literal[1], cs));
                for (int i = 2; i < size; i++) {
                    _and.operator+((const _Tie&)Token(literal[i], cs)); }
                this->operator=(_and); } }
            _setname(this, literal);  }
    explicit Lexem() :_Tie()
        {   _setname(this); }
    virtual ~Lexem()
        {   _safe_delete(this); }
    Lexem(const _Tie& link) :_Tie()
        {   _setname(this, 0); _clue(link); }
    Lexem& operator=(const Lexem& lexem)
        {   if (&lexem != this) _clue(lexem);
            return *this; }
    Lexem& operator=(const _Tie& link)
        {   _clue(link); return *this; }
};

/* interface class for BNF rules */
class Rule : public _Tie
{
    void* callback;
protected:  friend class _Tie; friend class _And;
    explicit Rule(const Rule* rl) :_Tie(rl), callback(rl->callback)
    {};
    virtual int _parse(_Base* parser) const throw()
        {   if (!use.size() || !parser->level)
                return eError|eBadRule;
            if (dynamic_cast<const Action*>(use[0])) {
                return use[0]->_parse(parser); }
            size_t size = parser->cntxV.size();
            std::pair<void*, int> up = parser->_pre_call(callback);
            int stat = use[0]->_parse(parser);
            if ((stat & eOk) && parser->cntxV.size() - size > 1) {
                parser->_do_call(up, callback, size, name.c_str());
                if (parser->cntxV.back() > parser->pstop) parser->pstop = parser->cntxV.back(); 
                parser->cntxV[(++size)++] = parser->cntxV.back(); }
            parser->cntxV.resize(size);
            parser->_post_call(up);
            return stat; }
public:
    explicit Rule() :_Tie(), callback(0)
        {   _setname(this); }
    virtual ~Rule()
        {   _safe_delete(this); }
    Rule(const _Tie& link) :_Tie(), callback(0)
        {   const Rule* rl = dynamic_cast<const Rule*>(&link);
            if (rl) { _clone(&link);  callback = rl->callback; name = rl->name; }
            else { _clue(link);   callback = 0; _setname(this);  } }
    Rule& operator=(const _Tie& link)
        {   _clue(link); return *this; }
    Rule& operator=(const Rule& rule)
        {   if (&rule == this) return *this;
            return this->operator=((const _Tie&)rule); }
    template <class U> friend Rule& Bind(Rule& rule, U (*callback)(std::vector<U>&));
    template <class U> Rule& operator[](U (*callback)(std::vector<U>&));
};

/* friendly debug interface */
#define LEXEM(lexem) Lexem lexem; lexem.setName(#lexem); lexem
#define RULE(rule) Rule rule; rule.setName(#rule); rule

/* internal class to support repeat constructions of BNF elements */
class _Cycle: public _Tie
{
    unsigned int min, max;
    int flag;
protected: friend class _Tie;
    explicit _Cycle(const _Cycle* u) :_Tie(u), min(u->min), max(u->max), flag(u->flag)
        {};
    _Cycle(const _Cycle& w) :_Tie(w), min(w.min), max(w.max), flag(w.flag)
        {};
    int _parse(_Base* parser) const throw()
        {   int stat; unsigned int i;
            for (stat = 0, i = 0; i < max; i++, stat &= ~(e1st|eTry|eSkip|eRet|eOk)) {
                stat |= use[0]->_parse(parser);
                if ((stat & (eOk|eError)) == eOk)
                    continue;
                return i < min? stat & ~eOk : stat | parser->_chk_stack() | eOk; }
            return stat | flag | eOk; }
    _Cycle(int at_least, const _Tie& link, int total = maxRepeate, int limit = maxRepeate)
        :_Tie(std::string("@")), min(at_least), max(total), flag(total < limit? eNone : eOver|eError)
        {   _clue(link); }
public:
    ~_Cycle()
        {   _safe_delete(this); }
    friend _Cycle operator*(int at_least, const _Tie& link);
    friend _Cycle Repeat(int at_least, const Rule& rule, int total, int limit);
    friend _Cycle Iterate(int at_least, const Lexem& lexem, int total, int limit);
    friend _Cycle Series(int at_least, const Token& token, int total, int limit);
};
inline _Cycle _Tie::operator*()
    {   return _Cycle(0, *this); }
inline _Cycle _Tie::operator!()
    {   return _Cycle(0, *this, 1); }
inline _Cycle _Tie::operator()(int at_least, int total)
    {   return _Cycle(at_least, *this, total); }
inline _Cycle operator*(int at_least, const _Tie& link)
    {   return _Cycle(at_least, link); }
inline _Cycle Repeat(int at_least, const Rule& rule, int total = maxLexemLength, int limit = maxRepeate)
    {   return _Cycle(at_least, rule, total, limit); }
inline _Cycle Iterate(int at_least, const Lexem& lexem, int total = maxLexemLength, int limit = maxLexemLength)
    {   return _Cycle(at_least, lexem, total, limit); }
inline _Cycle Series(int at_least, const Token& token, int total = maxLexemLength, int limit = maxCharNum)
    {   return _Cycle(at_least, token, total, limit); }

/* context class to support the second kind of callback */
template <class U> class _Parser : public _Base
{
protected:
    std::vector<U>* cntxU;
    unsigned int off;
    void _erase(int low, int up = 0)
        {   cntxV.erase(cntxV.begin() + low,  up? cntxV.begin() + up : cntxV.end() );
            if (cntxU && level)
                cntxU->erase(cntxU->begin() + (low - off) / 2,
                 up? cntxU->begin() + (up - off) / 2 : cntxU->end()); }
    virtual std::pair<void*, int> _pre_call(void* callback)
        {   std::pair<void*, int> up = std::make_pair(cntxU, off);
            cntxU = callback? new std::vector<U> : 0;
            off = callback? cntxV.size() : 0;
            return up; }
    virtual void  _post_call(std::pair<void*, int> up)
        {   if (cntxU) {
                delete cntxU; }
            cntxU = (std::vector<U>*)up.first;
            off = up.second; }
    virtual void _do_call(std::pair<void*, int> up, void* callback, size_t org, const char* name)
        {   if (callback) {
                if (up.first) {
                    ((std::vector<U>*)up.first)->push_back(U(reinterpret_cast<
                        U(*)(std::vector<U>&)>(callback)(*cntxU), cntxV[org], cntxV.back() - cntxV[org], name));
                } else { reinterpret_cast<U(*)(std::vector<U>&)>(callback)(*cntxU); }
            } else if (up.first) {
                    ((std::vector<U>*)up.first)->push_back(U(cntxV[org], cntxV.back() - cntxV[org], name)); } }
    virtual void _stub_call(size_t org, const char* name)
        {   if (cntxU) {
                cntxU->push_back(U(cntxV[org], cntxV.back() - cntxV[org], name)); } }
public:
    _Parser(const char* (*f)(const char*), std::vector<U>* v) :_Base(f), cntxU(v), off(0)
        {};
    virtual ~_Parser()
        {};
    int _get_result(U& u)
        {   if (cntxU && cntxU->size()) { u.data = cntxU->front().data; return 0; }
            else return eNull; }
    template <class W> friend Rule& Bind(Rule& rule, W (*callback)(std::vector<W>&));
};

inline int _Base::_analyze(_Tie& root, const char* text, size_t* plen)
{   cntxV.push_back(text); cntxV.push_back(text);
    int stat = root._parse(this);
    const char* ptr = zero_parse(pstop > cntxV.back() ? pstop : cntxV.back());
    if (plen) *plen = ptr - text;
    return stat | (*ptr? eError|eRest: 0);  }

/* User interface template to support the second kind of callback */
/* The user need to specify own 'Foo' abstract type to develop own callbaks */
/* like: Interface<Foo> CallBack(std::vector<Interface<Foo>>& res); */
template <typename Data = bool> struct Interface
{
    Data data;              //  user data element
    const char* text;       //  pointer to parsed text according to bound Rule
    size_t length;          //  length of parsed text according to bound Rule
    const char* name;       //  the name of bound Rule
    Interface(const Interface& ifc, const char* text, size_t length, const char* name)
        :data(ifc.data) , text(text), length(length), name(name)
        {}; // mandatory constructor with user data to be called from library
    Interface(const char* text, size_t length,  const char* name)
        :data(), text(text), length(length), name(name)
        {}; //  mandatory default constructor to be called from library
    Interface(Data data, std::vector<Interface>& res, const char* name = "")
        :data(data), text(res.size()? res[0].text: ""),
          length(res.size()? res[res.size() - 1].text
            - res[0].text + res[res.size() - 1].length : 0), name(name)
        {}; // constructor to pass data from user's callback to library
    Interface(const Interface& front, const Interface& back, const char* name = "")
        : data(), text(front.text), length(back.text - front.text + back.length), name(name)
        {}; // constructor to pass data from user's callback to library
    Interface(): data(), text(0), length(0), name(0)
        {}; // default constructor
    static Interface ByPass(std::vector<Interface>& res) // simplest user callback example
        {   return res.size()? res[0]: Interface(); }   // just to pass data to upper level
    int _get_pstop(const char** pstop)
        {   if (pstop) *pstop = text + length;
            return length ? eNone : eNull; }
};

/* Private parsing interface */
template <class U> inline int _Analyze(_Tie& root, U& u, const char* (*pre_parse)(const char*))
    {   if (typeid(U) == typeid(Interface<>)) {
                    _Base base(pre_parse); return base._analyze(root, u.text, &u.length);
        } else {    std::vector<U> v; _Parser<U> parser(pre_parse, &v);
                    return parser._analyze(root, u.text, &u.length) | parser._get_result(u); } }

/* Primary interface set to start parsing of text against constructed rules */
template <class U> inline int Analyze(_Tie& root, const char* text, const char** pstop, U& u, const char* (*pre_parse)(const char*) = 0)
    {   u.text = text; return _Analyze(root, u, pre_parse) | u._get_pstop(pstop); }
template <class U> inline int Analyze(_Tie& root, const char* text, U& u, const char* (*pre_parse)(const char*) = 0)
    {   u.text = text; return _Analyze(root, u, pre_parse) | u._get_pstop(0); }
inline int Analyze(_Tie& root, const char* text, const char** pstop = 0, const char* (*pre_parse)(const char*) = 0)
    {   Interface<> u; u.text = text;  return _Analyze(root, u, pre_parse) | u._get_pstop(pstop); }


/* Create association between Rule and user's callback */
template <class U> inline Rule& Bind(Rule& rule, U (*callback)(std::vector<U>&))
    {   rule.callback = reinterpret_cast<void*>(callback); return rule; }
template <class U> inline Rule& Rule::operator[](U (*callback)(std::vector<U>&)) // for C++11
    {   this->callback = reinterpret_cast<void*>(callback); return *this; }


}; // bnf::
#endif // BNFLITE_H
