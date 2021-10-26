#include <stdio.h>
#include <stdlib.h>
#include "slim_json.h"
#include "tests_slim_json.h"

int main(int argc, const char* argv[])
{
  printf("test_json_string_indexOf: %d\n", test_json_string_indexOf());
  printf("test_json_string_ltrim: %d\n", test_json_string_ltrim());
  printf("test_json_parse_string: %d\n", test_json_parse_string());
  printf("test_json_is_digit: %d\n", test_json_is_digit());
  printf("test_json_parse_number: %d\n", test_json_parse_number());
  printf("test_json_parse_bool: %d\n", test_json_parse_bool());
  printf("test_json_parse_value: %d\n", test_json_parse_value());
  printf("test_json_parse_value_object: %d\n", test_json_parse_value_object());
  printf("test_json_parse_objectAttribute: %d\n", test_json_parse_objectAttribute());
  printf("test_json_parse_object: %d\n", test_json_parse_object());
  printf("test_json_parse_array: %d\n", test_json_parse_array());
  printf("test_json_parse_arrayItem: %d\n", test_json_parse_arrayItem());
  printf("test_json_decode: %d\n", test_json_decode());
  printf("test_json_equal_strings: %d\n", test_json_equal_strings());

  return 0;
}
