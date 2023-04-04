#ifndef MP_ERRC_H_
#define MP_ERRC_H_

enum mp_errc {
    MP_ERRC_OK,
    MP_ERRC_NOT_ENOUGH_MEMORY,
    MP_ERRC_DIVIDE_BY_ZERO,
    MP_ERRC_VALUE_TOO_LARGE,
};

const char *mp_errc_message(enum mp_errc ec);

#endif
