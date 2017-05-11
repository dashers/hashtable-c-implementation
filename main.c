//
// This program reads a series of filenames from the command line.
// It opens and reads each file to count how many times each word
// appears in the collection of files.
//
// A word starts with a letter (either uppercase or lowercase) and
// continues until a non-letter (or EOF) is encountered. The program
// ignores non-words, words shorter than six characters in length,
// and words longer than fifty characters in length. All letters
// are converted to lowercase.
//
// After reading all the files, the program then prints the twenty
// words with the highest counts.
//
// Debajyoti Dash

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// support for a symbol table for storing words and their counts
#include "symtab.h"

#define MAX_WORD 50
#define MIN_WORD 6

#define TOP_COUNTS 20

// to get debugging output set this to 1
#define DEBUG 0

// get the next word in the current input file
//   returns NULL if at EOF
static char *getWord(FILE *fp)
{
  char buf[MAX_WORD+1];
  int c;
  int i = 0;

  //read until a letter or EOF is seen
  c = getc(fp);
  while (!isalpha(c))
  {
    if (c == EOF)
    {
      return NULL;
    }
    c = getc(fp);
  }

  // now read until a non-letter or EOF is seen
  while (isalpha(c))
  {
    // if word too long, stop storing letters
    if (i < MAX_WORD)
    {
      buf[i] = tolower(c);
    }
    i += 1;
    c = getc(fp);
  }

  // if word too short or too long, can discard it and try to read another
  if ((i < MIN_WORD) || (i > MAX_WORD))
  {
    // just recurse
    return getWord(fp);
  }
  // otherwise make a safe copy of word
  else {
    buf[i] = 0;
    char *p = malloc(strlen(buf)+1);
    strcpy(p, buf);
#if DEBUG
    printf("found word [%s]\n", p);
#endif
    return p;
  }
}

// read and process all the words in one file
static void processFile(char *filename, void *symtab)
{
  FILE *fp = fopen(filename, "r");
  if (fp == NULL)
  {
    fprintf(stderr, "could not open %s: ignored.\n", filename);
    return;
  }

  char *word = getWord(fp);
  while (word != NULL)
  {
    void *data = symtabLookup(symtab, word);
    if (data == NULL) // not yet in table
    {
      // so insert it with a count of 1
      // NOTE: count inserted in table as the void* value itself
      if (!symtabInstall(symtab, word, (void *) (long) 1))
      {
        fprintf(stderr,
          "symtabInstall failed on initial install - aborting!\n");
        exit(-1);
      }
#if DEBUG
      printf("installing [%s] with count 1\n", word);
#endif
    }
    else // already in table so increment the count
    {
      // NOTE: count is stored in the bits of the void* value itself 
      unsigned int oldCount = (unsigned long) data;
      // re-insert it with updated count
      if (!symtabInstall(symtab, word, (void *) (long) oldCount+1))
      {
        fprintf(stderr, "symtabInstall failed on re-install - aborting!\n");
        exit(-1);
      }
#if DEBUG
      printf("re-installing [%s] with count %d\n", word, oldCount+1);
#endif
    }
    // symbol table makes another safe copy of symbol so need to free this one
    free(word);
    word = getWord(fp);
  }
  fclose(fp);
}

int main(int argc, char *argv[])
{
  int i;

  if (argc < 2)
  {
    fprintf(stderr, "no filenames given!\n");
    return -1;
  }

  // create a symbol table to store the words with their counts
  void *symtab = symtabCreate(10000);
  if (symtab == NULL)
  {
    fprintf(stderr, "symtabCreate failed!\n");
    return -1;
  }

  // process all files, one at a time
  for (i = 1; i < argc; i++)
  {
    processFile(argv[i], symtab);
  } 
 
  // now iterate over the table to find the words with the highest counts.

  // initialize TOP_COUNTS number of counters
  unsigned int topCounts[TOP_COUNTS]; 
  for (i = 0; i < TOP_COUNTS; i++) topCounts[i] = 0;

  // symbols associated with top counters
  const char *topSymbols[TOP_COUNTS];
  for (i = 0; i < TOP_COUNTS; i++) topSymbols[i] = NULL;

  // create iterator for the symbol table
  void *iterator = symtabCreateIterator(symtab);

  // now iterate through the symbol table to find words with highest counts
  void *out;
  const char *p = symtabNext(iterator, &out);
  while (p != NULL)
  {
     // sleezy: unsigned integer count is stored in the bits of the void*
    unsigned int count = (unsigned long) out;

#if DEBUG
    printf("iterating: [%s] %d\n", p, count);
#endif

    // Keep the top counts sorted, so need to find the right place to
    // insert new count; topCounts[0] is the highest count seen so far,
    // topCounts[1] is the next highest, etc. Start the search at the
    // bottom of the list since most counts will be below this.
    for (i = TOP_COUNTS-1; i >= 0; i--)
    {
      if (count < topCounts[i]) break;
    }
    i += 1; // new count should go in this slot unless it is TOP_COUNTS
#if DEBUG
    printf("put it in slot %d\n", i);
#endif

    // If i < TOP_COUNTS then current count needs to be placed in slot i,
    // lower slots need to be shuffled down one position, and the lowest
    // count needs to be removed.
    if (i < TOP_COUNTS)
    {
#if DEBUG
    printf("shuffling down\n");
#endif
      int j;
      for (j = TOP_COUNTS-1; j > i; j--)
      {
        topCounts[j] = topCounts[j-1];
        topSymbols[j] = topSymbols[j-1];
      }
      topCounts[i] = count;
      topSymbols[i] = p;
    }
    p = symtabNext(iterator, &out);
  }

  // now print the top counts
  for (i = 0; i < TOP_COUNTS; i++)
  {
    if (topSymbols[i] == NULL) break;
    printf("%s %d\n", topSymbols[i], topCounts[i]);
  }

  // delete the symbol table and the iterator
  //symtabDeleteIterator(iterator);
  //symtabDelete(symtab);
  
  return 0;
}
