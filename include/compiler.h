#ifndef __COMPILER_H__
#define __COMPILER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define WEAK		__attribute__((weak))

#define UNUSED		__attribute__((unused))

#define USED		__attribute__((used))

#define ALIGNED(x)	__attribute__(aligned(x))

#define CONSTRUCTOR __attribute__((constructor)) // prio is 65535

#define DESTRUCTOR	__attribute__((destructor)) // prio is 65535

/* 0 - 100 is reserved */
#define CONSTRUCTOR_PRIO(x) __attribute__((constructor(x)))

#define DESTRUCTOR_PRIO(x)	__attribute__((destructor(x)))

#define SECTION(x)			__attribute__((section(x)))

#define PREINIT_ARRAY		SECTION(".preinit_array")

#define PREINIT(x)			static void *PREINIT##x PREINIT_ARRAY UNUSED = (void *)(x);

#ifdef __cplusplus
}
#endif

#endif