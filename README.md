# SlimJson
C Minimalistic Json Library

I made this JSON library to satisfy a need for a personal project. Currently it has these limitations:
- No UTF8 support
- Numbers are parsed as a double. It's a basic parser from a string like "-123.456" to the double -123.4560000.

# Install

Just copy slim_json.h and slim_json.c into your project and mention the license + copyright

# Usage Patterns

- Load the Json string into a JsonValue
- Use the JsonValue to get the needed data
- Free the JsonValue

```c
char json[] = "{\"a\":{\"1\":[{\"b\":[3,[4],{\"a\":1}]}]},\"b\":[2,{\"str\":\"aaa\"\"q\":true,\"rrr\":null}]}";

// Decoding the JSON string into the JsonValue structure:
JsonValue* v = json_decode(json);
if (v->type == JSON_ERROR) {
    JsonError e = json_get_errorMsg(v);
    printf("%s\n",e.msg);
}

double a = json_get_number(v, "{a}.{1}.0.{b}.0");
char b = json_get_bool(v, "{b}.1.{q}");
char c = json_get_null(v, "{b}.1.{rrr}");
const char* d = json_get_string(v, "{b}.1.{str}");

const JsonObject* obj = json_get_object(v, NULL);
const JsonArray* arr = json_get_object_array(obj, "b");

const JsonObject* obj2 = json_get_array_object(arr, 1);

double z = json_get_array_number(arr, 0);

// You can encode a JsonValue into a json string with:
char* encoded = json_encode(v);
printf("JSON: %s\n", encoded);
free(encoded)

// Free the JsonValue:
json_free(v);

```
