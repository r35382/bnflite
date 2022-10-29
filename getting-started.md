## BNF by Domain Specific Language in C++ Form

BackusNaur Form (BNF) is a notation for the formal description of computer languages. 
Simply stated, it is a syntax for describing syntax. 
BNF and its deviations(EBNF, ABNF ...) are commonly used to elaborate wide range of programming specifications.

BNF based on a declarative syntax that allows the user to define language constructions via "production rules".
Each rule except "terminal" is a conjunction of a series of more detailed rules or terminals:

`production_rule ::= <rule_1>...<rule_n> | <rule_n_1>...<rule_m>;`

For example:

    <digit> ::= <0> | <1> | <2> | <3> | <4> | <5> | <6> | <7> | <8> | <9>
    <number> ::= <digit> | <digit> <number>

which means that a number is either a single digit, or a single digit followed by another number. 
( the number is just a digit or another number with one more digit )

BNFlite implements embedded domain specific language approach for grammar specification 
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

For the previous example the terminal symbol "Token" can be specified like this:

    Token Digit("0123456789"); // more compact and optimal than above
    Token Digit('0', '9');   // one more compact form
	
One more way to create Token:	
	
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
The '|' operator implements 'or' relation. The Null() object return positive production with zero length. 
By default the parser tests all alternative productions and selects one with longest input.
The "Identifier" can not started with digit. But it can be one letter.

## Repeatable Constructions	

Let assume we need to support 32-character identifiers only, so a 33-character identifier should be treated as error.
The above `OptionalIdentifier` recursive repeat construction can not support such functionality. 
However it can be replaced by cyclic sequence mechanisms. 
BNFlite offers to use the following functions:

    Series(a, token, b);
    Iterate(a, lexem, b);
    Repeat(a, rule, b);

where `a` and `b` imply at least `a` and at most `b` occurrences	
So we can use: 

    Lexem Number = Series(1, Digit);  // 1 means at least one digit 
    Lexem Identifier = Letter + Iterate(0, LetterOrDigit, 32); // 0 - means zero or more at last 32

Additionally BNFlite supports compact style(ABNF) introducing several overloaded operators to implement repeatable constructions.

    Lexem Number = 1*Digit;  // 1 means at least one digit 
    Lexem Identifier = Letter + *LetterOrDigit; // * - means zero-or-one or more


## class Rule	

"Rule" object is used to define syntax productions:

    Rule Array = Identifier + "[" + Number + "]";

Now we can call the parser:

    int tst = Analyze(Identifier, "b0[16]");

The parser tries to find right correspondence between "Array" objects and parts of parsed text:
`Identifier + "[" + Number + "]"`  <=>  `"b0" + "["  + "16" +  "]"`

Practically the parser goes down through composed BNFlite objects achieving "Token" objects. 
Each Token can be fail or succeed against current character. 
In case of success the upper object continues parsing with next lower object. 
Otherwise, the upper object goes to next alternation(|) or fails if no more alternation is left.   

Lets consider another example:

    char* end;
    int tst = Analyze(Identifier, "b[16];", &end);

Parser decomposes text to  `"b0" + "["  + "16" +  "]"`. 
The `";"` character is left because no rule for the last character, so the `Analyze` returns a negative value.
The "end" variable contains the pointer to unrecognized `";"`.


## Lexing and Parsing Phases

Let assume we need to parse `buf[16]` text as C style array:
We can define it as a lexem:

    Lexem Identifier = Letter + OptionalIdentifier;
    Lexem Array = Identifier + "[" + Digit + "]";

Or as a production rule:

    Rule Identifier = Letter + OptionalIdentifier;
    Rule Array = Identifier + "[" + Digit + "]";

The result of `Analyze(Array, "buf0\t[ 16 ]");` depends on `Array` type.  
For the "Rule" case the input text is successfully parsed.
For the "Lexem" case any spaces or tabs in the input text is treated as error.

The "Rule" behavior to ignore some predefined constructions can be changed by the user.
In this case the custom handler `const char* pre_parse(const char* ptr)` should be introduced
to call `Analyze(Array, "buf [ 16 /*17*/ ]", pre_parse);`



## User's Callbacks

Intermediate parsing results can be obtain by callbacks. Two kinds of callback are supported.
 - Function with prototype  `bool fun(const char*, size_t)`  can be used as an expression element:

    bool SizeNumber(const char* number_string, size_t length_of_number) 
    {   int number;  	
        std::istringstream iss(std::string(number_string, length_of_number));
        iss >> number;
        if (iss.good() && number > 0 && number < MAX_NUMBER) return true;	
        else { std::cout << "bad size of array:"; std::cout.write(number_string, length_of_number); return false; }
    }
    //...
    Rule Array = Identifier + "[" + Digit + SizeNumber + "]";

The user callback has too parameters: the pointer to the latest found element and its length.
In this example `SizeNumber` callback accepts the string of a digit number.
The user callback can do some semantic analyzes and  return `1`(true) to continue parsing 
or `0`(false) to reject the current rule. 
	
 - Each "Rule" can be bound with the callback to be called in successful case
First of all the user needs to define own working type for his data. This type is used for specialization 
of BNFlite `Interface` template class to pass data between Rules. 

    typedef Interface<user_type> Usr;
    static Usr SizeNumber(std::vector<Usr>& usr) // user callback function
    { printf("Size of Array : %.*s;\n", usr[2].length, usr[2].text); return usr[2]; }
    Bind(Array, SizeNumber); //connection between rule `Array` and user callback 

The callback receives vector of Interface objects from lower Rules
and returns single `Interface` object as a result.
The final root result is in `Analyze` call.

    Usr usr; // results after parsing
    int tst = bnf::Analyze(Identifier, "b[16];", usr);

## Parameters for `Analize` API Function Set

 - `root` - top Rule for parsing 
 - `text` - text to be parsed 
 - `pstop` - the pointer where parser stops  (`*pstop != '\0'` - not enough rules or resources)
 - `u` - top variable of `Interface` template structure (see the second kind of callbacks)
 - `u.text` - pointer to text to be parsed (copy of `text`)
 - `u.length` - final length of parsed data to be returned after `Analize` call
 - `u.data` - final user data to be returned after `Analize` call
  
### Return Value 

`Analize()` returns a negative value in case of parsing error. 
Bit fields of the returned value can provide more information about parser behavior.

	
## Optimizations for Parser

Generally, BNFlite utilizes simple top-down parser with backtracking.
This parser may be not so good for complex grammar. 
However, the user has several special rules to make parsing smarter.

 - `Return()` - Choose current production (should be last in conjunction rule)
 - `AcceptFirst()` - Choose first appropriate production (should be first in disjunction rule)
 - `Skip()` - Accept result but not production itself (can not be first)

In some cases less optimal `MemRule()` can be used to remember unsuccessful parsing to reduce known overhead


## Debugging of BNFLite Grammar

Writing grammar by EDSL is easier if the user has some understanding the parser behavior. 
If the `Analyze` call returns an error the user always should take into consideration 
both possibilities:
 - syntax errors in the input text (incorrect text)
 - grammar bugs (incorrect rules).
The BNFLite provides several mechanisms to minimize debugging and testing overheads     

### Return Codes

Return code from `Analyze` call can contain flags related to the gramma. 
 - `eBadRule`, `eBadLexem` - the rule tree is not properly built 
 - `eEof` - "unexpected end of file" for most cases it is OK, just not enough text for applied rules
 - `eSyntax` - syntax error (controlled by the user)
 - `eOver` - too much data for cycle rules  
 - `eRest` - not all text has been parsed
 - `eNull` - no result


### Names and Breakpoints

The user can assign the internal name to the `Rule` object by `setName()`. 
It can help to track recursive descent parser looking `Rule::_parse` function calls. 
Debugger stack (history of function calls) can inform which Rule was applied and when. 
The user just needs to watch the `this->name` variable. It is not as difficult as it seems at first glance.

### Debugging of Complex Gramma 

The user can divide complex gramma for several parts to develop them independently.
`Analyze` function can be applied as unit test to any `Rule` representing subset of the gramma.

### Tracing

The first kind of callback or function with prototype `bool foo(const char* lexem, size_t len)` 
can be used in BNFLite rules to obtain the temporary result. 

This function will print the parsed number:

    static bool DebugNumber(const char* lexem, size_t len)
    {    printf("The number is: %.*s;\n", len, lexem);    return true; }
        /* … */
    Token digit1_9('1', '9');
    Token DIGIT("0123456789");
    Lexem I_DIGIT = 1*DIGIT; // Series(1, DIGIT);
    Lexem exp_ = "Ee" + !Token("+-") + I_DIGIT ;
    Lexem frac_ = "." + I_DIGIT;
    Lexem int_ = "0" | digit1_9  + *DIGIT; //Series(0, DIGIT);
    Lexem num_ = !Token("-") + int_ + !frac_ + !exp_;
    Rule number = num_ + DebugNumber;
	
The function need to return `true` because result is correct.

### Catching Warning

The first kind of callback can be used in BNFLite rules to inform about incorrect situations.
Let's assume numbers with leading `0` can be performed bat they are not wanted.

    static bool Check0Number(const char* lexem, size_t len)
    { printf("Warning: the number %.*s with leading zero found\n", len, lexem); return false;}
        /* … */
    Lexem num_0 = "0" + digit1_9 + *DIGIT; // special rule for numbers with one leading zero
    Rule number = num_ + DebugNumber | num_0 + Check0Number;

The function still should return `true` to allow the parser to perform such numbers. 
C++11 constructions like below are also possible:

     Rule number = num_  | num_0 + [](const char* lexem, size_t len)
    { return !!printf("Warning: the number %.*s with leading zero found\n", len, lexem); }

### Catching Errors

In some cases we need to force parser to stop correctly because input text is erroneous.
Let's assume the numbers with leading of several `0` should be treated as error.   	
	
    static int Error00Number(const char* lexem, size_t len)
    { printf("Error with leading zeros: %.*s\n", len, lexem); return false}
        /* … */
    Lexem num_00 = Series(2, "0") + digit1_9 + *DIGIT; // special rule for numbers with two or more leading zeros
    Rule number = num_ + DebugNumber | num_00 + Error00Number + Catch() | num_0 + Check0Number;

The function should return `false` because text is not applied. 
The `Catch()` special library statement forces the parser to stop and return `eSyntax` error flag.

Note:	
 - The order `num_ + num_00 + num_0` is important because `num_0` is subset of `num_00`

	
### Try for Syntax Errors

Below is the extended example for hex-decimal digit: 

    Token hex_digit("abcdefABCDEF", DIGIT); 
    Lexem HexDigits  = Token("0") + Token("Xx") + Series(1, hex_digit);


Let's assume the parser tries to apply the `HexDigits` rule to `0.0` and `OxZ` text elements.
Both elements are not fit, but `0.0` can be appropriate for some another rule of gramma.
But the `OxZ` is syntax error. We definitely know there is no rule for it.
So we should use `Try()` special statement to force the parser to chech the production is ok. 
Otherwise, parser stops with`eSyntax` error at the end.
In other words, the `Try()` forces `eSyntax` error if input text is not applied after `Try()` special rule.

Lexem HexDigits  = Token("0") + Token("Xx") + Try() + Series(1, hex_digit);

In case of `HexDigits` unsuccess the internal `catch_error` handler is called.
And the `Analize` returns `eSyntax` flag.

	
