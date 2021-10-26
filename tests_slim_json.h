#ifndef __TESTS_SLIM_JSON_H__
#define  __TESTS_SLIM_JSON_H__

#include "slim_json.h"

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

char test_json_string_ltrim() {
  char t[] = " \n\t\r Hello World!\n\\\"\"";
  size_t len = sizeof(t) - 1;
  JsonStream e;
  e.current = t;
  e.length = len;
  json_string_ltrim(&e);

  return e.current[0] == 'H';
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

char test_json_parse_bool() {
  char b1[] = " true ";
  size_t len = sizeof(b1) - 1;
  JsonStream e;
  e.current = b1;
  e.length = len;
  JsonBool* B1 = json_parse_bool(&e);
  char v1 = B1->value == 1;
  free(B1);

  char b2[] = " false ";
  len = sizeof(b2) - 1;
  e.current = b2;
  e.length = len;
  JsonBool* B2 = json_parse_bool(&e);
  char v2 = B2->value == 0;
  free(B2);

  return v1 && v2;
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


char test_json_parse_value() {
  char json[] = "\"FooBar\"";
  JsonStream* s = json_stream(json, sizeof(json) - 1);
  JsonValue* v = json_parse_value(s);
  char r1 = v->type == JSON_STRING;
  JsonString* r = (JsonString*)v->data;
  char r2 = r->value[0] == 'F' && r->value[5] == 'r' && r->value[6] == '\0';
  char r3 = r->length == 6;
  free(s);
  json_free(v);

  char json2[] = "1023";
  s = json_stream(json2, sizeof(json2) - 1);
  v = json_parse_value(s);
  char n1 = v->type == JSON_NUMBER;
  JsonNumber* n = (JsonNumber*)v->data;
  char n2 = n->value == 1023;
  free(s);
  json_free(v);

  char json3[] = "false";
  s = json_stream(json3, sizeof(json3) - 1);
  v = json_parse_value(s);
  char b1 = v->type == JSON_BOOL;
  JsonBool* b = (JsonBool*)v->data;
  char b2 = b->value == 0;
  free(s);
  json_free(v);

  char json4[] = "true";
  s = json_stream(json4, sizeof(json4) - 1);
  v = json_parse_value(s);
  char ba1 = v->type == JSON_BOOL;
  JsonBool* ba = (JsonBool*)v->data;
  char ba2 = ba->value == 1;
  free(s);
  json_free(v);

  char json5[] = "null";
  s = json_stream(json5, sizeof(json5) - 1);
  v = json_parse_value(s);
  char nu1 = v->type == JSON_NULL;
  JsonNull* nu = (JsonNull*)v->data;
  char nu2 = nu->value == 0;
  free(s);
  json_free(v);

  return r1 && r2 &&
      n1 && n2 &&
      b1 && b2 &&
      ba1 && ba2 &&
      nu1 && nu2;
}

char test_json_parse_value_object() {
  char json6[] = "{\"a\":\"1\",\"b\":2}";
  JsonStream* s = json_stream(json6, sizeof(json6) - 1);
  JsonValue* v = json_parse_value(s);
  char o1 = v->type == JSON_OBJECT;

  JsonObject* o = (JsonObject*)v->data;

  JsonObjectAttribute* oa = o->first;
  JsonString* oas = oa->name;
  char o2 = oas->value[0] == 'a' && oas->length == 1;
  JsonValue* od = (JsonValue*)oa->data;
  char o4 = od->type == JSON_STRING;
  JsonString* ods = (JsonString*)od->data;
  char o3 = ods->value[0] == '1' && ods->length == 1;

  oa = oa->next;
  oas = oa->name;
  o2 = o2 && oas->value[0] == 'b' && oas->length == 1;
  od = (JsonValue*)oa->data;
  o4 = o4 && od->type == JSON_NUMBER;
  JsonNumber* num = (JsonNumber*)od->data;
  o3 = o3 && ((float)num->value == (float)2.0);

  free(s);
  json_free(v);

  return o1 && o2 && o3 && o4;
}

char test_json_parse_object() {
  char json6[] = "{\"a\":\"1\"}";
  JsonStream* s = json_stream(json6, sizeof(json6) - 1);
  JsonObject* o = json_parse_object(s);
  JsonObjectAttribute* oa = o->first;
  JsonString* oas = oa->name;
  char o2 = oas->value[0] == 'a' && oas->length == 1;
  JsonValue* od = (JsonValue*)oa->data;
  char o4 = od->type == JSON_STRING;
  JsonString* ods = (JsonString*)od->data;
  char o3 = ods->value[0] == '1' && ods->length == 1;

  oa = json_get_objectAttribute(o, "a", 1);
  oas = oa->name;
  o2 = o2 && oas->value[0] == 'a' && oas->length == 1;
  od = (JsonValue*)oa->data;
  o4 = o4 && od->type == JSON_STRING;
  ods = (JsonString*)od->data;
  o3 = o3 && ods->value[0] == '1' && ods->length == 1;

  free(s);
  json_free_object(o);

  return o2 && o3 && o4;
}

char test_json_parse_objectAttribute() {
  char json6[] = "\"a\":\"1\"";
  JsonStream* s = json_stream(json6, sizeof(json6) - 1);
  JsonObjectAttribute* oa = json_parse_objectAttribute(s);
  JsonString* oas = oa->name;
  char o2 = oas->value[0] == 'a' && oas->length == 1;
  JsonValue* od = (JsonValue*)oa->data;
  char o4 = od->type == JSON_STRING;
  JsonString* ods = (JsonString*)od->data;
  char o3 = ods->value[0] == '1' && ods->length == 1;

  free(s);
  json_free_objectAttribute(oa);

  return o2 && o3 && o4;
}

char test_json_parse_array() {
  char json6[] = "[\"a\",\"1\"]";
  JsonStream* s = json_stream(json6, sizeof(json6) - 1);
  JsonArray* o = json_parse_array(s);
  JsonArrayItem* oa = o->first;
  JsonValue* oas = oa->data;
  char o1 = oas->type == JSON_STRING;
  JsonString* oas1 = (JsonString*)oas->data;
  char o2 = oas1->value[0] == 'a' && oas1->length == 1;
  oa = oa->next;
  oas = (JsonValue*)oa->data;
  oas1 = (JsonString*)oas->data;
  char o3 = oas1->value[0] == '1' && oas1->length == 1;
  char o4 = oas->type == JSON_STRING;

  oa = json_get_arrayItem(o, 0);
  oas = (JsonValue*)oa->data;
  oas1 = (JsonString*)oas->data;
  o2 = o2 && oas1->value[0] == 'a' && oas1->length == 1;

  oa = json_get_arrayItem(o, 1);
  oas = (JsonValue*)oa->data;
  oas1 = (JsonString*)oas->data;
  o2 = o2 && oas1->value[0] == '1' && oas1->length == 1;

  free(s);
  json_free_array(o);

  return o1 && o2 && o3 && o4;
}

char test_json_parse_arrayItem() {
  char json6[] = "\"a\"";
  JsonStream* s = json_stream(json6, sizeof(json6) - 1);
  JsonArrayItem* oa = json_parse_arrayItem(s);
  JsonValue* oas = oa->data;
  char o1 = oas->type == JSON_STRING;
  JsonString* oas1 = (JsonString*)oas->data;
  char o2 = oas1->value[0] == 'a' && oas1->length == 1;

  free(s);
  json_free_arrayItem(oa);

  return o1 && o2;
}

char test_json_decode() {
  char a = 1;

  char J1[] = "    a";
  JsonValue* j = json_decode(J1, sizeof(J1) - 1);
  a = a && j->type == JSON_ERROR;
  if (j->type == JSON_ERROR) {
    json_print_error(j);
  }
  json_free(j);

  char J2[] = "    {\"a\":1}  ";
  j = json_decode(J2, sizeof(J2) - 1);
  a = a && j->type != JSON_ERROR;
  json_free(j);

  char J3[] = "    {\"a\":1,}  ";
  j = json_decode(J3, sizeof(J3) - 1);
  a = a && j->type != JSON_ERROR;
  json_free(j);

  char J4[] = "    {\"a\":z,}  ";
  j = json_decode(J4, sizeof(J4) - 1);
  a = a && j->type == JSON_ERROR;
  if (j->type == JSON_ERROR) {
    json_print_error(j);
  }
  json_free(j);

  return a;
}

char test_json_equal_strings() {
  char a = 1;
  a = a && json_equal_strings("1234", 4, "1234", 4);
  a = a && !json_equal_strings("1235", 4, "1234", 4);
  a = a && !json_equal_strings("123", 3, "1234", 4);
  a = a && !json_equal_strings("", 0, "1234", 4);

  return a;
}

#endif /* __TESTS_SLIM_JSON_H__ */
