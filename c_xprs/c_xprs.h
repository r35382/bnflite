
/*************************************************************************\
*   This is C expression parser and calculator lib based on BNFlite       *
*   Copyright (c) 2018 by Anton G. & Alexander S.  ALL RIGHTS RESERVED.   *
*                                                                         *
*   This program is free software: you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation, either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
*                                                                         *
*   Recommendations for commercial use:                                   *
*   Commercial application should have dedicated LGPL licensed cpp-file   *
*   which exclusively includes GPL licensed "bnflit.h" file. In fact,     *
*   the law does not care how this cpp-file file is linked to other       *
*   binary applications. Just source code of this cpp-file has to be      *
*   published in accordance with LGPL license.                            *
\*************************************************************************/


#include "stdio.h"
#include <stack>
#include "bnflite.h"
using namespace bnf;


/*************************************************************************\
* Example of the expression tree for "4+3*2-1":                           *
*                       '-'                                               *
*                      /   \                                              *
*                   '+'     1                                             *
*                  /   \                                                  *
*                 4    '*'                                                *
*                     /   \                                               *
*                    3     2                                              *
\*************************************************************************/
struct XprsTree
{
    XprsTree(): operation(0), val(0), right(0), left(0) {};
    ~XprsTree()
    {   if(right) delete right;
        if(left) delete left;
    }
    unsigned int operation;
    int val;
    XprsTree* right;
    XprsTree* left;
};


class C_Xprs // tbd implemented as singleton, to be extended
{
private:

    template <class T> class Stack : public std::stack<T>
    {   public: 
        T getpop() {
            if (this->size() > 0)  { 
                T value = this->top(); 
                this->pop(); 
                return value; 
            }
            return 0;
        }
    };

    static Stack<XprsTree*> rootNode;
    static int lastNumber;


    Rule PrimaryXprs;
    Rule PostfixXprs; 
    Rule AdditiveXprs, AdditiveXprs0; 
    Rule MultiplicativeXprs, MultiplicativeXprs0; 
    Rule ShiftXprs, ShiftXprs0; 
    Rule EqualityXprs, EqualityXprs0; 
    Rule RelationalXprs, RelationalXprs0; 
    Rule BitwiseAndXprs, BitwiseAndXprs0; 
    Rule BitwiseOrXprs, BitwiseOrXprs0; 
    Rule LogicalAndXprs, LogicalAndXprs0; 
    Rule LogicalOrXprs, LogicalOrXprs0; 
    Rule ExclusiveOrXprs, ExclusiveOrXprs0; 
    Rule ConditionalXprs,ConditionalXprs0; 

    Rule UnaryXprs; 
    Rule MainXprs; 

    /* 1st kind of callback */
    static bool getHexNumber(const char* lexem, size_t len);
    static bool getNumber(const char* lexem, size_t len);
    static bool dbgPrint(const char* lexem, size_t len);
    static bool printMsg(const char* lexem, size_t len);
    static bool syntaxError(const char* lexem, size_t len);
    static bool numberAction(const char* lexem, size_t len);
    static bool buildBinaryAction(const char* lexem, size_t len);
    static bool buildUnaryAction(const char* lexem, size_t len);
    static bool unaryAction(const char* lexem, size_t len);
    static bool postfixAction(const char* lexem, size_t len);
    static bool binaryAction(const char* lexem, size_t len);
    static bool ifAction(const char* lexem, size_t len);
    static bool thenAction(const char* lexem, size_t len);
    static bool elseAction(const char* lexem, size_t len);


    static int GetOperationPriority(unsigned int op);
    static int Calcualte(XprsTree& node);

    bool ParseExpression(const char *expression);
    void GrammaInit();

public:
    bool Evaluate(const char *expression, int& result);
    C_Xprs() { GrammaInit(); }
    ~C_Xprs(){ MainXprs = Null(); UnaryXprs = Null(); };
};

C_Xprs::Stack<XprsTree*> C_Xprs::rootNode;
int C_Xprs::lastNumber;



bool C_Xprs::getHexNumber(const char* lexem, size_t len)
{   
    int i = 0;
    lastNumber = 0;
    if ( len > 1 && lexem[0] == '0' && (lexem[1] == 'X' || lexem[1] == 'x')) 
        i += 2;
    for (; i < len; i++) {
        if (lexem[i] >= '0' && lexem[i] <= '9') {
            lastNumber = 16 * lastNumber + (lexem[i] - '0');    
        } else if (lexem[i] >= 'A' && lexem[i] <= 'F') {
            lastNumber = 16 * lastNumber + (lexem[i] - 'A' + 10 );    
        } else if (lexem[i] >= 'a' && lexem[i] <= 'f') {
            lastNumber = 16 * lastNumber + (lexem[i] - 'a' + 10 );    
        } else break;
    }
    return true;
}

bool C_Xprs::getNumber(const char* lexem, size_t len)
{   
    lastNumber = 0;
    for (int i = 0; i < len && lexem[i] >= '0' && lexem[i] <= '9'; i++) {
        lastNumber = 10 * lastNumber + (lexem[i] - '0');    
    }
    return true;
}


bool C_Xprs::dbgPrint(const char* lexem, size_t len)
{   
    char operation[256] = {0};  //strcpy(operation, Lexem::getLastLexem()); 
    memcpy(operation, lexem, len); operation[len+1] = 0;
    return true;
}

bool C_Xprs::printMsg(const char* lexem, size_t len)
{
    printf("dbg lexem: %.*s;\n", len, lexem);
    return true;
}


bool C_Xprs::syntaxError(const char* lexem, size_t len)
{
    printf("forced syntax error for: %.*s;\n", len, lexem);
    return true;
}



bool C_Xprs::numberAction(const char* lexem, size_t len)
{
    XprsTree* node = new XprsTree();
    node->val =  lastNumber;
    rootNode.push(node);
    return true;
}


bool C_Xprs::buildBinaryAction(const char* lexem, size_t len)
{
   int getopprio(unsigned int op);
    if (rootNode.size() >= 2)
    {
        XprsTree* child = rootNode.getpop();
        XprsTree* parent = rootNode.top();

        if( GetOperationPriority(child->operation) == GetOperationPriority(parent->operation)) {
            parent->left = child->right;
            child->right = parent;
            rootNode.pop();
            rootNode.push(child);
        }
        else
        {   parent->left = child;
        }
    }
    return true;
}


bool C_Xprs::buildUnaryAction(const char* lexem, size_t len)
{
    if (rootNode.size() >= 2)
    {
        XprsTree* child = rootNode.getpop();
        XprsTree* parent = rootNode.top();
        parent->right = child;
    }
    return true;
}


bool C_Xprs::unaryAction(const char* lexem, size_t len)
{
    if (len > 1 && ((lexem[0] == '+' && lexem[1] == '+') || (lexem[0] == '-' && lexem[1] == '-'))) {
        return false; // not supported operations
    }
    XprsTree* node = new XprsTree();
    rootNode.push(node);
 
    node->operation =  lexem[0] << 8  | ' ';

    return true;
}


bool C_Xprs::postfixAction(const char* lexem, size_t len)
{
    return false;
}


bool C_Xprs::binaryAction(const char* lexem, size_t len)
{
    XprsTree* node = new XprsTree();
    node->right = rootNode.getpop();
    rootNode.push(node);
    node->operation = 0;
    for (int i = 0; i < len; i++) {
        node->operation = node->operation << 8 | lexem[i];
    }
    return true;
}

bool C_Xprs::ifAction(const char* lexem, size_t len)
{
    XprsTree* node = new XprsTree();
    node->right = rootNode.getpop();
    rootNode.push(node);
    node->left = new XprsTree();
    rootNode.push(node->left);
    node->operation =  '?';
    return true;
}


bool C_Xprs::thenAction(const char* lexem, size_t len)
{
    if (rootNode.size() > 0) {
        XprsTree* node = rootNode.getpop();
        rootNode.top()->right = node;
    }
    return true;
}

bool C_Xprs::elseAction(const char* lexem, size_t len)
{
    if (rootNode.size() > 0) {
        XprsTree* node = rootNode.getpop();
        rootNode.getpop()->left = node;
    }
    return true;
}


#define OPBBB(a, b, c) (a << 16 | b << 8 | c)
#define OPBB(a, b) (a << 8 | b)
#define OPB(a) (a)
#define OPU(a) (a << 8 | ' ')

int C_Xprs::GetOperationPriority(unsigned int op)
{
    switch (op) {
    case OPBB(':',':'): return 1;
    case OPBB('-','>'): case OPU('.'): return 2;
    case OPU('+'): case OPU('-'): case OPU('~'): case OPU('!'): return 3;
    case OPBB('+','+'):  case OPBB('-','-'): case OPU('*'): case OPU('&'): return 3;
    case OPBB('.','*'):  case OPBBB('.','>','*'): return 4;
    case OPB('*'): case OPB('/'): case OPB(':'): return 5;
    case OPB('+'): case OPB('-'): return 6;
    case OPBB('<','<'):  case OPBB('>','>'): return 7;
    case OPB('<'):  case OPBB('<','='): return 8;
    case OPB('>'):  case OPBB('>','='): return 8;
    case OPBB('=','='):  case OPBB('!','='): return 9;
    case OPB('&'): return 10;
    case OPB('^'): return 11;
    case OPB('|'): return 12;
    case OPBB('&','&'):  return 13;
    case OPBB('|','|'):  return 14;
    case OPB('='): case OPB('?'): return 15;
    case OPBB('+','='):  case OPBB('-','='): return 15;
    case OPBB('*','='):  case OPBB('/','='): case OPBB('%','='): return 15;
    case OPBBB('<','<','='):  case OPBBB('>','>','='): return 15;
    case OPBB('&','='):  case OPBB('^','='): case OPBB('|','='): return 15;
    case OPB(','): return 17;
    }
    return 0;
}

int C_Xprs::Calcualte(XprsTree& node)
{
    int val1, val2; 

    switch (node.operation & 0xFF) {
    case '?': val1 = node.right? Calcualte(*node.right) : 0;
              if (val1) return node.val = Calcualte(*node.left->right);
              else  return node.val = Calcualte(*node.left->left);
              break;
    case ' ': val1 = node.right? Calcualte(*node.right) : 0;
              break;
    default:  val1 = node.right? Calcualte(*node.right) : 0;
              val2 = node.left? Calcualte(*node.left) : 0;
              break;
    }

    switch (node.operation) {
    case 0: return node.val;

    case OPU('+'): return node.val = +val1;
    case OPU('-'): return node.val = -val1;
    case OPU('~'): return node.val = ~val1;
    case OPU('!'): return node.val = !val1;

    case OPB('*'): return node.val = val1 * val2;
    case OPB('/'): return  node.val = val1 / val2;
    case OPB('%'): return  node.val = val1 % val2;
    case OPB('+'): return  node.val = val1 + val2;
    case OPB('-'): return  node.val = val1 - val2;
    case OPBB('<','<'): return  node.val = val1 << val2;
    case OPBB('>','>'): return  node.val = val1 >> val2;
    case OPB('<'):  return  node.val = val1 < val2;
    case OPBB('<','='): return  node.val = val1 <= val2;
    case OPB('>'):  return  node.val = val1 > val2;
    case OPBB('>','='): return  node.val = val1 >= val2;
    case OPBB('=','='): return  node.val = val1 == val2; 
    case OPBB('!','='): return  node.val = val1 != val2;
    case OPB('&'): return  node.val = val1 & val2;
    case OPB('^'): return  node.val = val1 ^ val2;
    case OPB('|'): return  node.val = val1 | val2;
    case OPBB('&','&'):  return  node.val = val1 && val2;
    case OPBB('|','|'):  return  node.val = val1 || val2;
    default: return 0;
    }
}

void C_Xprs::GrammaInit()
{

    Token Any(1, 255);
    Token Digit("0123456789");
    Token HexDigit("abcdefABCDEF",Digit); 
    Token OctalDigit("01234567");
    Token BinaryDigit("01");
    Token Letter("abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    Token Character("0123456789",Letter);
    
    Lexem Identifier = Letter + Series(0, Character);
    Lexem HexDigits  = Lexem("0x",1) +  Series(1, HexDigit);
    Lexem Digits = Series(1, Digit);

    Rule Number = HexDigits + getHexNumber + Return() | Digits + getNumber;


    PostfixXprs = Lexem("++") + postfixAction + Return() |
                        Lexem("--") + postfixAction + Return() |
                        Null();

    PrimaryXprs =   Number + Try() + numberAction + PostfixXprs + Return() | 
                    "(" + Try() + MainXprs + ")" + Return() |
                    syntaxError + Syntax();

    UnaryXprs =
            Lexem("++") + Try() + unaryAction + PrimaryXprs + buildUnaryAction + Return() |
            Lexem("--") + Try() + unaryAction + PrimaryXprs + buildUnaryAction + Return() |
            ("-") + Try() + unaryAction + UnaryXprs + buildUnaryAction + Return() |
            ("+") + Try() + unaryAction + UnaryXprs + buildUnaryAction + Return() |
            ("!") + Try() + unaryAction + UnaryXprs + buildUnaryAction + Return() |
            ("~") + Try() + unaryAction + UnaryXprs + buildUnaryAction + Return() |
            PrimaryXprs;     

    MultiplicativeXprs0 = 
                ("*") + Try() + binaryAction + MultiplicativeXprs + buildBinaryAction + Return() |
                ("/") + Try() + binaryAction + MultiplicativeXprs + buildBinaryAction + Return() |
                ("%") + Try() + binaryAction + MultiplicativeXprs + buildBinaryAction;
    MultiplicativeXprs = UnaryXprs + Repeat(0, MultiplicativeXprs0);

    AdditiveXprs0 = 
                ("+") + Try() + binaryAction + AdditiveXprs + buildBinaryAction + Return() |
                ("-") + Try() + binaryAction + AdditiveXprs + buildBinaryAction;
    AdditiveXprs = MultiplicativeXprs + Repeat(0, AdditiveXprs0);
    
    ShiftXprs0 = 
                Lexem("<<") + Try() + binaryAction + AdditiveXprs + buildBinaryAction + Return() |
                Lexem(">>") + Try() + binaryAction + AdditiveXprs + buildBinaryAction;
    ShiftXprs = AdditiveXprs + Repeat(0, ShiftXprs0);    

    RelationalXprs0 = 
        Lexem(">") + Try() + binaryAction + ShiftXprs + buildBinaryAction + Return() |
        Lexem("<") + Try() + binaryAction + ShiftXprs + buildBinaryAction + Return() |
        Lexem(">=") + Try() + binaryAction + ShiftXprs + buildBinaryAction + Return() |
        Lexem("<=") + Try() + binaryAction + ShiftXprs + buildBinaryAction;
    RelationalXprs = ShiftXprs + Repeat(0, RelationalXprs0);

    EqualityXprs0 = 
        Lexem("==") + Try() + binaryAction + EqualityXprs + buildBinaryAction  + Return() |
        Lexem("!=") + Try() + binaryAction + EqualityXprs + buildBinaryAction;
    EqualityXprs = RelationalXprs + Repeat(0, EqualityXprs0); 

    Lexem NotLogicalAnd =  Token("&") + Token("&").Invert();
    BitwiseAndXprs0 = Skip() + NotLogicalAnd + Token("&") + Try() + binaryAction + EqualityXprs  + buildBinaryAction;
    BitwiseAndXprs = EqualityXprs + Repeat(0, BitwiseAndXprs0);

    ExclusiveOrXprs0 = "^" + Try() + binaryAction + BitwiseAndXprs + buildBinaryAction;
    ExclusiveOrXprs = BitwiseAndXprs + Repeat(0, ExclusiveOrXprs0);

    Lexem NotLogicalOr =  "|" + Token("|").Invert();
    BitwiseOrXprs0 = Skip() + NotLogicalOr + "|" + Try() + binaryAction + ExclusiveOrXprs + buildBinaryAction;
    BitwiseOrXprs = ExclusiveOrXprs + Repeat(0, BitwiseOrXprs0);

    /* Note Token("&") + Token("&") and Lexem("&&") constructions are the same
       from the language notation point of view, but callbuck bachavior is different */
    LogicalAndXprs0 = Lexem("&&") + Try() + binaryAction + BitwiseOrXprs + buildBinaryAction;
    LogicalAndXprs = BitwiseOrXprs + Repeat(0, LogicalAndXprs0);

    LogicalOrXprs0 = Lexem("||") + Try() + binaryAction + LogicalAndXprs + buildBinaryAction;
    LogicalOrXprs = LogicalAndXprs + Repeat(0, LogicalOrXprs0);

    ConditionalXprs0 =   "?" + Try() + ifAction + MainXprs + 
                         ":" + thenAction + ConditionalXprs + 
                         elseAction + Return() |
                         Null();
    ConditionalXprs = LogicalOrXprs + ConditionalXprs0;

    MainXprs = ConditionalXprs;
    
}


int catch_syntax_error(const char* ptr)
{   
    printf("caught syntax error for: %.80s;\n", ptr);
    return eSyntax;
}


bool C_Xprs::ParseExpression(const char *expression)
{
    int tst = Analyze(MainXprs, expression);
    if (tst < 0) {
        std::cout << "Expression not OK," << "Err: " << std::hex   
            << (tst&eOk?"eOk":"eErr")
            << (tst&eRest?", eRest":"")
            << (tst&eOver?", eOver":"")
            << (tst&eEof?", eEof":"")
            << (tst&eBadRule?", eBadRule":"")
            << (tst&eBadLexem?", eBadLexem":"")
            << (tst&eSyntax?", eSyntax":"")
            << (tst&eError?", eError":"") 
        << std::endl;
        return false;
    }
    return true;
}

bool C_Xprs::Evaluate(const char *expression, int& result)
{
    bool ok = 0;
    if (ParseExpression(expression) && rootNode.size() == 1 ) {
        result = Calcualte(*rootNode.top());
        ok = 1;
    } 
    while(!rootNode.empty()) {
       XprsTree* node = rootNode.getpop();
       delete node;
    }
    return ok;
}
