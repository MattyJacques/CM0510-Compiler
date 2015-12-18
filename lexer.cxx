// Title      : lexer.cxx
// Purpose    : Lexical analysis cxx for C--. For CM510 PG3.
// Author     : Matthew Jacques
// Date       : 11/10/2013

// Includes the header file from an external directory.
#include "lexer.h"
#include <fstream>  // Includes file IO library
#include <ctype.h>  // Includes header to classify and transform chars
#include <stdlib.h> // Includes the C++ standard library
#include <string>   // Includes the string class for C++



using namespace std; // Using standard library



void checkIdent(ifstream &inFile,                  // *In-Out* Input file
                LexToken &LexToken)                // *Out* Token lexed
{ // checkIdent will take in the next input as a char and then add it onto
  // the end of a string. It will then check for when the string ends and
  // then compare it to various reserved words and for bool literals. If
  // it comes across and none of these, then it will presume the string is
  // the name for an identifier.

  char next = ' ';           // Holds a character for the next char of input
  string  inputCheck = "";  // Holds a string for the current input

  inFile.get(next);         // Gets the next character of input

  while (isalnum(next) || next == '_')
  { // A While loop to place the next char into the string.
    inputCheck += next;
    inFile.get(next);
  }
  inFile.putback(next);

  // Checks inputCheck to see if the input was a matches any reserved words,
  // if so then it sets the token accordingly. It will also check for a
  // bool literal. If it is does not match any of the reserved words then
  // it will presume it is a indentifier name.
  if (inputCheck == "true" || inputCheck == "false")
  {
    LexToken.tag = BOOLLIT;
    LexToken.boolLit = inputCheck;
  }
  else if (inputCheck == "bool")
  {
    LexToken.tag = BOOL;
  }
  else if (inputCheck == "string")
  {
    LexToken.tag = STRING;
  }
  else if (inputCheck == "int")
  {
    LexToken.tag = INT;
  }
  else if (inputCheck == "float")
  {
    LexToken.tag = FLOAT;
  }
  else if (inputCheck == "let")
  {
    LexToken.tag = LET;
  }
  else if (inputCheck == "in")
  {
    LexToken.tag = IN;
  }
  else if (inputCheck == "end")
  {
    LexToken.tag = END;
  }
  else
  {
    LexToken.tag = IDENT;
    LexToken.ident = inputCheck;
  } // if statement
} // checkIdent

void getNewDigitString(ifstream &inFile,       // *In-Out* Input file
                       string &digitString,    // *In-Out* Hold next digit string
                       char &next)             // *In-Out* Hold next input char
{
  inFile.get(next);

  // While loop to add next into the string. Stops when not a digit
  // or end of file.
  while (isdigit(next) && inFile)
  {
    digitString += next;
    inFile.get(next);
  }
}

void lexIntLit(ifstream &inFile,     // *In-Out* Input file
               LexToken &LexToken,   // *Out* Token lexed
               ofstream &outFile)    // *In-Out* Output file   
{ // lexIntLit checks for a int literal, and stores it into
  // the correct token for output.

  char next = '0';            // Holds a character for the next char of input
  string digitString = "";    // Holds a string for the current input
  int Num = 0;

  LexToken.tag = INTLIT;      // Sets the token tag to a int lit.

  getNewDigitString(inFile, digitString, next);

  

  // Checks if next char is ^ if so call exit and output appropriate error
  // message.
  if (next == '^')
  {
    outFile << "Lexer error : missing '.' in float." << endl;
    exit(5);
  }
  
  // Checks if next chat is . if so continues on with the lex analysis.
  if (next == '.')
  {
    digitString += next;
    inFile.get(next);

    if (!isdigit(next))
    {
      outFile << "Lexer error : no digit after ." << endl;
      exit(6);
    }

    getNewDigitString(inFile, digitString, next);

    if (next == '.')
    {
      outFile << "Lexer error : multiple ." << endl;
      exit(7);
    }
    else if (next == '^')
    {
      digitString += next;
      inFile.get(next);

      if (!isdigit(next))
      {
        outFile << "Lexer error : no digit after ^" << endl;
        exit(8);
      }

      getNewDigitString(inFile, digitString, next);

      if (next == '^')
      {
        outFile << "Lexer error : multiple ^" << endl;
        exit(9);
      }
      else if (next == '.')
      {
        outFile << "Lexer eroor : multiple ." << endl;
        exit(10);
      }
    }

    LexToken.tag = FLOATLIT;
    LexToken.floatLit = digitString;
  }
  else
  {
    LexToken.tag = INTLIT;

    Num = atoi(digitString.c_str());

    if (Num > 32767 || Num < -32768)
    {
      exit(3);
    }
    else
    {
      LexToken.intLit = Num;
    }
  }

  inFile.putback(next); // Puts the input char that is not a digit back.

} // lexIntLit

void lexStringLit(ifstream &inFile,                // *In-Out* Input file
                  ofstream &outFile,               // *In-Out* Output file
                  LexToken &lexToken)              // *Out* Token lexed
{ // Read first ", read string, read second ". Return STRINGLIT token.
  // Must check for unexpected EOF and for non-printable characters.

  char ch = ' ';                                  // Read " into here

  // Read first " and initialise token.
  inFile.get(ch);
  lexToken.tag = STRINGLIT;
  lexToken.stringLit = "";

  // Read string characters until " or EOF encountered. If non-printable
  // character is encountered report lexer error and call exit to
  // terminate.
  inFile.get(ch);
  while ((ch != '\"') && inFile)
  {
    if (iscntrl(ch) && (ch != '\n') && (ch != '\t'))
    {
      outFile << "Lexer error : non printable character in string literal.\n";
      exit(3);
    }
    lexToken.stringLit = lexToken.stringLit + ch;
    inFile.get(ch);
  }

  // Check that second " was present. If not report a lexer error
  // and call exit to terminate.

  if (ch != '\"')
  {
    outFile << "Lexer error : missing \" on string literal\n";
    exit(4);
  }
} // lexStringLit

void skipWhiteComments(ifstream &inFile)           // *In-Out* Input file
{ // skipWhiteComments reads from input until the next non-whitespace
  // character is encountered or until end of file.
  // If the comment indicator "//" is encountered then skipWhiteComments
  // ignores all text up to the end of the line and then carries on.

  bool nonWhiteFound = false;                     // Carry on reading flag
  char next = ' ';                                // Next input character


  inFile.get(next);
  while (inFile && (!nonWhiteFound))
  { // If next is / check for another / ; if / found skip comment and read
    // next character; otherwise put both characters back on the input and
    // set nonWhiteFound.
    if (next == '/')
    {
      inFile.get(next);
      if (next == '/')
      {
        inFile.ignore(1024, '\n');
        inFile.get(next);
      }
      else
      {
        inFile.putback(next);
        inFile.putback('/');
        nonWhiteFound = true;
      }
    }
    else
    { // Check if next character is whitespace. If so read
      // the next character, if not put the character back on the input.
      nonWhiteFound = !(isspace(next) || (next == EOF));
      if (!nonWhiteFound)
        inFile.get(next);
      else
        inFile.putback(next);
    }
  }
} // skipWhiteComments

void writeToken(ofstream &outFile,                 // *In-Out* Output file
                LexToken lexToken)                 // *In* Token to print
{ // Write token to outFile.

  switch (lexToken.tag)
  {
  case IDENT        : { outFile << "IDENTIFIER : ";
                        outFile << lexToken.ident;
                      }
                      break;
  case BOOLLIT      : { outFile << "BOOLEAN LITERAL : ";
                        outFile << lexToken.boolLit;
                      }
                      break;
  case STRINGLIT    : { outFile << "STRING LITERAL : \"";
                        outFile << lexToken.stringLit << "\"";
                      }
                      break;
  case INTLIT       : { outFile << "INTEGER LITERAL : ";
                        outFile << lexToken.intLit;
                      }
                      break;
  case FLOATLIT     : { outFile << "FLOAT LITERAL : ";
                        outFile << lexToken.floatLit;
                      }
                      break;
  case ASSIGN       : { outFile << "ASSIGN";
                      }
                      break;
  case LPAREN       : { outFile << "LPAREN";
                      }
                      break;
  case RPAREN       : { outFile << "RPAREN";
                      }
                      break;
  case ADDOP        : { outFile << "ADDITIONAL OPERATOR : ";
                        outFile << lexToken.addOp;
                      }
                      break;
  case RELOP        : { outFile << "RELATIONAL OPERATOR : ";
                        outFile << lexToken.relOp;
                      }
                      break;
  case MULOP        : { outFile << "MULTIPLICATIVE OPERATOR : ";
                        outFile << lexToken.mulOp;
                      }
                      break;
  case NOTOP        : { outFile << "NOT OPERATOR";
                      }
                      break;
  case BOOL         : { outFile << "BOOL";
                      }
                      break;
  case STRING       : { outFile << "STRING";
                      }
                      break;
  case INT          : { outFile << "INT";
                      }
                      break;
  case FLOAT        : { outFile << "FLOAT";
                      }
                      break;
  case LET          : { outFile << "LET";
                      }
                      break;
  case IN           : { outFile << "IN";
                      }
                      break;
  case END          : { outFile << "END";
                      }
                      break;
  } // switch(lexToken.tag)
} // writeToken

void lexAnal(ifstream &inFile,                    // *In-Out* Input file
             ofstream &outFile,                   // *In-Out* Output file
             LexToken &LexToken)                  // *Out* Token lexed
{
  // LexAnal will get the next input character and attempt to give
  // LexToken the correct tag for the output. It has various catches to
  // be able to check for all token types. After the token has been checked
  // it will call skipWhiteComments to skip to the next input that could be
  // a token.

  char next = ' '; // A char to hold the next input

  // Gets the next char, then checks it for every single type of token
  // when it finds which one will set the appropriate tags or call the
  // approprate subprogam to deal with that type of token. If no match,
  // then it will display charcater not recognised.
  inFile.get(next);
  if (!inFile)
  {
    outFile << "End of file detected" << endl;
    exit(1);
  }
  else
  {
    if (isalpha(next))
    {
      inFile.putback(next);
      checkIdent(inFile, LexToken);
    }
    else  if (next == '\"')
    {
      inFile.putback(next);
      lexStringLit(inFile, outFile, LexToken);
    }
    else if (isdigit(next))
    {
      inFile.putback(next);
      lexIntLit(inFile, LexToken, outFile);
    }
    else if (next == '=')
    {
      inFile.get(next);
      if (next == '=')
      {
        LexToken.tag = RELOP;
        LexToken.relOp = "==";
      }
      else
      {
        inFile.putback(next);
        LexToken.tag = ASSIGN;
      }
    }
    else if (next == '(')
    {
      LexToken.tag = LPAREN;
    }
    else if (next == ')')
    {
      LexToken.tag = RPAREN;
    }
    else if (next == '+')
    {
      LexToken.tag = ADDOP;
      LexToken.addOp = '+';
    }
    else if (next == '-')
    {
      LexToken.tag = ADDOP;
      LexToken.addOp = '-';
    }
    else if (next == '|')
    {
      inFile.get(next);
      if (next == '|')
      {
        LexToken.tag = ADDOP;
        LexToken.addOp = "||";
      }
      else
      {
        inFile.putback(next);
        outFile << "Error Wrong Symbol";
        exit(3);
      }
    }
    else if (next == '<')
    {
      inFile.get(next);
      if (next == '=')
      {
        LexToken.tag = RELOP;
        LexToken.relOp = "<=";
      }
      else
      {
        inFile.putback(next);
        LexToken.relOp = "<";
      }
    }
    else if (next == '*')
    {
      LexToken.tag = MULOP;
      LexToken.mulOp = "*";
    }
    else if (next == '/')
    {
      LexToken.tag = MULOP;
      LexToken.mulOp = "/";
    }
    else if (next == '%')
    {
      LexToken.tag = MULOP;
      LexToken.mulOp = "%";
    }
    else if (next == '&')
    {
      inFile.get(next);
      if (next == '&')
      {
        LexToken.tag = MULOP;
        LexToken.mulOp = "&&";
      }
      else
      {
        inFile.putback(next);
      }
    }
    else if (next == '!')
    {
      inFile.get(next);
      if (next == '=')
      {
        LexToken.tag = RELOP;
        LexToken.relOp = "!=";
      }
      else
      {
        inFile.putback(next);
        LexToken.tag = NOTOP;
      }
    }
    else if (next == '>')
    {
      inFile.get(next);
      if (next == '=')
      {
        LexToken.tag = RELOP;
        LexToken.relOp = ">=";
      }
      else
      {
        inFile.putback(next);
        LexToken.tag = RELOP;
        LexToken.relOp = ">";
      }
    }
    else
    {
      outFile << "ERROR: Char not recognised" << endl;
      exit(2);
    }
    }// If statement

    // Calls skipWhiteComments to skip to the next legal input
    skipWhiteComments(inFile);
} // lexAnal





