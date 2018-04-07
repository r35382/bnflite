## BNF by domain specific language in C++ form

BackusNaur Form (BNF) is a notation for the formal description of computer languages. 
Simply stated, it is a syntax for describing syntax. 
BNF and its deviations(EBNF, ABNF ...) are commonly used to elaborate wide range of programming specifications.

BNF based on a declarative syntax that allows the user to define language constructions via "production rules".
Each rule except "terminal" is a conjunction of a series of of more concrete rules or terminals:

`production_rule ::= <rule_1>...<rule_n> | <rule_n_1>...<rule_m>;`

For example:

    <digit> ::= <0> | <1> | <2> | <3> | <4> | <5> | <6> | <7> | <8> | <9>
    <number> ::= <digit> | <digit> <number>

which means that a number is either a single digit, or a single digit followed by another number. 
( the number is just a digit or another number with one more digit )

BNFlite implements embedded domain specific language approach for gramma specification 
when all "production rules" are constructed as instances of C++ classes concatenated 
by means of overloaded arithmetic operators. 

Previous example can be presented in C++ form:
   
    #include "bnflite.h" // include lib
    using namespace bnf;
    Lexem Digit = Token("0") | "1"  | "2" | "4" | "5" | "6" | "7" | "8" | "9"; 
    Lexem Number;
    Number = Digit | Digit + Number;

Execution of this C++ code produces an internal date representation for further parsing. 

## class Token

In the previous example the terminal is a symbol specified as "Token". 

    Token Digit("0123456789"); // more compact and optimal than above
    Token Letter('0', '9');   // one more compact form
	Token NotPoint(1,127); NotPoint.Remove('.');  // all chars except point 
		
## class Lexem	

Lexical productions are introduced as "Lexem" object: 

    Token Letter('A', 'Z');   // Declare as upper-case
    Letter.Add('a', 'z');     // append lower-case to token
    Lexem LetterOrDigit = Letter | Digit;
    Lexem OptionalIdentifier;
    OptionalIdentifier = LetterOrDigit + OptionalIdentifier | Null();
    Lexem Identifier = Letter + OptionalIdentifier;

The C++ '=' operator is used as BNF '::='. 
The '+' operator implements sequential concatenation. 
The '|' operator implements 'or' relation. The Null() object return positive production with zero length. By default the parser tests all alternatives productions and selects one with longest input.
The "Identifier" can not started with digit. But it can be one letter.

## Repeatable Constructions	

In most later BNF-like metalanguages the BNF recursive repeat construct is replaced by a sequence mechanism. BNFlite offers to use the following functions:

    Series(a, token, b);
    Iterate(a, lexem, b);
    Repeat(a, rule, b);

So we can use: 

    Lexem Number = Series(1, Digit);  // 1 means at least one digit 
    Lexem Identifier = Letter + Iterate(0, LetterOrDigit); // 0 - means zero-or-one ore more

Such constructions are more readable and work faster.
Additionally BNFlite supports Advanced BNF style introducing several overloaded operators to support repeatable constructions.

## class Rule	

"Rule" object is used to define syntax productions:

    Rule Array = Identifier + "[" + Number + "]";

Now we can call the parser:

    int tst = Analyze(Identifier, "b0[16]");

The parser tries to find right correspondence between "Array" objects and parts of parsed text:
`Identifier + "[" + Number + "]"`  <=>  `"b0" + "["  + "16" +  "]"`

Practically the parser goes down through composed BNFlite objects achieving "Token" objects. Each Token can be fail or succeed against current character. In case of success upper object continues parsing with next lower object(+). Otherwise, upper object goes to next alternation(|) or fails if no more alternation is left.   

Lets consider another example:

    char* end;
    int tst = Analyze(Identifier, "b[16];", &end);

Parser decomposes text to  `"b0" + "["  + "16" +  "]"`. 
The ";" character is left because no rule for last character the `Analyze` returns negative value. 
The "end" variable contains pointer to unrecognised ";".


## Lexing and Parsing phases

Let assume we need to parse `buf[16]` text as C style array:
We can define it as lexem:

    Lexem Identifier = Letter + OptionalIdentifier;
    Lexem Array = Identifier + "[" + Digit + "]";

Or as production rule:

    Rule Identifier = Letter + OptionalIdentifier;
    Rule Array = Identifier + "[" + Digit + "]";

For the second case we can parse text with tab an spaces like this `"buf0\t[ 16 ]"`. 
For the first case we need to program all expected spaces as tokens. 
But it may be not enough and zero_parse construction should be overloaded by own handler. 
Dual licensed version has examples how to perform constructions like this `"buf [ 16 /*17*/ ]"`   

## User's Callbacks

Intermediate parsing results can be obtain by callbacks. Two kinds of callback are supported.
 - Function with prototype  `int fun(const char*, size_t)`  can be used as an expression element:

    int SizeNumber(const char* number_string, size_t length_of_number) 
    { printf("Size of Array : %.*s;\n", length_of_number, number_string); return true; }
    Rule Array = Identifier + "[" + Digit + SizeNumber + "]";

The user callback can return 1 for success or 0 to fail parsing manually.
	
 - Each Rule can be bound with callback to implement user needs
The user needs to define own working type for his data. This type is used for specialisation 
of BNFlite Interface template class to pass data between Rules. 

    typedef Interface<user_type> Usr;
    Usr SizeNumber(std::vector<Usr>& usr) 
    { printf("Size of Array : %.*s;\n", usr[2].length, usr[2].text); return usr[2]; }
    Bind(Array, SizeNumber);

Rule element callback receives vector of Interface objects from lower rules 
and returns single Interface object as result. Root result is in `Analyze` call.

    Usr usr;
    int tst = bnf::Analyze(Identifier, "b[16];", &end, usr);

	
## Optimizations for parser

Generally, BNFlite utilizes simple top-down parser with backtracking.
This parser may be not so good for complex grammar. However, the user has ways to make parsing smarter.

 - `Return()` - Choose current production
 - `AcceptFirst()` - Choose first appropriate production
 - `Skip()` - Accept result but not production itself

But most promising way is to inherit BNFline base classes to create own optimal constructions


## Debugging of BNFLite Grammar

Writing grammar by EDSL is unusual and the user does not have full understanding about the parser. If the Analyze call returns an error for the correct text, then the user always should take into consideration the possibility of grammar bugs.

### Return code

Return code from Analyze call can contain flags related to the grammar. For example, eBadRule, eBadLexem flags mean the tree of rules is not properly built.

### Names and breakpoints

The user can assign a name to the Rule. It can help to track recursive descent parser using Rule::_parse function. Debugger stack (history of function calls) can inform which Rule was applied and when. The user just needs to watch the this.name variable. It is not as difficult as it seems at first glance.

### Grammar subsets

Analyze function can be applied as unit test to any Rule representing subset of grammar.

### Tracing

The first kind of callback or function with prototype `bool foo(const char* lexem, size_t len)` can be used in BNFLite expressions for both reasons: to obtain temporary results and to inform about predicted errors.

This function will print the parsed number:

    static bool DebugNumber(const char* lexem, size_t len)
    {    printf("The number is: %.*s;\n", len, lexem);    return true; }
        /* … */
    Rule number = number_ + DebugNumber;
	
The function should return true if result is correct.


Let's assume the numbers with leading `+` are not allowed.

    static bool ErrorPlusNumber(const char* lexem, size_t len)
    { printf("The number %.*s with plus is not allowed\n", len, lexem); return false;}
        /* … */
    Lexem number_p = !Token("+") + int_ + !frac_ + !exp_ + ErrorPlusNumber;
    Rule number = number_ | number_p;

	The function should return false to inform the parser the result is not fit. C++11 constructions like below are also possible:

    Rule number = number_ | [](const char* lexem, size_t len)
    { return !printf("The number %.*s with plus is not allowed\n", len, lexem); }




...