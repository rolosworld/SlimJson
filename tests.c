#include "slim_json.h"
#include "stdio.h"

#define DEBUG 0
char test_1() {
  char json6[] = "{\"a\":\"1\",\"b\":2}";
  JsonValue* v = json_decode(json6);
  const JsonObject* o = json_get_object(v, NULL);
  char o1 = json_get_object_string(o, "a")[0] == '1';
  o1 = o1 && (float)json_get_object_number(o, "b") == (float)2.0;
  o1 = o1 && (float)json_get_number(v, "{b}") == (float)2.0;

  json_free(v);

  return o1;
}

char test_2() {
  char json6[] = "{\"a\":\"1\"}";
  JsonValue* v = json_decode(json6);
  const JsonObject* o = json_get_object(v, NULL);
  char o1 = json_get_object_string(o, "a")[0] == '1';

  const char* n = json_get_string(v, "{a}");
  o1 = o1 && n[0] == '1';

  json_free(v);

  return o1;
}

char test_3() {
  char json6[] = "[\"a\",\"1\"]";
  JsonValue* v = json_decode(json6);

  const JsonArray* o = json_get_array(v, NULL);
  char o1 = json_get_array_string(o, 0)[0] == 'a';
  o1 = o1 && json_get_array_string(o, 1)[0] == '1';
  o1 = o1 && json_get_string(v,"0")[0] == 'a';
  o1 = o1 && json_get_string(v,"1")[0] == '1';

  json_free(v);

  return o1;
}

char test_4() {
  char a = 1;

  char J1[] = "    a";
  JsonValue* j = json_decode(J1);
  a = a && j->type == JSON_ERROR;
#if DEBUG == 1
  if (j->type == JSON_ERROR) {
    JsonError e = json_get_errorMsg(j);
    printf("%s\n",e.msg);
  }
#endif
  json_free(j);

  char J2[] = "    {\"a\":1}  ";
  j = json_decode(J2);
  a = a && j->type != JSON_ERROR;
  json_free(j);

  char J3[] = "    {\"a\":1,}  ";
  j = json_decode(J3);
  a = a && j->type != JSON_ERROR;
  json_free(j);

  char J4[] = "    {\"a\":z,}  ";
  j = json_decode(J4);
  a = a && j->type == JSON_ERROR;
#if DEBUG == 1
  if (j->type == JSON_ERROR) {
    JsonError e = json_get_errorMsg(j);
    printf("%s\n",e.msg);
  }
#endif
  json_free(j);

  return a;
}

char test_5() {
  char json[] = "{\"a\":{\"1\":[{\"b\":[3,4]}]},\"b\":[2,{\"q\":true,\"rrr\":null}]}";
  JsonValue* v = json_decode(json);
  char r = 1;

  r = r && json_get_number(v, "{a}.{1}.0.{b}.1") == 4;
  r = r && json_get_bool(v, "{b}.1.{q}") == 1;
  r = r && json_get_null(v, "{b}.1.{rrr}") == 0;

  json_free(v);

  return r;
}

char test_6() {
  char json[] = "{\"a\":{\"1\":[{\"b\":[3,4]}]},\"b\":[2,{\"q\":true,\"rrr\":null}]}";
  JsonValue* v = json_decode(json);
  char r = 1;

  r = r && json_get_number(v, "{a}.{1}.0.{b}.1") == 4;
  r = r && json_get_bool(v, "{b}.1.{q}") == 1;
  r = r && json_get_null(v, "{b}.1.{rrr}") == 0;

  char* encoded = json_encode(v);
#if DEBUG == 1
  printf("JSON1: %s\n", json);
  printf("JSON2: %s\n", encoded);
#endif
  free(encoded);

  json_free(v);

  return r;
}

int main()
{
  printf("test_1: %d\n", test_1());
  printf("test_2: %d\n", test_2());
  printf("test_3: %d\n", test_3());
  printf("test_4: %d\n", test_4());
  printf("test_5: %d\n", test_5());
  printf("test_6: %d\n", test_6());

  return 0;
}

