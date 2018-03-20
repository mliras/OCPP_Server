/*
 * ocpp_ini_parser.c
 *
 *  Created on: Nov 8, 2017
 *      Author: root
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "ocpp_ini_parser.h"


#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#if !INI_USE_STACK
#include <stdlib.h>
#endif

#define MAX_SECTION 50
#define MAX_NAME 50

/* Used by ini_parse_string() to keep track of string parsing state. */
typedef struct {
    const char* ptr;
    size_t num_left;
} ini_parse_string_ctx;

/* Strip whitespace chars off end of given string, in place. Return s. */
static char* rstrip(char* s)
{
    char* p = s + strlen(s);
    while (p > s && (unsigned char)(*--p)==' ')
        *p = '\0';
    return s;
}

/* Return pointer to first non-whitespace char in given string. */
static char* lskip(const char* s)
{
    while (*s && (unsigned char)(*s)==' ')
        s++;
    return (char*)s;
}

/* Return pointer to first char (of chars) or inline comment in given string,
   or pointer to null at end of string if neither found. Inline comment must
   be prefixed by a whitespace character to register as a comment. */
static char* find_chars_or_comment(const char* s, const char* chars)
{
#if INI_ALLOW_INLINE_COMMENTS
    int was_space = 0;
    while (*s && (!chars || !strchr(chars, *s)) &&
           !(was_space && strchr(INI_INLINE_COMMENT_PREFIXES, *s))) {
        if ((unsigned char)(*s)==' ') was_space=1;
        s++;
    }
#else
    while (*s && (!chars || !strchr(chars, *s))) {
        s++;
    }
#endif
    return (char*)s;
}

/* Version of strncpy that ensures dest (size bytes) is null-terminated. */
static char* strncpy0(char* dest, const char* src, size_t size)
{
    strncpy(dest, src, size);
    dest[size - 1] = '\0';
    return dest;
}

/* See documentation in header file. */
int ini_parse_stream(ini_reader reader, void* stream, ini_handler handler,void* user)
{
    /* Uses a fair bit of stack (use heap instead if you need to) */
#if INI_USE_STACK
    char line[INI_MAX_LINE];
#else
    char* line;
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
    line = (char*)calloc(1,200);
    if (!line) {
        return -2;
    }
#endif

#if INI_HANDLER_LINENO
#define HANDLER(u, s, n, v) handler(u, s, n, v, lineno)
#else
#define HANDLER(u, s, n, v) handler(u, s, n, v)
#endif

    /* Scan through stream line by line */
    while (reader(line, 200, stream) != NULL) {
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

        if (*start == ';' || *start == '#') {
            /* Per Python configparser, allow both ; and # comments at the
               start of a line */
        }
#if INI_ALLOW_MULTILINE
        else if (*prev_name && *start && start > line) {
            /* Non-blank line with leading whitespace, treat as continuation
               of previous name's value (as per Python configparser). */
            if (!HANDLER(user, section, prev_name, start) && !error)
                error = lineno;
        }
#endif
        else if (*start == '[') {
            /* A "[section]" line */
            end = find_chars_or_comment(start + 1, "]");
            if (*end == ']') {
                *end = '\0';
                strncpy0(section, start + 1, sizeof(section));
                *prev_name = '\0';
            }
            else if (!error) {
                /* No ']' found on section line */
                error = lineno;
            }
        }
        else if (*start) {
            /* Not a comment, must be a name[=:]value pair */
            end = find_chars_or_comment(start, "=:");
            if (*end == '=' || *end == ':') {
                *end = '\0';
                name = rstrip(start);
                value = end + 1;
#if INI_ALLOW_INLINE_COMMENTS
                end = find_chars_or_comment(value, NULL);
                if (*end)
                    *end = '\0';
#endif
                value = lskip(value);
                rstrip(value);

                /* Valid name[=:]value pair found, call handler */
                strncpy0(prev_name, name, sizeof(prev_name));
                if (!HANDLER(user, section, name, value) && !error)
                    error = lineno;
            }
            else if (!error) {
                /* No '=' or ':' found on name[=:]value line */
                error = lineno;
            }
        }

#if INI_STOP_ON_FIRST_ERROR
        if (error)
            break;
#endif
    }

#if !INI_USE_STACK
    free(line);
#endif

    return error;
}

/* See documentation in header file. */
int ini_parse_file(FILE* file, ini_handler handler, void* user)
{
    return ini_parse_stream((ini_reader)fgets, file, handler, user);
}

/* See documentation in header file. */
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

/* An ini_reader function to read the next line from a string buffer. This
   is the fgets() equivalent used by ini_parse_string(). */
static char* ini_reader_string(char* str, int num, void* stream) {
    ini_parse_string_ctx* ctx = (ini_parse_string_ctx*)stream;
    const char* ctx_ptr = ctx->ptr;
    size_t ctx_num_left = ctx->num_left;
    char* strp = str;
    char c;

    if (ctx_num_left == 0 || num < 2)
        return NULL;

    while (num > 1 && ctx_num_left != 0) {
        c = *ctx_ptr++;
        ctx_num_left--;
        *strp++ = c;
        if (c == '\n')
            break;
        num--;
    }

    *strp = '\0';
    ctx->ptr = ctx_ptr;
    ctx->num_left = ctx_num_left;
    return str;
}

/* See documentation in header file. */
int ini_parse_string(const char* string, ini_handler handler, void* user) {
    ini_parse_string_ctx ctx;

    ctx.ptr = string;
    ctx.num_left = strlen(string);
    return ini_parse_stream((ini_reader)ini_reader_string, &ctx, handler,user);
}

int handler(void* user, const char* section, const char* name,const char* value)
{
#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

	//PROTOCOL
   if (MATCH("Protocol", "Version")) {
	   modifyConfigurationKey("ProtocolVersion",strndup(value, strlen(value)-1));
   } else   if (MATCH("Protocol", "Name")) {
	   modifyConfigurationKey("ProtocolName",strndup(value, strlen(value)-1));

      //CENTRAL SYSTEM
   } else  if (MATCH("CentralSystem", "VendorIdList")) {
	   modifyConfigurationKey("vendorIdList",strndup(value, strlen(value)-1));
   } else  if (MATCH("CentralSystem", "messageIdList")) {
	   modifyConfigurationKey("messageIdList",strndup(value, strlen(value)-1));


   //CONFIGURATION
   } else  if (MATCH("Configuration", "Heartbeat")) {
	   modifyConfigurationKey("HeartbeatInterval",strndup(value, strlen(value)-1));
   } else  if (MATCH("Configuration", "MaxDataTransferBytes")) {
	   modifyConfigurationKey("MaxDataTransferBytes",strndup(value, strlen(value)-1));
	} else  if (MATCH("Configuration", "LogAlarmLevel")) {
		modifyConfigurationKey("LogAlarmLevel",strndup(value, strlen(value)-1));
	//DATABASE
    } else  if (MATCH("Database", "Server")) {
    	modifyConfigurationKey("DatabaseServer",strndup(value, strlen(value)-1));
	} else  if (MATCH("Database", "Port")) {
		modifyConfigurationKey("DatabasePort",strndup(value, strlen(value)-1));
	} else  if (MATCH("Database", "User")) {
		modifyConfigurationKey("DatabaseUser",strndup(value, strlen(value)-1));
	} else  if (MATCH("Database", "Password")) {
		modifyConfigurationKey("DatabasePassword",strndup(value, strlen(value)-1));

   } else {
	   return 1;
   }

   return 0;
}
