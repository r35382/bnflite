
## About

BNFLite is a C++ template library for lightweight flexible grammar parsers.
BNFLite offers creative approach when the developer can specify 
a language for further parsing directly in the C++ code.
Moreover, such "specifications" are executable now!

## Purpose

BNFLite is intended to parse: 
 - command line arguments; 
 - small configuration files; 
 - output of different tools


## Preface

Once the author participated in the development of some tool which was invented to integrate together a lot of proprietary modules.
There were thousands combinations of command line options, poor examples, ambiguous docs. 
So the command line was not compatible from version to version. 
Up-to-date formal BNF specs of the command line language could help but not for projects with limited budget.
Starting YACC era, there is a solution to support some extra executable code describing language specifics. 
As a rule, usage of such means is too heavy because it is extra stuff and it is not BNF.
BNFLite does not have such drawbacks! 


## Usage

You just need to include bnflite.h in to your C++ application:

   `#include "bnflite.h"`


## Concept

### BNF Notation

The BNF (Backus–Naur form) specifies rules of a context-free grammar.
Each computer language should have a complete BNF syntactic specification.
Formal BNF term is called "production rule". Each rule except "terminal"
is a conjunction of a series of more concrete rules or terminals:

`production_rule ::= <rule_1>...<rule_n> | <rule_n_1>...<rule_m>;`

For example:

    <digit> ::= <0> | <1> | <2> | <3> | <4> | <5> | <6> | <7> | <8> | <9>
    <number> ::= <digit> | <digit> <number>
 
which means that the number is just a digit or another number with one more digit.

Generally terminal is a symbol called "token". There are two types of productions rules:
Lexical production is called "lexem". We will call syntax production rule as just a "rule".

### BNFlite notation

All above can be presented in C++ friendly notation:

    Lexem Digit = Token("0") | "1"  | "2" | "4" | "5" | "6" | "7" | "8" | "9"; //C++11: = "0"_T + "1" ...
    LEXEM(Number) = Digit | Digit + Number;
 
These both expressions are executable due to this "bnflite.h" source code library
which supports "Token", "Lexem" and "Rule" classes with overloaded "+" and "|" operators.
More practical and faster way is to use simpler form:

    Token Digit("01234567");
    Lexem Number = Iterate(1, Digit);
  
Now e.g. `bnf::Analyze(Number, "532")` can be called with success.

### ABNF Notation

Augmented BNF [specifications](https://tools.ietf.org/html/rfc5234) introduce constructions like `"<a>*<b><element>"`
to support repetition where `<a>` and `<b>` imply at least `<a>` and at most `<b>` occurrences of the element.
For example,  `3*3<element>` allows exactly three and `1*2<element>` allows one or two.
Simplified construction `*<element>` allows any number(from 0 to infinity). Alternatively `1*<element>`
 requires at least one.
BNFLite offers to use the following constructions:
 `Series(a, token, b);`
 `Iterate(a, lexem, b);`
 `Repeat(a, rule, b);`
	
But BNFLite also supports ABNF-like forms:

    Token DIGIT("0123456789");
    Lexem AB_DIGIT = DIGIT(2,3)  /* <2>*<3><element> - any 2 or 3 digit number */
    Lexem I_DIGIT = 1*DIGIT;     /* 1*<element> any number  */
    Lexem O_DIGIT = *DIGIT;      /* *<element> - any number or nothing */
    Lexem N_DIGIT = !DIGIT;      /* <0>*<1><element> - one digit or nothing */```
	
So, you can almost directly transform ABNF specifications to BNFLite

### User's Callbacks

To receive intermediate parsing results the callback system can be used.
The first kind of callback can be used as expression element:

    bool MyNumber(const char* number_string, size_t length_of_number) //...
    Lexem Number = Iterate(1, Digit) + MyNumber;
	
The second kind of callback can be bound to production Rule.
The user need to define own context type and work with it:

    typedef Interface<user_cntext> Usr;
    Usr DoNothing(std::vector<Usr>& usr) {  return usr[0]; }
    //...
    Rule Foo;
    Bind(Foo, DoNothing);

### Restrictions for Recursion in Rules

Lite version have some restrictions for rule recursion.
You can not write:

`Lexem Number = Digit | Digit + Number; /* failure */`
	
because Number is not initialized yet in the expressions.
You can use macro LEXEM for such constructions

`LEXEM(Number) = Digit | Digit + Number;`
	
that means

`Lexem Number;  Number = Digit | Digit + Number;`

when parsing is finished (after Analyze call) you have to break recursion manually
like this:

`Number = Null();`
	
Otherwise not all BNFlite internal objects will be released (memory leaks expected)


## Design Notes

BNFlite is а class library. It is not related to the template 
[Boost::Spirit](https://www.boost.org/doc/libs/1_64_0/libs/spirit/doc/html/index.html) library
This is expendable approach, for example, the user can inherit public lib classes to create own constructions to parse and perform simultaneously. It fact, parser goes from  implementation of domain specific language here.  
The prior-art is rather ["A BNF Parser in Forth"](http://www.bradrodriguez.com/papers/bnfparse.htm).


## Examples

1. examples/cmd.cpp - simple command line parser
2. examples/cfg.cpp - parser of restricted custom xml configuration
3. examples/ini.cpp - parser of ini-files (custom parsing example to perform grammar spaces and comments)
4. examples/calc.cpp - arithmetic calculator

>$cd examples

>$ g++ -I. -I.. calc.cpp

>$ ./a.exe "2+(1+3)*2"

>Result of 2+(1+3)*2 = 10

Examples have been tested on several msvc and gcc compilers.


## Unit Test ( C-like expression parser and calculator )

1. c_xprs/utest.cpp - simple unit test
2. c_xprs/c_xprs.h - parser of C-like expressions

>$cd c_xprs

>$ g++ -I. -I.. utest.cpp

>$ ./a.exe 

Output result of several C-like expressions   


## Demo (simplest formula compiler & bite-code interpreter)

* formula_compiler/main.cpp - starter of byte-code formula compiler and interpreter
* formula_compiler/parser.cpp - BNF-lite parser with grammar section and callbacks
* formula_compiler/code_gen.cpp - byte-code generator
* formula_compiler/code_lib.cpp - several examples of embedded functions (e.g POW(2,3) - power: 2*2*2)
* formula_compiler/code_run.cpp - byte-code interpreter (used SSE2 for parallel calculation of 4 formulas)

To build and run (remove option `-march=pentium4` if it needed for arm or 64 build):

>$ cd formula_compiler

>$ g++ -O2 -march=pentium4 -std=c++14 -I.. main.cpp parser.cpp code_gen.cpp code_lib.cpp code_run.cpp

>$ ./a.exe `"2 + 3 *GetX()"`  

> 5 byte-codes in: `2 + 3 *GetX()`

> Byte-code: Int(2),Int(3),opCall<1>,opMul<I,I>,opAdd<I,I>;

> result = 2, 5, 8, 11;

Note: The embedded function `GetX()` returns a sequential integer number started from `0`.
So, the result is four parallel computations: 
`2 + 3 * 0 = 2; 2 + 3 * 1 = 5; 2 + 3 * 2 = 8; 2 + 3 * 3 = 11`.


## Contacts

Alexander Semjonov : alexander.as0@mail.ru


## Contributing

If you have any idea, feel free to fork it and submit your changes back to me.


## Donations

If you think that the library you obtained here is worth of some money
and are willing to pay for it, feel free to send any amount
through WebMoney WMID: 047419562122


## Roadmap

- Productize several approaches to catch syntax errors by means of this library (done in unit test)
- Generate fastest C code parser from C++ BNFlite statements (..looking for customer)
- Support wide characters (several approaches, need customer reqs, ..looking for customer)
- Support releasing of ringed Rules (see "Restrictions for Recursion"), in fact the code exists but it is not "lite"


## License

 - MIT

