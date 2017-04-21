/*
 * Copyright (c) 2008-2016 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

/*
 * IMPORTANT: This header file describes INTERNAL interfaces to libdispatch
 * which are subject to change in future releases of Mac OS X. Any applications
 * relying on these interfaces WILL break.
 */

#ifndef __DISPATCH_SHIMS_ATOMIC__
#define __DISPATCH_SHIMS_ATOMIC__

#if !__has_extension(c_atomic) || \
		!__has_extension(c_generic_selections) || \
		!__has_include(<stdatomic.h>)
#error libdispatch requires C11 with <stdatomic.h> and generic selections
#endif

#include <stdatomic.h>

#define memory_order_ordered    memory_order_seq_cst
#define memory_order_dependency memory_order_acquire

#if __has_extension(c_generic_selections) && __has_extension(c_atomic)
#define os_atomic(type) _Atomic(type)
#else
#define os_atomic(type) type volatile
#endif

#define _os_atomic_type_cases(type, expr) \
		type *: expr, \
		type volatile *: expr, \
		_Atomic(type) *: expr, \
		_Atomic(type) volatile *: expr

#define _os_atomic_basetypeof(p) \
		typeof(*_Generic((p), \
		_os_atomic_type_cases(char, (char *)(p)), \
		_os_atomic_type_cases(signed char, (signed char *)(p)), \
		_os_atomic_type_cases(unsigned char, (unsigned char *)(p)), \
		_os_atomic_type_cases(short, (short *)(p)), \
		_os_atomic_type_cases(unsigned short, (unsigned short *)(p)), \
		_os_atomic_type_cases(int, (int *)(p)), \
		_os_atomic_type_cases(unsigned int, (unsigned int *)(p)), \
		_os_atomic_type_cases(long, (long *)(p)), \
		_os_atomic_type_cases(unsigned long, (unsigned long *)(p)), \
		_os_atomic_type_cases(long long, (long long *)(p)), \
		_os_atomic_type_cases(unsigned long long, (unsigned long long *)(p)), \
		_os_atomic_type_cases(void *, (void **)(p)), \
		_os_atomic_type_cases(const void *, (const void **)(p)), \
		default: (void**)(p)))

#define _os_atomic_c11_atomic(p) \
		_Generic((p), \
		_os_atomic_type_cases(char, (_Atomic(char)*)(p)), \
		_os_atomic_type_cases(signed char, (_Atomic(signed char)*)(p)), \
		_os_atomic_type_cases(unsigned char, (_Atomic(unsigned char)*)(p)), \
		_os_atomic_type_cases(short, (_Atomic(short)*)(p)), \
		_os_atomic_type_cases(unsigned short, (_Atomic(unsigned short)*)(p)), \
		_os_atomic_type_cases(int, (_Atomic(int)*)(p)), \
		_os_atomic_type_cases(unsigned int, (_Atomic(unsigned int)*)(p)), \
		_os_atomic_type_cases(long, (_Atomic(long)*)(p)), \
		_os_atomic_type_cases(unsigned long, (_Atomic(unsigned long)*)(p)), \
		_os_atomic_type_cases(long long, (_Atomic(long long)*)(p)), \
		_os_atomic_type_cases(unsigned long long, (_Atomic(unsigned long long)*)(p)), \
		_os_atomic_type_cases(void *, (_Atomic(void*)*)(p)), \
		_os_atomic_type_cases(const void *, (_Atomic(const void*)*)(p)), \
		default: (_Atomic(void*)*)(p))

#define os_atomic_thread_fence(m)  atomic_thread_fence(memory_order_##m)
// see comment in dispatch_once.c
#define os_atomic_maximally_synchronizing_barrier() \
		atomic_thread_fence(memory_order_seq_cst)

#define os_atomic_load(p, m) \
		({ _os_atomic_basetypeof(p) _r = \
		atomic_load_explicit(_os_atomic_c11_atomic(p), \
		memory_order_##m); (typeof(*(p)))_r; })
#define os_atomic_store(p, v, m) \
		({ _os_atomic_basetypeof(p) _v = (v); \
		atomic_store_explicit(_os_atomic_c11_atomic(p), _v, \
		memory_order_##m); })
#define os_atomic_xchg(p, v, m) \
		({ _os_atomic_basetypeof(p) _v = (v), _r = \
		atomic_exchange_explicit(_os_atomic_c11_atomic(p), _v, \
		memory_order_##m); (typeof(*(p)))_r; })
#define os_atomic_cmpxchg(p, e, v, m) \
		({ _os_atomic_basetypeof(p) _v = (v), _r = (e); \
		atomic_compare_exchange_strong_explicit(_os_atomic_c11_atomic(p), \
		&_r, _v, memory_order_##m, \
		memory_order_relaxed); })
#define os_atomic_cmpxchgv(p, e, v, g, m) \
		({ _os_atomic_basetypeof(p) _v = (v), _r = (e); _Bool _b = \
		atomic_compare_exchange_strong_explicit(_os_atomic_c11_atomic(p), \
		&_r, _v, memory_order_##m, \
		memory_order_relaxed); *(g) = (typeof(*(p)))_r; _b; })
#define os_atomic_cmpxchgvw(p, e, v, g, m) \
		({ _os_atomic_basetypeof(p) _v = (v), _r = (e); _Bool _b = \
		atomic_compare_exchange_weak_explicit(_os_atomic_c11_atomic(p), \
		&_r, _v, memory_order_##m, \
		memory_order_relaxed); *(g) = (typeof(*(p)))_r;  _b; })

#define _os_atomic_c11_op(p, v, m, o, op) \
		({ _os_atomic_basetypeof(p) _v = (v), _r = \
		atomic_fetch_##o##_explicit(_os_atomic_c11_atomic(p), _v, \
		memory_order_##m); (typeof(*(p)))(_r op _v); })
#define _os_atomic_c11_op_orig(p, v, m, o, op) \
		({ _os_atomic_basetypeof(p) _v = (v), _r = \
		atomic_fetch_##o##_explicit(_os_atomic_c11_atomic(p), _v, \
		memory_order_##m); (typeof(*(p)))_r; })
#define os_atomic_add(p, v, m) \
		_os_atomic_c11_op((p), (v), m, add, +)
#define os_atomic_add_orig(p, v, m) \
		_os_atomic_c11_op_orig((p), (v), m, add, +)
#define os_atomic_sub(p, v, m) \
		_os_atomic_c11_op((p), (v), m, sub, -)
#define os_atomic_sub_orig(p, v, m) \
		_os_atomic_c11_op_orig((p), (v), m, sub, -)
#define os_atomic_and(p, v, m) \
		_os_atomic_c11_op((p), (v), m, and, &)
#define os_atomic_and_orig(p, v, m) \
		_os_atomic_c11_op_orig((p), (v), m, and, &)
#define os_atomic_or(p, v, m) \
		_os_atomic_c11_op((p), (v), m, or, |)
#define os_atomic_or_orig(p, v, m) \
		_os_atomic_c11_op_orig((p), (v), m, or, |)
#define os_atomic_xor(p, v, m) \
		_os_atomic_c11_op((p), (v), m, xor, ^)
#define os_atomic_xor_orig(p, v, m) \
		_os_atomic_c11_op_orig((p), (v), m, xor, ^)

#define os_atomic_force_dependency_on(p, e) (p)
#define os_atomic_load_with_dependency_on(p, e) \
		os_atomic_load(os_atomic_force_dependency_on(p, e), relaxed)
#define os_atomic_load_with_dependency_on2o(p, f, e) \
		os_atomic_load_with_dependency_on(&(p)->f, e)

#define os_atomic_rmw_loop(p, ov, nv, m, ...)  ({ \
		bool _result = false; \
		typeof(p) _p = (p); \
		ov = os_atomic_load(_p, relaxed); \
		do { \
			__VA_ARGS__; \
			_result = os_atomic_cmpxchgvw(_p, ov, nv, &ov, m); \
		} while (os_unlikely(!_result)); \
		_result; \
	})
#define os_atomic_rmw_loop2o(p, f, ov, nv, m, ...) \
		os_atomic_rmw_loop(&(p)->f, ov, nv, m, __VA_ARGS__)
#define os_atomic_rmw_loop_give_up_with_fence(m, expr) \
		({ os_atomic_thread_fence(m); expr; __builtin_unreachable(); })
#define os_atomic_rmw_loop_give_up(expr) \
		os_atomic_rmw_loop_give_up_with_fence(relaxed, expr)

#define os_atomic_load2o(p, f, m) \
		os_atomic_load(&(p)->f, m)
#define os_atomic_store2o(p, f, v, m) \
		os_atomic_store(&(p)->f, (v), m)
#define os_atomic_xchg2o(p, f, v, m) \
		os_atomic_xchg(&(p)->f, (v), m)
#define os_atomic_cmpxchg2o(p, f, e, v, m) \
		os_atomic_cmpxchg(&(p)->f, (e), (v), m)
#define os_atomic_cmpxchgv2o(p, f, e, v, g, m) \
		os_atomic_cmpxchgv(&(p)->f, (e), (v), (g), m)
#define os_atomic_cmpxchgvw2o(p, f, e, v, g, m) \
		os_atomic_cmpxchgvw(&(p)->f, (e), (v), (g), m)
#define os_atomic_add2o(p, f, v, m) \
		os_atomic_add(&(p)->f, (v), m)
#define os_atomic_add_orig2o(p, f, v, m) \
		os_atomic_add_orig(&(p)->f, (v), m)
#define os_atomic_sub2o(p, f, v, m) \
		os_atomic_sub(&(p)->f, (v), m)
#define os_atomic_sub_orig2o(p, f, v, m) \
		os_atomic_sub_orig(&(p)->f, (v), m)
#define os_atomic_and2o(p, f, v, m) \
		os_atomic_and(&(p)->f, (v), m)
#define os_atomic_and_orig2o(p, f, v, m) \
		os_atomic_and_orig(&(p)->f, (v), m)
#define os_atomic_or2o(p, f, v, m) \
		os_atomic_or(&(p)->f, (v), m)
#define os_atomic_or_orig2o(p, f, v, m) \
		os_atomic_or_orig(&(p)->f, (v), m)
#define os_atomic_xor2o(p, f, v, m) \
		os_atomic_xor(&(p)->f, (v), m)
#define os_atomic_xor_orig2o(p, f, v, m) \
		os_atomic_xor_orig(&(p)->f, (v), m)

#define os_atomic_inc(p, m) \
		os_atomic_add((p), 1, m)
#define os_atomic_inc_orig(p, m) \
		os_atomic_add_orig((p), 1, m)
#define os_atomic_inc2o(p, f, m) \
		os_atomic_add2o(p, f, 1, m)
#define os_atomic_inc_orig2o(p, f, m) \
		os_atomic_add_orig2o(p, f, 1, m)
#define os_atomic_dec(p, m) \
		os_atomic_sub((p), 1, m)
#define os_atomic_dec_orig(p, m) \
		os_atomic_sub_orig((p), 1, m)
#define os_atomic_dec2o(p, f, m) \
		os_atomic_sub2o(p, f, 1, m)
#define os_atomic_dec_orig2o(p, f, m) \
		os_atomic_sub_orig2o(p, f, 1, m)

#if defined(__x86_64__) || defined(__i386__)
#undef os_atomic_maximally_synchronizing_barrier
#ifdef __LP64__
#define os_atomic_maximally_synchronizing_barrier() \
		({ unsigned long _clbr; __asm__ __volatile__( \
		"cpuid" \
		: "=a" (_clbr) : "0" (0) : "rbx", "rcx", "rdx", "cc", "memory"); })
#else
#ifdef __llvm__
#define os_atomic_maximally_synchronizing_barrier() \
		({ unsigned long _clbr; __asm__ __volatile__( \
		"cpuid" \
		: "=a" (_clbr) : "0" (0) : "ebx", "ecx", "edx", "cc", "memory"); })
#else // gcc does not allow inline i386 asm to clobber ebx
#define os_atomic_maximally_synchronizing_barrier() \
		({ unsigned long _clbr; __asm__ __volatile__( \
		"pushl	%%ebx\n\t" \
		"cpuid\n\t" \
		"popl	%%ebx" \
		: "=a" (_clbr) : "0" (0) : "ecx", "edx", "cc", "memory"); })
#endif
#endif
#endif // defined(__x86_64__) || defined(__i386__)

#endif // __DISPATCH_SHIMS_ATOMIC__
