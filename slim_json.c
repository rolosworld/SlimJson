#include <stdio.h>
#include <stdlib.h>

#define JSON_OBJECT '{'
#define JSON_ARRAY  '['
#define JSON_STRING '"'
#define JSON_NUMBER 2
#define JSON_BOOL   1
#define JSON_NULL   0
#define JSON_OBJECT_END '}'
#define JSON_ARRAY_END  ']'

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
typedef struct {
  struct JsonItem* next;
  Json* data;
} JsonItem;

typedef struct {
  JsonItem* first;
  JsonItem* last;
  size_t length;
} JsonArray;

// Object
typedef struct {
  struct JsonObjectAttribute* next;
  JsonString* name;
  Json* value;
} JsonObjectAttribute;

typedef struct {
  JsonObjectAttribute* first;
  JsonObjectAttribute* last;
} JsonObject;

#define JSON_ERROR(err) do { printf("ERROR: %s", err); } while (0)

// skip_escaped: Skip if the character has a \ before it
static size_t json_string_indexOf(char _c, const char* _str, size_t _len, unsigned char skip_escaped) {
  if (_len == 0) {
    return -1;
  }

  while (--_len) {
    if (_str[_len] == '\0') {
      return -1;
    }
    else if (_str[_len] == _c) {
      if (skip_escaped == 1 && _len > 0 && _str[_len - 1] == '\\') {
	continue;
      }

      return _len;
    }
  }

  return -1;
}

static char test_json_string_indexOf() {
  char t[] = "Hello World!\n\\\"'";
  size_t len = sizeof(t) - 1;
  char t1 = json_string_indexOf(' ', t, len, 0);
  char t2 = json_string_indexOf('!', t, len, 0);
  char t3 = json_string_indexOf('\n', t, len, 0);
  char t4 = json_string_indexOf('"', t, len, 0);
  char t5 = json_string_indexOf('\'', t, len, 1);

  return t[t1] == ' ' &&
    t[t2] == '!' &&
    t[t3] == '\n' &&
    t[t4] == '"' &&
    t[t5] == '\'';
}

static const char* json_string_ltrim(const char* _encoded, size_t* _len) {
  if (*_len == 0) {
    return NULL;
  }

  size_t pos = 0;
  while (*_len > pos) {
    switch(_encoded[pos]) {
    case '\0': return NULL;
    case ' ': case '\n': case '\r': case '\t': pos++; continue;
    }
    break;
  }

  *_len -= pos;
  return _encoded+pos;
}

static char test_json_string_ltrim() {
  char t[] = " \n\t\r Hello World!\n\\\"\"";
  size_t len = sizeof(t) - 1;
  const char* t1 = json_string_ltrim(t, &len);

  return t1[0] == 'H';
}

static JsonString* json_string(const char* _str, size_t _len) {
  if (_len == 0) {
    return NULL;
  }

  JsonString* str = malloc(sizeof(JsonString));
  str->length = _len;
  char* v = malloc(sizeof(char) * _len + 1);
  v[_len] = '\0';
  str->value = v;

  while(_len--) {
    v[_len] = _str[_len];
  }

  return str;
}

static JsonString* json_parse_string(const char* _encoded, size_t _len) {
  _encoded = json_string_ltrim(_encoded, &_len);

  if (_encoded == NULL || _encoded[0] != '"') {
    return NULL;
  }

  size_t end = json_string_indexOf('"', ++_encoded, --_len, 1);
  if (end < 0) {
    return NULL;
  }

  return json_string(_encoded, end);
}

static void json_free_string(JsonString* _str) {
  if (_str->value != NULL) {
    free(_str->value);
  }
  free(_str);
}

static char test_json_parse_string() {
  char t[] = "  \n\t\r \"Foo\\\"Bar\",  ";
  size_t len = sizeof(t) - 1;
  JsonString* T = json_parse_string(t, len);
  char t1 = T->value[0] == 'F';
  char t2 = T->value[T->length-1] == 'r';
  char t3 = T->value[T->length] == '\0';
  json_free_string(T);
  return t1 && t2 && t3;
}

static char json_is_digit(char c) {
  return c >= '0' && c <= '9';
}

static char test_json_is_digit() {
  return json_is_digit('0') &&
    json_is_digit('1') &&
    json_is_digit('2') &&
    json_is_digit('3') &&
    json_is_digit('4') &&
    json_is_digit('5') &&
    json_is_digit('6') &&
    json_is_digit('7') &&
    json_is_digit('8') &&
    json_is_digit('9') &&
    !json_is_digit('a');
}

static JsonNumber* json_parse_number(const char* _encoded, size_t _len) {
  if (_len == 0) {
    return NULL;
  }

  _encoded = json_string_ltrim(_encoded, &_len);

  if (_encoded == NULL) {
    return NULL;
  }

  size_t pos = 0;
  char negative = _encoded[0] == '-' ? 1 : 0;
  if (negative == 1) {
    pos++;
  }

  if (json_is_digit(_encoded[pos]) == 0) {
    return NULL;
  }

  double value = 0;
  char fraction = 0;
  while (_len > pos) {
    if (json_is_digit(_encoded[pos]) == 1) {
      value *= 10;
      value += _encoded[pos] - '0';
      if (fraction) {
	fraction++;
      }
    }
    else if (_encoded[pos] == '.') {
      if (fraction != 0) {
	return NULL;
      }

      fraction++;
    }
    else {
      break;
    }

    pos++;
  }

  if (fraction--) {
    while(fraction--) {
      value /= 10;
    }
  }

  if (negative) {
    value *= -1.0;
  }

  JsonNumber* num = malloc(sizeof(JsonNumber));
  num->value = value;
  return num;
}

static char test_json_parse_number() {
  char n1[] = " -1.023 ";
  JsonNumber* num1 = json_parse_number(n1, sizeof(n1) - 1);
  char v1 = ((float) num1->value) == ((float) -1.023);
  free(num1);

  char n2[] = " 1.023 ";
  JsonNumber* num2 = json_parse_number(n2, sizeof(n2) - 1);
  char v2 = ((float) num2->value) == ((float) 1.023);
  free(num2);

  char n3[] = "1023";
  JsonNumber* num3 = json_parse_number(n3, sizeof(n3) - 1);
  char v3 = ((float) num3->value) == ((float) 1023);
  free(num3);

  return v1 && v2 && v3;
}

// false or true
static JsonBool* json_parse_bool(const char* _encoded, size_t _len) {
  if (_len == 0) {
    return NULL;
  }

  _encoded = json_string_ltrim(_encoded, &_len);

  if (_encoded == NULL) {
    return NULL;
  }

  char type = _encoded[0];
  if (type != 't' && type != 'f') {
    return NULL;
  }

  if (type == 't') {
    if (_len < 4) {
      return NULL;
    }
  }
  else if(_len < 5) {
    return NULL;
  }

  char T[] = "true";
  char F[] = "false";
  char* B = type == 't' ? T : F;
  size_t pos = type == 't' ? 4 : 5;
  while (pos--) {
    if (_encoded[pos] != B[pos]) {
      return NULL;
    }
  }

  JsonBool* bol = malloc(sizeof(JsonBool));
  bol->value = type == 't' ? 1 : 0;
  return bol;
}

static char test_json_parse_bool() {
  char b1[] = " true ";
  JsonBool* B1 = json_parse_bool(b1, sizeof(b1) - 1);
  char v1 = B1->value == 1;

  char b2[] = " false ";
  JsonBool* B2 = json_parse_bool(b2, sizeof(b2) - 1);
  char v2 = B2->value == 0;

  return v1 && v2;
}

// null
static JsonNull* json_parse_null(const char* _encoded, size_t _len) {
  if (_len == 0) {
    return NULL;
  }

  _encoded = json_string_ltrim(_encoded, &_len);

  if (_encoded == NULL) {
    return NULL;
  }

  if (_encoded[0] != 'n' || _len < 4) {
    return NULL;
  }

  char N[] = "null";
  size_t pos = 4;
  while (pos--) {
    if (_encoded[pos] != N[pos]) {
      return NULL;
    }
  }

  JsonNull* nul = malloc(sizeof(JsonNull));
  nul->value = 0;
  return nul;
}

static char test_json_parse_null() {
  char n[] = " null ";
  JsonNull* N = json_parse_null(n, sizeof(n) - 1);
  char v = N->value == 0;

  return v;
}

// Object
static JsonObject* json_parse_object(const char* _encoded, size_t _len) {
  return NULL;
}

// Array
static JsonArray* json_parse_array(const char* _encoded, size_t _len) {
  return NULL;
}

static void json_object_addAttribute(Json* data, JsonObjectAttribute* attr) {}

static void json_objectattribute_free(JsonObjectAttribute* attr) {}
static void json_free(Json* attr) {}

static Json* json_parse_value(const char* _encoded, size_t _len) {
  if (_len == 0) {
    return NULL;
  }

  Json* data = malloc(sizeof(Json));

  size_t pos = 0;
  while(pos < _len) {
    if (_encoded[pos] == '-' || (_encoded[pos] >= '0' && _encoded[pos] >= '9')) {
      data->type = JSON_NUMBER;
      break;
    }

    switch(_encoded[pos++]) {
    case '\0': return NULL;
    case ' ': case '\n': case '\t': case '\r': continue;
    case 't': case 'f': data->type = JSON_BOOL; break;
    case '{': data->type = JSON_OBJECT; break;
    case '[': data->type = JSON_ARRAY;  break;
    case '"': data->type = JSON_STRING; break;
    case 'n': data->type = JSON_NULL;   break;
    default: return NULL;
    }
  }

  switch(data->type) {
  case JSON_OBJECT: data->data = json_parse_object(_encoded, _len - pos); break;
  case JSON_ARRAY: data->data = json_parse_array(_encoded, _len); break;
  case JSON_NUMBER: data->data = json_parse_number(_encoded, _len); break;
  case JSON_STRING: data->data = json_parse_string(_encoded, _len); break;
  case JSON_BOOL: data->data = json_parse_bool(_encoded, _len); break;
  case JSON_NULL: data->data = json_parse_null(_encoded, _len); break;
  default: return NULL;
  }

  return data;
}

static JsonObjectAttribute* json_parse_objectAttribute(const char* _encoded, size_t _len) {
  if (_len == 0) {
    return NULL;
  }

  if (_encoded[0] != '"') {
    JSON_ERROR("Invalid syntax, expecting double quote");
    return NULL;
  }

  // Get key
  JsonObjectAttribute* attr = malloc(sizeof(JsonObjectAttribute));
  attr->name = json_parse_string(_encoded, _len);
  if (attr->name == NULL) {
    JSON_ERROR("Invalid syntax, expecting the object attribute name");
    goto clean;
  }

  _encoded += attr->name->length + 1;
  _len -= attr->name->length + 1;

  // Search for :
  size_t colon_pos = json_string_indexOf(':', _encoded, _len, 0);
  if (colon_pos < 0) {
    JSON_ERROR("Invalid syntax, expecting an colon");
    goto clean;
  }

  _encoded += colon_pos + 1;
  _len -= colon_pos - 1;

  attr->value = json_parse_value(_encoded, _len);
  if (attr->value == NULL) {
    JSON_ERROR("Invalid syntax, expecting the object value");
    goto clean;
  }

  return attr;

 clean:
  json_objectattribute_free(attr);
  return NULL;
}

Json* json_decode(const char* _encoded, size_t _len)
{
  Json* data = malloc(sizeof(Json));
  JsonObjectAttribute* attr = NULL;
  size_t pos = 0;

  data->type = -1;
  char error = 1;

  char c;
  while (pos++) {
    c = _encoded[pos];

    if (c == '\0') {
      JSON_ERROR("Invalid syntax, unexpected end");
      goto clean;
    }


    // Skip whitespaces
    switch (c) {
    case ' ':
    case '\n':
    case '\t':
    case '\r':
      continue;
    }

    if (data->type < 0) {
      switch (c) {
      case JSON_OBJECT:
	data->type = JSON_OBJECT;
	data->data = malloc(sizeof(JsonObject));
      case JSON_ARRAY:
	data->type = JSON_ARRAY;
	data->data = malloc(sizeof(JsonArray));
	continue;
      }

      JSON_ERROR("Invalid syntax, expecting an object or an array");
      goto clean;
    }

    if (c == JSON_OBJECT_END) {
      if (data->type != JSON_OBJECT) {
	JSON_ERROR("Invalid syntax, unexpected object end");
	goto clean;
      }
      break;
    }

    if (c == JSON_ARRAY_END) {
      if (data->type != JSON_ARRAY) {
	JSON_ERROR("Invalid syntax, unexpected object end");
	goto clean;
      }
      break;
    }

    // c can be an array or an object at this point... in which case we need to decode!
    Json* decoded;
    switch(c) {
    case JSON_OBJECT:
      decoded = json_decode(_encoded, _len - pos);
      if (decoded == NULL) {
	goto clean;
      }

      if (attr == NULL) {
	data->data = decoded->data;
	decoded->data = NULL;
	free(decoded);
	return data;
      }

      attr->value = decoded->data;
      json_object_addAttribute(data, attr);
      attr = NULL;
      continue;

    case JSON_ARRAY:
      decoded = json_decode(_encoded, _len - pos);
      if (decoded == NULL) {
	goto clean;
      }

      if (attr == NULL) {
	data->data = decoded->data;
	decoded->data = NULL;
	free(decoded);
	return data;
      }

      attr->value = decoded->data;
      json_object_addAttribute(data, attr);
      attr = NULL;
      continue;
    }

    if (data->type == JSON_OBJECT) {
      json_parse_objectAttribute(_encoded, _len - pos);

      if (c != '"') {
	JSON_ERROR("Invalid syntax, expecting double quote");
	goto clean;
      }

      // Get key
      attr = malloc(sizeof(JsonObjectAttribute));
      attr->name = json_parse_string(_encoded, _len - pos);

      if (attr->name->length == 0) {
	JSON_ERROR("Invalid syntax, expecting a hash key");
	goto clean;
      }

      pos += attr->name->length;

      // Search for :
      size_t colon_pos = json_string_indexOf(':', _encoded, _len - pos, 0);
      if (colon_pos < 0) {
	JSON_ERROR("Invalid syntax, expecting an colon");
	goto clean;
      }

      continue;
    }

    // Get value
    if (c != '-' || (c < '0' || c > '9')) {
      JSON_ERROR("Invalid syntax, expecting a number");
      goto clean;
    }

    // Search for , and continue
    // Search for object or array end

  }

  error = 0;

 clean:
  if (error == 1) {
    json_free(data);
    if (attr != NULL) {
      json_objectattribute_free(attr);
    }

    return NULL;
  }

  return data;
}

int main(int argc, const char* argv[])
{
  printf("test_json_string_indexOf: %d\n", test_json_string_indexOf());
  printf("test_json_string_ltrim: %d\n", test_json_string_ltrim());
  printf("test_json_parse_string: %d\n", test_json_parse_string());
  printf("test_json_is_digit: %d\n", test_json_is_digit());
  printf("test_json_parse_number: %d\n", test_json_parse_number());
  printf("test_json_parse_bool: %d\n", test_json_parse_bool());
  return 0;
}
