// Title   : lexer.h
// Purpose : Lexical analysis header file for SCL. For CM510 PG3 phase 4.
// Author  : Matthew Jacques
// Date    : 24/11/13


#ifndef LEXER_H
#define LEXER_H



// Using standard libraries.
using namespace std;

// Include file IO library and standard string library.
#include <fstream> // Standard file I/O
#include <string>  // Standard C++ strings librarys



// Struct for lexical tokens. Uses a struct with a tag field rather
// than a union type because strings aren't allowed as union type
// members. Somewhat inefficient but it's simple and it works.

enum LexTokenTag {
  IDENT,
  BOOLLIT, STRINGLIT, INTLIT, FLOATLIT,
  ASSIGN,
  LPAREN, RPAREN,
  ADDOP, RELOP, MULOP, NOTOP,
  BOOL, STRING, INT, FLOAT,
  LET, IN,
  END
}; // LexTokenTag


struct LexToken
{
  LexTokenTag tag;                                // Tag field
  string      ident;                              // Identifier name
  string      boolLit;                            // Boolean literal
  string      stringLit;                          // String literal
  int         intLit;                             // Integer literal
  string      floatLit;                           // Float literal
  string      addOp;                              // Additional operator
  string      relOp;                              // Relational operator
  string      mulOp;                              // Multiplicative operator
}; // Token



// lexAnal reads the next token from input and puts it in token.
// If a lexical error is detected calls exit to terminate the program.
// Assumes that the next input character is the start of the next lexical
// token.
void lexAnal(ifstream &inFile,         // *In-Out* Input file
  ofstream &outFile,                   // *In-Out* Output file
  LexToken &lexToken);                 // *Out* Token lexed



// skipWhiteComments reads from input until the next non-whitespace
// character is encountered or until end of file.
// If the comment indicator "//" is encountered then skipWhiteComments
// ignores all text up to the end of the line and then carries on.
void skipWhiteComments(ifstream &inFile);         // *In-Out* Input file




// writeToken writes a lexical token to cout.

void writeToken(ofstream &outFile,                 // *In-Out* Output file
                LexToken lexToken);                // *In* Token to print
#endif

