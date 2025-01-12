#pragma once

#ifdef _WIN32
#ifdef SANEENGINE_EXPORTS
#define SANEENGINE_API __declspec(dllexport)
#else
#define SANEENGINE_API __declspec(dllimport)
#endif
#else
#define SANEENGINE_API
#endif