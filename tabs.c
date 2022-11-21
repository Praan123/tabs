#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "tabs.h"

// silly hack to make LABEL_LEN work (otherwise it would have to be hardcoded)
// https://stackoverflow.com/questions/70815044/how-to-make-macro-replacement-text-into-a-string-in-c-c
#define STRING(x) #x
#define ASTRING(x) STRING(x)

/* 
   all config lines are of the form
   <label> <symbol> <amount> <note>
   e.x.
   CC l 1 1001
   (for left crash)
   
   SN w 2 1002
   (for double stroke on snare)

   CC r 8 1001
   (roughly a cymbal roll)
*/

// global mutables, initialized in read_config()
struct def *defs;
int ndefs = 0;

// error messages need to know line number
int linenum = 0;

// process definition
// example:
// SN w 2 1001
void proc_def(char *s)
{
  struct def d;

  // read fields from s into d
  char symb[2]; // intermediate storage is needed because %1s has to be used instead of %c, which does not skip whitespace
  char trail[2]; // for detecting trailing non-whitespace characters
  int n = sscanf(s, "%" ASTRING(LABEL_LEN) "s %1s %d %d %1s", d.lab, symb, &d.amt, &d.note, trail);
  d.symb = *symb;

  // check for syntax errors
  if(n == EOF) // whitespace-only line, skip
    return;
  if(n < 4) // not whitespace-only, inadequate input
  {
    fprintf(stderr, "%s: line %d: missing fields\n", __func__, linenum);
    exit(1);
  }
  if(n == 5) // trailing non-whitespace characters
  {
    fprintf(stderr, "%s: line %d: trailing characters\n", __func__, linenum);
    exit(1);
  }

  // append d to defs
  assert(defs);
  if(ndefs == MAX_DEFS)
  {
    fprintf(stderr, "%s: line %d: maximum allowed definitions %d exceeded\n", __func__, linenum, MAX_DEFS);
    exit(1);
  }
  defs[ndefs++] = d;

}

// process command passed by read_tabs()
void proc_command(char *s)
{
  assert(*s == '!'); // command start
  
  if(!strncmp(s+1, "def", 3) && isspace(s[4])) // definition
  {
    proc_def(s+4);
  }
}

// read tabs from stream, create midi output
void read_tabs(FILE *stream)
{
  // initialize defs
  {
    struct def temp[MAX_DEFS];
    defs = temp;
  }
  
  char s[MAX_LINE];
  
  while(fgets(s, MAX_LINE, stream))
  {
    linenum++;
    
    if(s[strlen(s)-1] != '\n') // full line was not read
    {
      fprintf(stderr, "%s: line %d: maximum line length %d exceeded\n", __func__, linenum, MAX_LINE);
      exit(1);
    }
    
    if(*s == '#') // comment, skip
      continue;
    
    if(*s == '!') // command, pass to proc_command()
    {
      proc_command(s);
      continue;
    }
  }

  // uninitialize defs
  defs = 0;
}
