
## About

BNFLite is a C++ template library for lightweight flexible grammar parsers.
It is intended to parse: 
 - command line arguments; 
 - small configuration files; 
 - output of different tools


## Purpose

Some time ago author dealt with the "ffmpeg" tool which was invented to integrate together a lot of parametrized video/audio codecs.
The tool have a comprehensive command line with thousands combinations of options. Examples are poor,
parameters sometime are ambiguous, the command line is not compatible from version to version.
Formal BNF specs of the command line language could help. However, for projects with limited budget
a process solution does not exist. The tool still is in progress and it is really difficult
to support any requirements document on-time.
But without any formal descriptions world users have a really problems reporting something that not work.
A potential defect can be either the coding bug or the requirement issue when some functionality
can not be achieved(or really difficult to achieve).
If submitted issue is not properly addressed it will be never performed and corrected!

BNFLite offers another approach when developer can specify
a language of command line parameters directly in the code.
Moreover, the "specifications" are executable now!


## Usage

You just need to include bnflite.h in to your C++ application:

   `#include "bnflite.h"`


## Concept

### BNF Notation

BNF (Backus–Naur form) specificates rules of a context-free grammar.
Each computer language should have a complete BNF syntactic specification.
Formal BNF term is called "production rule". Each rule except "terminal"
is a conjunction of a series of more concrete rules or terminals:

`production_rule ::= <rule_1>...<rule_n> | <rule_n_1>...<rule_m>;`

For example:

    <digit> ::= <0> | <1> | <2> | <3> | <4> | <5> | <6> | <7> | <8> | <9>
    <number> ::= <digit> | <digit> <number>
 
which means that the number is just a digit or another number with one more digit.
Generally terminal is a symbol called "token". There are two kind of productions rules:
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

Augmented BNF specifications introduce constructions like `"<a>*<b><element>"`
to support repetition where `<a>` and `<b>` imply at least `<a>` and at most `<b>` occurrences of the element.
For example,  `3*3<element>` allows exactly three and `1*2<element>` allows one or two.
Simplified construction `*<element>` allows any number(from 0 to infinity). Alternatively `1*<element>`
 requires at least one.
BNF Lite offers to use the following constructions:
 `Series(a, token, b);`
 `Iterate(a, lexem, b);`
 `Repeat(a, rule, b);`
	
But BNF Lite also supports ABNF-like forms:

    Token DIGIT("0123456789");
    Lexem AB_DIGIT = DIGIT(2,3)  /* <2>*<3><element> - any 2 or 3 digit number */
    Lexem I_DIGIT = 1*DIGIT;     /* 1*<element> any number  */
    Lexem O_DIGIT = *DIGIT;      /* *<element> - any number or nothing */
    Lexem N_DIGIT = !DIGIT;      /* <0>*<1><element> - one digit or nothing */```
	
So, you can almost directly transform ABNF specifications to BNF Lite

### User's Callbacks

To receive intermediate parsing results callback system can be used.
The first kind of callback can be used as expression element:

    int MyNumber(const char* nuber_string, size_t length_of_number) //...
    Lexem Number = Iterate(1, Digit) + MyNumber;```
	
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

    Lexem Number;
    Number = Digit | Digit + Number;

when parsing is finished (after Analyze call) you had to break recursion manually
like this

`Number = Null();`
	
Otherwise not all bnflite internal objects will be released (memory leaks expected)


## Design Notes

The prior-art is rather  ""A BNF Parser in Forth"  http://www.bradrodriguez.com/papers/bnfparse.htm .
And this lib is not related to `Boost::Spirit` in this context. Parser goes from  implementation of domain specific language here. This is expendable approach, for example, the user can inherit public lib classes to create own constructions to parse and perform simultaneously. 


## Examples

1. cmd.cpp - simple command line parser
2. cfg.cpp - parser of restricted xml configuration
3. ini.cpp - parser of ini-files (custom parsing example to perform grammar spaces and comments)
4. calc.cpp - arithmetic calculator

>$ g++ calc.cpp

>$ a.exe "2+(1+3)*2"

>Result of 2+(1+3)*2 = 10

Examples have been tested on several msvc and gcc compilers.


## Demo (simplest formula compiler & bite-code interpreter)

1. main.cpp - starter of byte-code formula compiler and interpreter
2. parser.cpp - BNF-lite parser with grammar section and callbacks
3. code_gen.cpp - byte-code generator
4. code_lib.cpp - several examples of embedded functions (e.g POW(2,3) - power: 2*2*2)
5. code_run.cpp - byte-code interpreter (used SSE2 for parallel calculation of 4 formulas)

To build and run:

>$ g++ -O2 -march=pentium4 -std=c++14 -I.. code_gen.cpp  parser.cpp  code_lib.cpp  main.cpp code_run.cpp

> $ a.exe "2+(1+3)*2"

> 7 byte-codes in 2+(1+3)*2

> Byte-code: Int(2),Int(1),Int(3),opAdd<I,I>,Int(2),opMul<I,I>,opAdd<I,I>

> result = 10, 10, 10, 10


## Contacts

Alexander Semjonov : alexander.as0@mail.ru


## Contributing

If you have any idea, feel free to fork it and submit your changes back to me.


## Donations

If you think that the library you obtained here is worth of some money
and are willing to pay for it, feel free to send any amount
through WebMoney WMID: 047419562122


## Roadmap

- Productize several approaches to catch syntax errors by means of this library
- Generate fastest C code parser from C++ BNF lite statements (..looking for customer)


## License

 - GPL
 
#### License Note after some feedbacks

This work has been done to contribute to open source community. 
Commercial usage is possible on the manner of proprietary Linux drivers.

The user should follow modular design and have grammar part developed like this:

        /-------------------\          /--------------------\
    /---+--------\  Gramma  |          |      Commercial    |
    |BNFlite(GPL)|  Module  | <======> |     Application    |
    \---+--------/  (LGPL)  |          | (proprietary code) |
        \-------------------/          \--------------------/

But NOT like this:
		
                        /--------------------\		
        /-------------------\   Commercial   |    
    /---+--------\  Gramma  |   Application  | 
    |BNFlite(GPL)|  Module  |     (but not   |
    \---+--------/  (LGPL)  |    proprietary | 
        \-------------------/      code!)    |
                        \--------------------/		
		
Practically it means that user need to have all parser stuff in separate module and other code should work with internal representation of parsed data. It is good design and definitely not a burden for the user. Best way is to include GPL `bnflite.h` only once to LGPL licensed `myparser.cpp`. The law does not care how `myparser.cpp` will be linked with other application code, it just obligates the user to distribute `myparser.cpp` sources together with application binares. This is the way for the user to contribute to open source community too!
   
If it is not an option you can request dual licensed commercial `bnflite+` for small fee(it is not ready yet but something can be released).



