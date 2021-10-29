#include "slim_json.h"

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

  JsonValue* val = json_get(v, "\"b\"");
  o4 = o4 && val->type == JSON_NUMBER;
  num = (JsonNumber*)val->data;
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

char test_json_get() {
  char json[] = "{\"a\":{\"1\":[{\"b\":[3,4]}]},\"b\":[2,{\"q\":true,\"rrr\":null}]}";
  JsonValue* v = json_decode(json, sizeof(json) - 1);
  char r = 1;

  JsonValue* val = json_get(v, "{a}.{1}.0.{b}.1");
  r = r && val->type == JSON_NUMBER;
  JsonNumber* n = (JsonNumber*)val->data;
  r = r && n->value == 4;

  val = json_get(v, "{b}.1.{q}");
  r = r && val->type == JSON_BOOL;
  JsonBool* b = (JsonBool*)val->data;
  r = r && b->value == 1;

  val = json_get(v, "{b}.1.{rrr}");
  r = r && val->type == JSON_NULL;
  JsonNull* nu = (JsonNull*)val->data;
  r = r && nu->value == 0;

  json_free(v);

  return r;
}

int main(int argc, const char* argv[])
{
  printf("test_json_parse_value_object: %d\n", test_json_parse_value_object());
  printf("test_json_parse_object: %d\n", test_json_parse_object());
  printf("test_json_parse_array: %d\n", test_json_parse_array());
  printf("test_json_decode: %d\n", test_json_decode());
  printf("test_json_get: %d\n", test_json_get());

  return 0;
}
