//Title   : syner.cxx
//Purpose : Syntax analysis subprograms for C--. For CM510 PG3 phase 2.
//Author  : Matthew Jacques
//Date    : 9/11/13

//Some of the syntax analysis subprogram must lookahead one token beyond
//the end of the construct they are parsing and some need not. To handle
//this we adopt the following convention  :
//
//     1. Each syntax analysis subprogram takes a LexToken called lexToken
//        as a reference parameter. When called they assume that lexToken
//        holds the first token of the construct they are attempting to parse.
//
//     2. Before returning each syntax analysis subprogram will lex the
//        next token into their lexToken reference parameter.
//
//In other words, lexToken holds the lookahead token and we lookahead after
//one token at the end of every syntax analysis subprogram *except*
//synStatements. We don't do it in synStatements because that would lead
//to reading past end of file when syntax analysing the top-level compound
//statement.


//Using standard libraries.
using namespace std;


//Include standard IO files library, standard IO manipulators library,
//standard character library, standard definition library, standard standard
//library.
#include <fstream>
#include <iomanip>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>

//Include string library, the syntax analysis header file and the lexical
///analysis header file.
#include <string>
#include "syner.h"
#include "lexer.h"



//Include AST and ST Output Subprograms.
#include "printers.cxx"



//***************************************************************************
//Symbol table lookup subprogram
//***************************************************************************

bool lookup(LexToken lexToken,                     //*In* Identifier token
  SymTab *st,                            //*In* Symbol table
  SymTab *&match)                        //*Out* Entry found or null
{ //lookup looks for an entry in the SymTab which has the same identifier
  //as the given lexToken. If a matching entry is found a pointer to it is
  //returned via the reference parameter match and lookup returns the
  //value true. If a matching entry is not found lookup returns false.
  //Note that this is a value returning subprogram which, as a side effect,
  //may also modify one of its parameters. This is disgusting programming
  //practice and I'm ashamed of myself for doing it. In my defence I can
  //only say that writing lookup in this way is standard practice for
  //compilers written in C-family languages.

  bool  found = false;                            //Entry found flag

  //Search Symbol table until a match is found or we reach the end
  //of the list.
  while ((st != NULL) && !found)
  {
    if (st->ident == lexToken.ident)
    {
      found = true;
    }
    else
    {
      st = st->next;
    }
  }

  //If entry found set match to point to it, if not set match to NULL.
  if (found)
  {
    match = st;
  }
  else
  {
    match = NULL;
  }

  //Return whether or not the entry was found.
  return(found);
} //lookup

//***************************************************************************
//End of symbol table lookup subprogram
//***************************************************************************



//***************************************************************************
//Variable / Constant Declaration Syntax analysis subprograms.
//***************************************************************************


void synDec(ifstream &inFile,                      // *In-Out* Input file
  ofstream &outFile,                     // *In-Out Output file
  SymTab   *&st,                         // *In-Out* Symbol table
  LexToken &lexToken)                    // *In-Out* Current token
{ // synDec gets lexical tokens from lexAnal and attempts to parse them
  // as a C-- local variable or constant declaration. If the parse is
  // successful synDec adds the declaration to the symbol table.
  // If the parse is unsuccessful synDec throws an exception.

  SymTab* newEntry; //For this Declaration
  SymTab* dummy;    //For the lookup

  newEntry = new SymTab;              // Sets new SymTab for newEntry
  newEntry->ident = "";               // Initialise the ident tag
  newEntry->type = VOIDDATA;          // Initialise newEntry type
  newEntry->initialise = NULL;        // Initialise newEntry
  newEntry->next = st;                // Sets next tag to st

  dummy = new SymTab;              // Sets new SymTab for dummy
  dummy->ident = "";               // Initialise the ident tag
  dummy->type = VOIDDATA;          // Initialise dummy type
  dummy->initialise = NULL;        // Initialise dummy
  dummy->next = NULL;                // Sets next tag to st

  lexAnal(inFile, outFile, lexToken);

  // Checks if the tokens tag is BOOL, if so it will pass the
  // type onto newEntry. If it is not a ident it will throw
  // the appropriate report case.
  if (lexToken.tag == BOOL)
  {
    newEntry->type = BOOLDATA;
  }
  // Checks if the tokens tag is STRING, if so it will pass the
  // type onto newEntry. If it is not a ident it will throw
  // the appropriate report case.
  else if (lexToken.tag == STRING)
  {
    newEntry->type = STRINGDATA;
  }
  // Checks if the tokens tag is INT, if so it will pass the
  // type onto newEntry. If it is not a ident it will throw
  // the appropriate report case.
  else if (lexToken.tag == INT)
  {
    newEntry->type = INTDATA;
  }
  else if (lexToken.tag == FLOAT)
  {
    newEntry->type = FLOATDATA;
  }
  else
  {
    throw Report(18, lexToken); // If no type is found, throws error 18.
  }

  lexAnal(inFile, outFile, lexToken);

  // Checks if the tokens tag is IDENT, if so it will pass the
  // identifier onto newEntry. If it is not a ident it will throw
  // apprpriate report case.
  if (lexToken.tag == IDENT)
    newEntry->ident = lexToken.ident;
  else
    throw Report(1, lexToken);

  // Calls lookup to make sure that the variable has not already been declared
  // as you cannot have 2 identifiers with the same name. If an identifier
  // already exists with that name then it will throw report case 101.
  if (lookup(lexToken, st, dummy))
    throw Report(101, lexToken);

  lexAnal(inFile, outFile, lexToken); // Get the next token.

  // If the lexToken tag is ASSIGN, then the code will initialise newEntry
  // to a new factor and then lex the next token.
  if (lexToken.tag == ASSIGN)
  {
    newEntry->initialise = new Factor;
    lexAnal(inFile, outFile, lexToken);

    // If the lexToken tag is BOOLLIT, then the code will first check if
    // newEntry type is BOOLDATA, if the tag is BOOLLIT and the type is not
    // BOOLDATA, then a syntax error has occured and the correct report case
    // will be thrown. If there is no syntax error, then newEntry tags are
    // set appropriately.
    if (lexToken.tag == BOOLLIT)
    {
      if (newEntry->type != BOOLDATA)
        throw Report(225, lexToken);

      newEntry->initialise->literal = true;
      newEntry->initialise->type = BOOLDATA;
      newEntry->initialise->litBool = lexToken.boolLit;
    }
    // If the lexToken tag is STRINGLIT, then the code will first check if
    // newEntry type is STRINGDATA, if the tag is STRINGLIT and the type is not
    // STRINGDATA, then a syntax error has occured and the correct report case
    // will be thrown. If there is no syntax error, then newEntry tags are
    // set appropriately.
    else if (lexToken.tag == STRINGLIT)
    {
      if (newEntry->type != STRINGDATA)
        throw Report(225, lexToken);

      newEntry->initialise->literal = true;
      newEntry->initialise->type = STRINGDATA;
      newEntry->initialise->litString = lexToken.stringLit;
    }
    // If the lexToken tag is INTLIT, then the code will first check if
    // newEntry type is INTDATA, if the tag is INTLIT and the type is not
    // INTDATA, then a syntax error has occured and the correct report case
    // will be thrown. If there is no syntax error, then newEntry tags are
    // set appropriately.
    else if (lexToken.tag == INTLIT)
    {
      if (newEntry->type != INTDATA)
        throw Report(225, lexToken);

      newEntry->initialise->literal = true;
      newEntry->initialise->type = INTDATA;
      newEntry->initialise->litInt = lexToken.intLit;
    }
    else if (lexToken.tag == FLOATLIT)
    {
      if (newEntry->type != FLOATDATA)
        throw Report(225, lexToken);

      newEntry->initialise->literal = true;
      newEntry->initialise->type = FLOATDATA;
      newEntry->initialise->litFloat = lexToken.floatLit;
    }
    // An else as if it is not of these tags then there is a syntax error so
    // the correct report case is thrown.
    else
    {
      throw Report(2, lexToken);
    }

    lexAnal(inFile, outFile, lexToken); // Get the next lextoken.
  }
  else
  {
    throw Report(4, lexToken);
  }

  st = newEntry; // Adds new entry to the end of the stack.

  if (lexToken.tag != IN)
    throw Report(6, lexToken);
 
} // synDec



//void synDeclarations(ifstream &inFile,             //*In-Out* Input file
//  ofstream &outFile,            //*In-Out Output file
//  SymTab   *&st,                //*In-Out* Symbol table
//  LexToken &lexToken)           //*In-Out* Current token
//{ //synDeclarations gets lexical tokens from lexAnal and attempts to parse
//  //them as a sequence of C-- variable and constant declarations. If the
//  //parse is successful synDeclarations adds the declarations to the
//  //symbol table.
//  //If the parse is unsuccessful synDeclarations throws an exception.
//
//  //If the token is LBRACE then we've got to the end of the declarations.
//  //Until that happens call synDec to handle each declaration.
//  //Lex and discard the separating terminators.
//
//  while (lexToken.tag != LBRACE)
//  {
//    //Until the first LBRACE parse
//    synDec(inFile, outFile, st, lexToken);
//
//    if (lexToken.tag != TERMINATOR)
//    {
//      //if the next token is not a terminator, throw an exception
//      throw Report(3, lexToken);
//    }
//    else
//    {
//      lexAnal(inFile, outFile, lexToken);
//    }
//  }
//} //synDeclarations

//***************************************************************************
//End Of Variable / Constant Declaration Syntax analysis subprograms.
//***************************************************************************



//***************************************************************************
//Expression syntax checking subprograms.
//***************************************************************************
//synExpression must be forward declared as it is mutually recursive with
//synFactor.
void synExpression(ifstream   &inFile,             //*In-Out* Input file
  ofstream   &outFile,            //*In-Out Output file
  SymTab     *st,                 //*In* Symbol table
  Expression *&expr,              //*Out* Expression parsed
  LexToken   &lexToken,           //*In-Out* Current token
  DataType   &type);             //*Out* Expression type



void synFactor(ifstream   &inFile,                 //*In-Out* Input file
  ofstream   &outFile,                //*In-Out Output file
  SymTab *st,                         //*In* Symbol table
  Factor *&fact,                      //*Out* Factor parsed
  LexToken &lexToken)                 //*In-Out* Current token
{ //synFactor gets lexical tokens from lexAnal and attempts to parse them
  //as a C-- factor.
  //If the parse is unsuccessful synFactor throws an exception.
  //If the parse is successful synFactor returns the ast for the
  //Factor via the fact parameter.

  SymTab *dummy = NULL;         // Initialises dummy to null.
  fact = new Factor;           // Sets new Factor for fact.
  fact->type = VOIDDATA;        // Initialise type to VOIDDATA.
  fact->ident = NULL;           // Initialise ident tag.
  fact->bExp = NULL;            // Initialise basic expression
  fact->nFactor = NULL;         // Initialise nFactor

  // Checks if the lexToken tag is BOOLLIT, if so sets literal
  // to true, sets the type to BOOLDATA and then stores the literal
  // in the corrrect tag.
  if (lexToken.tag == BOOLLIT)
  {
    fact->literal = true;
    fact->type = BOOLDATA;
    fact->litBool = lexToken.boolLit;
    lexAnal(inFile, outFile, lexToken);
  }
  // Checks if the lexToken tag is STRINGLIT, if so sets literal
  // to true, sets the type to STRINGDATA and then stores the literal
  // in the corrrect tag.
  else if (lexToken.tag == STRINGLIT)
  {
    fact->literal = true;
    fact->type = STRINGDATA;
    fact->litString = lexToken.stringLit;
    lexAnal(inFile, outFile, lexToken);
  }
  // Checks if the lexToken tag is INTLIT, if so sets literal
  // to true, sets the type to INTDATA and then stores the literal
  // in the corrrect tag.
  else if (lexToken.tag == INTLIT)
  {
    fact->literal = true;
    fact->type = INTDATA;
    fact->litInt = lexToken.intLit;
    lexAnal(inFile, outFile, lexToken);
  }
  else if (lexToken.tag == FLOATLIT)
  {
    fact->literal = true;
    fact->type = FLOATDATA;
    fact->litFloat = lexToken.floatLit;
    lexAnal(inFile, outFile, lexToken);
  }
  // Checks if the lexToken tag is IDENT, then calls lookup to see
  // if it is already decared, if not throws the correct case.
  // If already declared then sets ident, literal and type.
  else if (lexToken.tag == IDENT)
  {
    if (!lookup(lexToken, st, dummy))
      throw Report(102, lexToken);

    fact->ident = dummy;
    fact->literal = false;
    fact->type = fact->ident->type;
    lexAnal(inFile, outFile, lexToken);
  }
  // Checks if the tag is LPAREN, if so sets literal to false and
  // calls synExpression. If after the expression a RPAREN is not found
  // then throws the correct report case.
  else if (lexToken.tag == LPAREN)
  {
    lexAnal(inFile, outFile, lexToken);
    fact->literal = false;

    synExpression(inFile, outFile, st, fact->bExp, lexToken, fact->type);

    if (lexToken.tag != RPAREN)
      throw Report(17, lexToken);

    lexAnal(inFile, outFile, lexToken);

  }
  // Checks to see if the tag is a NOTOP, if so gets the next token and calls
  // synFactor. If the factor is not BOOLDATA then throws the correct report
  // case. Sets literal to false and type to BOOLDATA.
  else if (lexToken.tag == NOTOP)
  {
    lexAnal(inFile, outFile, lexToken);

    synFactor(inFile, outFile, st, fact->nFactor, lexToken);

    if (fact->nFactor->type != BOOLDATA)
      throw Report(215, lexToken);

    fact->literal = false;
    fact->type = BOOLDATA;
  }
} //synFactor




void synTerm(ifstream &inFile,                     //*In-Out* Input file
  ofstream &outFile,                    //*In-Out Output file
  SymTab   *st,                         //*In* Symbol table
  Term     *&term,                      //*Out* Term parsed
  LexToken &lexToken,                   //*In-Out* Current token
  DataType &type)                       //*Out* Term type
{ //synTerm gets lexical tokens from lexAnal and attempts to parse them
  //as a C-- Term.
  //If the parse is unsuccessful synTerm exits with an error code.
  //If the parse is successful synTerm returns the ast for the
  //term via the term parameter and the type of the term
  //via the type parameter.

  DataType type1 = VOIDDATA;    // Type1 declaration and initialised
  DataType type2 = VOIDDATA;    // Type2 declaration and initialised

  term = new Term;              // Creates new Term for term
  term->fact = NULL;            // Initialise fact
  term->term = NULL;            // Initialise term

  type = VOIDDATA;              // Initialise type to VOIDDATA

  // Calls synFactor to get type
  synFactor(inFile, outFile, st, term->fact, lexToken);

  type1 = term->fact->type; // Sets type1 to term type

  // Checks if the tag is MULOP, if so, sets mulOp and gets the next
  // term for type2
  if (lexToken.tag == MULOP)
  {
    term->mulOp = lexToken.mulOp;

    lexAnal(inFile, outFile, lexToken);

    synTerm(inFile, outFile, st, term->term, lexToken, type2);

    // Compares the types to make sure there is not a mismatch.
    if (type1 != type2)
      throw Report(210, lexToken);

    // Checks to see if a mulOp is being used on a non-int.
    // If found, then throws the appropriate case.
    if (type1 != INTDATA)
    {
      if (term->mulOp == "*")
        throw Report(211, lexToken);

      if (term->mulOp == "/")
        throw Report(212, lexToken);

      if (term->mulOp == "%")
        throw Report(213, lexToken);
    }

    // Checks to see if && is being used on something other than
    // a bool. If so, throws the appropriate case.
    if (type1 != BOOLDATA)
    {
      if (term->mulOp == "&&")
        throw Report(214, lexToken);
    }
  }

  type = type1; // Sets type to type1
} //synTerm




void synBasicExp(ifstream &inFile,                 //*In-Out* Input file
  ofstream &outFile,                //*In-Out Output file
  SymTab   *st,                     //*In* Symbol table
  BasicExp *&bexp,                  //*Out* BExp parsed
  LexToken &lexToken,               //*In-Out* Current token
  DataType &type)                   //*Out* Term type
{ //synBasicExp gets lexical tokens from lexAnal and attempts to parse them
  //as a C-- Basic Expression.
  //If the parse is unsuccessful synBasicExp exits with an error code.
  //If the parse is successful synBasicExp returns the ast for the
  //Basic Expression via the term parameter and the type of the Basic
  //Expression via the type parameter.

  DataType type1 = VOIDDATA;        // Type1 declaration and initialised
  DataType type2 = VOIDDATA;        // Type2 declaration and initialised

  bexp = new BasicExp;              // Creates a new BasicExp for bexp.

  bexp->term = NULL;                // Initialise term to null
  bexp->bexp = NULL;                // Initialise bexp to null

  type = VOIDDATA;                  // Initialise type to VOIDDATA

  // Calls synTerm to get the term of expression for type1
  synTerm(inFile, outFile, st, bexp->term, lexToken, type1);

  // Checks to see if the tag is ADDOP, if so stores the addop and
  // calls synBasicExp and sets it to type2
  if (lexToken.tag == ADDOP)
  {
    bexp->addOp = lexToken.addOp;

    lexAnal(inFile, outFile, lexToken);
    synBasicExp(inFile, outFile, st, bexp->bexp, lexToken, type2);

    // Makes sure there is no type mismatch, if there is, throws
    // the correct case.
    if (type1 != type2)
      throw Report(206, lexToken);

    // Checks to see if an addop is being used on a non-int,
    // if so throws case
    if (type1 != INTDATA)
    {
      if (bexp->addOp == "+")
      {
        throw Report(207, lexToken);
      }
      if (bexp->addOp == "-")
      {
        throw Report(208, lexToken);
      }
    }
    // Check to see if or operator is being used on anything
    // non-nool
    if (type1 != BOOLDATA)
    {
      if (bexp->addOp == "||")
      {
        throw Report(209, lexToken);
      }
    }
  }

  type = type1; // Sets type to type1

} //synBasicExp




void synExpression(ifstream   &inFile,             // *In-Out* Input file
  ofstream   &outFile,            // *In-Out Output file
  SymTab     *st,                 // *In* Symbol table
  Expression *&expr,              // *Out* Expression parsed
  LexToken   &lexToken,           // *In-Out* Current token
  DataType   &type)               // *Out* Expression type
{ // synExpression gets lexical tokens from lexAnal and attempts to parse them
  // as a C-- expression.
  // If the parse is unsuccessful synExpression exits with an error code.
  // If the parse is successful synExpression returns the ast for the
  // expression via the expr parameter and returns the type of the expression
  // via the type parameter.
  // Note that if the expression consists of a single BasicExp then the
  // type parameter must be set to the type of the BasicExp. However if
  // there are two BasicExps separated by a relational operator then the
  // type parameter must be set to BOOLDATA.

  DataType type1 = VOIDDATA;            // Type1 declaration and initialised
  DataType type2 = VOIDDATA;            // Type2 declaration and initialised

  expr = new Expression;                // Creates new Expression for expr
  expr->be1 = NULL;                     // Initialise be1 to null
  expr->be2 = NULL;                     // Initialise be2 to null
  type = VOIDDATA;                      // Set type to VOIDDATA

  // Calls synBasicExp, sets result to be1 and type1
  synBasicExp(inFile, outFile, st, expr->be1, lexToken, type1);

  // Checks tag for RELOP, if found stores the relop, and then
  // calls synBasicExp for rest of parse
  if (lexToken.tag == RELOP)
  {
    expr->relOp = lexToken.relOp;

    lexAnal(inFile, outFile, lexToken);
    synBasicExp(inFile, outFile, st, expr->be2, lexToken, type2);

    // Checks for type mismatch if found throws report
    if (type1 != type2)
      throw Report(204, lexToken);

    // Checks if relOp is attempted to be used on STRINGDATA, if so
    // throws the appropriate report.
    if (type1 == STRINGDATA)
    {
      if (expr->relOp == "==")
      {
        throw Report(218, lexToken);
      }
      else if (expr->relOp == "!=")
      {
        throw Report(219, lexToken);
      }
      else if (expr->relOp == ">")
      {
        throw Report(220, lexToken);
      }
      else if (expr->relOp == "<")
      {
        throw Report(221, lexToken);
      }
      else if (expr->relOp == ">=")
      {
        throw Report(222, lexToken);
      }
      else if (expr->relOp == "<=")
      {
        throw Report(223, lexToken);
      }
    }

    type1 = BOOLDATA; // Sets type1 to BOOLDATA
  }

  type = type1; // Sets type to type1
} // synExpression


//***************************************************************************
//End Of Expression Syntax analysis subprograms.
//***************************************************************************



//////***************************************************************************
//////Statement syntax checking subprograms.
//////***************************************************************************
////
//////synStatements must be forward declared as it is mutually recursive with
//////synIfSt and synWhileSt.
////void synStatements(ifstream  &inFile,              //*In-Out* Input file
////  ofstream  &outFile,             //*In-Out Output file
////  SymTab    *st,                  //*In* Symbol table
////  AST       *&ast,                //*In-Out* AST
////  int       &label,               //*In-Out* Label number
////  LexToken  lexToken);           //*In-Out* Current token
////
////
////void synIfSt(ifstream &inFile,                     //*In-Out* Input file
////  ofstream &outFile,                    //*In-Out Output file
////  SymTab   *st,                         //*In* Symbol table
////  AST      *&ast,                       //*In-Out* AST
////  int      &label,                      //*In-Out* Label number
////  LexToken &lexToken)                   //*In-Out* Current token
////{ //synIfSt gets lexical tokens from lexAnal and attempts to parse them
////  //as a C-- if statement.
////  //If the parse is unsuccessful synIfSt exits with an error code.
////  //If the parse is successful synIfSt returns the ast for the statement.
////
////  DataType   type = VOIDDATA;    // Declare and initalise type
////  Expression *expr = NULL;        // Initialise expr to null
////  AST        *thenpart = NULL;        // Initialise thenpart to null
////  AST        *elsepart = NULL;        // Initialise elsepart to null
////
////
////  ast->tag = IFST;                    // Sets tag to ifst
////  ast->ifst = new IfSt;               // Sets ifst to new IfSt
////  ast->ifst->elselabel = label++;     // Not entirely sure what these two are
////  ast->ifst->endlabel = label++;     // doing but they are on Daves slides
////
////  lexAnal(inFile, outFile, lexToken); // Gets the next token
////
////  // If the next token is not a LPAREN, throw syntax error.
////  if (lexToken.tag != LPAREN)
////    throw Report(6, lexToken);
////
////  lexAnal(inFile, outFile, lexToken); // Get next token
////
////  // Parse expression of ifSt
////  synExpression(inFile, outFile, st, expr, lexToken, type);
////
////  // Makes sure the expression is of BOOL type, if not throws
////  // report
////  if (type != BOOLDATA)
////    throw Report(202, lexToken);
////
////  // Makes suure after the expression is parsed there is a RPAREN.
////  // If not, throw report
////  if (lexToken.tag != RPAREN)
////    throw Report(7, lexToken);
////
////  lexAnal(inFile, outFile, lexToken); // Get next token
////
////  // Parse statements in the If statement
////  synStatements(inFile, outFile, st, thenpart, label, lexToken);
////
////  lexAnal(inFile, outFile, lexToken); // Get next token
////
////  // Checks for else part, if so parses those statements too
////  if (lexToken.tag == ELSE)
////  {
////    lexAnal(inFile, outFile, lexToken);
////
////    synStatements(inFile, outFile, st, elsepart, label, lexToken);
////    lexAnal(inFile, outFile, lexToken);
////  }
////
////  ast->ifst->condition = expr;             // Sets condition to the expression
////  ast->ifst->thenstats = thenpart;         // Sets thenpart to then statements
////  ast->ifst->elsestats = elsepart;         // Sets elsepart to else statements
////} //synIfSt
////
////
////
////void synWhileSt(ifstream &inFile,                  //*In-Out* Input file
////  ofstream &outFile,                 //*In-Out Output file
////  SymTab   *st,                      //*In* Symbol table
////  AST      *&ast,                    //*In-Out* AST
////  int      &label,                   //*In-Out* Label number
////  LexToken &lexToken)                //*In-Out* Current token
////{ //synWhileSt gets lexical tokens from lexAnal and attempts to parse them
////  //as a C-- while statement.
////  //If the parse is unsuccessful synWhileSt exits with an error code.
////  //If the parse is successful synWhileSt returns the ast for the statement.
////
////  DataType type = VOIDDATA;             // Type declaration and initialised
////  Expression *expr = NULL;              // Declares expr and initialised
////
////  ast->tag = WHILEST;                   // Sets tag to whilest
////  ast->whilest = new WhileSt;           // Creates a new whilest
////  ast->whilest->condition = NULL;       // Initialises condition
////  ast->whilest->stats = NULL;           // Initialises stats
////  ast->whilest->startlabel = label++;   // No idea what these two do, but they
////  ast->whilest->endlabel = label++;     // make it work
////
////  lexAnal(inFile, outFile, lexToken);   // Get next token
////
////  // Make sure next token is LPAREN otherwise throw syntax error
////  if (lexToken.tag != LPAREN)
////    throw Report(15, lexToken);
////
////  // Get next token then parse expression
////  lexAnal(inFile, outFile, lexToken);
////  synExpression(inFile, outFile, st, expr, lexToken, type);
////
////  // If expression is not booldata, throw syntax error
////  if (type != BOOLDATA)
////    throw Report(203, lexToken);
////
////  ast->whilest->condition = expr; // Sets expr to condition
////
////  // Checks for RPAREN after expression, if not throw report
////  if (lexToken.tag != RPAREN)
////    throw Report(16, lexToken);
////
////  // Get next token, parse statements, then get next token.
////  lexAnal(inFile, outFile, lexToken);
////  synStatements(inFile, outFile, st, ast->whilest->stats, label, lexToken);
////  lexAnal(inFile, outFile, lexToken);
////} //synWhileSt
////
////
////
////void synCinSt(ifstream &inFile,                    //*In-Out* Input file
////  ofstream &outFile,                   //*In-Out Output file
////  SymTab   *st,                        //*In* Symbol table
////  AST      *&ast,                      //*In-Out* AST
////  int      &label,                     //*In-Out* Label number
////  LexToken &lexToken)                  //*In-Out* Current token
////{ //synCinSt gets lexical tokens from lexAnal and attempts to parse them
////  //as a C-- cin statement.
////  //If the parse is unsuccessful synCinSt exits with an error code.
////  //If the parse is successful synCinSt returns the ast for the statement.
////
////  ast->tag = CINST;                         // Sets tag to cinst
////  ast->cinst = new CinSt;                   // Creates new Cinst
////
////  lexAnal(inFile, outFile, lexToken);       // Get next token
////
////  // If next token is not inop, throw report
////  if (lexToken.tag != INOP)
////    throw Report(9, lexToken);
////
////
////  lexAnal(inFile, outFile, lexToken); // Get next token
////
////  // If next token is not an IDENT, throw report
////  if (lexToken.tag != IDENT)
////    throw Report(11, lexToken);
////
////  // If IDENT is not delcared, throw report
////  if (!lookup(lexToken, st, ast->cinst->invar))
////    throw Report(102, lexToken);
////
////  // If input is something other than int, throw report
////  if (ast->cinst->invar->type != INTDATA)
////    throw Report(216, lexToken);
////
////  // If trying to input into a const variable, throw report
////  if (ast->cinst->invar->constFlag)
////    throw Report(105, lexToken);
////
////  lexAnal(inFile, outFile, lexToken); // Get next token
////
////} //synCinSt
////
////
////
////void synCoutSt(ifstream &inFile,                   //*In-Out* Input file
////  ofstream &outFile,                  //*In-Out Output file
////  SymTab   *st,                       //*In* Symbol table
////  AST      *&ast,                     //*In-Out* AST
////  int      &label,                    //*In-Out* Label number
////  LexToken &lexToken)                 //*In-Out* Current token
////{ //synCoutSt gets lexical tokens from lexAnal and attempts to parse them
////  //as a C-- cout statement.
////  //If the parse is unsuccessful synCoutSt exits with an error code.
////  //If the parse is successful synCoutSt returns the ast for the statement.
////
////  ast->tag = COUTST;                      // Set tag to coutst
////  ast->coutst = new CoutSt;               // Create new CoutSt
////
////  lexAnal(inFile, outFile, lexToken);     // Get next token
////
////  // If next token is not OUTOP, throw report
////  if (lexToken.tag != OUTOP)
////    throw Report(10, lexToken);
////
////  lexAnal(inFile, outFile, lexToken); // Get next token
////
////  // If next token is not IDENT, throw report
////  if (lexToken.tag != IDENT)
////    throw Report(12, lexToken);
////
////  // If variable not delcared, throw report
////  if (!lookup(lexToken, st, ast->coutst->outvar))
////    throw Report(102, lexToken);
////
////  // If trying to output something without string or int type
////  // throw report
////  if ((ast->coutst->outvar->type != INTDATA) &&
////    (ast->coutst->outvar->type != STRINGDATA))
////    throw Report(217, lexToken);
////
////  lexAnal(inFile, outFile, lexToken); // Get next token
////} //synCoutSt
////
////
////
////void synReturnSt(ifstream &inFile,                 //*In-Out* Input file
////  ofstream &outFile,                //*In-Out Output file
////  SymTab   *st,                     //*In* Symbol table
////  AST      *&ast,                   //*In-Out* AST
////  int      &label,                  //*In-Out* Label number
////  LexToken &lexToken)               //*In-Out* Current token
////{ //synReturnSt gets lexical tokens from lexAnal and attempts to parse them
////  //as a C-- return statement.
////  //If the parse is unsuccessful synReturnSt exits with an error code.
////  //If the parse is successful synReturnSt returns the ast for the statement.
////
////
////  ast->tag = RETURNST;                        // Set tag to RETURNST
////  ast->returnst = new ReturnSt;               // Create new RETURNST
////
////
////  lexAnal(inFile, outFile, lexToken);         // Get next token
////
////  // If not returning IDENT, throw report
////  if (lexToken.tag != IDENT)
////    throw Report(19, lexToken);
////
////  // If variable not delcared, throw report
////  if (!lookup(lexToken, st, ast->returnst->returnvar))
////    throw Report(102, lexToken);
////
////  // If return type not int, throw report
////  if (ast->returnst->returnvar->type != INTDATA)
////    throw Report(224, lexToken);
////
////  lexAnal(inFile, outFile, lexToken); // Get next token
////} //synReturnSt
////
////
////
////void synAssignSt(ifstream &inFile,                 //*In-Out* Input file
////  ofstream &outFile,                //*In-Out Output file
////  SymTab   *st,                     //*In* Symbol table
////  AST      *&ast,                   //*In-Out* AST
////  int      &label,                  //*In-Out* Label number
////  LexToken &lexToken)               //*In-Out* Current token
////{ //synAssignSt gets lexical tokens from lexAnal and attempts to parse them
////  //as a C-- assignment statement.
////  //If the parse is unsuccessful synAssignSt exits with an error code.
////  //If the parse is successful synAssignSt returns the ast for the statement.
////
////  DataType type = VOIDDATA;                    // Declare and initialise type
////
////  ast->tag = ASSIGNST;                         // Set tag to ASSIGNST
////  ast->assignst = new AssignSt;                // Create new ASSIGNST
////  ast->assignst->target = NULL;                // Initialise target
////  ast->assignst->expr = NULL;                  // Initialise expr
////
////  // If ident undeclared, throw report
////  if (!lookup(lexToken, st, (ast->assignst->target)))
////    throw Report(102, lexToken);
////
////  // If trying to assign to a const, throw report
////  if (ast->assignst->target->constFlag)
////    throw Report(106, lexToken);
////
////  lexAnal(inFile, outFile, lexToken); // Get next token
////
////  // If tag not ASSIGN, throw report
////  if (lexToken.tag != ASSIGN)
////    throw Report(14, lexToken);
////
////  lexAnal(inFile, outFile, lexToken); // Get next token
////
////  // Parse expression for assignment
////  synExpression(inFile, outFile, st, (ast->assignst->expr), lexToken, type);
////
////  // Checks to make sure assignment type is same as ident type, if not
////  // throw report
////  if ((ast->assignst->target->type) != type)
////  {
////    throw Report(201, lexToken);
////  }
////
////} //synAssignSt
////
////
////
////void synStatement(ifstream &inFile,                //*In-Out* Input file
////  ofstream &outFile,               //*In-Out Output file
////  SymTab   *st,                    //*In* Symbol table
////  AST      *&ast,                  //*In-Out* AST
////  int      &label,                 //*In-Out* Label number
////  LexToken &lexToken)              //*In-Out* Current token
////{ //synStatement gets lexical tokens from lexAnal and attempts to parse them
////  //as a C-- statement.
////  //If the parse is unsuccessful synStatement exits with an error code.
////  //If the parse is successful synStatement returns the ast for the statement.
////  //Use lexToken to decide what type of statement we've got
////  //and call the relevant subprogram to handle it.
////
////  if (lexToken.tag == IF)
////    synIfSt(inFile, outFile, st, ast, label, lexToken);
////  else if (lexToken.tag == WHILE)
////    synWhileSt(inFile, outFile, st, ast, label, lexToken);
////  else if (lexToken.tag == CIN)
////    synCinSt(inFile, outFile, st, ast, label, lexToken);
////  else if (lexToken.tag == COUT)
////    synCoutSt(inFile, outFile, st, ast, label, lexToken);
////  else if (lexToken.tag == RETURN)
////    synReturnSt(inFile, outFile, st, ast, label, lexToken);
////  else if (lexToken.tag == IDENT)
////    synAssignSt(inFile, outFile, st, ast, label, lexToken);
////  else
////    throw Report(20, lexToken);  //Throws error 20
////} //synStatement
////
////
////
////void synStatements(ifstream &inFile,               //*In-Out* Input file
////  ofstream &outFile,              //*In-Out Output file
////  SymTab   *st,                   //*In* Symbol table
////  AST      *&ast,                 //*In-Out* AST
////  int      &label,                //*In-Out* Label number
////  LexToken lexToken)              //*In* Current token
////{ //synStatements gets lexical tokens from lexAnal and attempts to parse them
////  //as a set of C-- statements. synStatements terminates when an RBRACE token
////  //is encountered (at either the end of the program or the end of a
////  //compound statement.
////  //If the parse is unsuccessful synStatements exits with an error code.
////  //If the parse is successful synStatements returns the ast for the
////  //statements. The ast will be in reverse order of statements so the
////  //last thing synStatements does it to reverse it.
////  //Note that synStatements does not do lookahead (i.e. on termination
////  //lexToken does *not* contain the next token. This prevents reading past
////  //the end of file.
////
////  AST *newast = NULL; //Next statement
////  AST *temp1 = NULL; //For reversing
////  AST *temp2 = NULL; //the statement
////  AST *temp3 = NULL; //list
////
////  // Makes sure the first token in the statement is LBRACE,
////  // if not, throw report
////  if (lexToken.tag != LBRACE)
////    throw Report(4, lexToken);
////
////  lexAnal(inFile, outFile, lexToken); // Get next token
////
////  // While the tag is not a RBRACE keeping parsing the statements
////  while (lexToken.tag != RBRACE)
////  {
////    newast = new AST; // Create new AST
////    // Parse next statement
////    synStatement(inFile, outFile, st, newast, label, lexToken);
////
////    // If no terminator found, throw report
////    if (lexToken.tag != TERMINATOR)
////      throw Report(8, lexToken);
////
////    newast->next = ast; // Push onto
////    ast = newast;       // stack
////
////    lexAnal(inFile, outFile, lexToken); // Get next token
////  }
////
////  temp1 = ast; // Set temp1 to ast;
////
////  // Reverse the order of the list so statements are in correct order
////  while (temp1 != NULL)
////  {
////    temp3 = temp1->next;
////    temp1->next = temp2;
////    temp2 = temp1;
////    temp1 = temp3;
////  }
////
////  ast = temp2; // Set ast to temp2
////} //synStatements
////
//////***************************************************************************
//////End Of Statement syntax checking subprograms.
//////***************************************************************************



//***************************************************************************
//Syntax analysis subprogram.
//***************************************************************************

void synAnal(ifstream &inFile,                     //*In-Out* Input file
  ofstream &outFile,                    //*In-Out Output file
  SymTab   *&st,                        //*Out* Symbol table
  AST      *&ast,                       //*Out* Abs syntax tree
  int      &label)                      //*In-Out* Label number
{ //Syntax analysis for C--. Calls skipWhiteComments to set things up
  //for the lexer, call lexAnal to set lookahead correctly, sets
  //the SymTab and AST to NULL, sets the label number to 0, syntax analyses
  //the declarations and statements.
  //Returns the SymTab and AST which results if the syntax analysis is
  //successful and terminates with an error message otherwise.

  LexToken lexToken;                             //Current token
  DataType type = VOIDDATA;
  AST* stCheck;                             // Declare statement check
  bool isStValid = false;                   // Declare intialise stValid

  //Call skipWhiteComments to set things up for the lexer ; call lexAnal
  //to set lookahead up for synDeclarations.
  skipWhiteComments(inFile);
  lexAnal(inFile, outFile, lexToken);

  //Set SynTab and AST to NULL.
  st = NULL;
  ast = NULL;

  try //try-catch block for trapping syntax, static semantic and
    //type errors.
  {
    while (lexToken.tag == LET)
    //Parse the declarations.
    synDec(inFile, outFile, st, lexToken);

    lexAnal(inFile, outFile, lexToken);

    // Parse the statements.
    synExpression(inFile, outFile, st, ast->expr, lexToken, type);
    if (lexToken.tag != END)  // if lexToken.tag is not END
    { // Throws error 8 "Expected end after expression." with lexToken
      throw Report(8, lexToken);
    }  // End of while

    if (inFile)  // If lexToken.tag = END was not the last statement
    { // Throws error 9 "Unexpected Token after end." with lexToken
      throw Report(9, lexToken);
    }
  }
  //Catch syntax, static semantic and type errors.
  //r contains the lexical token at which the error was discovered
  //and the number of the error. This handler prints an error message to
  //outFile and uses writeToken to print the lexical token to outFile.
  //For a list of the error numbers and corresponding error message RTFC.
  //Error numbers from 1 to 99 are for syntax errors; error numbers from
  //101 to 199 are static semantic errors; error number from 201 to 299
  //are for type errors.
  catch (Report r)
  {
    // If syntax error, output error type and error number
    if ((r.getNumber() > minSyntaxError) &&
      (r.getNumber() < maxSyntaxError))
    {
      outFile << "Syntax error " << r.getNumber();
      outFile << ".\n";
      outFile << syntax[r.getNumber() - minSyntaxError];
    }
    // If semantic error, output error type and error number
    else if ((r.getNumber() > minStaticError) &&
      (r.getNumber() < maxStaticError + minStaticError))
    {
      outFile << "Static semantic error " << r.getNumber();
      outFile << ".\n";
      outFile << statics[r.getNumber() - minStaticError];
    }
    // If type error, output error type and error number
    else if ((r.getNumber() > minTypeError) &&
      (r.getNumber() < maxTypeError + minTypeError))
    {
      outFile << "Type error " << r.getNumber();
      outFile << ".\n";
      outFile << type[r.getNumber() - minTypeError];
    }
    // output unknown error type
    else
      outFile << "Unknown parse error.\n";

    outFile << "Found : ";
    writeToken(outFile, r.getLexToken());
    outFile << endl;
  } // catch report
} //synAnal

//***************************************************************************
//End Of syntax analysis subprogram.
//***************************************************************************


