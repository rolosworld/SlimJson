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

#define JSON_ERROR(_err) do { printf("ERROR: %s", _err); } while (0)

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
JsonString* json_parse_string(const char* _encoded, size_t _len);
JsonNumber* json_parse_number(const char* _encoded, size_t _len);
JsonBool* json_parse_bool(const char* _encoded, size_t _len);
JsonNull* json_parse_null(const char* _encoded, size_t _len);
JsonObject* json_parse_object(const char* _encoded, size_t _len);
JsonArray* json_parse_array(const char* _encoded, size_t _len);
Json* json_parse_value(const char* _encoded, size_t _len);
JsonObjectAttribute* json_parse_objectAttribute(const char* _encoded, size_t _len);

// Add
void json_add_objectAttribute(JsonObject* _obj, JsonObjectAttribute* _attr);
void json_add_arrayItem(JsonArray* _arr, JsonArrayItem* _item);

// Decode
Json* json_decode(const char* _encoded, size_t _len);

// String
JsonString* json_string(const char* _str, size_t _len);
size_t json_string_indexOf(char _c, const char* _str, size_t _len, unsigned char skip_escaped);
const char* json_string_ltrim(const char* _encoded, size_t* _len);

// Number
char json_is_digit(char c);

#endif /* __SLIM_JSON_H__ */
