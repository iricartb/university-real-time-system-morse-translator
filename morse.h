#define MAX_SIGNALS 20
#define MAX_LETTERS 54

#define FREQUENCY         500
#define SIZE_MESSAGE    10000

static const struct {
   const char letter;
   char * signals;
} morse_table[] = {
{ 'A', ". ..." },               { 'B', "... . . ." },           { 'C', "... . ... ." },         {'D', "... . ." },
{ 'E', "." },                   { 'F', ". . ... ." },           { 'G', "... ... ." },           { 'H', ". . . ." },
{ 'I', ". ."},                  { 'J', ". ... ... ..." },       { 'K', "... . ..." },           { 'L', ". ... . ." },
{ 'M', "... ..."},              { 'N', "... ." },               { 'O', "... ... ..." },         { 'P', ". ... ... ." },
{ 'Q', "... ... . ..." },       { 'R', ". ... ." },             { 'S', ". . ." },               { 'T', "..." },
{ 'U', ". . ..." },             { 'V', ". . . ..."},            { 'W', ". ... ..." },           { 'X', "... . . ..." },
{ 'Y', "... . ... ..." },       { 'Z', "... ... . ."},          { '0', "... ... ... ... ..." }, { '1', ". ... ... ... ..." },
{ '2', ". . ... ... ..." },     { '3', ". . . ... ..." },       { '4', ". . . . ..." },         { '5', ". . . . ." },
{ '6', "... . . . ." },         { '7', "... ... . . ." },       { '8', "... ... ... . ." },     { '9', "... ... ... ... ." },
{ '.', ". ... . ... . ..." },   { ',', "... ... . . ... ..." }, { '?', ". . ... ... . ." },     { 39,  ". ... ... ... ... ." },
{ '!', "... . ... . ... ..." }, { '/', "... . . ... ." },       { '(', "... . ... ... ." },     { ')', "... . ... ... . ..." },
{ '&', ". ... . . ." },         { ':', "... ... ... . . ." },   { ';', "... . ... . ... ." },   { '=', "... . . . ..." },
{ '+', ". ... . ... ." },       { '-', "... . . . . ..." },     { '_', ". . ... ... . ..." },    { '"', ". ... . . ... ." },
{ '$', ". . . ... . . ..." },   { '@', ". ... ... . ... ." } 
}; 

typedef struct {
   char buffer[SIZE_MESSAGE];
   long usec;
   unsigned int counter;
} message_t;