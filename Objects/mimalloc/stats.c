/* ----------------------------------------------------------------------------
Copyright (c) 2018-2021, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#include "mimalloc.h"
#include "mimalloc-internal.h"
#include "mimalloc-atomic.h"

#include <stdio.h>  // fputs, stderr
#include <string.h> // memset

#if defined(_MSC_VER) && (_MSC_VER < 1920)
#pragma warning(disable:4204)  // non-constant aggregate initializer
#endif

/* -----------------------------------------------------------
  Statistics operations
----------------------------------------------------------- */

static bool mi_is_in_main(void* stat) {
  return ((uint8_t*)stat >= (uint8_t*)&_mi_stats_main
         && (uint8_t*)stat < ((uint8_t*)&_mi_stats_main + sizeof(mi_stats_t)));  
}

static void mi_stat_update(mi_stat_count_t* stat, int64_t amount) {
  if (amount == 0) return;
  if (mi_is_in_main(stat))
  {
    // add atomically (for abandoned pages)
    int64_t current = mi_atomic_addi64_relaxed(&stat->current, amount);
    mi_atomic_maxi64_relaxed(&stat->peak, current + amount);
    if (amount > 0) {
      mi_atomic_addi64_relaxed(&stat->allocated,amount);
    }
    else {
      mi_atomic_addi64_relaxed(&stat->freed, -amount);
    }
  }
  else {
    // add thread local
    stat->current += amount;
    if (stat->current > stat->peak) stat->peak = stat->current;
    if (amount > 0) {
      stat->allocated += amount;
    }
    else {
      stat->freed += -amount;
    }
  }
}

void _mi_stat_counter_increase(mi_stat_counter_t* stat, size_t amount) {  
  if (mi_is_in_main(stat)) {
    mi_atomic_addi64_relaxed( &stat->count, 1 );
    mi_atomic_addi64_relaxed( &stat->total, (int64_t)amount );
  }
  else {
    stat->count++;
    stat->total += amount;
  }
}

void _mi_stat_increase(mi_stat_count_t* stat, size_t amount) {
  mi_stat_update(stat, (int64_t)amount);
}

void _mi_stat_decrease(mi_stat_count_t* stat, size_t amount) {
  mi_stat_update(stat, -((int64_t)amount));
}

// must be thread safe as it is called from stats_merge
static void mi_stat_add(mi_stat_count_t* stat, const mi_stat_count_t* src, int64_t unit) {
  if (stat==src) return;
  if (src->allocated==0 && src->freed==0) return;
  mi_atomic_addi64_relaxed( &stat->allocated, src->allocated * unit);
  mi_atomic_addi64_relaxed( &stat->current, src->current * unit);
  mi_atomic_addi64_relaxed( &stat->freed, src->freed * unit);
  // peak scores do not work across threads.. 
  mi_atomic_addi64_relaxed( &stat->peak, src->peak * unit);
}

static void mi_stat_counter_add(mi_stat_counter_t* stat, const mi_stat_counter_t* src, int64_t unit) {
  if (stat==src) return;
  mi_atomic_addi64_relaxed( &stat->total, src->total * unit);
  mi_atomic_addi64_relaxed( &stat->count, src->count * unit);
}

// must be thread safe as it is called from stats_merge
static void mi_stats_add(mi_stats_t* stats, const mi_stats_t* src) {
  if (stats==src) return;
  mi_stat_add(&stats->segments, &src->segments,1);
  mi_stat_add(&stats->pages, &src->pages,1);
  mi_stat_add(&stats->reserved, &src->reserved, 1);
  mi_stat_add(&stats->committed, &src->committed, 1);
  mi_stat_add(&stats->reset, &src->reset, 1);
  mi_stat_add(&stats->page_committed, &src->page_committed, 1);

  mi_stat_add(&stats->pages_abandoned, &src->pages_abandoned, 1);
  mi_stat_add(&stats->segments_abandoned, &src->segments_abandoned, 1);
  mi_stat_add(&stats->threads, &src->threads, 1);

  mi_stat_add(&stats->malloc, &src->malloc, 1);
  mi_stat_add(&stats->segments_cache, &src->segments_cache, 1);
  mi_stat_add(&stats->normal, &src->normal, 1);
  mi_stat_add(&stats->huge, &src->huge, 1);
  mi_stat_add(&stats->giant, &src->giant, 1);

  mi_stat_counter_add(&stats->pages_extended, &src->pages_extended, 1);
  mi_stat_counter_add(&stats->mmap_calls, &src->mmap_calls, 1);
  mi_stat_counter_add(&stats->commit_calls, &src->commit_calls, 1);

  mi_stat_counter_add(&stats->page_no_retire, &src->page_no_retire, 1);
  mi_stat_counter_add(&stats->searches, &src->searches, 1);
  mi_stat_counter_add(&stats->normal_count, &src->normal_count, 1);
  mi_stat_counter_add(&stats->huge_count, &src->huge_count, 1);
  mi_stat_counter_add(&stats->giant_count, &src->giant_count, 1);
#if MI_STAT>1
  for (size_t i = 0; i <= MI_BIN_HUGE; i++) {
    if (src->normal_bins[i].allocated > 0 || src->normal_bins[i].freed > 0) {
      mi_stat_add(&stats->normal_bins[i], &src->normal_bins[i], 1);
    }
  }
#endif
}

/* -----------------------------------------------------------
  Display statistics
----------------------------------------------------------- */

// unit > 0 : size in binary bytes 
// unit == 0: count as decimal
// unit < 0 : count in binary
static void mi_printf_amount(int64_t n, int64_t unit, mi_output_fun* out, void* arg, const char* fmt) {
  char buf[32];
  int  len = 32;
  const char* suffix = (unit <= 0 ? " " : "b");
  const int64_t base = (unit == 0 ? 1000 : 1024);
  if (unit>0) n *= unit;

  const int64_t pos = (n < 0 ? -n : n);
  if (pos < base) {
    snprintf(buf, len, "%d %s ", (int)n, suffix);
  }
  else {
    int64_t divider = base;
    const char* magnitude = "k";
    if (pos >= divider*base) { divider *= base; magnitude = "m"; }
    if (pos >= divider*base) { divider *= base; magnitude = "g"; }
    const int64_t tens = (n / (divider/10));
    const long whole = (long)(tens/10);
    const long frac1 = (long)(tens%10);
    snprintf(buf, len, "%ld.%ld %s%s", whole, (frac1 < 0 ? -frac1 : frac1), magnitude, suffix);
  }
  _mi_fprintf(out, arg, (fmt==NULL ? "%11s" : fmt), buf);
}


static void mi_print_amount(int64_t n, int64_t unit, mi_output_fun* out, void* arg) {
  mi_printf_amount(n,unit,out,arg,NULL);
}

static void mi_print_count(int64_t n, int64_t unit, mi_output_fun* out, void* arg) {
  if (unit==1) _mi_fprintf(out, arg, "%11s"," ");
          else mi_print_amount(n,0,out,arg);
}

static void mi_stat_print(const mi_stat_count_t* stat, const char* msg, int64_t unit, mi_output_fun* out, void* arg ) {
  _mi_fprintf(out, arg,"%10s:", msg);
  if (unit>0) {
    mi_print_amount(stat->peak, unit, out, arg);
    mi_print_amount(stat->allocated, unit, out, arg);
    mi_print_amount(stat->freed, unit, out, arg);
    mi_print_amount(stat->current, unit, out, arg);
    mi_print_amount(unit, 1, out, arg);
    mi_print_count(stat->allocated, unit, out, arg);
    if (stat->allocated > stat->freed)
      _mi_fprintf(out, arg, "  not all freed!\n");
    else
      _mi_fprintf(out, arg, "  ok\n");
  }
  else if (unit<0) {
    mi_print_amount(stat->peak, -1, out, arg);
    mi_print_amount(stat->allocated, -1, out, arg);
    mi_print_amount(stat->freed, -1, out, arg);
    mi_print_amount(stat->current, -1, out, arg);
    if (unit==-1) {
      _mi_fprintf(out, arg, "%22s", "");
    }
    else {
      mi_print_amount(-unit, 1, out, arg);
      mi_print_count((stat->allocated / -unit), 0, out, arg);
    }
    if (stat->allocated > stat->freed)
      _mi_fprintf(out, arg, "  not all freed!\n");
    else
      _mi_fprintf(out, arg, "  ok\n");
  }
  else {
    mi_print_amount(stat->peak, 1, out, arg);
    mi_print_amount(stat->allocated, 1, out, arg);
    _mi_fprintf(out, arg, "%11s", " ");  // no freed 
    mi_print_amount(stat->current, 1, out, arg);
    _mi_fprintf(out, arg, "\n");
  }
}

static void mi_stat_counter_print(const mi_stat_counter_t* stat, const char* msg, mi_output_fun* out, void* arg ) {
  _mi_fprintf(out, arg, "%10s:", msg);
  mi_print_amount(stat->total, -1, out, arg);
  _mi_fprintf(out, arg, "\n");
}

static void mi_stat_counter_print_avg(const mi_stat_counter_t* stat, const char* msg, mi_output_fun* out, void* arg) {
  const int64_t avg_tens = (stat->count == 0 ? 0 : (stat->total*10 / stat->count)); 
  const long avg_whole = (long)(avg_tens/10);
  const long avg_frac1 = (long)(avg_tens%10);
  _mi_fprintf(out, arg, "%10s: %5ld.%ld avg\n", msg, avg_whole, avg_frac1);
}


static void mi_print_header(mi_output_fun* out, void* arg ) {
  _mi_fprintf(out, arg, "%10s: %10s %10s %10s %10s %10s %10s\n", "heap stats", "peak  ", "total  ", "freed  ", "current  ", "unit  ", "count  ");
}

#if MI_STAT>1
static void mi_stats_print_bins(const mi_stat_count_t* bins, size_t max, const char* fmt, mi_output_fun* out, void* arg) {
  bool found = false;
  char buf[64];
  for (size_t i = 0; i <= max; i++) {
    if (bins[i].allocated > 0) {
      found = true;
      int64_t unit = _mi_bin_size((uint8_t)i);
      snprintf(buf, 64, "%s %3lu", fmt, (long)i);
      mi_stat_print(&bins[i], buf, unit, out, arg);
    }
  }
  if (found) {
    _mi_fprintf(out, arg, "\n");
    mi_print_header(out, arg);
  }
}
#endif



//------------------------------------------------------------
// Use an output wrapper for line-buffered output
// (which is nice when using loggers etc.)
//------------------------------------------------------------
typedef struct buffered_s {
  mi_output_fun* out;   // original output function
  void*          arg;   // and state
  char*          buf;   // local buffer of at least size `count+1`
  size_t         used;  // currently used chars `used <= count`  
  size_t         count; // total chars available for output
} buffered_t;

static void mi_buffered_flush(buffered_t* buf) {
  buf->buf[buf->used] = 0;
  _mi_fputs(buf->out, buf->arg, NULL, buf->buf);
  buf->used = 0;
}

static void mi_buffered_out(const char* msg, void* arg) {
  buffered_t* buf = (buffered_t*)arg;
  if (msg==NULL || buf==NULL) return;
  for (const char* src = msg; *src != 0; src++) {
    char c = *src;
    if (buf->used >= buf->count) mi_buffered_flush(buf);
    mi_assert_internal(buf->used < buf->count);
    buf->buf[buf->used++] = c;
    if (c == '\n') mi_buffered_flush(buf);
  }
}

//------------------------------------------------------------
// Print statistics
//------------------------------------------------------------

static void mi_stat_process_info(mi_msecs_t* elapsed, mi_msecs_t* utime, mi_msecs_t* stime, size_t* current_rss, size_t* peak_rss, size_t* current_commit, size_t* peak_commit, size_t* page_faults);

static void _mi_stats_print(mi_stats_t* stats, mi_output_fun* out0, void* arg0) mi_attr_noexcept {
  // wrap the output function to be line buffered
  char buf[256];
  buffered_t buffer = { out0, arg0, NULL, 0, 255 };
  buffer.buf = buf;
  mi_output_fun* out = &mi_buffered_out;
  void* arg = &buffer;

  // and print using that
  mi_print_header(out,arg);
  #if MI_STAT>1
  mi_stats_print_bins(stats->normal_bins, MI_BIN_HUGE, "normal",out,arg);
  #endif
  #if MI_STAT
  mi_stat_print(&stats->normal, "normal", (stats->normal_count.count == 0 ? 1 : -(stats->normal.allocated / stats->normal_count.count)), out, arg);
  mi_stat_print(&stats->huge, "huge", (stats->huge_count.count == 0 ? 1 : -(stats->huge.allocated / stats->huge_count.count)), out, arg);
  mi_stat_print(&stats->giant, "giant", (stats->giant_count.count == 0 ? 1 : -(stats->giant.allocated / stats->giant_count.count)), out, arg);
  mi_stat_count_t total = { 0,0,0,0 };
  mi_stat_add(&total, &stats->normal, 1);
  mi_stat_add(&total, &stats->huge, 1);
  mi_stat_add(&total, &stats->giant, 1);
  mi_stat_print(&total, "total", 1, out, arg);
  #endif
  #if MI_STAT>1
  mi_stat_print(&stats->malloc, "malloc req", 1, out, arg);
  _mi_fprintf(out, arg, "\n");
  #endif
  mi_stat_print(&stats->reserved, "reserved", 1, out, arg);
  mi_stat_print(&stats->committed, "committed", 1, out, arg);
  mi_stat_print(&stats->reset, "reset", 1, out, arg);
  mi_stat_print(&stats->page_committed, "touched", 1, out, arg);
  mi_stat_print(&stats->segments, "segments", -1, out, arg);
  mi_stat_print(&stats->segments_abandoned, "-abandoned", -1, out, arg);
  mi_stat_print(&stats->segments_cache, "-cached", -1, out, arg);
  mi_stat_print(&stats->pages, "pages", -1, out, arg);
  mi_stat_print(&stats->pages_abandoned, "-abandoned", -1, out, arg);
  mi_stat_counter_print(&stats->pages_extended, "-extended", out, arg);
  mi_stat_counter_print(&stats->page_no_retire, "-noretire", out, arg);
  mi_stat_counter_print(&stats->mmap_calls, "mmaps", out, arg);
  mi_stat_counter_print(&stats->commit_calls, "commits", out, arg);
  mi_stat_print(&stats->threads, "threads", -1, out, arg);
  mi_stat_counter_print_avg(&stats->searches, "searches", out, arg);
  _mi_fprintf(out, arg, "%10s: %7i\n", "numa nodes", _mi_os_numa_node_count());
  
  mi_msecs_t elapsed;
  mi_msecs_t user_time;
  mi_msecs_t sys_time;
  size_t current_rss;
  size_t peak_rss;
  size_t current_commit;
  size_t peak_commit;
  size_t page_faults;
  mi_stat_process_info(&elapsed, &user_time, &sys_time, &current_rss, &peak_rss, &current_commit, &peak_commit, &page_faults);
  _mi_fprintf(out, arg, "%10s: %7ld.%03ld s\n", "elapsed", elapsed/1000, elapsed%1000);
  _mi_fprintf(out, arg, "%10s: user: %ld.%03ld s, system: %ld.%03ld s, faults: %lu, rss: ", "process",
              user_time/1000, user_time%1000, sys_time/1000, sys_time%1000, (unsigned long)page_faults );
  mi_printf_amount((int64_t)peak_rss, 1, out, arg, "%s");
  if (peak_commit > 0) {
    _mi_fprintf(out, arg, ", commit: ");
    mi_printf_amount((int64_t)peak_commit, 1, out, arg, "%s");
  }
  _mi_fprintf(out, arg, "\n");  
}

static mi_msecs_t mi_process_start; // = 0

static mi_stats_t* mi_stats_get_default(void) {
  mi_heap_t* heap = mi_heap_get_default();
  return &heap->tld->stats;
}

static void mi_stats_merge_from(mi_stats_t* stats) {
  if (stats != &_mi_stats_main) {
    mi_stats_add(&_mi_stats_main, stats);
    memset(stats, 0, sizeof(mi_stats_t));
  }
}

void mi_stats_reset(void) mi_attr_noexcept {
  mi_stats_t* stats = mi_stats_get_default();
  if (stats != &_mi_stats_main) { memset(stats, 0, sizeof(mi_stats_t)); }
  memset(&_mi_stats_main, 0, sizeof(mi_stats_t));
  if (mi_process_start == 0) { mi_process_start = _mi_clock_start(); };
}

void mi_stats_merge(void) mi_attr_noexcept {
  mi_stats_merge_from( mi_stats_get_default() );
}

void _mi_stats_done(mi_stats_t* stats) {  // called from `mi_thread_done`
  mi_stats_merge_from(stats);
}

void mi_stats_print_out(mi_output_fun* out, void* arg) mi_attr_noexcept {
  mi_stats_merge_from(mi_stats_get_default());
  _mi_stats_print(&_mi_stats_main, out, arg);
}

void mi_stats_print(void* out) mi_attr_noexcept {
  // for compatibility there is an `out` parameter (which can be `stdout` or `stderr`)
  mi_stats_print_out((mi_output_fun*)out, NULL);
}

void mi_thread_stats_print_out(mi_output_fun* out, void* arg) mi_attr_noexcept {
  _mi_stats_print(mi_stats_get_default(), out, arg);
}


// ----------------------------------------------------------------
// Basic timer for convenience; use milli-seconds to avoid doubles
// ----------------------------------------------------------------
#ifdef _WIN32
#include <windows.h>
static mi_msecs_t mi_to_msecs(LARGE_INTEGER t) {
  static LARGE_INTEGER mfreq; // = 0
  if (mfreq.QuadPart == 0LL) {
    LARGE_INTEGER f;
    QueryPerformanceFrequency(&f);
    mfreq.QuadPart = f.QuadPart/1000LL;
    if (mfreq.QuadPart == 0) mfreq.QuadPart = 1;
  }
  return (mi_msecs_t)(t.QuadPart / mfreq.QuadPart);  
}

mi_msecs_t _mi_clock_now(void) {
  LARGE_INTEGER t;
  QueryPerformanceCounter(&t);
  return mi_to_msecs(t);
}
#else
#include <time.h>
#ifdef CLOCK_REALTIME
mi_msecs_t _mi_clock_now(void) {
  struct timespec t;
  clock_gettime(CLOCK_REALTIME, &t);
  return ((mi_msecs_t)t.tv_sec * 1000) + ((mi_msecs_t)t.tv_nsec / 1000000);
}
#else
// low resolution timer
mi_msecs_t _mi_clock_now(void) {
  return ((mi_msecs_t)clock() / ((mi_msecs_t)CLOCKS_PER_SEC / 1000));
}
#endif
#endif


static mi_msecs_t mi_clock_diff;

mi_msecs_t _mi_clock_start(void) {
  if (mi_clock_diff == 0.0) {
    mi_msecs_t t0 = _mi_clock_now();
    mi_clock_diff = _mi_clock_now() - t0;
  }
  return _mi_clock_now();
}

mi_msecs_t _mi_clock_end(mi_msecs_t start) {
  mi_msecs_t end = _mi_clock_now();
  return (end - start - mi_clock_diff);
}


// --------------------------------------------------------
// Basic process statistics
// --------------------------------------------------------

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#pragma comment(lib,"psapi.lib")

static mi_msecs_t filetime_msecs(const FILETIME* ftime) {
  ULARGE_INTEGER i;
  i.LowPart = ftime->dwLowDateTime;
  i.HighPart = ftime->dwHighDateTime;
  mi_msecs_t msecs = (i.QuadPart / 10000); // FILETIME is in 100 nano seconds
  return msecs;
}

static void mi_stat_process_info(mi_msecs_t* elapsed, mi_msecs_t* utime, mi_msecs_t* stime, size_t* current_rss, size_t* peak_rss, size_t* current_commit, size_t* peak_commit, size_t* page_faults) 
{
  *elapsed = _mi_clock_end(mi_process_start);
  FILETIME ct;
  FILETIME ut;
  FILETIME st;
  FILETIME et;
  GetProcessTimes(GetCurrentProcess(), &ct, &et, &st, &ut);
  *utime = filetime_msecs(&ut);
  *stime = filetime_msecs(&st);
  PROCESS_MEMORY_COUNTERS info;
  GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
  *current_rss    = (size_t)info.WorkingSetSize;
  *peak_rss       = (size_t)info.PeakWorkingSetSize;
  *current_commit = (size_t)info.PagefileUsage;
  *peak_commit    = (size_t)info.PeakPagefileUsage;
  *page_faults    = (size_t)info.PageFaultCount;  
}

#elif defined(__unix__) || defined(__unix) || defined(unix) || defined(__APPLE__) || defined(__HAIKU__)
#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>

#if defined(__APPLE__)
#include <mach/mach.h>
#endif

#if defined(__HAIKU__)
#include <kernel/OS.h>
#endif

static mi_msecs_t timeval_secs(const struct timeval* tv) {
  return ((mi_msecs_t)tv->tv_sec * 1000L) + ((mi_msecs_t)tv->tv_usec / 1000L);
}

static void mi_stat_process_info(mi_msecs_t* elapsed, mi_msecs_t* utime, mi_msecs_t* stime, size_t* current_rss, size_t* peak_rss, size_t* current_commit, size_t* peak_commit, size_t* page_faults)
{
  *elapsed = _mi_clock_end(mi_process_start);
  struct rusage rusage;
  getrusage(RUSAGE_SELF, &rusage);
  *utime = timeval_secs(&rusage.ru_utime);
  *stime = timeval_secs(&rusage.ru_stime);
#if !defined(__HAIKU__)
  *page_faults = rusage.ru_majflt;
#endif
  // estimate commit using our stats
  *peak_commit    = (size_t)(mi_atomic_loadi64_relaxed((_Atomic(int64_t)*)&_mi_stats_main.committed.peak));
  *current_commit = (size_t)(mi_atomic_loadi64_relaxed((_Atomic(int64_t)*)&_mi_stats_main.committed.current));
  *current_rss    = *current_commit;  // estimate 
#if defined(__HAIKU__)
  // Haiku does not have (yet?) a way to
  // get these stats per process
  thread_info tid;
  area_info mem;
  ssize_t c;
  get_thread_info(find_thread(0), &tid);
  while (get_next_area_info(tid.team, &c, &mem) == B_OK) {
    *peak_rss += mem.ram_size;
  }
#elif defined(__APPLE__)
  *peak_rss = rusage.ru_maxrss;         // BSD reports in bytes
  struct mach_task_basic_info info;
  mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
  if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &infoCount) == KERN_SUCCESS) {
    *current_rss = (size_t)info.resident_size;
  }
#else
  *peak_rss = rusage.ru_maxrss * 1024;  // Linux reports in KiB
#endif  
}

#else
#ifndef __wasi__
// WebAssembly instances are not processes
#pragma message("define a way to get process info")
#endif

static void mi_stat_process_info(mi_msecs_t* elapsed, mi_msecs_t* utime, mi_msecs_t* stime, size_t* current_rss, size_t* peak_rss, size_t* current_commit, size_t* peak_commit, size_t* page_faults)
{
  *elapsed = _mi_clock_end(mi_process_start);
  *peak_commit    = (size_t)(mi_atomic_loadi64_relaxed((_Atomic(int64_t)*)&_mi_stats_main.committed.peak));
  *current_commit = (size_t)(mi_atomic_loadi64_relaxed((_Atomic(int64_t)*)&_mi_stats_main.committed.current));
  *peak_rss    = *peak_commit;
  *current_rss = *current_commit;
  *page_faults = 0;
  *utime = 0;
  *stime = 0;
}
#endif


mi_decl_export void mi_process_info(size_t* elapsed_msecs, size_t* user_msecs, size_t* system_msecs, size_t* current_rss, size_t* peak_rss, size_t* current_commit, size_t* peak_commit, size_t* page_faults) mi_attr_noexcept
{
  mi_msecs_t elapsed = 0;
  mi_msecs_t utime = 0;
  mi_msecs_t stime = 0;
  size_t current_rss0 = 0;
  size_t peak_rss0 = 0;
  size_t current_commit0 = 0;
  size_t peak_commit0 = 0;
  size_t page_faults0 = 0;  
  mi_stat_process_info(&elapsed,&utime, &stime, &current_rss0, &peak_rss0, &current_commit0, &peak_commit0, &page_faults0);
  if (elapsed_msecs!=NULL)  *elapsed_msecs = (elapsed < 0 ? 0 : (elapsed < (mi_msecs_t)PTRDIFF_MAX ? (size_t)elapsed : PTRDIFF_MAX));
  if (user_msecs!=NULL)     *user_msecs     = (utime < 0 ? 0 : (utime < (mi_msecs_t)PTRDIFF_MAX ? (size_t)utime : PTRDIFF_MAX));
  if (system_msecs!=NULL)   *system_msecs   = (stime < 0 ? 0 : (stime < (mi_msecs_t)PTRDIFF_MAX ? (size_t)stime : PTRDIFF_MAX));
  if (current_rss!=NULL)    *current_rss    = current_rss0;
  if (peak_rss!=NULL)       *peak_rss       = peak_rss0;
  if (current_commit!=NULL) *current_commit = current_commit0;
  if (peak_commit!=NULL)    *peak_commit    = peak_commit0;
  if (page_faults!=NULL)    *page_faults    = page_faults0;
}

