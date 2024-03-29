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
#include <stdio.h>

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

static size_t json_string_length(const char* _str) {
  size_t l = 0;
  while (*_str++ != '\0') {
    l++;
  }
  return l;
}

static void json_string_cat(char* _dest, size_t _len, const char* _src) {
  size_t pos = 0;
  while (pos < _len) {
    if (_dest[pos] == '\0') {
      break;
    }
    pos++;
  }

  while (pos < _len && *_src != '\0') {
    _dest[pos++] = *_src++;
  }

  _dest[_len] = '\0';
}

// skip_escaped: Skip if the character has a \ before it
static ssize_t json_string_indexOf(char _c, const char* _str, size_t _len, unsigned char _skip_escaped) {
  if (_str == NULL || _len == 0) {
    return -1;
  }

  for (size_t i = 0; i < _len; i++) {
    if (_str[i] == '\0') {
      return -1;
    }
    else if (_str[i] == _c) {
      if (_skip_escaped == 1 && i > 0 && _str[i - 1] == '\\') {
	continue;
      }

      return i;
    }
  }

  return -1;
}

/*************
 ** DECODER **
 *************/

static JsonValue* json_decode_value(JsonStream* _enc);

static char json_equal_strings(const char* _strA, size_t _lenA, const char* _strB, size_t _lenB) {
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

static size_t json_string_hash(const char* _str, size_t _len) {
  size_t h = 0;
  while (_len--) {
    h = (h ^ *(_str++)) << 1;
  }

  return h;
}

static JsonStream* json_stream(const char* _json, size_t _len) {
  JsonStream* enc = malloc(sizeof(JsonStream));
  enc->current = _json;
  enc->start = _json;
  enc->length = _len;
  enc->position = 0;
  return enc;
}

static char json_is_digit(char c) {
  return c >= '0' && c <= '9';
}

static ssize_t json_string_to_size(const char* _str, size_t _len) {
  ssize_t s = 0;
  while (_len--) {
    if (json_is_digit(*_str) != 1) {
      return -1;
    }
    s *= 10;
    s += *_str - '0';
    _str++;
  }

  return s;
}

static void json_move_stream(JsonStream* _s, size_t _amount) {
  _s->position += _amount;
  _s->current += _amount;
  _s->length -= _amount;
}

static char* json_substring(const char* _str, size_t _len) {
  char* v = malloc(_len + 1);
  v[_len] = '\0';
  while(_len--) {
    v[_len] = _str[_len];
  }
  return v;
}

static void json_string_ltrim(JsonStream* _enc) {
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

static JsonString* json_decode_string(JsonStream* _enc) {
  if (_enc == NULL || _enc->length == 0) {
    return NULL;
  }

  json_string_ltrim(_enc);

  if (_enc->current == NULL || _enc->current[0] != JSON_STRING) {
    return NULL;
  }

  json_move_stream(_enc, 1);

  ssize_t end = json_string_indexOf(JSON_STRING, _enc->current, _enc->length, 1);
  if (end < 0) {
    return NULL;
  }

  JsonString* str = malloc(sizeof(JsonString));
  str->length = end;
  str->value = json_substring(_enc->current, end);

  json_move_stream(_enc, str->length + 1);

  return str;
}

static void json_free_string(JsonString* _str) {
  if (_str->value == NULL) {
    return;
  }

  free(_str->value);
  free(_str);
}

static JsonNumber* json_decode_number(JsonStream* _enc) {
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
      value = value*10 + _enc->current[pos] - '0';
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
static const char TRUE_STR[] = "true";
static const char FALSE_STR[] = "false";
static JsonBool* json_decode_bool(JsonStream* _enc) {
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

  const char* B = type == 't' ? TRUE_STR : FALSE_STR;
  size_t pos = type == 't' ? 4 : 5;
  size_t end = pos;
  while (pos--) {
    if (_enc->current[pos] != B[pos]) {
      return NULL;
    }
  }

  JsonBool* bol = malloc(sizeof(JsonBool));
  bol->value = type == 't' ? 1 : 0;

  json_move_stream(_enc, end);

  return bol;
}

// null
static const char NULL_STR[] = "null";
static JsonNull* json_decode_null(JsonStream* _enc) {
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

  size_t pos = 4;
  while (pos--) {
    if (_enc->current[pos] != NULL_STR[pos]) {
      return NULL;
    }
  }

  json_move_stream(_enc, 4);

  JsonNull* nul = malloc(sizeof(JsonNull));
  nul->value = 0;
  return nul;
}

// Object
static void json_free_objectAttribute(JsonObjectAttribute* _attr) {
  if (_attr == NULL) {
    return;
  }

  json_free_string(_attr->name);
  json_free(_attr->data);
  free(_attr);
}

static JsonObjectAttribute* json_decode_objectAttribute(JsonStream* _enc) {
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
  attr->name = json_decode_string(_enc);
  if (attr->name == NULL) {
    goto clean;
  }

  // Search for :
  ssize_t colon_pos = json_string_indexOf(':', _enc->current, _enc->length, 0);
  if (colon_pos < 0) {
    goto clean;
  }

  json_move_stream(_enc, colon_pos + 1);

  // Get Value
  attr->data = json_decode_value(_enc);
  if (attr->data == NULL) {
    goto clean;
  }

  return attr;

 clean:
  json_free_objectAttribute(attr);
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
  _obj->length++;
}


static void json_free_object(JsonObject* _obj) {
  if (_obj == NULL) {
    return;
  }

  for (size_t i = 0; i < _obj->length; i++) {
    JsonObjectDataNode* node = _obj->object[i];
    JsonObjectDataNode* node2 = NULL;
    if (node != NULL) {
      node2 = node->next;
      free(node);
    }

    while (node2 != NULL) {
      node = node2->next;
      free(node2);
      node2 = node;
    }
  }

  if (_obj->length) {
    free(_obj->object);
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

static JsonObject* json_decode_object(JsonStream* _enc) {
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
  obj->object = NULL;
  obj->length = 0;

  while (_enc->current[0] != JSON_OBJECT_END) {
    JsonObjectAttribute* attr = json_decode_objectAttribute(_enc);
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

  // Generate the hash map
  if (obj->length) {
    obj->object = malloc(sizeof(JsonObjectDataNode*) * obj->length);
    JsonObjectAttribute* node = obj->first;
    size_t i;

    // Init array without using memset
    for (i = 0; i < obj->length; i++) {
      obj->object[i] = NULL;
    }

    JsonObjectDataNode* node2;
    JsonObjectDataNode* node3;
    while (node) {
      i = json_string_hash(node->name->value, node->name->length) % obj->length;

      node3 = malloc(sizeof(JsonObjectDataNode));
      node3->attribute = node;
      node3->next = NULL;


      node2 = obj->object[i];
      if (node2 == NULL) {
	obj->object[i] = node3;
      }
      else {
	while (node2->next) {
	  node2 = node2->next;
	}
	node2->next = node3;
      }

      node = node->next;
    }
  }

  return obj;

 clean:
  json_free_object(obj);
  return NULL;
}

static JsonObjectAttribute* json_get_objectAttribute(const JsonObject* _obj, const char* _name, size_t _len) {
  if (_obj == NULL) {
    return NULL;
  }

  if (_len < 1) {
    return NULL;
  }

  if (_obj->length == 0) {
    return NULL;
  }

  size_t i = json_string_hash(_name, _len) % _obj->length;
  JsonObjectDataNode* node = _obj->object[i];
  while (node) {
    if (json_equal_strings(node->attribute->name->value, node->attribute->name->length, _name, _len) == 1) {
      return node->attribute;
    }
    node = node->next;
  }

  return NULL;
}

// Array
static void json_free_arrayItem(JsonArrayItem* _item) {
  if (_item == NULL) {
    return;
  }

  json_free(_item->data);
  free(_item);
}

static JsonArrayItem* json_decode_arrayItem(JsonStream* _enc) {
  if (_enc == NULL || _enc->length == 0) {
    return NULL;
  }

  json_string_ltrim(_enc);

  JsonArrayItem*  item = malloc(sizeof(JsonArrayItem));
  item->next = NULL;

  // Get Value
  item->data = json_decode_value(_enc);
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

static void json_free_array(JsonArray* _arr) {
  if (_arr == NULL) {
    return;
  }

  if (_arr->array != NULL) {
    free(_arr->array);
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

static JsonArray* json_decode_array(JsonStream* _enc) {
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
  arr->array = NULL;
  arr->length = 0;

  while (_enc->current[0] != JSON_ARRAY_END) {
    JsonArrayItem* item = json_decode_arrayItem(_enc);
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

  json_move_stream(_enc, 1);

  // Create the array
  if (arr->length) {
    arr->array = malloc(sizeof(JsonArrayItem*) * arr->length);
    JsonArrayItem* node = arr->first;
    size_t i = 0;
    while (node) {
      arr->array[i++] = node;
      node = node->next;
    }
  }

  return arr;

 clean:
  json_free_array(arr);
  return NULL;
}

static JsonArrayItem* json_get_arrayItem(const JsonArray* _arr, size_t _index) {
  if (_arr == NULL) {
    return NULL;
  }

  if (_index >= _arr->length) {
    return NULL;
  }

  return _arr->array[_index];
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

static JsonValue* json_decode_value(JsonStream* _enc) {
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
  case JSON_OBJECT: data->data = json_decode_object(_enc); break;
  case JSON_ARRAY: data->data = json_decode_array(_enc); break;
  case JSON_NUMBER: data->data = json_decode_number(_enc); break;
  case JSON_STRING: data->data = json_decode_string(_enc); break;
  case JSON_BOOL: data->data = json_decode_bool(_enc); break;
  case JSON_NULL: data->data = json_decode_null(_enc); break;
  default: goto clean;
  }

 clean:
  if (data->data == NULL) {
    json_free(data);
    return NULL;
  }

  return data;
}

JsonValue* json_decode(const char* _json)
{
  size_t len = json_string_length(_json);

  if (len < 1) {
    return NULL;
  }

  JsonValue* data = NULL;
  JsonStream* enc = json_stream(_json, len);

  json_string_ltrim(enc);

  if (enc->current[0] != JSON_OBJECT && enc->current[0] != JSON_ARRAY) {
    goto clean;
  }

  data = json_decode_value(enc);

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

JsonError json_get_errorMsg(JsonValue* _e) {
  JsonError e = {0};
  size_t len = sizeof(e.msg) - 1;

  if (_e == NULL) {
    json_string_cat(e.msg, len, "ERROR: No Json error data passed");
    return e;
  }

  if (_e->type != JSON_ERROR) {
    json_string_cat(e.msg, len, "ERROR: Not a valid Json error data passed");
    return e;
  }

  JsonStream* s = (JsonStream*)_e->data;
  snprintf(e.msg, len, "ERROR: Invalid syntax at offset( %ld ): %s", s->position, s->current);
  return e;
}

// 0.name
// obj.name
const JsonValue* json_get(const JsonValue* _v, const char* _path) {
  if (_v == NULL) {
    return NULL;
  }

  size_t len = json_string_length(_path);
  if (len < 1) {
    return NULL;
  }

  const JsonValue* v = NULL;
  JsonObject* obj = NULL;
  JsonArray* arr = NULL;
  JsonObjectAttribute* objA = NULL;
  JsonArrayItem* arrI = NULL;

  char type = json_is_digit(_path[0]) ? JSON_ARRAY : JSON_OBJECT;

  ssize_t end = json_string_indexOf('.', _path, len, 0);
  char last = 0;
  if (end < 0) {
    last = 1;
    end = len;
  }

  char* sub = json_substring(_path, end);
  char* name = sub;

  if (type != _v->type) {
    goto clean;
  }

  switch (_v->type) {
  case JSON_OBJECT:
    obj = (JsonObject*)_v->data;
    name++;
    objA = json_get_objectAttribute(obj, name, end - 2);
    if (objA == NULL) {
      goto clean;
    }
    v = objA->data;
    break;

  case JSON_ARRAY:
    arr = (JsonArray*)_v->data;
    ssize_t index = json_string_to_size(name, end);
    if (index < 0) {
      goto clean;
    }
    arrI = json_get_arrayItem(arr, index);
    if (arrI == NULL) {
      goto clean;
    }
    v = arrI->data;
    break;

  default:
    goto clean;
  }

  if (last != 1) {
    v = json_get(v, _path + end + 1);
  }

 clean:
  free(sub);

  return v;
}

/*************
 ** ENCODER **
 *************/
typedef struct JsonStringNode {
  char* value;
  size_t length;
  struct JsonStringNode* next;
} JsonStringNode;

static JsonStringNode* json_encode_value(JsonValue* _val);

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

static JsonStringNode* json_new_stringNode(size_t size) {
  JsonStringNode* node = malloc(sizeof(JsonStringNode));
  node->value = malloc(size);
  node->next = NULL;
  node->length = size - 1;
  while (size--) {
    node->value[size] = '\0';
  }
  return node;
}

static JsonStringNode* json_merge_stringNode(JsonStringNode* _node, size_t _start, size_t _offset) {
  size_t len = 0;
  JsonStringNode* node = _node;
  while (node != NULL) {
    len += node->length;
    node = node->next;
  }

  JsonStringNode* merge = json_new_stringNode(len + _offset + 1);
  char* val = merge->value + _start;

  node = _node;
  len = 0;
  while (node != NULL) {
    json_string_cat(val + len, node->length, node->value);
    len += node->length;
    node = node->next;
  }

  json_free_stringNode(_node);
  return merge;
}

static JsonStringNode* json_encode_string(JsonString* _str) {
  JsonStringNode* node = json_new_stringNode(_str->length + 3);
  size_t len = _str->length + 2;
  node->value[0] = '"';
  node->value[len - 1] = '"';
  json_string_cat(node->value + 1, len - 1, _str->value);
  return node;
}

static JsonStringNode* json_encode_number(JsonNumber* _num) {
  JsonStringNode* node = json_new_stringNode(330);
  snprintf(node->value, node->length, "%9.16f", _num->value);
  ssize_t index = json_string_indexOf('.', node->value, node->length, 0);
  node->length = json_string_length(node->value);
  if (index > -1) {
    size_t len = node->length;
    while (len--) {
      if (node->value[len] == '0') {
	node->value[len] = '\0';
	node->length--;
	continue;
      }

      if (node->value[len] == '.') {
	node->value[len] = '\0';
	node->length--;
      }

      break;
    }
  }
  return node;
}

static JsonStringNode* json_encode_bool(JsonBool* _bol) {
  size_t len = _bol->value ? 4 : 5;
  JsonStringNode* node = json_new_stringNode(len + 1);
  json_string_cat(node->value, len, _bol->value ? TRUE_STR : FALSE_STR);
  return node;
}

static JsonStringNode* json_encode_null() {
  size_t len = 4;
  JsonStringNode* node = json_new_stringNode(len + 1);
  json_string_cat(node->value, len, NULL_STR);
  return node;
}

static JsonStringNode* json_encode_objectAttribute(JsonObjectAttribute* _attr) {
  JsonStringNode* name = json_encode_string(_attr->name);
  JsonStringNode* colon = json_new_stringNode(1);
  JsonStringNode* value = json_encode_value(_attr->data);
  colon->value[0] = ':';
  colon->length++;
  name->next = colon;
  colon->next = value;
  return json_merge_stringNode(name, 0, 0);
}

static JsonStringNode* json_encode_object(JsonObject* _obj) {
  JsonStringNode* first = NULL;
  JsonStringNode* node = NULL;

  JsonObjectAttribute* attr = _obj->first;
  while (attr != NULL) {
    JsonStringNode* attrNode = json_encode_objectAttribute(attr);
    if (first == NULL) {
      first = attrNode;
    }
    else {
      node->next = attrNode;
    }
    node = attrNode;
    attr = attr->next;

    if (attr != NULL) {
      JsonStringNode* commaNode = json_new_stringNode(1);
      commaNode->value[0] = ',';
      commaNode->length++;
      node->next = commaNode;
      node = commaNode;
    }
  }

  node = json_merge_stringNode(first, 1, 2);
  node->value[0] = '{';
  node->value[node->length - 1] = '}';

  return node;
}

static JsonStringNode* json_encode_arrayItem(JsonArrayItem* _item) {
  return json_encode_value(_item->data);
}

static JsonStringNode* json_encode_array(JsonArray* _arr) {
  JsonStringNode* first = NULL;
  JsonStringNode* node = NULL;

  JsonArrayItem* item = _arr->first;
  while (item != NULL) {
    JsonStringNode* itemNode = json_encode_arrayItem(item);
    if (first == NULL) {
      first = itemNode;
    }
    else {
      node->next = itemNode;
    }
    node = itemNode;
    item = item->next;

    if (item != NULL) {
      JsonStringNode* commaNode = json_new_stringNode(1);
      commaNode->value[0] = ',';
      commaNode->length++;
      node->next = commaNode;
      node = commaNode;
    }
  }

  node = json_merge_stringNode(first, 1, 2);
  node->value[0] = '[';
  node->value[node->length - 1] = ']';

  return node;
}

static JsonStringNode* json_encode_value(JsonValue* _val) {
  switch (_val->type) {
  case JSON_BOOL:
    return json_encode_bool((JsonBool*)_val->data);
    break;
  case JSON_NUMBER:
    return json_encode_number((JsonNumber*)_val->data);
    break;
  case JSON_NULL:
    return json_encode_null();
    break;
  case JSON_STRING:
    return json_encode_string((JsonString*)_val->data);
    break;
  case JSON_OBJECT:
    return json_encode_object((JsonObject*)_val->data);
    break;
  case JSON_ARRAY:
    return json_encode_array((JsonArray*)_val->data);
    break;
  }

  return NULL;
}

char* json_encode(JsonValue* _value) {
  JsonStringNode* node = json_encode_value(_value);
  char* result = node->value;
  node->value = NULL;
  json_free_stringNode(node);
  return result;
}

/*****************
 ** GET HELPERS **
 *****************/
const JsonValue* GetValue(const JsonValue* _v, const char* _path) {
    const JsonValue* v = json_get(_v, _path);
    return v;
}

static const char* json_value_toString(const JsonValue* _v) {
    if (_v == NULL || _v->type != JSON_STRING) {
      return NULL;
    }

    const JsonString* s = (const JsonString*)_v->data;
    return s->value;
}

static double json_undefined() {
      double a = 0;
      return a/a;
}

static double json_value_toNumber(const JsonValue* _v) {
    if (_v == NULL || _v->type != JSON_NUMBER) {
      return json_undefined();
    }

    const JsonNumber* n = (const JsonNumber*)_v->data;
    return n->value;
}

static char json_value_toBool(const JsonValue* _v) {
    if (_v == NULL || _v->type != JSON_BOOL) {
      return -1;
    }

    const JsonBool* b = (const JsonBool*)_v->data;
    return b->value;
}

static char json_value_toNull(const JsonValue* _v) {
    if (_v == NULL || _v->type != JSON_NULL) {
      return -1;
    }

    const JsonNull* n = (const JsonNull*)_v->data;
    return n->value;
}

static const JsonObject* json_value_toObject(const JsonValue* _v) {
    if (_v == NULL || _v->type != JSON_OBJECT) {
      return NULL;
    }

    return (const JsonObject*)_v->data;
}

static const JsonArray* json_value_toArray(const JsonValue* _v) {
    if (_v == NULL || _v->type != JSON_ARRAY) {
      return NULL;
    }

    return (const JsonArray*)_v->data;
}

const char* json_get_string(const JsonValue* _v, const char* _path) {
    return json_value_toString(json_get(_v, _path));
}

double json_get_number(const JsonValue* _v, const char* _path) {
    return json_value_toNumber(json_get(_v, _path));
}

char json_get_bool(const JsonValue* _v, const char* _path) {
    return json_value_toBool(json_get(_v, _path));
}

char json_get_null(const JsonValue* _v, const char* _path) {
    return json_value_toNull(json_get(_v, _path));
}

const JsonObject* json_get_object(const JsonValue* _v, const char* _path) {
    if (_path != NULL) {
        _v = json_get(_v, _path);
    }

    return json_value_toObject(_v);
}

const char* json_get_object_string(const JsonObject* _obj, const char* _attributeName) {
    JsonObjectAttribute* attr = json_get_objectAttribute(_obj, _attributeName, json_string_length(_attributeName));
    if (attr == NULL) {
      return NULL;
    }
    return json_value_toString(attr->data);
}

double json_get_object_number(const JsonObject* _obj, const char* _attributeName) {
    JsonObjectAttribute* attr = json_get_objectAttribute(_obj, _attributeName, json_string_length(_attributeName));
    if (attr == NULL) {
      return json_undefined();
    }
    return json_value_toNumber(attr->data);
}

char json_get_object_bool(const JsonObject* _obj, const char* _attributeName) {
    JsonObjectAttribute* attr = json_get_objectAttribute(_obj, _attributeName, json_string_length(_attributeName));
    if (attr == NULL) {
      return '\0';
    }
    return json_value_toBool(attr->data);
}

char json_get_object_null(const JsonObject* _obj, const char* _attributeName) {
    JsonObjectAttribute* attr = json_get_objectAttribute(_obj, _attributeName, json_string_length(_attributeName));
    if (attr == NULL) {
      return '\0';
    }
    return json_value_toNull(attr->data);
}

const JsonArray* json_get_object_array(const JsonObject* _obj, const char* _attributeName) {
    JsonObjectAttribute* attr = json_get_objectAttribute(_obj, _attributeName, json_string_length(_attributeName));
    if (attr == NULL) {
      return NULL;
    }
    return json_value_toArray(attr->data);
}

const JsonObject* json_get_object_object(const JsonObject* _obj, const char* _attributeName) {
    JsonObjectAttribute* attr = json_get_objectAttribute(_obj, _attributeName, json_string_length(_attributeName));
    if (attr == NULL) {
      return NULL;
    }
    return json_value_toObject(attr->data);
}

const JsonArray* json_get_array(const JsonValue* _v, const char* _path) {
    if (_path != NULL) {
        _v = json_get(_v, _path);
    }

    return json_value_toArray(_v);
}

const char* json_get_array_string(const JsonArray* _arr, size_t _itemIndex) {
    JsonArrayItem* item = json_get_arrayItem(_arr, _itemIndex);
    if (item == NULL) {
      return NULL;
    }
    return json_value_toString(item->data);
}

double json_get_array_number(const JsonArray* _arr, size_t _itemIndex) {
    JsonArrayItem* item = json_get_arrayItem(_arr, _itemIndex);
    if (item == NULL) {
      return json_undefined();
    }
    return json_value_toNumber(item->data);
}

char json_get_array_bool(const JsonArray* _arr, size_t _itemIndex) {
    JsonArrayItem* item = json_get_arrayItem(_arr, _itemIndex);
    if (item == NULL) {
      return '\0';
    }
    return json_value_toBool(item->data);
}

char json_get_array_null(const JsonArray* _arr, size_t _itemIndex) {
    JsonArrayItem* item = json_get_arrayItem(_arr, _itemIndex);
    if (item == NULL) {
      return '\0';
    }
    return json_value_toNull(item->data);
}

const JsonArray* json_get_array_array(const JsonArray* _arr, size_t _itemIndex) {
    JsonArrayItem* item = json_get_arrayItem(_arr, _itemIndex);
    if (item == NULL) {
      return NULL;
    }
    return json_value_toArray(item->data);
}

const JsonObject* json_get_array_object(const JsonArray* _arr, size_t _itemIndex) {
    JsonArrayItem* item = json_get_arrayItem(_arr, _itemIndex);
    if (item == NULL) {
      return NULL;
    }
    return json_value_toObject(item->data);
}
