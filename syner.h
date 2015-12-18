// Title   : syner.h
// Purpose : Syntax analysis header file for SCL. For CM510 PG3 phase 4.
// Author  : Matthew Jacques
// Date    : 24/11/13

#ifndef SYNER_H
#define SYNER_H



// A program is a (potentially infinite) sequence of constant and variable
// definitions followed by a (potentially infinite) sequence of statements.

// The statements are represented by an AST (Abstract Syntax Tree) variable.
// The declarations are represented by an SymTab (Symbol Table) variable.


// Using standard libraries.
using namespace std;



// Include standard string library and lexer header file for LexToken type.
#include <string>  // Standard C++ strings library
#include "lexer.h" // header for lexer.cxx


// Forward declaration of structs for the Abstract Syntax Tree (AST) and
// the Symbol Table (SymTab).
struct AST;                                // Abstract Syntax Tree
struct Expression;                                // Expression
struct BasicExp;                                // Basic expression
struct Term;                                // Term
struct Factor;                                // Factor
struct SymTab;                                // Symbol Table



// Have to declare the enum types first because C++ won't allow
// enums to be forward declared. Bloody awful language if you ask me.



// The different kinds of SCL data types. VOIDDATA is included
// to allow a DataType variable to be initialised to a value which
// is not a possible type for a variable/constant in SCL. Its major
// use is when parsing a factor (see below).
enum DataType { VOIDDATA, BOOLDATA, STRINGDATA, INTDATA, FLOATDATA };




// The symbol table is a linked list of entries for the declarations. The
// declaration currently being parsed is the head of that list so that the
// table ends up in reverse order of declaration.
// Each SymTab entry contains the variable or constant identifier, its type,
// whether it's a constant, a pointer to its initialisation literal (or NULL)
// and a pointer to the rest of the entries in the table (or NULL).
struct SymTab                                       // Symbol Table
{
  string     ident;                          // Var name
  DataType   type;                          // Var type
  Factor     *initialise;                          // Initialisation literal
  SymTab     *next;                          // Rest of entries
}; // SymTab



// An AST is a linked list of entries for the statements. Uses a struct with
// a tag field rather than a union type because strings aren't allowed as
// union type members. Somewhat inefficient but it's simple and it works.
// Each AST entry contains a pointer to a structure representing a statement
// (assignment, if, while, input or output) and a pointer to the entry for
// the rest of the statements (or NULL).
struct AST                                         // AST for statements
{
  Expression  *expr;                              // Expression to assign
  AST         *next;                              // Rest of statements
}; // AST



// An expression is a pointer to a basic expression, potentially followed
// by a relational operator and a pointer to another basic expression.
struct Expression                                  // Expressions
{
  BasicExp *be1;                            // First basic expression
  string   relOp;                            // Relational operator
  BasicExp *be2;                            // Second basic expression
}; // Expression



// A basic expression is a pointer to a term, potentially followed
// by zero or more - i.e. a list - of additional operators and terms. If there
// are no further operators and terms bexp is set to NULL.
struct BasicExp                                     // Basic expression
{
  Term     *term;                           // First Term
  string   addOp;                           // Addition operator
  BasicExp *bexp;                           // List of terms
}; // BasicExp



// A term is a pointer to a factor followed by zero or more - i.e. a list -
// of multiplicative operators and factors. If there are no further operators
// and factors term is set to NULL.
struct Term                                        // Term
{
  Factor *fact;                            // First factor
  string mulOp;                            // Multiplicative operator
  Term   *term;                            // List of factors
}; // Term



// A factor is either a literal constant, an identifier, a bracketed
// Expression or '!' followed by a factor. The literal field identifies which
// of the fields of Factor is to be used. If the literal field is set to true
// the lit field for the data type given by the type is to be used.
// The values of the literals are stored; an identifier is stored by a
// pointer to the SymTab entry for that identifier; bracketed expressions and
// negated factors are stored by pointers to the relevant AST structures.
// Note that an integer literal *may* be negative although it is impossible
// to declare/initialise and integer with/to a negative value.
struct Factor                                      // Factor
{
  bool       literal;                          // Tag field
  DataType   type;                          // Type field
  string     litBool;                          // Boolean literal
  string     litString;                          // String literal
  int        litInt;                          // Integer literal
  string     litFloat;                          // Float literal
  SymTab     *ident;                          // Identifier
  Expression *bExp;                          // Bracket expression
  Factor     *nFactor;                          // Negated factor
}; // Factor



// All possible SCL error messages are held in one of three constant
// arrays, one array for syntax errors, one for static semantic errors and
// one for type errors.
// Since C++ arrays start at index 0 and a return code of 0 indicates
// no error we add a dummy element to the front of the error message
// arrays to occupy position 0.

// First, declare the type of class to throw for exceptions.
// For full object orientation I should make this a subclass of
// the class exception but that's not necessary so I won't bother.
// Oh all right, I didn't know what virtual methods exception
// wanted and I didn't have time to find out.

class Report                                       // Syntactic exceptions
{
private:
  // Data members.
  int number;                            // Error number
  LexToken lexToken;                            // Offending token
public:
  // Class constructor.
  Report(int n,                                  // *In* number value
    LexToken lT)                            // *In* lexToken value
  {
    number = n;                            // Copy parameters
    lexToken = lT;                            // into data members
  }

  // Accessor methods.
  int getNumber()
  {
    return number;
  }
  LexToken getLexToken()
  {
    return lexToken;
  }
}; // class Report




// Now the error numbers and messages.
const int maxSyntaxError = 10;                   // Nmr of syntax errors
const int maxStaticError = 4;                   // Nmr of static errors
const int maxTypeError = 20;                   // Nmr of type errors
const int minSyntaxError = 0;                   // First syntax error
const int minStaticError = 100;                   // First static error
const int minTypeError = 200;                   // First type error

const string syntax[maxSyntaxError]                // Syntax error messages
= { "Not a syntax error.\n",                                        //  0

"Unknown syntax error.\n",                                      //  1
"Expected type identifier in declaration.\n",                   //  2
"Expected identifier in declaration.\n",                        //  3

"Expected = in declaration.\n",                                 //  4
"Expected literal in declaration.\n",                           //  5

"Expected in after declaration.\n",                             //  6
"Expected right parenthesis in factor.\n",                      //  7

"Expected end after expression.\n",                             //  8

"Unknown syntax error.\n"
}; // Syntax error messages



const string statics[maxStaticError]               // Static error messages
= { "Not a static semantic error.\n",                               // 100

"Attempt to re-declare identifier.\n",                          // 101
"Attempt to use undeclared identifier.\n",                      // 102

"Unknown static semantic error.\n"
};

const string type[maxTypeError]                  // Type error messages
= { "Not a type error.\n",                                          // 200

"Mismatch between types in an expression.\n",                   // 201

"Mismatch between types in a basic expression.\n",              // 202
"Attempt to use + operator with non-numeric operands.\n",       // 203

"Attempt to use - operator with non-numeric operands.\n",       // 204
"Attempt to use || operator with non-boolean operands.\n",      // 205
"Mismatch between types in a term.\n",                          // 206
"Attempt to use * operator with non-numeric operands.\n",       // 207
"Attempt to use / operator with non-numeric operands.\n",       // 208
"Attempt to use % operator with non-numeric operands.\n",       // 209
"Attempt to use && operator with non-boolean operands.\n",      // 210
"Attempt to use ! operator with non-boolean operand.\n",        // 211
"Attempt to use == operator with a string value.\n",            // 212
"Attempt to use != operator with a string value.\n",            // 213
"Attempt to use > operator with a string value.\n",             // 214
"Attempt to use < operator with a string value.\n",             // 215
"Attempt to use >= operator with a string value.\n",            // 216
"Attempt to use <= operator with a string value.\n",            // 217

"Mismatch between types in a declaration.\n",                   // 218

"Unknown type error.\n"
};


// synAnal parses a complete SCL program. Calls skipWhiteComments to set
// things up for the lexer, then parses the declarations and statements.
// Returns the SymTab and AST which results if the parse is
// successful and terminates with an error message otherwise.
void synAnal(ifstream &inFile,                    // *In-Out* Input file
  ofstream &outFile,                   // *In-Out* Output file
  SymTab *&st,                         // *Out* Symbol table
  AST *&ast,                           // *Out* Abs syntax tree
  int &label);                        // *In-Out* Label number



// Prints out the Symbol Table to cout.
void printST(ofstream &outFile,                    // *In-Out* Output file
  SymTab   *st);                       // *In* Symbol table

// Prints the Abstract Syntax Tree to cout.
void printAST(ofstream &outFile,                   // *In-Out* Output file
  AST *ast);                           // *In* Abs syntax tree


#endif

