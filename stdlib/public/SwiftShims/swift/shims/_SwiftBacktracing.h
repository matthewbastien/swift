//===--- _SwiftBacktracing.h - Swift Backtracing Support --------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2023 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
//  Defines types and support functions for the Swift backtracing code.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_BACKTRACING_H
#define SWIFT_BACKTRACING_H

#include "SwiftStdbool.h"
#include "SwiftStddef.h"
#include "SwiftStdint.h"
#include "Target.h"

// Replace the stdint types for the rest of this file; we have to use the
// preprocessor for this, because e.g. __swift_int8_t might be a typedef of
// int8_t, and so we can't re-typedef int8_t.
//
// Mainly we're doing this so CrashInfo.h can use the normal type names, but
// it also avoids a lot of underscores later on in this file.
#define int8_t __swift_int8_t
#define uint8_t __swift_uint8_t
#define int16_t __swift_int16_t
#define uint16_t __swift_uint16_t
#define int32_t __swift_int32_t
#define uint32_t __swift_uint32_t
#define int64_t __swift_int64_t
#define uint64_t __swift_uint64_t
#define intptr_t __swift_intptr_t
#define uintptr_t __swift_uintptr_t

#include "CrashInfo.h"

#ifdef __cplusplus
namespace swift {
extern "C" {
#endif

// .. C library ................................................................

static inline __swift_size_t _swift_backtrace_strlen(const char *s) {
  extern __swift_size_t strlen(const char *);
  return strlen(s);
}

void *_swift_backtrace_dlopen_lazy(const char *path);
void *_swift_backtrace_dlsym(void *handle, const char *name);

typedef struct __swift_backtrace_FILE __swift_backtrace_FILE;

static inline __swift_backtrace_FILE *_swift_backtrace_fopen(const char *fname,
                                                             const char *mode) {
  extern __swift_backtrace_FILE *fopen(const char *, const char *);
  return fopen(fname, mode);
}
static inline int _swift_backtrace_fclose(__swift_backtrace_FILE *fp) {
  extern int fclose(__swift_backtrace_FILE *);
  return fclose(fp);
}
static inline int _swift_backtrace_feof(__swift_backtrace_FILE *fp) {
  extern int feof(__swift_backtrace_FILE *);
  return feof(fp);
}
static inline int _swift_backtrace_ferror(__swift_backtrace_FILE *fp) {
  extern int ferror(__swift_backtrace_FILE *);
  return ferror(fp);
}
static inline char *_swift_backtrace_fgets(char *buffer, int size,
                                           __swift_backtrace_FILE *fp) {
  extern char *fgets(char *, int, __swift_backtrace_FILE *);
  return fgets(buffer, size, fp);
}

// .. Core Foundation ..........................................................

#if SWIFT_TARGET_OS_DARWIN

#if __LLP64__
typedef long long __swift_backtrace_CFIndex;
#else
typedef long __swift_backtrace_CFIndex;
#endif

struct __swift_backtrace_CFRange {
  __swift_backtrace_CFIndex location;
  __swift_backtrace_CFIndex length;
 };

typedef struct {
  uint8_t byte0;
  uint8_t byte1;
  uint8_t byte2;
  uint8_t byte3;
  uint8_t byte4;
  uint8_t byte5;
  uint8_t byte6;
  uint8_t byte7;
  uint8_t byte8;
  uint8_t byte9;
  uint8_t byte10;
  uint8_t byte11;
  uint8_t byte12;
  uint8_t byte13;
  uint8_t byte14;
  uint8_t byte15;
} __swift_backtrace_CFUUIDBytes;

#endif // SWIFT_TARGET_OS_DARWIN

// .. Processor specifics ......................................................

struct x86_64_gprs {
  uint64_t _r[16];
  uint64_t rflags;
  uint16_t cs, fs, gs, _pad0;
  uint64_t rip;
  uint64_t valid;
};

struct i386_gprs {
  uint32_t _r[8];
  uint32_t eflags;
  uint16_t segreg[6];
  uint32_t eip;
  uint32_t valid;
};

struct arm64_gprs {
  uint64_t _x[32];
  uint64_t pc;
  uint64_t valid;
};

struct arm_gprs {
  uint32_t _r[16];
  uint32_t valid;
};

// .. Darwin specifics .........................................................

#if SWIFT_TARGET_OS_DARWIN

// From libproc
int proc_name(int pid, void * buffer, uint32_t buffersize);

/* Darwin thread states.  We can't import these from the system header because
   it uses all kinds of macros and the Swift importer can't cope with that.
   So declare them here in a form it can understand. */
#define ARM_THREAD_STATE64 6
struct darwin_arm64_thread_state {
  uint64_t _x[29];
  uint64_t fp;
  uint64_t lr;
  uint64_t sp;
  uint64_t pc;
  uint32_t cpsr;
  uint32_t __pad;
};

struct darwin_arm64_exception_state {
  uint64_t far;
  uint32_t esr;
  uint32_t exception;
};

struct darwin_arm64_mcontext {
  struct darwin_arm64_exception_state es;
  struct darwin_arm64_thread_state    ss;
  // followed by NEON state (which we don't care about)
};

#define x86_THREAD_STATE64 4
struct darwin_x86_64_thread_state {
  uint64_t rax;
  uint64_t rbx;
  uint64_t rcx;
  uint64_t rdx;
  uint64_t rdi;
  uint64_t rsi;
  uint64_t rbp;
  uint64_t rsp;
  uint64_t r8;
  uint64_t r9;
  uint64_t r10;
  uint64_t r11;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;
  uint64_t rip;
  uint64_t rflags;
  uint64_t cs;
  uint64_t fs;
  uint64_t gs;
};

struct darwin_x86_64_exception_state {
  uint16_t trapno;
  uint16_t cpu;
  uint32_t err;
  uint64_t faultvaddr;
};

struct darwin_x86_64_mcontext {
  struct darwin_x86_64_exception_state es;
  struct darwin_x86_64_thread_state    ss;
  // followed by FP/AVX/AVX512 state (which we don't care about)
};

typedef unsigned int __swift_task_t;
typedef unsigned int __swift_thread_t;
typedef unsigned int __swift_kern_return_t;
typedef unsigned char __swift_uuid_t[16];
typedef uint64_t __swift_vm_address_t;
typedef uint64_t __swift_vm_size_t;
typedef int __swift_thread_state_flavor_t;
typedef unsigned int __swift_natural_t;
typedef __swift_natural_t __swift_msg_type_number_t;
typedef __swift_natural_t *__swift_thread_state_t;

#define _SWIFT_KERN_SUCCESS 0

static inline __swift_task_t
_swift_backtrace_task_self() {
  extern __swift_task_t mach_task_self_;
  return mach_task_self_;
}

static inline __swift_kern_return_t
_swift_backtrace_vm_read(__swift_task_t task,
                         __swift_vm_address_t address,
                         __swift_vm_size_t size,
                         void *buffer,
                         __swift_vm_size_t *length) {
  extern __swift_kern_return_t mach_vm_read_overwrite(__swift_task_t,
                                                      __swift_vm_address_t,
                                                      __swift_vm_size_t,
                                                      __swift_vm_address_t,
                                                      __swift_vm_size_t *);
  return mach_vm_read_overwrite(task,
                                address,
                                size,
                                (__swift_vm_address_t)buffer,
                                (__swift_vm_size_t *)length);
}

#define _SWIFT_X86_THREAD_STATE64 6
#define _SWIFT_ARM_THREAD_STATE64 6

static inline __swift_kern_return_t
_swift_backtrace_thread_get_state(__swift_thread_t target_act,
                                  __swift_thread_state_flavor_t flavor,
                                  __swift_thread_state_t old_state,
                                  __swift_msg_type_number_t *old_stateCnt) {
  extern __swift_kern_return_t thread_get_state(__swift_thread_t,
                                                __swift_thread_state_flavor_t,
                                                __swift_thread_state_t,
                                                __swift_msg_type_number_t *);
  return thread_get_state(target_act, flavor, old_state, old_stateCnt);
}

/* DANGER!  These are SPI.  They may change (or vanish) at short notice, may
   not work how you expect, and are generally dangerous to use. */
struct dyld_process_cache_info {
  __swift_uuid_t cacheUUID;
  uint64_t       cacheBaseAddress;
  __swift_bool   noCache;
  __swift_bool   privateCache;
};
typedef struct dyld_process_cache_info dyld_process_cache_info;
typedef const struct dyld_process_info_base* dyld_process_info;

extern dyld_process_info _dyld_process_info_create(__swift_task_t task, uint64_t timestamp, __swift_kern_return_t* kernelError);
extern void  _dyld_process_info_release(dyld_process_info info);
extern void  _dyld_process_info_retain(dyld_process_info info);
extern void  _dyld_process_info_get_cache(dyld_process_info info, dyld_process_cache_info* cacheInfo);
extern void _dyld_process_info_for_each_image(dyld_process_info info, void (^callback)(uint64_t machHeaderAddress, const __swift_uuid_t uuid, const char* path));
extern void _dyld_process_info_for_each_segment(dyld_process_info info, uint64_t machHeaderAddress, void (^callback)(uint64_t segmentAddress, uint64_t segmentSize, const char* segmentName));

/* DANGER!  CoreSymbolication is a private framework.  This is all SPI. */
typedef int32_t cpu_type_t;
typedef int32_t cpu_subtype_t;

struct _CSArchitecture {
  cpu_type_t	cpu_type;
  cpu_subtype_t	cpu_subtype;
};

typedef struct _CSArchitecture CSArchitecture;

#define CPU_ARCH_ABI64          0x01000000      /* 64 bit ABI */
#define CPU_ARCH_ABI64_32       0x02000000      /* ABI for 64-bit hardware with 32-bit types; LP32 */

#define CPU_TYPE_X86            ((cpu_type_t) 7)
#define CPU_TYPE_I386           CPU_TYPE_X86            /* compatibility */

#define CPU_SUBTYPE_INTEL(f, m) ((cpu_subtype_t) (f) + ((m) << 4))
#define CPU_SUBTYPE_I386_ALL                    CPU_SUBTYPE_INTEL(3, 0)

#define CPU_TYPE_ARM            ((cpu_type_t) 12)

#define CPU_SUBTYPE_ARM64_ALL           ((cpu_subtype_t) 0)
#define CPU_SUBTYPE_ARM_V7K             ((cpu_subtype_t) 12)

static const CSArchitecture kCSArchitectureI386 = { CPU_TYPE_I386, CPU_SUBTYPE_I386_ALL };
static const CSArchitecture kCSArchitectureX86_64 = { CPU_TYPE_I386|CPU_ARCH_ABI64, CPU_SUBTYPE_I386_ALL };
static const CSArchitecture kCSArchitectureArm64 =   { CPU_TYPE_ARM | CPU_ARCH_ABI64, CPU_SUBTYPE_ARM64_ALL };
static const CSArchitecture kCSArchitectureArm64_32 =   { CPU_TYPE_ARM | CPU_ARCH_ABI64_32, CPU_SUBTYPE_ARM64_ALL };
static const CSArchitecture kCSArchitectureArmV7K =  { CPU_TYPE_ARM, CPU_SUBTYPE_ARM_V7K };

typedef struct _CSBinaryRelocationInformation {
  __swift_vm_address_t base;
  __swift_vm_address_t extent;
  char name[17];
} CSBinaryRelocationInformation;

typedef struct _CSBinaryImageInformation {
  __swift_vm_address_t base;
  __swift_vm_address_t extent;
  __swift_backtrace_CFUUIDBytes uuid;
  CSArchitecture arch;
  const char *path;
  CSBinaryRelocationInformation *relocations;
  uint32_t relocationCount;
  uint32_t flags;
} CSBinaryImageInformation;

typedef uint64_t CSMachineTime;

static const CSMachineTime kCSBeginningOfTime = 0;
static const CSMachineTime kCSEndOfTime = (1ull<<63) - 1;
static const CSMachineTime kCSNow = (1ull<<63);
static const CSMachineTime kCSAllTimes = (1ull<<63) + 1;

struct _CSTypeRef {
  uintptr_t _opaque_1;
  uintptr_t _opaque_2;
};

typedef struct _CSTypeRef CSTypeRef;

typedef CSTypeRef CSNullRef;
typedef CSTypeRef CSSymbolicatorRef;
typedef CSTypeRef CSSymbolOwnerRef;
typedef CSTypeRef CSSymbolRef;
typedef CSTypeRef CSSourceInfoRef;

static const CSNullRef kCSNull = { 0, 0 };

typedef void (^CSSymbolOwnerIterator)(CSSymbolOwnerRef owner);
typedef void (^CSStackFrameIterator)(CSSymbolRef symbol, CSSourceInfoRef info);

typedef struct _CSNotificationData {
    CSSymbolicatorRef symbolicator;
    union {
        struct Ping {
            uint32_t value;
        } ping;

        struct DyldLoad {
            CSSymbolOwnerRef symbolOwner;
        } dyldLoad;

        struct DyldUnload {
            CSSymbolOwnerRef symbolOwner;
        } dyldUnload;
    } u;
} CSNotificationData;

typedef void (^CSNotificationBlock)(uint32_t type, CSNotificationData data);

struct _CSRange {
  __swift_vm_address_t	location;
  __swift_vm_size_t	length;
};

typedef struct _CSRange CSRange;

/* DANGER! This is also SPI */
enum {
  kCRSanitizePathGlobLocalHomeDirectories = 1,
  kCRSanitizePathGlobLocalVolumes = 2,
  kCRSanitizePathGlobAllTypes = 0xff,

  kCRSanitizePathNormalize = 0x100 << 0,
  kCRSanitizePathKeepFile = 0x100 << 1,
};

#endif // SWIFT_TARGET_OS_DARWIN

#ifdef __cplusplus
} // extern "C"
} // namespace swift
#endif

#endif // SWIFT_BACKTRACING_H
