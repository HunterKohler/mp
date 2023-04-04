#include <stdatomic.h>
#include <stdlib.h>
#include <mp/memory.h>

void mp_allocator_construct(
    struct mp_allocator *alloc, struct mp_allocator_interface *interface)
{
    alloc->_interface =
        interface ? interface : mp_get_default_allocator()->_interface;
}

void *mp_allocator_allocate(
    struct mp_allocator *alloc, mp_size bytes, mp_size alignment)
{
    return alloc->_interface->allocate(alloc, bytes, alignment);
}

void mp_allocator_deallocate(
    struct mp_allocator *alloc, void *p, mp_size bytes, mp_size alignment)
{
    alloc->_interface->deallocate(alloc, p, bytes, alignment);
}

mp_bool mp_allocator_is_equal(
    const struct mp_allocator *a, const struct mp_allocator *b)
{
    return a->_interface->is_equal(a, b);
}

const struct mp_allocator_interface *mp_allocator_get_interface(
    const struct mp_allocator *alloc)
{
    return alloc->_interface;
}

static void *mp_null_allocator_allocate(struct mp_allocator *, mp_size, mp_size)
{
    return NULL;
}

static void mp_null_allocator_deallocate(
    struct mp_allocator *, void *, mp_size, mp_size)
{}

static mp_bool mp_null_allocator_is_equal(
    const struct mp_allocator *self, const struct mp_allocator *other)
{
    return self == other;
}

static const struct mp_allocator_interface mp_null_allocator_interface = {
    .allocate = mp_null_allocator_allocate,
    .deallocate = mp_null_allocator_deallocate,
    .is_equal = mp_null_allocator_is_equal,
};

static struct mp_allocator mp_null_allocator_value = {
    &mp_null_allocator_interface
};

struct mp_allocator *mp_null_allocator(void)
{
    return &mp_null_allocator_value;
}

static void *mp_stdc_allocator_allocate(
    struct mp_allocator *, mp_size bytes, mp_size alignment)
{
    return aligned_alloc(alignment, bytes);
}

static void mp_stdc_allocator_deallocate(
    struct mp_allocator *, void *p, mp_size, mp_size)
{
    free(p);
}

static mp_bool mp_stdc_allocator_is_equal(
    const struct mp_allocator *self, const struct mp_allocator *other)
{
    return self->_interface == other->_interface;
}

static const struct mp_allocator_interface mp_stdc_allocator_interface = {
    .allocate = mp_stdc_allocator_allocate,
    .deallocate = mp_stdc_allocator_deallocate,
    .is_equal = mp_stdc_allocator_is_equal,
};

static struct mp_allocator mp_stdc_allocator_value = {
    ._interface = &mp_stdc_allocator_interface
};

struct mp_allocator *mp_stdc_allocator(void)
{
    return &mp_stdc_allocator_value;
}

static struct mp_allocator *_Atomic mp_default_allocator =
    &mp_stdc_allocator_value;

struct mp_allocator *mp_get_default_allocator(void)
{
    return atomic_load(&mp_default_allocator);
}

struct mp_allocator *mp_set_default_allocator(struct mp_allocator *new_default)
{
    if (!new_default) {
        new_default = mp_stdc_allocator();
    }

    return atomic_exchange(&mp_default_allocator, new_default);
}
