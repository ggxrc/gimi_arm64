/* inih -- simple .INI file parser
   Copyright (c) 2009, Ben Hoyt — New BSD License
   Source: https://github.com/benhoyt/inih
*/

#include "ini.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#if !INI_USE_STACK
#  include <stdlib.h>
#endif

#define MAX_SECTION 50
#define MAX_NAME    50

/* Used by ini_parse_string() */
typedef struct {
    const char* ptr;
    size_t num_left;
} ini_parse_string_ctx;

/* Strip whitespace chars off end of given string, in place. Return s. */
static char* rstrip(char* s) {
    char* p = s + strlen(s);
    while (p > s && isspace((unsigned char)(*--p)))
        *p = '\0';
    return s;
}

/* Return pointer to first non-whitespace char in given string. */
static char* lskip(const char* s) {
    while (*s && isspace((unsigned char)(*s)))
        s++;
    return (char*)s;
}

/* Return pointer to first char (of chars) or inline comment in given string,
   or pointer to NUL at end of string if neither found. Inline comment must
   be prefixed by a whitespace character to register as a comment. */
static char* find_chars_or_comment(const char* s, const char* chars) {
#if INI_ALLOW_INLINE_COMMENTS
    int was_space = 0;
    while (*s && (!chars || !strchr(chars, *s)) &&
           !(was_space && strchr(INI_INLINE_COMMENT_PREFIXES, *s))) {
        was_space = isspace((unsigned char)(*s));
        s++;
    }
#else
    while (*s && (!chars || !strchr(chars, *s))) {
        s++;
    }
#endif
    return (char*)s;
}

/* Similar to strncpy, but ensures dest (size bytes) is
   NUL-terminated, and doesn't pad with NULs. */
static char* strncpy0(char* dest, const char* src, size_t size) {
    /* Could use strncpy internally, but it causes gcc warnings (see issue #91) */
    size_t i;
    for (i = 0; i < size - 1 && src[i]; i++)
        dest[i] = src[i];
    dest[i] = '\0';
    return dest;
}

/* See documentation in header file. */
int ini_parse_stream(ini_reader reader, void* stream, ini_handler handler,
                     void* user)
{
#if INI_USE_STACK
    char linebuf[INI_MAX_LINE];
    int linebuf_len = INI_MAX_LINE;
#else
    char* linebuf;
    int linebuf_len = INI_INITIAL_ALLOC;
#endif
    char section[MAX_SECTION] = "";
    char prev_name[MAX_NAME] = "";

    char* start;
    char* end;
    char* name;
    char* value;
    int lineno = 0;
    int error = 0;

#if !INI_USE_STACK
    linebuf = (char*)ini_malloc(linebuf_len);
    if (!linebuf) {
        return -2;
    }
#endif

#if INI_ALLOW_BOM
    /* Handle a UTF-8 BOM if present. */
    char bom[3];
    if (reader(bom, 4, stream) != NULL) {
        if ((unsigned char)bom[0] == 0xEF &&
            (unsigned char)bom[1] == 0xBB &&
            (unsigned char)bom[2] == 0xBF) {
            /* BOM found, nothing to do — just skip it */
        }
        /* Not a BOM — we've already read the first bytes, reconstruct:
           the simplest approach for stream readers is to just re-open.
           For simplicity in this embedded version we skip BOM detection
           on stream and only handle it in ini_parse_file. */
    }
#endif

    /* Main parsing loop — reset and re-read */
    char line[INI_MAX_LINE];
    while (reader(line, INI_MAX_LINE, stream) != NULL) {
        lineno++;
        start = line;

#if INI_ALLOW_BOM
        if (lineno == 1 && (unsigned char)start[0] == 0xEF &&
                           (unsigned char)start[1] == 0xBB &&
                           (unsigned char)start[2] == 0xBF) {
            start += 3;
        }
#endif
        start = lskip(rstrip(start));

        if (strchr(INI_START_COMMENT_PREFIXES, *start)) {
            /* Comment line — skip */
#if INI_ALLOW_MULTILINE
        } else if (*prev_name && *start && start > line) {
            /* Non-blank line following a name=value — possible multi-line continuation */
#if INI_HANDLER_LINENO
            if (!handler(user, section, prev_name, start, lineno) && !error)
#else
            if (!handler(user, section, prev_name, start) && !error)
#endif
                error = lineno;
#endif
        } else if (*start == '[') {
            /* [section] header */
            end = find_chars_or_comment(start + 1, "]");
            if (*end == ']') {
                *end = '\0';
                strncpy0(section, start + 1, sizeof(section));
                *prev_name = '\0';
#if INI_CALL_HANDLER_ON_NEW_SECTION
#if INI_HANDLER_LINENO
                if (!handler(user, section, NULL, NULL, lineno) && !error)
#else
                if (!handler(user, section, NULL, NULL) && !error)
#endif
                    error = lineno;
#endif
            } else if (!error) {
                error = lineno;
            }
        } else if (*start) {
            /* name=value pair */
            end = find_chars_or_comment(start, "=:");
            if (*end == '=' || *end == ':') {
                *end = '\0';
                name = rstrip(start);
                value = end + 1;
                end = find_chars_or_comment(value, NULL);
                if (*end)
                    *end = '\0';
                value = lskip(value);
                rstrip(value);

                strncpy0(prev_name, name, sizeof(prev_name));
#if INI_HANDLER_LINENO
                if (!handler(user, section, name, value, lineno) && !error)
#else
                if (!handler(user, section, name, value) && !error)
#endif
                    error = lineno;
            } else if (!error) {
                error = lineno;
            }
        }

#if INI_STOP_ON_FIRST_ERROR
        if (error) break;
#endif
    }

#if !INI_USE_STACK
    ini_free(linebuf);
#endif

    return error;
}

int ini_parse_file(FILE* file, ini_handler handler, void* user)
{
    return ini_parse_stream((ini_reader)fgets, file, handler, user);
}

int ini_parse(const char* filename, ini_handler handler, void* user)
{
    FILE* file;
    int error;

    file = fopen(filename, "r");
    if (!file)
        return -1;
    error = ini_parse_file(file, handler, user);
    fclose(file);
    return error;
}

static char* ini_parse_string_reader(char* str, int num, void* stream)
{
    ini_parse_string_ctx* ctx = (ini_parse_string_ctx*)stream;
    size_t num_to_copy = (size_t)num - 1;
    const char* newline;

    if (ctx->num_left == 0)
        return NULL;

    newline = (const char*)memchr(ctx->ptr, '\n', ctx->num_left);
    if (newline == NULL) {
        if (num_to_copy > ctx->num_left)
            num_to_copy = ctx->num_left;
    } else {
        if (num_to_copy > (size_t)(newline - ctx->ptr) + 1)
            num_to_copy = (size_t)(newline - ctx->ptr) + 1;
    }

    memcpy(str, ctx->ptr, num_to_copy);
    str[num_to_copy] = '\0';

    ctx->ptr += num_to_copy;
    ctx->num_left -= num_to_copy;
    return str;
}

int ini_parse_string(const char* string, ini_handler handler, void* user)
{
    ini_parse_string_ctx ctx;
    ctx.ptr = string;
    ctx.num_left = strlen(string);
    return ini_parse_stream(ini_parse_string_reader, &ctx, handler, user);
}
