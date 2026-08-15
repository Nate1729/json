#ifndef JSON_TOKENIZER_H
#define JSON_TOKENIZER_H

#include <stdio.h>

typedef struct {
  void *(*memory_create)(unsigned long);
  void (*memory_destroy)(void *);
} Allocator;

enum JSONToken {
  /* Objects */
  OBJECT_BRACKET_OPEN,
  OBJECT_BRACKET_CLOSE,
  /* Arrays */
  ARRAY_BRACKET_OPEN,
  ARRAY_BRACKET_CLOSE,
  /* Types */
  TYPE_STRING,
  TYPE_NUMBER,
  /* Literals */
  LITERAL_TRUE,
  LITERAL_FALSE,
  LITERAL_NULL,
  /* Other stuff */
  COMMA,
  COLON,

  JSON_TOKEN_COUNT,
};

typedef struct {
  enum JSONToken type;
  /* Some types require a start and a stop. */
  unsigned long data[2];
} Token;

typedef struct TokenArray_t TokenArray;

TokenArray *json_tokenizer_buffer(Allocator *allocator, char *buffer,
				  unsigned long length);

void token_array_debug_print(TokenArray *token_array, char *buffer, FILE *f);
#endif /* JSON_TOKENIZER_H */
