#ifndef PRINTF_H
#define PRINTF_H
#ifdef DEBUG
void printf(char *format, ...);
#else
void printf(char *format, ...);
void printf(char *format, ...){}
#endif
#endif
