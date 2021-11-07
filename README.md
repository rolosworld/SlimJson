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
char jsonString[] = "{\"a\":{\"1\":[{\"b\":[3,4]}]},\"b\":[2,{\"q\":true,\"rrr\":null}]}";

// Decoding the JSON string into the JsonValue structure:
JsonValue* v = json_decode(jsonString);
if (v->type == JSON_ERROR) {
    JsonError e = json_get_errorMsg(v);
    printf("%s\n",e.msg);
}

/*
Getting a value from the JsonValue structure. Notice I passed a weird looking string. This string helps selecting the desired value.

The rules for the search string are:
- Encapsulate attribute names between {}
- Array indexes don't need encapsulation
- Separate with a dot '.'

In this example we want to:
- Get the value on the attribuete "a"
- Then get the attribute "1"
- Then get the index 0
- Then get the attribute "b"
- Then get the index 1

So, json_get will return the value on index 1 which is 4.
*/
const JsonValue* val = json_get(v, "{a}.{1}.0.{b}.1");

// Verify what type of value it is:
if (val->type == JSON_NUMBER) {
    // val->data is void* so we must assign the proper data type:
    JsonNumber* n = (JsonNumber*)val->data;
    printf("%lf", n->value);
}

// Free the JsonValue:
json_free(v);

```
