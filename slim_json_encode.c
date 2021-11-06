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
#include "slim_json_helpers.h"
#include "slim_json.h"
#include <stdio.h>

/**
 * ENCODE 
 **/
typedef struct JsonStringNode {
  char* value;
  size_t length;
  struct JsonStringNode* next;
} JsonStringNode;

static char* json_string_copy(const char* _str);
static JsonStringNode* json_encode_string(JsonString* _str);
static JsonStringNode* json_encode_number(JsonNumber* _num);

static JsonStringNode* json_encode_bool(JsonBool* _bol);
static JsonStringNode* json_encode_null(JsonNull* _nul);
static JsonStringNode* json_encode_object(JsonObject* _obj);
static JsonStringNode* json_encode_array(JsonArray* _arr);
static JsonStringNode* json_encode_value(JsonValue* _val);
static JsonStringNode* json_encode_objectAttribute(JsonObjectAttribute* _attr);
static JsonStringNode* json_encode_arrayItem(JsonArrayItem* _item);

static char* json_string_copy(const char* _str) {
  size_t len = json_string_length(_str);
  char* str = malloc(len + 1);
  while (len) {
    str[len] = _str[len--];
  }
  return str;
}

static char json_string_isEscapable(char _c) {
  switch (_c) {
  case '"':
  case '\\':
  case '/':
  case '\b':
  case '\f':
  case '\n':
  case '\r':
  case '\t':
    return 1;
  }
  return 0;
}

static size_t json_string_escapableCount(const char* _str, size_t _len) {
  size_t escapables = 0;
  while (_len) {
    if (json_string_isEscapable(_str[_len--])) {
      escapables++;
    }
  }
  return escapables;
}

static char* json_string_escapedCopy(const char* _str) {
  size_t len = json_string_length(_str);
  size_t len2 = len;
  len2 += json_string_escapableCount(_str, len);

  char* str = malloc(len2 + 1);
  while (len) {
    str[len2--] = _str[len];
    if (json_string_isEscapable(_str[len--])) {
      str[len2--] = '\\';
    }
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

static JsonStringNode* json_new_stringNode(size_t size) {
  JsonStringNode* node = malloc(sizeof(JsonStringNode));
  node->value = malloc(size);
  node->next = NULL;
  node->length = size - 1;
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
  node->value[len] = '\0';
  json_string_cat(node->value + 1, len - 2, _str->value);
  return node;
}

static JsonStringNode* json_encode_number(JsonNumber* _num) {
  JsonStringNode* node = json_new_stringNode(330);
  sprintf(node->value, "%9.16f", _num->value);
  return node;
}

static JsonStringNode* json_encode_bool(JsonBool* _bol) {
  //true false
  size_t len = _bol->value ? 4 : 5;
  JsonStringNode* node = json_new_stringNode(len + 1);
  json_string_cat(node->value, len, _bol->value ? "true" : "false");
  return node;
}

static JsonStringNode* json_encode_null(JsonNull* _nul) {
  size_t len = 4;
  JsonStringNode* node = json_new_stringNode(len + 1);
  json_string_cat(node->value, len, "null");
  return node;
}

static JsonStringNode* json_encode_object(JsonObject* _obj) {
  JsonStringNode* first = NULL;
  JsonStringNode* node = NULL;
  size_t len = 0;

  JsonObjectAttribute* attr = _obj->first;
  while (attr != NULL) {
    JsonStringNode* attrNode = json_encode_objectAttribute(attr);
    len += attrNode->length;
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
      len++;
      node->next = commaNode;
      node = commaNode;
    }
  }

  node = json_merge_stringNode(first, 1, 2);
  node->value[0] = '{';
  node->value[len - 1] = '}';

  return node;
}

static JsonStringNode* json_encode_array(JsonArray* _arr) {
  JsonStringNode* first = NULL;
  JsonStringNode* node = NULL;
  size_t len = 0;

  JsonArrayItem* item = _arr->first;
  while (item != NULL) {
    JsonStringNode* itemNode = json_encode_arrayItem(item);
    len += itemNode->length;
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
      len++;
      node->next = commaNode;
      node = commaNode;
    }
  }

  node = json_merge_stringNode(first, 1, 2);
  node->value[0] = '[';
  node->value[len - 1] = ']';

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
    return json_encode_null((JsonNull*)_val->data);
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
}

static JsonStringNode* json_encode_objectAttribute(JsonObjectAttribute* _attr) {
  JsonStringNode* name = json_encode_string(_attr->name);
  JsonStringNode* colon = json_new_stringNode(1);
  colon->value[0] = ':';
  colon->length++;
  JsonStringNode* value = json_encode_value(_attr->data);
  name->next = colon;
  colon->next = value;
  return name;
}

static JsonStringNode* json_encode_arrayItem(JsonArrayItem* _item) {
  return json_encode_value(_item->data);
}

const char* json_encode(JsonValue* _value) {
  JsonStringNode* node = json_encode_value(_value);
  const char* result = node->value;
  node->value = NULL;
  json_free_stringNode(node);
  return result;
}
