#ifndef __SLIM_JSON_H__
#define  __SLIM_JSON_H__

#define JSON_OBJECT '{'
#define JSON_ARRAY  '['
#define JSON_STRING '"'
#define JSON_NUMBER 2
#define JSON_BOOL   1
#define JSON_NULL   0

#define JSON_OBJECT_END '}'
#define JSON_ARRAY_END  ']'

typedef enum {
  JSON_ERROR
} JsonErrorType;

typedef struct {
  const char* current;
  const char* start;
  size_t position;
  size_t length;
  JsonErrorType error;
} JsonStream;

typedef struct {
  char type;
  void* data;
  size_t size;
} Json;


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
  Json* data;
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
  Json* data;
} JsonObjectAttribute;

typedef struct {
  JsonObjectAttribute* first;
  JsonObjectAttribute* last;
} JsonObject;

// Free
void json_free(Json* _data);
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
Json* json_parse_value(JsonStream* _enc);
JsonObjectAttribute* json_parse_objectAttribute(JsonStream* _enc);
JsonArrayItem* json_parse_arrayItem(JsonStream* _enc);

// Add
void json_add_objectAttribute(JsonObject* _obj, JsonObjectAttribute* _attr);
void json_add_arrayItem(JsonArray* _arr, JsonArrayItem* _item);

// Decode
Json* json_decode(JsonStream* _enc);

// String
size_t json_string_indexOf(char _c, JsonStream* _enc, unsigned char _skip_escaped);
void json_string_ltrim(JsonStream* _enc);

// Number
char json_is_digit(char c);

#endif /* __SLIM_JSON_H__ */
