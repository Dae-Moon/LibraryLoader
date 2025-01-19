#ifndef CONSOLE_H
#define CONSOLE_H

extern PColor24 Console_TextColor;

extern bool Console_InitializeColor(void* gameConsole);
extern void Console_PrintColor(byte R, byte G, byte B, const char* fmt, ...);
extern void Console_PrintSuccess(const char* fmt, ...);
extern void Console_PrintLog(const char* fmt, ...);
extern void Console_PrintWarning(const char* fmt, ...);
extern void Console_PrintError(const char* fmt, ...);

#endif CONSOLE_H
