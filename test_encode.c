#include "slim_json.h"
#include "stdio.h"

char test_json_encode() {
  char json[] = "{\"a\":{\"1\":[{\"b\":[3,4]}]},\"b\":[2,{\"q\":true,\"rrr\":null}]}";
  JsonValue* v = json_decode(json, sizeof(json) - 1);
  char r = 1;

  const JsonValue* val = json_get(v, "{a}.{1}.0.{b}.1");
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

  char* encoded = json_encode(v);
  printf("JSON1: %s\n", json);
  printf("JSON2: %s\n", encoded);
  free(encoded);

  json_free(v);

  return r;
}

int main(int argc, const char* argv[])
{
  printf("test_json_encode: %d\n", test_json_encode());
  return 0;
}
