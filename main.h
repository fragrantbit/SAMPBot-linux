
#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <time.h>
#include "entries.h"

extern bool logdata; 

template<class ...Args>
void _log(char const *format, Args... args) {
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    printf("[%02d:%02d:%02d] ", 
        timeinfo->tm_hour, 
        timeinfo->tm_min, 
        timeinfo->tm_sec);

    printf(format, args...);
    printf("\n");
}

template<class ...Args>
void _datalog(char const *format, Args... args) {
    if(logdata) _log(format, args...);
}

template<class ...Args>
void _datalog(bool emp = false) {
    if(emp)
        if(logdata) printf("\n");
}

#endif