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
#ifndef __SLIM_JSON_H__
#define  __SLIM_JSON_H__

#include <stdio.h>
#include <stdlib.h>

#define JSON_OBJECT '{'
#define JSON_ARRAY  '['
#define JSON_STRING '"'
#define JSON_NUMBER '#'
#define JSON_BOOL   '!'
#define JSON_NULL   '~'
#define JSON_ERROR  '?'

#define JSON_OBJECT_END '}'
#define JSON_ARRAY_END  ']'

typedef struct {
  const char* current;
  const char* start;
  size_t position;
  size_t length;
} JsonStream;

typedef struct {
  char type;
  void* data;
} JsonValue;


// String
typedef struct {
  char* value;
  size_t length;
} JsonString;

// Number
typedef struct {
  double value;
} JsonNumber;

// Bool
typedef struct {
  unsigned char value;
} JsonBool;

// Null
typedef struct {
  unsigned char value;
} JsonNull;

// Array
typedef struct JsonArrayItem {
  struct JsonArrayItem* next;
  JsonValue* data;
} JsonArrayItem;

typedef struct {
  JsonArrayItem* first;
  JsonArrayItem* last;
  size_t length;
} JsonArray;

// Object
typedef struct JsonObjectAttribute {
  struct JsonObjectAttribute* next;
  JsonString* name;
  JsonValue* data;
} JsonObjectAttribute;

typedef struct {
  JsonObjectAttribute* first;
  JsonObjectAttribute* last;
} JsonObject;

// Free
void json_free(JsonValue* _data);
void json_free_objectAttribute(JsonObjectAttribute* _attr);
void json_free_objectAttribute(JsonObjectAttribute* _attr);
void json_free_object(JsonObject* _obj);
void json_free_string(JsonString* _str);
void json_free_arrayItem(JsonArrayItem* _item);
void json_free_array(JsonArray* _arr);

// Parse
JsonString* json_parse_string(JsonStream* _enc);
JsonNumber* json_parse_number(JsonStream* _enc);
JsonBool* json_parse_bool(JsonStream* _enc);
JsonNull* json_parse_null(JsonStream* _enc);
JsonObject* json_parse_object(JsonStream* _enc);
JsonArray* json_parse_array(JsonStream* _enc);
JsonValue* json_parse_value(JsonStream* _enc);
JsonObjectAttribute* json_parse_objectAttribute(JsonStream* _enc);
JsonArrayItem* json_parse_arrayItem(JsonStream* _enc);

// Add
void json_add_objectAttribute(JsonObject* _obj, JsonObjectAttribute* _attr);
void json_add_arrayItem(JsonArray* _arr, JsonArrayItem* _item);

// Decode
JsonValue* json_decode(const char* _json, size_t _len);

// String
ssize_t json_string_indexOf(char _c, JsonStream* _enc, unsigned char _skip_escaped);
void json_string_ltrim(JsonStream* _enc);
char json_equal_strings(const char* _strA, size_t _lenA, const char* _strB, size_t _lenB);

// Number
char json_is_digit(char c);

// Stream
JsonStream* json_stream(const char* _json, size_t _len);

// Error
void json_print_error(JsonValue* _e);

// Get
JsonObjectAttribute* json_get_objectAttribute(JsonObject* _obj, const char* _name, size_t _len); 
JsonArrayItem* json_get_arrayItem(JsonArray* _arr, size_t _index); 

#endif /* __SLIM_JSON_H__ */
