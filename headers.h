#pragma once
#define no_init_all
#define _CRT_SECURE_NO_WARNINGS

#if !defined PROJECT_NAME
#define PROJECT_NAME ""
#endif

#include <Windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <tlHelp32.h>
#include <map>

#include "memory.h"
#include "sdk.h"