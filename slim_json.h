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
#define __SLIM_JSON_H__

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
  char msg[250];
} JsonError;

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
  JsonArrayItem** array; // Array of items, created after the decode is completed
  size_t length;
} JsonArray;

// Object
typedef struct JsonObjectAttribute {
  struct JsonObjectAttribute* next;
  JsonString* name;
  JsonValue* data;
} JsonObjectAttribute;

typedef struct JsonObjectDataNode {
  struct JsonObjectDataNode* next; // If the hash collide, insert here so we can compare the item key's
  JsonObjectAttribute* attribute;
} JsonObjectDataNode;

typedef struct {
  JsonObjectAttribute* first;
  JsonObjectAttribute* last;
  JsonObjectDataNode** object; // Array for the hash
  size_t length;
} JsonObject;

// Decode
JsonValue* json_decode(const char* _json);

// Free
void json_free(JsonValue* _data);

// Error
JsonError json_get_errorMsg(JsonValue* _e);

// Get
const JsonValue* json_get(const JsonValue* _v, const char* _path);

const char* json_get_string(const JsonValue* _v, const char* _path);
double json_get_number(const JsonValue* _v, const char* _path);
char json_get_bool(const JsonValue* _v, const char* _path);
char json_get_null(const JsonValue* _v, const char* _path);

// Object
const JsonObject* json_get_object(const JsonValue* _v, const char* _path);
const char* json_get_object_string(const JsonObject* _obj, const char* _attributeName);
const char* json_get_object_string(const JsonObject* _obj, const char* _attributeName);
double json_get_object_number(const JsonObject* _obj, const char* _attributeName);
char json_get_object_bool(const JsonObject* _obj, const char* _attributeName);
char json_get_object_null(const JsonObject* _obj, const char* _attributeName);
const JsonArray* json_get_object_array(const JsonObject* _obj, const char* _attributeName);
const JsonObject* json_get_object_object(const JsonObject* _obj, const char* _attributeName);

// Array
const JsonArray* json_get_array(const JsonValue* _v, const char* _path);
const char* json_get_array_string(const JsonArray* _arr, size_t _itemIndex);
double json_get_array_number(const JsonArray* _arr, size_t _itemIndex);
char json_get_array_bool(const JsonArray* _arr, size_t _itemIndex);
char json_get_array_null(const JsonArray* _arr, size_t _itemIndex);
const JsonArray* json_get_array_array(const JsonArray* _arr, size_t _itemIndex);
const JsonObject* json_get_array_object(const JsonArray* _arr, size_t itemIndex);

// Encode
char* json_encode(JsonValue* _value);

#endif /* __SLIM_JSON_H__ */
