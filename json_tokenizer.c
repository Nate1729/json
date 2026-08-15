#include <stdlib.h>
#include <string.h>

#include "json_tokenizer.h"

#define DEFAULT_TOKEN_ARRAY_CAPACITY 1000

const char *JSONTokenStringMap[JSON_TOKEN_COUNT] = {
    "OBJECT_BRACKET_OPEN",
    "OBJECT_BRACKET_CLOSE",
    "ARRAY_BRACKET_OPEN",
    "ARRAY_BRACKET_CLOSE",
    "TYPE_STRING",
    "TYPE_NUMBER",
    "LITERAL_TRUE",
    "LITERAL_FALSE",
    "LITERAL_NULL",
    "COMMA",
    "COLON",
};

struct TokenArray_t {
  Token *tokens;
  unsigned long length;
  unsigned long capacity;
};

/* Return the new offset that points to the next non-whitespace
   character
*/
unsigned long _skip_whitespace(char *buffer, unsigned long current_loc,
                               unsigned long length) {
  unsigned long i;
  for (i = current_loc; i < length; i++) {
    switch (buffer[i]) {
    case '\r':
    case '\n':
    case '\t':
    case ' ':
      break;
    default:
      return i; /* Found a non-whitespace */
    }
  }
  return length;
}

int _is_number_char(char c) {
  if (c >= '0' && c <= '9')
    return 1;

  return 0;
}

unsigned long _find_char_in_string(char *buffer, char c, unsigned long offset,
                                   unsigned long length) {
  unsigned long i;
  for (i = offset; i < length; i++) {
    if (buffer[i] == c)
      return i;
  }
  return 0;
}

unsigned long _find_end_of_number_in_string(char *buffer, unsigned long offset,
                                            unsigned long length) {
  unsigned i;
  for (i = offset; i < length; i++) {
    if (!_is_number_char(buffer[i]))
      return i - 1;
  }

  return 0;
}

int _token_array_append(TokenArray *token_array, enum JSONToken type,
                        unsigned long start, unsigned long end) {
  Token *next_token;
  if (token_array->length == token_array->capacity) {
    fprintf(stderr,
            "Dynamic TokenArray sizing is not available. Exiting Program.\n");
    exit(1);
  }
  next_token = &token_array->tokens[token_array->length];

  next_token->type = type;
  next_token->data[0] = start;
  next_token->data[1] = end;
  token_array->length++;
  return 0;
}

TokenArray *json_tokenize_buffer(Allocator *allocator, char *buffer,
                                 unsigned long length) {
  void *base_pointer;
  TokenArray *token_array;
  base_pointer = (TokenArray *)allocator->memory_create(
      sizeof(struct TokenArray_t) +
      (DEFAULT_TOKEN_ARRAY_CAPACITY + sizeof(Token)));
  if (!base_pointer)
    return NULL;

  token_array = (TokenArray *)base_pointer;
  token_array->length = 0;
  token_array->capacity = DEFAULT_TOKEN_ARRAY_CAPACITY;
  token_array->tokens = (Token*)(base_pointer + sizeof(struct TokenArray_t));

  /* Now we can start the tokenization process */
  unsigned long offset, end;

  offset = _skip_whitespace(buffer, 0, length);
  while (offset < length) {
    if (buffer[offset] == '{') {
      _token_array_append(token_array, OBJECT_BRACKET_OPEN, offset, offset);
      offset++;
    } else if (buffer[offset] == '}') {
      _token_array_append(token_array, OBJECT_BRACKET_CLOSE, offset, offset);
      offset++;
    } else if (buffer[offset] == '[') {
      _token_array_append(token_array, ARRAY_BRACKET_OPEN, offset, offset);
      offset++;
    } else if (buffer[offset] == ']') {
      _token_array_append(token_array, ARRAY_BRACKET_CLOSE, offset, offset);
      offset++;
    } else if (buffer[offset] == ':') {
      _token_array_append(token_array, COLON, offset, offset);
      offset++;
    } else if (buffer[offset] == ',') {
      _token_array_append(token_array, COMMA, offset, offset);
      offset++;
    } else if (!memcmp(buffer + offset, "true", 4)) {
      _token_array_append(token_array, LITERAL_TRUE, offset, offset + 3);
      offset += 4;
    } else if (!memcmp(buffer + offset, "false", 5)) {
      _token_array_append(token_array, LITERAL_FALSE, offset, offset + 4);
      offset += 5;
    } else if (!memcmp(buffer + offset, "null", 4)) {
      _token_array_append(token_array, LITERAL_NULL, offset, offset + 3);
      offset += 4;
    } else if (buffer[offset] == '"') {
      end = _find_char_in_string(buffer, '"', offset + 1, length);
      if (end == 0) {
        fprintf(stderr, "Unterminated string! Exiting early.\n");
        exit(1);
      }
      _token_array_append(token_array, TYPE_STRING, offset + 1, end - 1);
      offset = end + 1;
    } else if (_is_number_char(buffer[offset])) {
      end = _find_end_of_number_in_string(buffer, offset + 1, length);
      if (end == 0) {
        fprintf(stderr, "Unterminated number literal! Exiting early.\n");
        exit(1);
      }
      _token_array_append(token_array, TYPE_NUMBER, offset, end);
      offset = end + 1;
    } else {
      fprintf(stderr, "Unrecognized character %c. At offset %lu\n",
              buffer[offset], offset);
      exit(1);
    }

    offset = _skip_whitespace(buffer, offset, length);
  }

  return token_array;
}

void token_array_debug_print(TokenArray *token_array, char *buffer, FILE *f) {
  unsigned long i;
  for (i = 0; i < token_array->length; i++) {
    fprintf(f, "type: %s, length: %lu\n",
            JSONTokenStringMap[token_array->tokens[i].type],
            token_array->tokens[i].data[1] - token_array->tokens[i].data[0]);
  }
}
