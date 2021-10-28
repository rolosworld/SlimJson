#include "slim_json.h"

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

char test_json_parse_value_object() {
  char json6[] = "{\"a\":\"1\",\"b\":2}";
  JsonValue* v = json_decode(json6, sizeof(json6) - 1);
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

  json_free(v);

  return o1 && o2 && o3 && o4;
}

char test_json_parse_object() {
  char json6[] = "{\"a\":\"1\"}";
  JsonValue* v = json_decode(json6, sizeof(json6) - 1);
  JsonObject* o = (JsonObject*)v->data;
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

  json_free(v);

  return o2 && o3 && o4;
}

char test_json_parse_array() {
  char json6[] = "[\"a\",\"1\"]";
  JsonValue* v = json_decode(json6, sizeof(json6) - 1);
  JsonArray* o = (JsonArray*)v->data;

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

  JsonValue* val = json_get(v, "0");
  o1 = o1 && val->type == JSON_STRING;
  oas1 = (JsonString*)val->data;
  o2 = o2 && oas1->value[0] == 'a' && oas1->length == 1;

  val = json_get(v, "1");
  o1 = o1 && val->type == JSON_STRING;
  oas1 = (JsonString*)val->data;
  o2 = o2 && oas1->value[0] == '1' && oas1->length == 1;

  json_free(v);

  return o1 && o2 && o3 && o4;
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

int main(int argc, const char* argv[])
{
  printf("test_json_parse_string: %d\n", test_json_parse_string());
  printf("test_json_parse_number: %d\n", test_json_parse_number());
  printf("test_json_parse_bool: %d\n", test_json_parse_bool());
  printf("test_json_parse_value_object: %d\n", test_json_parse_value_object());
  printf("test_json_parse_object: %d\n", test_json_parse_object());
  printf("test_json_parse_array: %d\n", test_json_parse_array());
  printf("test_json_decode: %d\n", test_json_decode());

  return 0;
}
