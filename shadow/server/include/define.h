#ifndef SERVER_DEFINED_H
#define SERVER_DEFINED_H

#include <cstdint>

//#include "spdlog/fmt/bundled/core.h"

enum class AppState {
    UNDEFINED = 0,
    INIT,
    RUN,
    PAUSE,
    STOP,
};

enum class ErrCode {
    FAILURE = -1,
    SUCCESS,
    INVALID_PARAM,
    BAD_ALLOC,
};

#endif// SERVER_DEFINED_H
