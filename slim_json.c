#include <stdio.h>
#include <stdlib.h>
#include "slim_json.h"

// skip_escaped: Skip if the character has a \ before it
size_t json_string_indexOf(char _c, JsonStream* _enc, unsigned char _skip_escaped) {
  if (_enc == NULL || _enc->length == 0) {
    return -1;
  }

  size_t len = _enc->length;
  while (--len) {
    if (_enc->current[len] == '\0') {
      return -1;
    }
    else if (_enc->current[len] == _c) {
      if (_skip_escaped == 1 && len > 0 && _enc->current[len - 1] == '\\') {
	continue;
      }

      return len;
    }
  }

  return -1;
}

char test_json_string_indexOf() {
  char t[] = "Hello World!\n\\\"'";
  size_t len = sizeof(t) - 1;
  JsonStream e;
  e.current = t;
  e.length = len;
  char t1 = json_string_indexOf(' ', &e, 0);
  char t2 = json_string_indexOf('!', &e, 0);
  char t3 = json_string_indexOf('\n', &e, 0);
  char t4 = json_string_indexOf('"', &e, 0);
  char t5 = json_string_indexOf('\'', &e, 1);

  return t[t1] == ' ' &&
    t[t2] == '!' &&
    t[t3] == '\n' &&
    t[t4] == '"' &&
    t[t5] == '\'';
}

void json_string_ltrim(JsonStream* _enc) {
  if (_enc == NULL || _enc->length == 0) {
    return;
  }

  size_t pos = 0;
  while (_enc->length > pos) {
    switch(_enc->current[pos]) {
    case '\0': return;
    case ' ': case '\n': case '\r': case '\t': pos++; continue;
    }
    break;
  }

  _enc->length -= pos;
  _enc->current += pos;
}

char test_json_string_ltrim() {
  char t[] = " \n\t\r Hello World!\n\\\"\"";
  size_t len = sizeof(t) - 1;
  JsonStream e;
  e.current = t;
  e.length = len;
  json_string_ltrim(&e);

  return e.current[0] == 'H';
}

JsonString* json_parse_string(JsonStream* _enc) {
  if (_enc == NULL || _enc->length == 0) {
    return NULL;
  }

  json_string_ltrim(_enc);

  if (_enc->current == NULL || _enc->current[0] != JSON_STRING) {
    return NULL;
  }

  _enc->length--;
  _enc->current++;

  size_t end = json_string_indexOf(JSON_STRING, _enc, 1);
  if (end < 0) {
    return NULL;
  }

  JsonString* str = malloc(sizeof(JsonString));
  str->length = end;
  char* v = malloc(sizeof(char) * end + 1);
  v[end] = '\0';
  str->value = v;

  while(end--) {
    v[end] = _enc->current[end];
  }

  _enc->current += end;
  _enc->length -= end;

  return str;
}

void json_free_string(JsonString* _str) {
  if (_str->value != NULL) {
    free(_str->value);
  }
  free(_str);
}

char test_json_parse_string() {
  char t[] = "  \n\t\r \"Foo\\\"Bar\",  ";
  JsonStream e;
  e.current = t;
  e.length = sizeof(t) - 1;
  JsonString* T = json_parse_string(&e);
  char t1 = T->value[0] == 'F';
  char t2 = T->value[T->length-1] == 'r';
  char t3 = T->value[T->length] == '\0';
  json_free_string(T);
  return t1 && t2 && t3;
}

char json_is_digit(char c) {
  return c >= '0' && c <= '9';
}

char test_json_is_digit() {
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

JsonNumber* json_parse_number(JsonStream* _enc) {
  if (_enc == NULL || _enc->length == 0) {
    return NULL;
  }

  json_string_ltrim(_enc);

  if (_enc->current == NULL) {
    return NULL;
  }

  size_t pos = 0;
  char negative = _enc->current[0] == '-' ? 1 : 0;
  if (negative == 1) {
    pos++;
  }

  if (json_is_digit(_enc->current[pos]) == 0) {
    return NULL;
  }

  double value = 0;
  char fraction = 0;
  while (_enc->length > pos) {
    if (json_is_digit(_enc->current[pos]) == 1) {
      value *= 10;
      value += _enc->current[pos] - '0';
      if (fraction) {
	fraction++;
      }
    }
    else if (_enc->current[pos] == '.') {
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

  _enc->length -= pos;

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

char test_json_parse_number() {
  char n1[] = " -1.023 ";
  size_t len = sizeof(n1) - 1;
  JsonStream e;
  e.current = n1;
  e.length = len;
  JsonNumber* num1 = json_parse_number(&e);
  char v1 = ((float) num1->value) == ((float) -1.023);
  free(num1);

  char n2[] = " 1.023 ";
  len = sizeof(n2) - 1;
  e.current = n2;
  e.length = len;
  JsonNumber* num2 = json_parse_number(&e);
  char v2 = ((float) num2->value) == ((float) 1.023);
  free(num2);

  char n3[] = "1023";
  len = sizeof(n3) - 1;
  e.current = n3;
  e.length = len;
  JsonNumber* num3 = json_parse_number(&e);
  char v3 = ((float) num3->value) == ((float) 1023);
  free(num3);

  return v1 && v2 && v3;
}

// false or true
JsonBool* json_parse_bool(JsonStream* _enc) {
  if (_enc == NULL || _enc->length == 0) {
    return NULL;
  }

  json_string_ltrim(_enc);

  if (_enc->current == NULL) {
    return NULL;
  }

  char type = _enc->current[0];
  if (type != 't' && type != 'f') {
    return NULL;
  }

  if (type == 't') {
    if (_enc->length < 4) {
      return NULL;
    }
  }
  else if(_enc->length < 5) {
    return NULL;
  }

  char T[] = "true";
  char F[] = "false";
  char* B = type == 't' ? T : F;
  size_t pos = type == 't' ? 4 : 5;
  while (pos--) {
    if (_enc->current[pos] != B[pos]) {
      return NULL;
    }
  }

  _enc->length -= pos;

  JsonBool* bol = malloc(sizeof(JsonBool));
  bol->value = type == 't' ? 1 : 0;
  return bol;
}

char test_json_parse_bool() {
  char b1[] = " true ";
  size_t len = sizeof(b1) - 1;
  JsonStream e;
  e.current = b1;
  e.length = len;
  JsonBool* B1 = json_parse_bool(&e);
  char v1 = B1->value == 1;

  char b2[] = " false ";
  len = sizeof(b2) - 1;
  e.current = b2;
  e.length = len;
  JsonBool* B2 = json_parse_bool(&e);
  char v2 = B2->value == 0;

  return v1 && v2;
}

// null
JsonNull* json_parse_null(JsonStream* _enc) {
  if (_enc == NULL || _enc->length == 0) {
    return NULL;
  }

  json_string_ltrim(_enc);

  if (_enc->current == NULL) {
    return NULL;
  }

  if (_enc->current[0] != 'n' || _enc->length < 4) {
    return NULL;
  }

  char N[] = "null";
  size_t pos = 4;
  while (pos--) {
    if (_enc->current[pos] != N[pos]) {
      return NULL;
    }
  }

  _enc->length -= pos;

  JsonNull* nul = malloc(sizeof(JsonNull));
  nul->value = 0;
  return nul;
}

char test_json_parse_null() {
  char n[] = " null ";
  size_t len = sizeof(n) - 1;
  JsonStream e;
  e.current = n;
  e.length = len;
  JsonNull* N = json_parse_null(&e);
  char v = N->value == 0;

  return v;
}

// Object
JsonObject* json_parse_object(JsonStream* _enc) {
  if (_enc == NULL || _enc->length == 0) {
    return NULL;
  }

  json_string_ltrim(_enc);

  if (_enc->current == NULL) {
    return NULL;
  }

  if (_enc->current[0] != JSON_OBJECT) {
    return NULL;
  }

  _enc->current++;
  _enc->length--;
  json_string_ltrim(_enc);

  JsonObject* obj = malloc(sizeof(JsonObject));

  while (_enc->current[0] != JSON_OBJECT_END) {
    JsonObjectAttribute* attr = json_parse_objectAttribute(_enc);
    if (attr == NULL) {
      goto clean;
    }
    json_add_objectAttribute(obj, attr);

    json_string_ltrim(_enc);
    if (_enc->current[0] != ',' && _enc->current[0] != JSON_OBJECT_END) {
      goto clean;
    }
  }

  return obj;

 clean:
  json_free_object(obj);
  return NULL;
}

void json_add_objectAttribute(JsonObject* _obj, JsonObjectAttribute* _attr) {
  if (_obj->first == NULL) {
    _obj->first = _attr;
  }
  else {
    _obj->last->next = _attr;
  }

  _obj->last = _attr;
}

void json_free_objectAttribute(JsonObjectAttribute* _attr) {
  json_free_string(_attr->name);
  json_free(_attr->data);
}

void json_free_object(JsonObject* _obj) {
  JsonObjectAttribute* attr = _obj->first;
  JsonObjectAttribute* attr2;
  while (attr) {
    attr2 = attr;
    attr = attr->next;
    json_free_objectAttribute(attr2);
  }

  free(_obj);
}


// Array
JsonArray* json_parse_array(JsonStream* _enc) {
  if (_enc == NULL || _enc->length == 0) {
    return NULL;
  }

  json_string_ltrim(_enc);

  if (_enc->current == NULL) {
    return NULL;
  }

  if (_enc->current[0] != JSON_ARRAY) {
    return NULL;
  }

  _enc->current++;
  _enc->length--;
  json_string_ltrim(_enc);

  JsonArray* arr = malloc(sizeof(JsonArray));

  while (_enc->current[0] != JSON_ARRAY_END) {
    JsonArrayItem* item = json_parse_arrayItem(_enc);
    if (item == NULL) {
      goto clean;
    }
    json_add_arrayItem(arr, item);

    json_string_ltrim(_enc);
    if (_enc->current[0] != ',' && _enc->current[0] != JSON_ARRAY_END) {
      goto clean;
    }
  }

  return arr;

 clean:
  json_free_array(arr);
  return NULL;
}

JsonArrayItem* json_parse_arrayItem(JsonStream* _enc) {
  if (_enc == NULL || _enc->length == 0) {
    return NULL;
  }

  json_string_ltrim(_enc);

  JsonArrayItem*  item = malloc(sizeof(JsonArrayItem));

  // Get Value
  item->data = json_parse_value(_enc);
  if (item->data == NULL) {
    goto clean;
  }

  return item;

 clean:
  json_free_arrayItem(item);
  return NULL;
}

void json_add_arrayItem(JsonArray* _arr, JsonArrayItem* _item) {
  if (_arr->first == NULL) {
    _arr->first = _item;
  }
  else {
    _arr->last->next = _item;
  }

  _arr->last = _item;
  _arr->length++;
}

void json_free_arrayItem(JsonArrayItem* _item) {
  json_free(_item->data);
}

void json_free_array(JsonArray* _arr) {
  JsonArrayItem* item = _arr->first;
  JsonArrayItem* item2;
  while (item) {
    item2 = item;
    item = item->next;
    json_free_arrayItem(item2);
  }

  free(_arr);
}

void json_free(Json* _data) {
  switch(_data->type) {
  case JSON_OBJECT:
    json_free_object(_data->data);
    break;
  case JSON_ARRAY:
    json_free_array(_data->data);
    break;
  case JSON_STRING:
    json_free_string(_data->data);
    break;
  case JSON_NUMBER:
  case JSON_BOOL:
  case JSON_NULL:
    free(_data->data);
    break;
  }

  free(_data);
}

Json* json_parse_value(JsonStream* _enc) {
  if (_enc == NULL || _enc->length == 0) {
    return NULL;
  }

  json_string_ltrim(_enc);

  if (_enc->current == NULL) {
    return NULL;
  }

  Json* data = malloc(sizeof(Json));

  size_t pos = 0;
  while(pos < _enc->length) {
    if (_enc->current[pos] == '-' || (_enc->current[pos] >= '0' && _enc->current[pos] >= '9')) {
      data->type = JSON_NUMBER;
      break;
    }

    switch(_enc->current[pos++]) {
    case 't': case 'f': data->type = JSON_BOOL; break;
    case '{': data->type = JSON_OBJECT; break;
    case '[': data->type = JSON_ARRAY;  break;
    case '"': data->type = JSON_STRING; break;
    case 'n': data->type = JSON_NULL;   break;
    default: return NULL;
    }
  }

  _enc->length -= pos;

  switch(data->type) {
  case JSON_OBJECT: data->data = json_parse_object(_enc); break;
  case JSON_ARRAY: data->data = json_parse_array(_enc); break;
  case JSON_NUMBER: data->data = json_parse_number(_enc); break;
  case JSON_STRING: data->data = json_parse_string(_enc); break;
  case JSON_BOOL: data->data = json_parse_bool(_enc); break;
  case JSON_NULL: data->data = json_parse_null(_enc); break;
  default: return NULL;
  }

  if (data->data == NULL) {
    json_free(data);
    return NULL;
  }

  return data;
}

JsonObjectAttribute* json_parse_objectAttribute(JsonStream* _enc) {
  if (_enc == NULL || _enc->length == 0) {
    return NULL;
  }

  json_string_ltrim(_enc);

  if (_enc->current[0] != JSON_STRING) {
    return NULL;
  }

  // Get key
  JsonObjectAttribute* attr = malloc(sizeof(JsonObjectAttribute));
  attr->name = json_parse_string(_enc);
  if (attr->name == NULL) {
    goto clean;
  }

  _enc->current += attr->name->length + 1;
  _enc->length -= attr->name->length + 1;

  // Search for :
  size_t colon_pos = json_string_indexOf(':', _enc, 0);
  if (colon_pos < 0) {
    goto clean;
  }

  _enc->current += colon_pos + 1;
  _enc->length -= colon_pos - 1;

  // Get Value
  attr->data = json_parse_value(_enc);
  if (attr->data == NULL) {
    goto clean;
  }

  return attr;

 clean:
  json_free_objectAttribute(attr);
  return NULL;
}

Json* json_decode(const char* _json, size_t _len)
{
  JsonStream enc;
  enc.current = _json;
  enc.length = _len;

  Json* data = json_parse_value(_enc);

  if (data == NULL) {
    goto clean;
  }

  return data;

 clean:
  if (_enc->length > 100) {
    _enc->length = 100;
    _enc->current[_enc->length] = '\0';
  }
  printf("ERROR: Invalid syntax near: %s%s\n", _enc->current, (_enc->length == 100? "...":""));
  json_free(data);
  return NULL;
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
