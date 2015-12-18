// Title   : printers.cxx
// Purpose : Print subprograms for SCL AST and symbol table.
//           For CM510 PG3 phase 4.
// Author  : Matthew Jacques
// Date    : 24/11/13

#include "syner.h" // header for syner.cxx
#include <iomanip> // Standard IO manipulators library

// ***************************************************************************
// SymTab Output Subprograms.
// ***************************************************************************

// Forward declarations for every print function
void printST(ofstream &outFile,                    // *In-Out* Output file
             SymTab   *st);                        // *In* Symbol table

void printFactor(ofstream &outFile,                // *In-Out* Output file
                 Factor   *fact);                  // *In* Factor

void printTerm(ofstream &outFile,                  // *In-Out* Output file
               Term     *term);                    // *In* Term

void printBasicExp(ofstream &outFile,              // *In-Out* Output file
                   BasicExp *bexp);                // *In* BasicExp

void printExpression(ofstream   &outFile,          // *In-Out* Output file
                     Expression *expr);            // *In* Expression

void printAST(ofstream &outFile,                   // *In-Out* Output file
              AST      *ast);                      // *In* Abst. syntax tree



void printST(ofstream &outFile,                    // *In-Out* Output file
             SymTab   *st)                         // *In* Symbol table
{ // Prints out the Symbol Table.

  outFile << "\n\nName\tType\tValue\n" ;
  while (st != NULL)
  { // Print the current datatype entry.
    outFile << st->ident ;
    if (st->type == BOOLDATA)
      outFile << "\tbool" ;
    else if (st->type == STRINGDATA)
      outFile << "\tstring" ;
    else if (st->type == INTDATA)
      outFile << "\tint" ;
    else if (st->type == FLOATDATA)
      outFile << "\tfloat" ;

    // Print the current literal of the datatype
    if (st->initialise != NULL)
    { if (st->initialise->type == BOOLDATA)
        outFile << '\t' << st->initialise->litBool << endl ;
      else if (st->initialise->type == STRINGDATA)
        outFile << '\t' << st->initialise->litString << endl ;
      else if (st->initialise->type == INTDATA)
        outFile << '\t' << st->initialise->litInt << endl ;
      else if (st->initialise->type == FLOATDATA)
        outFile << '\t' << st->initialise->litFloat << endl ;
    }
    else
      outFile << "\tNone\n" ;

    // Move to next entry.
    st = st->next ;
  } // while
} // printST


void printFactor(ofstream &outFile,                // *In-Out* Output file
                 Factor   *fact)                   // *In* Factor
{ // Prints out a Factor.

  // Check for a literal and print it if present.
  if (fact->literal)
  { if (fact->type == BOOLDATA)
      outFile << fact->litBool ;
    else if (fact->type == STRINGDATA)
      outFile << fact->litString ;
    else if (fact->type == INTDATA)
      outFile << fact->litInt ;
    else if (fact->type == FLOATDATA)
      outFile << fact->litFloat ;
    else
      outFile << "Error : claimed literal is not a literal.\n" ;
  }

  // Must be either an identifier, a bracketed expression or a negated
  // boolean factor. Check for each and print if present.
  else
  { if (fact->ident != NULL)
      outFile << fact->ident->ident  ;
    else if (fact->bExp != NULL)
      { outFile << '(' ;
        printExpression(outFile, fact->bExp) ;
        outFile << ')' ;
      }
    else if (fact->nFactor != NULL)
      { outFile << "!(" ;
        printFactor(outFile, fact->nFactor) ;
        outFile << ')' ;
      }
    else
      outFile << "Error : empty factor.\n" ;
  }
} // printFactor


void printTerm(ofstream &outFile,                  // *In-Out* Output file
               Term     *term)                     // *In* Term
{ // Prints out a Term.

  // Call printFactor to print the factor.
  printFactor(outFile, term->fact) ;

  // If there is another term print the mulOp and call printTerm to
  // print the rest of the terms.
  if (term->term != NULL)
  { outFile << ' ' << term->mulOp << ' ' ;
    term = term->term ;
    printTerm(outFile, term) ;
  }
} // printTerm


void printBasicExp(ofstream &outFile,              // *In-Out* Output file
                   BasicExp *bexp)                 // *In* BasicExp
{ // Prints out a BasicExp.

  // Call printTerm to print the term.
  printTerm(outFile, bexp->term) ;

  // If there is another basicexp print the addOp and call printBasicExp to
  // print the rest of the basicexps.
  if (bexp->bexp != NULL)
  { outFile << ' ' << bexp->addOp << ' ' ;
    bexp = bexp->bexp ;
    printBasicExp(outFile, bexp) ;
  }
} // printBasicExp



void printExpression(ofstream   &outFile,          // *In-Out* Output file
                     Expression *expr)             // *In* Expression
{ // Prints out an Expression.

  // Call printBasicExp to print the basicexp.
  printBasicExp(outFile, expr->be1) ;

  // If there is another basicexp print the relOp and call printBasicExp to
  // print the rest of the basicexps.
  if (expr->be2 != NULL)
  { outFile << ' ' << expr->relOp << ' ' ;
    printBasicExp(outFile, expr->be2) ;
  }
} // printExpression



void printAST(ofstream &outFile,                   // *In-Out* Output file
              AST      *ast)                       // *In* Abst. syntax tree
{ // Prints the AST
 

  // While loop to print each statement in turn.
  while  (ast != NULL)
  { // If ast-expr not NULL, print Expression
    if (ast->expr != NULL)
      printExpression(outFile, ast->expr);

    // Move on to next statement.
    ast = ast->next ;
  }

} // printAST

//***************************************************************************
// End of SymTab Output Subprograms.
//***************************************************************************
