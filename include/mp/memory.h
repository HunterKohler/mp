#ifndef MP_MEMORY_H_
#define MP_MEMORY_H_

#include <stdlib.h>
#include <mp/mp.h>

struct mp_allocator {
    /** @private */
    const struct mp_allocator_interface *_interface;
};

struct mp_allocator_interface {
    void *(*allocate)(struct mp_allocator *self, mp_size bytes,
                      mp_size alignment);

    void (*deallocate)(struct mp_allocator *self, void *p, mp_size bytes,
                       mp_size alignment);

    mp_bool (*is_equal)(const struct mp_allocator *self,
                        const struct mp_allocator *other);
};

void mp_allocator_construct(
    struct mp_allocator *alloc, struct mp_allocator_interface *interface);

void *mp_allocator_allocate(
    struct mp_allocator *alloc, mp_size bytes, mp_size alignment);

void mp_allocator_deallocate(
    struct mp_allocator *alloc, void *p, mp_size bytes, mp_size alignment);

mp_bool mp_allocator_is_equal(
    const struct mp_allocator *a, const struct mp_allocator *b);

const struct mp_allocator_interface *mp_allocator_get_interface(
    const struct mp_allocator *alloc);

struct mp_allocator *mp_null_allocator(void);

struct mp_allocator *mp_stdc_allocator(void);

struct mp_allocator *mp_get_default_allocator(void);

struct mp_allocator *mp_set_default_allocator(struct mp_allocator *new_default);

#endif
