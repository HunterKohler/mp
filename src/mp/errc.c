#include <mp/errc.h>

const char *mp_errc_message(enum mp_errc ec)
{
    switch (ec) {
    case MP_ERRC_OK:
        return "no error";
    case MP_ERRC_NOT_ENOUGH_MEMORY:
        return "not enough memory";
    case MP_ERRC_DIVIDE_BY_ZERO:
        return "divide by zero";
    case MP_ERRC_VALUE_TOO_LARGE:
        return "value too large";
    case MP_ERRC_INVALID_ARGUMENT:
        return "invalid argument";
    default:
        return "";
    }
}
