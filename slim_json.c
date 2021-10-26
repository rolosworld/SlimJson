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

void json_move_stream(JsonStream* _s, size_t _amount) {
  _s->position += _amount;
  _s->current += _amount;
  _s->length -= _amount;
}

// skip_escaped: Skip if the character has a \ before it
ssize_t json_string_indexOf(char _c, JsonStream* _enc, unsigned char _skip_escaped) {
  if (_enc == NULL || _enc->length == 0) {
    return -1;
  }

  for (int i = 0; i < _enc->length; i++) {
    if (_enc->current[i] == '\0') {
      return -1;
    }
    else if (_enc->current[i] == _c) {
      if (_skip_escaped == 1 && i > 0 && _enc->current[i - 1] == '\\') {
	continue;
      }

      return i;
    }
  }

  return -1;
}

void json_string_ltrim(JsonStream* _enc) {
  if (_enc == NULL || _enc->length == 0) {
    return;
  }

  size_t pos = 0;
  while (_enc->length > pos) {
    switch(_enc->current[pos]) {
    case '\0': return;
    case ' ':
    case '\n':
    case '\r':
    case '\t':
      pos++;
      continue;
    }
    break;
  }

  json_move_stream(_enc, pos);
}

char json_equal_strings(const char* _strA, size_t _lenA, const char* _strB, size_t _lenB) {
  if (_strA == NULL || _strB == NULL) {
    return 0;
  }

  if (_lenA != _lenB) {
    return 0;
  }

  while (_lenA--) {
    if (*(_strA++) != *(_strB++)) {
      return 0;
    }
  }

  return 1;
}

JsonString* json_parse_string(JsonStream* _enc) {
  if (_enc == NULL || _enc->length == 0) {
    return NULL;
  }

  json_string_ltrim(_enc);

  if (_enc->current == NULL || _enc->current[0] != JSON_STRING) {
    return NULL;
  }

  json_move_stream(_enc, 1);

  ssize_t end = json_string_indexOf(JSON_STRING, _enc, 1);
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

  json_move_stream(_enc, str->length + 1);

  return str;
}

void json_free_string(JsonString* _str) {
  if (_str->value == NULL) {
    return;
  }

  free(_str->value);
  free(_str);
}

char json_is_digit(char c) {
  return c >= '0' && c <= '9';
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

  json_move_stream(_enc, pos);

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
  for (int i = 0; i < pos; i++, _enc->length--) {
    if (_enc->current[i] != B[i]) {
      return NULL;
    }
  }

  JsonBool* bol = malloc(sizeof(JsonBool));
  bol->value = type == 't' ? 1 : 0;
  return bol;
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

  json_move_stream(_enc, 1);

  JsonNull* nul = malloc(sizeof(JsonNull));
  nul->value = 0;
  return nul;
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

  json_move_stream(_enc, 1);
  json_string_ltrim(_enc);

  JsonObject* obj = malloc(sizeof(JsonObject));
  obj->first = NULL;
  obj->last = NULL;

  while (_enc->current[0] != JSON_OBJECT_END) {
    JsonObjectAttribute* attr = json_parse_objectAttribute(_enc);
    if (attr == NULL) {
      goto clean;
    }
    json_add_objectAttribute(obj, attr);

    json_string_ltrim(_enc);
    if (_enc->current[0] == ',') {
      json_move_stream(_enc, 1);
    }
    else if (_enc->current[0] != JSON_OBJECT_END) {
      goto clean;
    }
  }

  json_move_stream(_enc, 1);

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

JsonObjectAttribute* json_get_objectAttribute(JsonObject* _obj, const char* _name, size_t _len) {
  if (_obj == NULL) {
    return NULL;
  }

  JsonObjectAttribute* attr = _obj->first;
  while (attr != NULL) {
    if (json_equal_strings(attr->name->value, attr->name->length, _name, _len)) {
      return attr;
    }
    attr = attr->next;
  }

  return NULL;
}

void json_free_objectAttribute(JsonObjectAttribute* _attr) {
  if (_attr == NULL) {
    return;
  }

  json_free_string(_attr->name);
  json_free(_attr->data);
  free(_attr);
}

void json_free_object(JsonObject* _obj) {
  if (_obj == NULL) {
    return;
  }

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

  json_move_stream(_enc, 1);
  json_string_ltrim(_enc);

  JsonArray* arr = malloc(sizeof(JsonArray));
  arr->first = NULL;
  arr->last = NULL;
  arr->length = 0;

  while (_enc->current[0] != JSON_ARRAY_END) {
    JsonArrayItem* item = json_parse_arrayItem(_enc);
    if (item == NULL) {
      goto clean;
    }
    json_add_arrayItem(arr, item);

    json_string_ltrim(_enc);
    if (_enc->current[0] == ',') {
      json_move_stream(_enc, 1);
    }
    else if (_enc->current[0] != JSON_ARRAY_END) {
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
  item->next = NULL;

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

JsonArrayItem* json_get_arrayItem(JsonArray* _arr, size_t _index) {
  if (_arr == NULL) {
    return NULL;
  }

  if (_index >= _arr->length) {
    return NULL;
  }

  JsonArrayItem* item = _arr->first;
  while (_index--) {
    item = item->next;
  }

  return item;
}

void json_free_arrayItem(JsonArrayItem* _item) {
  if (_item == NULL) {
    return;
  }

  json_free(_item->data);
  free(_item);
}

void json_free_array(JsonArray* _arr) {
  if (_arr == NULL) {
    return;
  }

  JsonArrayItem* item = _arr->first;
  JsonArrayItem* item2;
  while (item) {
    item2 = item;
    item = item->next;
    json_free_arrayItem(item2);
  }

  free(_arr);
}

void json_free(JsonValue* _data) {
  if (_data == NULL) {
    return;
  }

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
  case JSON_ERROR:
    free(_data->data);
    break;
  }

  free(_data);
}

JsonValue* json_parse_value(JsonStream* _enc) {
  if (_enc == NULL || _enc->length == 0) {
    return NULL;
  }

  json_string_ltrim(_enc);

  if (_enc->current == NULL) {
    return NULL;
  }

  JsonValue* data = malloc(sizeof(JsonValue));
  data->type = -1;
  data->data = NULL;

  size_t pos = 0;
  while(pos < _enc->length && data->type < 0) {
    if (_enc->current[pos] == '+' ||
	_enc->current[pos] == '-' ||
	(_enc->current[pos] >= '0' && _enc->current[pos] <= '9')) {
      data->type = JSON_NUMBER;
      break;
    }

    switch(_enc->current[pos++]) {
    case 't': case 'f': data->type = JSON_BOOL; break;
    case '{': data->type = JSON_OBJECT; break;
    case '[': data->type = JSON_ARRAY;  break;
    case '"': data->type = JSON_STRING; break;
    case 'n': data->type = JSON_NULL;   break;
    default: goto clean;
    }
  }

  switch(data->type) {
  case JSON_OBJECT: data->data = json_parse_object(_enc); break;
  case JSON_ARRAY: data->data = json_parse_array(_enc); break;
  case JSON_NUMBER: data->data = json_parse_number(_enc); break;
  case JSON_STRING: data->data = json_parse_string(_enc); break;
  case JSON_BOOL: data->data = json_parse_bool(_enc); break;
  case JSON_NULL: data->data = json_parse_null(_enc); break;
  default: goto clean;
  }

 clean:
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
  attr->next = NULL;
  attr->name = json_parse_string(_enc);
  if (attr->name == NULL) {
    goto clean;
  }

  // Search for :
  ssize_t colon_pos = json_string_indexOf(':', _enc, 0);
  if (colon_pos < 0) {
    goto clean;
  }

  json_move_stream(_enc, colon_pos + 1);

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

JsonValue* json_decode(const char* _json, size_t _len)
{
  JsonValue* data = NULL;
  JsonStream* enc = json_stream(_json, _len);

  json_string_ltrim(enc);

  if (enc->current[0] != JSON_OBJECT && enc->current[0] != JSON_ARRAY) {
    goto clean;
  }

  data = json_parse_value(enc);

  if (data == NULL) {
    goto clean;
  }

  free(enc);
  return data;

 clean:
  json_free(data);

  data = malloc(sizeof(JsonValue));
  data->type = JSON_ERROR;
  data->data = enc;
  return data;
}

void json_print_error(JsonValue* _e) {
  if (_e == NULL) {
    printf("ERROR: No Json error data passed\n");
    return;
  }

  if (_e->type != JSON_ERROR) {
    printf("ERROR: Not a valid Json error data passed\n");
    return;
  }

  JsonStream* s = (JsonStream*)_e->data;
  printf("ERROR: Invalid syntax at offset( %ld ): %.100s\n", s->position, s->current);
}

JsonStream* json_stream(const char* _json, size_t _len) {
  JsonStream* enc = malloc(sizeof(JsonStream));
  enc->current = _json;
  enc->start = _json;
  enc->length = _len;
  enc->position = 0;
  return enc;
}
