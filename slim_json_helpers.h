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
#include <stdlib.h>

#include <sys/types.h>

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

static size_t json_string_length(const char* _str) {
  size_t l = 0;
  while (_str[0] != '\0') {
    l++;
    _str++;
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

  size_t i = 0;
  while (pos < _len && _src[i] != '\0') {
    _dest[pos++] = _src[i++];
  }

  _dest[_len] = '\0';
}

// skip_escaped: Skip if the character has a \ before it
static ssize_t json_string_indexOf(char _c, const char* _str, size_t _len, unsigned char _skip_escaped) {
  if (_str == NULL || _len == 0) {
    return -1;
  }

  for (int i = 0; i < _len; i++) {
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
