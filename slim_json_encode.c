/*

  MIT License

  Copyright (c) 2021 Rolando Gonzalez-Chevere

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/
#include "slim_json.h"

/**
 * ENCODE 
 **/
typedef struct JsonStringNode {
  char* value;
  JsonStringNode* next;
} JsonStringNode;

static char* json_string_copy(const char* _str);
static JsonStringNode* json_encode_string(JsonString* _str);
static JsonNumber* json_encode_number(JsonStream* _enc);

static JsonBool* json_encode_bool(JsonStream* _enc);
static JsonNull* json_encode_null(JsonStream* _enc);
static JsonObject* json_encode_object(JsonStream* _enc);
static JsonArray* json_encode_array(JsonStream* _enc);
static JsonValue* json_encode_value(JsonStream* _enc);
static JsonObjectAttribute* json_encode_objectAttribute(JsonStream* _enc);
static JsonArrayItem* json_encode_arrayItem(JsonStream* _enc);

static char* json_string_copy(const char* _str) {
  size_t len = json_string_length(_str);
  char* str = malloc(len + 1);
  while (len) {
    str[len] = _str[len--];
  }
  return str;
}

static size_t json_string_escapableCount(const char* _str, size_t _len) {
  size_t escapables = 0;
  while (_len) {
    switch (_str[_len--]) {
    case '"':
    case '\\':
    case '/':
    case '\b':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\u':
      escapables++;
    }
  }
  return escapables;
}

static char* json_string_escapedCopy(const char* _str) {
  size_t len = json_string_length(_str);
  size_t escapables = json_string_escapableCount(_str, len);

  char* str = malloc(len + 1);
  while (len) {
    str[len] = _str[len--];
  }
  return str;
}

static void json_free_stringNode(JsonStringNode* _node) {
  if (_node == NULL) {
    return;
  }

  JsonStringNode* node = _node;
  while (node) {
    _node = node->next;
    free(node->value);
    free(node);
    node = _node;
  }
}

static JsonStringNode* json_encode_string(JsonString* _str) {
  JsonStringNode* node = malloc(sizeof(JsonStringNode));
  size_t len = _str->length + 2;
  node->value = malloc(len + 1); // ""\0
  node->value[0] = '"';
  node->value[len - 1] = '"';
  json_string_cat(node->value + 1, len - 2, _str->value);
  node->next = NULL;
  return node;
}

static JsonNumber* json_encode_number(JsonStream* _enc) {
}
