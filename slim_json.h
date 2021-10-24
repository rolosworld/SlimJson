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

typedef struct {
  char* string;
  size_t length;
  // TODO:
  // Stream Position
  // size_t pos;
  // Error enum
  // JsonError error;
} EncodedJson;

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
JsonString* json_parse_string(EncodedJson* _enc);
JsonNumber* json_parse_number(EncodedJson* _enc);
JsonBool* json_parse_bool(EncodedJson* _enc);
JsonNull* json_parse_null(EncodedJson* _enc);
JsonObject* json_parse_object(EncodedJson* _enc);
JsonArray* json_parse_array(EncodedJson* _enc);
Json* json_parse_value(EncodedJson* _enc);
JsonObjectAttribute* json_parse_objectAttribute(EncodedJson* _enc);
JsonArrayItem* json_parse_arrayItem(EncodedJson* _enc);

// Add
void json_add_objectAttribute(JsonObject* _obj, JsonObjectAttribute* _attr);
void json_add_arrayItem(JsonArray* _arr, JsonArrayItem* _item);

// Decode
Json* json_decode(EncodedJson* _enc);

// String
size_t json_string_indexOf(char _c, EncodedJson* _enc, unsigned char _skip_escaped);
void json_string_ltrim(EncodedJson* _enc);

// Number
char json_is_digit(char c);

#endif /* __SLIM_JSON_H__ */
