/*
 * Copyright (c) 2016 Mieszko Mazurek
 */

#ifndef __LOG_H
#define __LOG_H
#ifdef __USE_LOG
#  include <stdio.h>
#  include <string.h>
#  ifndef LOG_TAG
#    define LOG_TAG (strchr(__FILE__, '/') ? strrchr(__FILE__, '/') : __FILE__)
#  endif
#  define log_i(...) printf("INFO %s: ", LOG_TAG); printf(__VA_ARGS__); printf("\n");
#  define log_d(...) printf("DEBUG %s: ", LOG_TAG); printf(__VA_ARGS__); printf("\n");
#  define log_e(...) printf("ERROR %s: ", LOG_TAG); printf(__VA_ARGS__); printf("\n");
#else
#  define log_i(...)
#  define log_d(...)
#  define log_e(...)
#endif
#endif
