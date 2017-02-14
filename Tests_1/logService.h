#ifndef LOGSERVICE_H
#define LOGSERVICE_H

#include <mutex>

#include "typeDefs.h"
#include "Stream.h"

#define TITLE(x) SetWindowText(GetConsoleWindow(),x)

static std::mutex _consoleMutex;

void __stdcall LOG(STRING format, ...);

#endif