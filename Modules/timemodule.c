/* Time module */

#include "Python.h"

#include <ctype.h>

#ifdef HAVE_SYS_TIMES_H
#include <sys/times.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if defined(HAVE_SYS_RESOURCE_H)
#include <sys/resource.h>
#endif

#ifdef QUICKWIN
#include <io.h>
#endif

#if defined(HAVE_PTHREAD_H)
#  include <pthread.h>
#endif

#if defined(__WATCOMC__) && !defined(__QNX__)
#include <i86.h>
#else
#ifdef MS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "pythread.h"
#endif /* MS_WINDOWS */
#endif /* !__WATCOMC__ || __QNX__ */

#define SEC_TO_NS (1000 * 1000 * 1000)

/* Forward declarations */
static int pysleep(_PyTime_t);


static PyObject*
_PyFloat_FromPyTime(_PyTime_t t)
{
    double d = _PyTime_AsSecondsDouble(t);
    return PyFloat_FromDouble(d);
}


static PyObject *
time_time(PyObject *self, PyObject *unused)
{
    _PyTime_t t = _PyTime_GetSystemClock();
    return _PyFloat_FromPyTime(t);
}


PyDoc_STRVAR(time_doc,
"time() -> floating point number\n\
\n\
Return the current time in seconds since the Epoch.\n\
Fractions of a second may be present if the system clock provides them.");

static PyObject *
time_time_ns(PyObject *self, PyObject *unused)
{
    _PyTime_t t = _PyTime_GetSystemClock();
    return _PyTime_AsNanosecondsObject(t);
}

PyDoc_STRVAR(time_ns_doc,
"time_ns() -> int\n\
\n\
Return the current time in nanoseconds since the Epoch.");

#if defined(HAVE_CLOCK)

#ifndef CLOCKS_PER_SEC
#  ifdef CLK_TCK
#    define CLOCKS_PER_SEC CLK_TCK
#  else
#    define CLOCKS_PER_SEC 1000000
#  endif
#endif

static int
_PyTime_GetClockWithInfo(_PyTime_t *tp, _Py_clock_info_t *info)
{
    static int initialized = 0;
    clock_t ticks;

    if (!initialized) {
        initialized = 1;

        /* must sure that _PyTime_MulDiv(ticks, SEC_TO_NS, CLOCKS_PER_SEC)
           above cannot overflow */
        if ((_PyTime_t)CLOCKS_PER_SEC > _PyTime_MAX / SEC_TO_NS) {
            PyErr_SetString(PyExc_OverflowError,
                            "CLOCKS_PER_SEC is too large");
            return -1;
        }
    }

    if (info) {
        info->implementation = "clock()";
        info->resolution = 1.0 / (double)CLOCKS_PER_SEC;
        info->monotonic = 1;
        info->adjustable = 0;
    }

    ticks = clock();
    if (ticks == (clock_t)-1) {
        PyErr_SetString(PyExc_RuntimeError,
                        "the processor time used is not available "
                        "or its value cannot be represented");
        return -1;
    }
    *tp = _PyTime_MulDiv(ticks, SEC_TO_NS, (_PyTime_t)CLOCKS_PER_SEC);
    return 0;
}
#endif /* HAVE_CLOCK */

static PyObject*
perf_counter(_Py_clock_info_t *info)
{
    _PyTime_t t;
    if (_PyTime_GetPerfCounterWithInfo(&t, info) < 0) {
        return NULL;
    }
    return _PyFloat_FromPyTime(t);
}

#if defined(MS_WINDOWS) || defined(HAVE_CLOCK)
#define PYCLOCK
static PyObject*
pyclock(_Py_clock_info_t *info)
{
    if (PyErr_WarnEx(PyExc_DeprecationWarning,
                      "time.clock has been deprecated in Python 3.3 and will "
                      "be removed from Python 3.8: "
                      "use time.perf_counter or time.process_time "
                      "instead", 1) < 0) {
        return NULL;
    }

#ifdef MS_WINDOWS
    return perf_counter(info);
#else
    _PyTime_t t;
    if (_PyTime_GetClockWithInfo(&t, info) < 0) {
        return NULL;
    }
    return _PyFloat_FromPyTime(t);
#endif
}

static PyObject *
time_clock(PyObject *self, PyObject *unused)
{
    return pyclock(NULL);
}

PyDoc_STRVAR(clock_doc,
"clock() -> floating point number\n\
\n\
Return the CPU time or real time since the start of the process or since\n\
the first call to clock().  This has as much precision as the system\n\
records.");
#endif

#ifdef HAVE_CLOCK_GETTIME
static PyObject *
time_clock_gettime(PyObject *self, PyObject *args)
{
    int ret;
    struct timespec tp;

#if defined(_AIX) && (SIZEOF_LONG == 8)
    long clk_id;
    if (!PyArg_ParseTuple(args, "l:clock_gettime", &clk_id)) {
#else
    int clk_id;
    if (!PyArg_ParseTuple(args, "i:clock_gettime", &clk_id)) {
#endif
        return NULL;
    }

    ret = clock_gettime((clockid_t)clk_id, &tp);
    if (ret != 0) {
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }
    return PyFloat_FromDouble(tp.tv_sec + tp.tv_nsec * 1e-9);
}

PyDoc_STRVAR(clock_gettime_doc,
"clock_gettime(clk_id) -> float\n\
\n\
Return the time of the specified clock clk_id.");

static PyObject *
time_clock_gettime_ns(PyObject *self, PyObject *args)
{
    int ret;
    int clk_id;
    struct timespec ts;
    _PyTime_t t;

    if (!PyArg_ParseTuple(args, "i:clock_gettime", &clk_id)) {
        return NULL;
    }

    ret = clock_gettime((clockid_t)clk_id, &ts);
    if (ret != 0) {
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }
    if (_PyTime_FromTimespec(&t, &ts) < 0) {
        return NULL;
    }
    return _PyTime_AsNanosecondsObject(t);
}

PyDoc_STRVAR(clock_gettime_ns_doc,
"clock_gettime_ns(clk_id) -> int\n\
\n\
Return the time of the specified clock clk_id as nanoseconds.");
#endif   /* HAVE_CLOCK_GETTIME */

#ifdef HAVE_CLOCK_SETTIME
static PyObject *
time_clock_settime(PyObject *self, PyObject *args)
{
    int clk_id;
    PyObject *obj;
    _PyTime_t t;
    struct timespec tp;
    int ret;

    if (!PyArg_ParseTuple(args, "iO:clock_settime", &clk_id, &obj))
        return NULL;

    if (_PyTime_FromSecondsObject(&t, obj, _PyTime_ROUND_FLOOR) < 0)
        return NULL;

    if (_PyTime_AsTimespec(t, &tp) == -1)
        return NULL;

    ret = clock_settime((clockid_t)clk_id, &tp);
    if (ret != 0) {
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }
    Py_RETURN_NONE;
}

PyDoc_STRVAR(clock_settime_doc,
"clock_settime(clk_id, time)\n\
\n\
Set the time of the specified clock clk_id.");

static PyObject *
time_clock_settime_ns(PyObject *self, PyObject *args)
{
    int clk_id;
    PyObject *obj;
    _PyTime_t t;
    struct timespec ts;
    int ret;

    if (!PyArg_ParseTuple(args, "iO:clock_settime", &clk_id, &obj)) {
        return NULL;
    }

    if (_PyTime_FromNanosecondsObject(&t, obj) < 0) {
        return NULL;
    }
    if (_PyTime_AsTimespec(t, &ts) == -1) {
        return NULL;
    }

    ret = clock_settime((clockid_t)clk_id, &ts);
    if (ret != 0) {
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }
    Py_RETURN_NONE;
}

PyDoc_STRVAR(clock_settime_ns_doc,
"clock_settime_ns(clk_id, time)\n\
\n\
Set the time of the specified clock clk_id with nanoseconds.");
#endif   /* HAVE_CLOCK_SETTIME */

#ifdef HAVE_CLOCK_GETRES
static PyObject *
time_clock_getres(PyObject *self, PyObject *args)
{
    int ret;
    int clk_id;
    struct timespec tp;

    if (!PyArg_ParseTuple(args, "i:clock_getres", &clk_id))
        return NULL;

    ret = clock_getres((clockid_t)clk_id, &tp);
    if (ret != 0) {
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }

    return PyFloat_FromDouble(tp.tv_sec + tp.tv_nsec * 1e-9);
}

PyDoc_STRVAR(clock_getres_doc,
"clock_getres(clk_id) -> floating point number\n\
\n\
Return the resolution (precision) of the specified clock clk_id.");
#endif   /* HAVE_CLOCK_GETRES */

#ifdef HAVE_PTHREAD_GETCPUCLOCKID
static PyObject *
time_pthread_getcpuclockid(PyObject *self, PyObject *args)
{
    unsigned long thread_id;
    int err;
    clockid_t clk_id;
    if (!PyArg_ParseTuple(args, "k:pthread_getcpuclockid", &thread_id)) {
        return NULL;
    }
    err = pthread_getcpuclockid((pthread_t)thread_id, &clk_id);
    if (err) {
        errno = err;
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }
    return PyLong_FromLong(clk_id);
}

PyDoc_STRVAR(pthread_getcpuclockid_doc,
"pthread_getcpuclockid(thread_id) -> int\n\
\n\
Return the clk_id of a thread's CPU time clock.");
#endif /* HAVE_PTHREAD_GETCPUCLOCKID */

static PyObject *
time_sleep(PyObject *self, PyObject *obj)
{
    _PyTime_t secs;
    if (_PyTime_FromSecondsObject(&secs, obj, _PyTime_ROUND_TIMEOUT))
        return NULL;
    if (secs < 0) {
        PyErr_SetString(PyExc_ValueError,
                        "sleep length must be non-negative");
        return NULL;
    }
    if (pysleep(secs) != 0)
        return NULL;
    Py_RETURN_NONE;
}

PyDoc_STRVAR(sleep_doc,
"sleep(seconds)\n\
\n\
Delay execution for a given number of seconds.  The argument may be\n\
a floating point number for subsecond precision.");

static PyStructSequence_Field struct_time_type_fields[] = {
    {"tm_year", "year, for example, 1993"},
    {"tm_mon", "month of year, range [1, 12]"},
    {"tm_mday", "day of month, range [1, 31]"},
    {"tm_hour", "hours, range [0, 23]"},
    {"tm_min", "minutes, range [0, 59]"},
    {"tm_sec", "seconds, range [0, 61])"},
    {"tm_wday", "day of week, range [0, 6], Monday is 0"},
    {"tm_yday", "day of year, range [1, 366]"},
    {"tm_isdst", "1 if summer time is in effect, 0 if not, and -1 if unknown"},
    {"tm_zone", "abbreviation of timezone name"},
    {"tm_gmtoff", "offset from UTC in seconds"},
    {0}
};

static PyStructSequence_Desc struct_time_type_desc = {
    "time.struct_time",
    "The time value as returned by gmtime(), localtime(), and strptime(), and\n"
    " accepted by asctime(), mktime() and strftime().  May be considered as a\n"
    " sequence of 9 integers.\n\n"
    " Note that several fields' values are not the same as those defined by\n"
    " the C language standard for struct tm.  For example, the value of the\n"
    " field tm_year is the actual year, not year - 1900.  See individual\n"
    " fields' descriptions for details.",
    struct_time_type_fields,
    9,
};

static int initialized;
static PyTypeObject StructTimeType;


static PyObject *
tmtotuple(struct tm *p
#ifndef HAVE_STRUCT_TM_TM_ZONE
        , const char *zone, time_t gmtoff
#endif
)
{
    PyObject *v = PyStructSequence_New(&StructTimeType);
    if (v == NULL)
        return NULL;

#define SET(i,val) PyStructSequence_SET_ITEM(v, i, PyLong_FromLong((long) val))

    SET(0, p->tm_year + 1900);
    SET(1, p->tm_mon + 1);         /* Want January == 1 */
    SET(2, p->tm_mday);
    SET(3, p->tm_hour);
    SET(4, p->tm_min);
    SET(5, p->tm_sec);
    SET(6, (p->tm_wday + 6) % 7); /* Want Monday == 0 */
    SET(7, p->tm_yday + 1);        /* Want January, 1 == 1 */
    SET(8, p->tm_isdst);
#ifdef HAVE_STRUCT_TM_TM_ZONE
    PyStructSequence_SET_ITEM(v, 9,
        PyUnicode_DecodeLocale(p->tm_zone, "surrogateescape"));
    SET(10, p->tm_gmtoff);
#else
    PyStructSequence_SET_ITEM(v, 9,
        PyUnicode_DecodeLocale(zone, "surrogateescape"));
    PyStructSequence_SET_ITEM(v, 10, _PyLong_FromTime_t(gmtoff));
#endif /* HAVE_STRUCT_TM_TM_ZONE */
#undef SET
    if (PyErr_Occurred()) {
        Py_XDECREF(v);
        return NULL;
    }

    return v;
}

/* Parse arg tuple that can contain an optional float-or-None value;
   format needs to be "|O:name".
   Returns non-zero on success (parallels PyArg_ParseTuple).
*/
static int
parse_time_t_args(PyObject *args, const char *format, time_t *pwhen)
{
    PyObject *ot = NULL;
    time_t whent;

    if (!PyArg_ParseTuple(args, format, &ot))
        return 0;
    if (ot == NULL || ot == Py_None) {
        whent = time(NULL);
    }
    else {
        if (_PyTime_ObjectToTime_t(ot, &whent, _PyTime_ROUND_FLOOR) == -1)
            return 0;
    }
    *pwhen = whent;
    return 1;
}

static PyObject *
time_gmtime(PyObject *self, PyObject *args)
{
    time_t when;
    struct tm buf;

    if (!parse_time_t_args(args, "|O:gmtime", &when))
        return NULL;

    errno = 0;
    if (_PyTime_gmtime(when, &buf) != 0)
        return NULL;
#ifdef HAVE_STRUCT_TM_TM_ZONE
    return tmtotuple(&buf);
#else
    return tmtotuple(&buf, "UTC", 0);
#endif
}

#ifndef HAVE_TIMEGM
static time_t
timegm(struct tm *p)
{
    /* XXX: the following implementation will not work for tm_year < 1970.
       but it is likely that platforms that don't have timegm do not support
       negative timestamps anyways. */
    return p->tm_sec + p->tm_min*60 + p->tm_hour*3600 + p->tm_yday*86400 +
        (p->tm_year-70)*31536000 + ((p->tm_year-69)/4)*86400 -
        ((p->tm_year-1)/100)*86400 + ((p->tm_year+299)/400)*86400;
}
#endif

PyDoc_STRVAR(gmtime_doc,
"gmtime([seconds]) -> (tm_year, tm_mon, tm_mday, tm_hour, tm_min,\n\
                       tm_sec, tm_wday, tm_yday, tm_isdst)\n\
\n\
Convert seconds since the Epoch to a time tuple expressing UTC (a.k.a.\n\
GMT).  When 'seconds' is not passed in, convert the current time instead.\n\
\n\
If the platform supports the tm_gmtoff and tm_zone, they are available as\n\
attributes only.");

static PyObject *
time_localtime(PyObject *self, PyObject *args)
{
    time_t when;
    struct tm buf;

    if (!parse_time_t_args(args, "|O:localtime", &when))
        return NULL;
    if (_PyTime_localtime(when, &buf) != 0)
        return NULL;
#ifdef HAVE_STRUCT_TM_TM_ZONE
    return tmtotuple(&buf);
#else
    {
        struct tm local = buf;
        char zone[100];
        time_t gmtoff;
        strftime(zone, sizeof(zone), "%Z", &buf);
        gmtoff = timegm(&buf) - when;
        return tmtotuple(&local, zone, gmtoff);
    }
#endif
}

#if defined(__linux__) && !defined(__GLIBC__)
static const char *utc_string = NULL;
#endif

PyDoc_STRVAR(localtime_doc,
"localtime([seconds]) -> (tm_year,tm_mon,tm_mday,tm_hour,tm_min,\n\
                          tm_sec,tm_wday,tm_yday,tm_isdst)\n\
\n\
Convert seconds since the Epoch to a time tuple expressing local time.\n\
When 'seconds' is not passed in, convert the current time instead.");

/* Convert 9-item tuple to tm structure.  Return 1 on success, set
 * an exception and return 0 on error.
 */
static int
gettmarg(PyObject *args, struct tm *p, const char *format)
{
    int y;

    memset((void *) p, '\0', sizeof(struct tm));

    if (!PyTuple_Check(args)) {
        PyErr_SetString(PyExc_TypeError,
                        "Tuple or struct_time argument required");
        return 0;
    }

    if (!PyArg_ParseTuple(args, format,
                          &y, &p->tm_mon, &p->tm_mday,
                          &p->tm_hour, &p->tm_min, &p->tm_sec,
                          &p->tm_wday, &p->tm_yday, &p->tm_isdst))
        return 0;

    if (y < INT_MIN + 1900) {
        PyErr_SetString(PyExc_OverflowError, "year out of range");
        return 0;
    }

    p->tm_year = y - 1900;
    p->tm_mon--;
    p->tm_wday = (p->tm_wday + 1) % 7;
    p->tm_yday--;
#ifdef HAVE_STRUCT_TM_TM_ZONE
    if (Py_TYPE(args) == &StructTimeType) {
        PyObject *item;
        item = PyTuple_GET_ITEM(args, 9);
        if (item != Py_None) {
            p->tm_zone = PyUnicode_AsUTF8(item);
            if (p->tm_zone == NULL) {
                return 0;
            }
#if defined(__linux__) && !defined(__GLIBC__)
            // Make an attempt to return the C library's own timezone strings to
            // it. musl refuses to process a tm_zone field unless it produced
            // it. See issue #34672.
            if (utc_string && strcmp(p->tm_zone, utc_string) == 0) {
                p->tm_zone = utc_string;
            }
            else if (tzname[0] && strcmp(p->tm_zone, tzname[0]) == 0) {
                p->tm_zone = tzname[0];
            }
            else if (tzname[1] && strcmp(p->tm_zone, tzname[1]) == 0) {
                p->tm_zone = tzname[1];
            }
#endif
        }
        item = PyTuple_GET_ITEM(args, 10);
        if (item != Py_None) {
            p->tm_gmtoff = PyLong_AsLong(item);
            if (PyErr_Occurred())
                return 0;
        }
    }
#endif /* HAVE_STRUCT_TM_TM_ZONE */
    return 1;
}

/* Check values of the struct tm fields before it is passed to strftime() and
 * asctime().  Return 1 if all values are valid, otherwise set an exception
 * and returns 0.
 */
static int
checktm(struct tm* buf)
{
    /* Checks added to make sure strftime() and asctime() does not crash Python by
       indexing blindly into some array for a textual representation
       by some bad index (fixes bug #897625 and #6608).

       Also support values of zero from Python code for arguments in which
       that is out of range by forcing that value to the lowest value that
       is valid (fixed bug #1520914).

       Valid ranges based on what is allowed in struct tm:

       - tm_year: [0, max(int)] (1)
       - tm_mon: [0, 11] (2)
       - tm_mday: [1, 31]
       - tm_hour: [0, 23]
       - tm_min: [0, 59]
       - tm_sec: [0, 60]
       - tm_wday: [0, 6] (1)
       - tm_yday: [0, 365] (2)
       - tm_isdst: [-max(int), max(int)]

       (1) gettmarg() handles bounds-checking.
       (2) Python's acceptable range is one greater than the range in C,
       thus need to check against automatic decrement by gettmarg().
    */
    if (buf->tm_mon == -1)
        buf->tm_mon = 0;
    else if (buf->tm_mon < 0 || buf->tm_mon > 11) {
        PyErr_SetString(PyExc_ValueError, "month out of range");
        return 0;
    }
    if (buf->tm_mday == 0)
        buf->tm_mday = 1;
    else if (buf->tm_mday < 0 || buf->tm_mday > 31) {
        PyErr_SetString(PyExc_ValueError, "day of month out of range");
        return 0;
    }
    if (buf->tm_hour < 0 || buf->tm_hour > 23) {
        PyErr_SetString(PyExc_ValueError, "hour out of range");
        return 0;
    }
    if (buf->tm_min < 0 || buf->tm_min > 59) {
        PyErr_SetString(PyExc_ValueError, "minute out of range");
        return 0;
    }
    if (buf->tm_sec < 0 || buf->tm_sec > 61) {
        PyErr_SetString(PyExc_ValueError, "seconds out of range");
        return 0;
    }
    /* tm_wday does not need checking of its upper-bound since taking
    ``% 7`` in gettmarg() automatically restricts the range. */
    if (buf->tm_wday < 0) {
        PyErr_SetString(PyExc_ValueError, "day of week out of range");
        return 0;
    }
    if (buf->tm_yday == -1)
        buf->tm_yday = 0;
    else if (buf->tm_yday < 0 || buf->tm_yday > 365) {
        PyErr_SetString(PyExc_ValueError, "day of year out of range");
        return 0;
    }
    return 1;
}

#ifdef MS_WINDOWS
   /* wcsftime() doesn't format correctly time zones, see issue #10653 */
#  undef HAVE_WCSFTIME
#endif
#define STRFTIME_FORMAT_CODES \
"Commonly used format codes:\n\
\n\
%Y  Year with century as a decimal number.\n\
%m  Month as a decimal number [01,12].\n\
%d  Day of the month as a decimal number [01,31].\n\
%H  Hour (24-hour clock) as a decimal number [00,23].\n\
%M  Minute as a decimal number [00,59].\n\
%S  Second as a decimal number [00,61].\n\
%z  Time zone offset from UTC.\n\
%a  Locale's abbreviated weekday name.\n\
%A  Locale's full weekday name.\n\
%b  Locale's abbreviated month name.\n\
%B  Locale's full month name.\n\
%c  Locale's appropriate date and time representation.\n\
%I  Hour (12-hour clock) as a decimal number [01,12].\n\
%p  Locale's equivalent of either AM or PM.\n\
\n\
Other codes may be available on your platform.  See documentation for\n\
the C library strftime function.\n"

#ifdef HAVE_STRFTIME
#ifdef HAVE_WCSFTIME
#define time_char wchar_t
#define format_time wcsftime
#define time_strlen wcslen
#else
#define time_char char
#define format_time strftime
#define time_strlen strlen
#endif

static PyObject *
time_strftime(PyObject *self, PyObject *args)
{
    PyObject *tup = NULL;
    struct tm buf;
    const time_char *fmt;
#ifdef HAVE_WCSFTIME
    wchar_t *format;
#else
    PyObject *format;
#endif
    PyObject *format_arg;
    size_t fmtlen, buflen;
    time_char *outbuf = NULL;
    size_t i;
    PyObject *ret = NULL;

    memset((void *) &buf, '\0', sizeof(buf));

    /* Will always expect a unicode string to be passed as format.
       Given that there's no str type anymore in py3k this seems safe.
    */
    if (!PyArg_ParseTuple(args, "U|O:strftime", &format_arg, &tup))
        return NULL;

    if (tup == NULL) {
        time_t tt = time(NULL);
        if (_PyTime_localtime(tt, &buf) != 0)
            return NULL;
    }
    else if (!gettmarg(tup, &buf,
                       "iiiiiiiii;strftime(): illegal time tuple argument") ||
             !checktm(&buf))
    {
        return NULL;
    }

#if defined(_MSC_VER) || defined(sun) || defined(_AIX)
    if (buf.tm_year + 1900 < 1 || 9999 < buf.tm_year + 1900) {
        PyErr_SetString(PyExc_ValueError,
                        "strftime() requires year in [1; 9999]");
        return NULL;
    }
#endif

    /* Normalize tm_isdst just in case someone foolishly implements %Z
       based on the assumption that tm_isdst falls within the range of
       [-1, 1] */
    if (buf.tm_isdst < -1)
        buf.tm_isdst = -1;
    else if (buf.tm_isdst > 1)
        buf.tm_isdst = 1;

#ifdef HAVE_WCSFTIME
    format = PyUnicode_AsWideCharString(format_arg, NULL);
    if (format == NULL)
        return NULL;
    fmt = format;
#else
    /* Convert the unicode string to an ascii one */
    format = PyUnicode_EncodeLocale(format_arg, "surrogateescape");
    if (format == NULL)
        return NULL;
    fmt = PyBytes_AS_STRING(format);
#endif

#if defined(MS_WINDOWS) && !defined(HAVE_WCSFTIME)
    /* check that the format string contains only valid directives */
    for (outbuf = strchr(fmt, '%');
        outbuf != NULL;
        outbuf = strchr(outbuf+2, '%'))
    {
        if (outbuf[1] == '#')
            ++outbuf; /* not documented by python, */
        if (outbuf[1] == '\0')
            break;
        if ((outbuf[1] == 'y') && buf.tm_year < 0) {
            PyErr_SetString(PyExc_ValueError,
                        "format %y requires year >= 1900 on Windows");
            Py_DECREF(format);
            return NULL;
        }
    }
#elif (defined(_AIX) || defined(sun)) && defined(HAVE_WCSFTIME)
    for (outbuf = wcschr(fmt, '%');
        outbuf != NULL;
        outbuf = wcschr(outbuf+2, '%'))
    {
        if (outbuf[1] == L'\0')
            break;
        /* Issue #19634: On AIX, wcsftime("y", (1899, 1, 1, 0, 0, 0, 0, 0, 0))
           returns "0/" instead of "99" */
        if (outbuf[1] == L'y' && buf.tm_year < 0) {
            PyErr_SetString(PyExc_ValueError,
                            "format %y requires year >= 1900 on AIX");
            return NULL;
        }
    }
#endif

    fmtlen = time_strlen(fmt);

    /* I hate these functions that presume you know how big the output
     * will be ahead of time...
     */
    for (i = 1024; ; i += i) {
        outbuf = (time_char *)PyMem_Malloc(i*sizeof(time_char));
        if (outbuf == NULL) {
            PyErr_NoMemory();
            break;
        }
#if defined _MSC_VER && _MSC_VER >= 1400 && defined(__STDC_SECURE_LIB__)
        errno = 0;
#endif
        _Py_BEGIN_SUPPRESS_IPH
        buflen = format_time(outbuf, i, fmt, &buf);
        _Py_END_SUPPRESS_IPH
#if defined _MSC_VER && _MSC_VER >= 1400 && defined(__STDC_SECURE_LIB__)
        /* VisualStudio .NET 2005 does this properly */
        if (buflen == 0 && errno == EINVAL) {
            PyErr_SetString(PyExc_ValueError, "Invalid format string");
            PyMem_Free(outbuf);
            break;
        }
#endif
        if (buflen > 0 || i >= 256 * fmtlen) {
            /* If the buffer is 256 times as long as the format,
               it's probably not failing for lack of room!
               More likely, the format yields an empty result,
               e.g. an empty format, or %Z when the timezone
               is unknown. */
#ifdef HAVE_WCSFTIME
            ret = PyUnicode_FromWideChar(outbuf, buflen);
#else
            ret = PyUnicode_DecodeLocaleAndSize(outbuf, buflen, "surrogateescape");
#endif
            PyMem_Free(outbuf);
            break;
        }
        PyMem_Free(outbuf);
    }
#ifdef HAVE_WCSFTIME
    PyMem_Free(format);
#else
    Py_DECREF(format);
#endif
    return ret;
}

#undef time_char
#undef format_time
PyDoc_STRVAR(strftime_doc,
"strftime(format[, tuple]) -> string\n\
\n\
Convert a time tuple to a string according to a format specification.\n\
See the library reference manual for formatting codes. When the time tuple\n\
is not present, current time as returned by localtime() is used.\n\
\n" STRFTIME_FORMAT_CODES);
#endif /* HAVE_STRFTIME */

static PyObject *
time_strptime(PyObject *self, PyObject *args)
{
    PyObject *module, *func, *result;
    _Py_IDENTIFIER(_strptime_time);

    module = PyImport_ImportModuleNoBlock("_strptime");
    if (!module)
        return NULL;

    func = _PyObject_GetAttrId(module, &PyId__strptime_time);
    Py_DECREF(module);
    if (!func) {
        return NULL;
    }

    result = PyObject_Call(func, args, NULL);
    Py_DECREF(func);
    return result;
}


PyDoc_STRVAR(strptime_doc,
"strptime(string, format) -> struct_time\n\
\n\
Parse a string to a time tuple according to a format specification.\n\
See the library reference manual for formatting codes (same as\n\
strftime()).\n\
\n" STRFTIME_FORMAT_CODES);

static PyObject *
_asctime(struct tm *timeptr)
{
    /* Inspired by Open Group reference implementation available at
     * http://pubs.opengroup.org/onlinepubs/009695399/functions/asctime.html */
    static const char wday_name[7][4] = {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };
    static const char mon_name[12][4] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    return PyUnicode_FromFormat(
        "%s %s%3d %.2d:%.2d:%.2d %d",
        wday_name[timeptr->tm_wday],
        mon_name[timeptr->tm_mon],
        timeptr->tm_mday, timeptr->tm_hour,
        timeptr->tm_min, timeptr->tm_sec,
        1900 + timeptr->tm_year);
}

static PyObject *
time_asctime(PyObject *self, PyObject *args)
{
    PyObject *tup = NULL;
    struct tm buf;

    if (!PyArg_UnpackTuple(args, "asctime", 0, 1, &tup))
        return NULL;
    if (tup == NULL) {
        time_t tt = time(NULL);
        if (_PyTime_localtime(tt, &buf) != 0)
            return NULL;
    }
    else if (!gettmarg(tup, &buf,
                       "iiiiiiiii;asctime(): illegal time tuple argument") ||
             !checktm(&buf))
    {
        return NULL;
    }
    return _asctime(&buf);
}

PyDoc_STRVAR(asctime_doc,
"asctime([tuple]) -> string\n\
\n\
Convert a time tuple to a string, e.g. 'Sat Jun 06 16:26:11 1998'.\n\
When the time tuple is not present, current time as returned by localtime()\n\
is used.");

static PyObject *
time_ctime(PyObject *self, PyObject *args)
{
    time_t tt;
    struct tm buf;
    if (!parse_time_t_args(args, "|O:ctime", &tt))
        return NULL;
    if (_PyTime_localtime(tt, &buf) != 0)
        return NULL;
    return _asctime(&buf);
}

PyDoc_STRVAR(ctime_doc,
"ctime(seconds) -> string\n\
\n\
Convert a time in seconds since the Epoch to a string in local time.\n\
This is equivalent to asctime(localtime(seconds)). When the time tuple is\n\
not present, current time as returned by localtime() is used.");

#ifdef HAVE_MKTIME
static PyObject *
time_mktime(PyObject *self, PyObject *tup)
{
    struct tm buf;
    time_t tt;
#ifdef _AIX
    time_t clk;
    int     year = buf.tm_year;
    int     days = 0;
#endif

    if (!gettmarg(tup, &buf,
                  "iiiiiiiii;mktime(): illegal time tuple argument"))
    {
        return NULL;
    }
#ifndef _AIX
    buf.tm_wday = -1;  /* sentinel; original value ignored */
    tt = mktime(&buf);
#else
    /* year < 1902 or year > 2037 */
    if ((buf.tm_year < 2) || (buf.tm_year > 137))  {
        /* Issue #19748: On AIX, mktime() doesn't report overflow error for
         * timestamp < -2^31 or timestamp > 2**31-1. */
        PyErr_SetString(PyExc_OverflowError,
                        "mktime argument out of range");
        return NULL;
    }
    /* year < 1970 */
    if (year < 70) do {
        buf.tm_year += 4;
        days -= (366 + (365 * 3));
    } while (buf.tm_year < 70);

    buf.tm_wday = -1;
    clk = mktime(&buf);
    buf.tm_year = year;

    if (buf.tm_wday != -1 && days)
        buf.tm_wday = (buf.tm_wday + days) % 7;

    tt = clk + (days * (24 * 3600));
#endif
    /* Return value of -1 does not necessarily mean an error, but tm_wday
     * cannot remain set to -1 if mktime succeeded. */
    if (tt == (time_t)(-1)
        /* Return value of -1 does not necessarily mean an error, but
         * tm_wday cannot remain set to -1 if mktime succeeded. */
        && buf.tm_wday == -1)
    {
        PyErr_SetString(PyExc_OverflowError,
                        "mktime argument out of range");
        return NULL;
    }
    return PyFloat_FromDouble((double)tt);
}

PyDoc_STRVAR(mktime_doc,
"mktime(tuple) -> floating point number\n\
\n\
Convert a time tuple in local time to seconds since the Epoch.\n\
Note that mktime(gmtime(0)) will not generally return zero for most\n\
time zones; instead the returned value will either be equal to that\n\
of the timezone or altzone attributes on the time module.");
#endif /* HAVE_MKTIME */

#ifdef HAVE_WORKING_TZSET
static void PyInit_timezone(PyObject *module);

static PyObject *
time_tzset(PyObject *self, PyObject *unused)
{
    PyObject* m;

    m = PyImport_ImportModuleNoBlock("time");
    if (m == NULL) {
        return NULL;
    }

    tzset();

    /* Reset timezone, altzone, daylight and tzname */
    PyInit_timezone(m);
    Py_DECREF(m);
    if (PyErr_Occurred())
        return NULL;

    Py_RETURN_NONE;
}

PyDoc_STRVAR(tzset_doc,
"tzset()\n\
\n\
Initialize, or reinitialize, the local timezone to the value stored in\n\
os.environ['TZ']. The TZ environment variable should be specified in\n\
standard Unix timezone format as documented in the tzset man page\n\
(eg. 'US/Eastern', 'Europe/Amsterdam'). Unknown timezones will silently\n\
fall back to UTC. If the TZ environment variable is not set, the local\n\
timezone is set to the systems best guess of wallclock time.\n\
Changing the TZ environment variable without calling tzset *may* change\n\
the local timezone used by methods such as localtime, but this behaviour\n\
should not be relied on.");
#endif /* HAVE_WORKING_TZSET */

static PyObject *
time_monotonic(PyObject *self, PyObject *unused)
{
    _PyTime_t t = _PyTime_GetMonotonicClock();
    return _PyFloat_FromPyTime(t);
}

PyDoc_STRVAR(monotonic_doc,
"monotonic() -> float\n\
\n\
Monotonic clock, cannot go backward.");

static PyObject *
time_monotonic_ns(PyObject *self, PyObject *unused)
{
    _PyTime_t t = _PyTime_GetMonotonicClock();
    return _PyTime_AsNanosecondsObject(t);
}

PyDoc_STRVAR(monotonic_ns_doc,
"monotonic_ns() -> int\n\
\n\
Monotonic clock, cannot go backward, as nanoseconds.");

static PyObject *
time_perf_counter(PyObject *self, PyObject *unused)
{
    return perf_counter(NULL);
}

PyDoc_STRVAR(perf_counter_doc,
"perf_counter() -> float\n\
\n\
Performance counter for benchmarking.");

static PyObject *
time_perf_counter_ns(PyObject *self, PyObject *unused)
{
    _PyTime_t t = _PyTime_GetPerfCounter();
    return _PyTime_AsNanosecondsObject(t);
}

PyDoc_STRVAR(perf_counter_ns_doc,
"perf_counter_ns() -> int\n\
\n\
Performance counter for benchmarking as nanoseconds.");

static int
_PyTime_GetProcessTimeWithInfo(_PyTime_t *tp, _Py_clock_info_t *info)
{
#if defined(MS_WINDOWS)
    HANDLE process;
    FILETIME creation_time, exit_time, kernel_time, user_time;
    ULARGE_INTEGER large;
    _PyTime_t ktime, utime, t;
    BOOL ok;

    process = GetCurrentProcess();
    ok = GetProcessTimes(process, &creation_time, &exit_time,
                         &kernel_time, &user_time);
    if (!ok) {
        PyErr_SetFromWindowsErr(0);
        return -1;
    }

    if (info) {
        info->implementation = "GetProcessTimes()";
        info->resolution = 1e-7;
        info->monotonic = 1;
        info->adjustable = 0;
    }

    large.u.LowPart = kernel_time.dwLowDateTime;
    large.u.HighPart = kernel_time.dwHighDateTime;
    ktime = large.QuadPart;

    large.u.LowPart = user_time.dwLowDateTime;
    large.u.HighPart = user_time.dwHighDateTime;
    utime = large.QuadPart;

    /* ktime and utime have a resolution of 100 nanoseconds */
    t = _PyTime_FromNanoseconds((ktime + utime) * 100);
    *tp = t;
    return 0;
#else

    /* clock_gettime */
#if defined(HAVE_CLOCK_GETTIME) \
    && (defined(CLOCK_PROCESS_CPUTIME_ID) || defined(CLOCK_PROF))
    struct timespec ts;
#ifdef CLOCK_PROF
    const clockid_t clk_id = CLOCK_PROF;
    const char *function = "clock_gettime(CLOCK_PROF)";
#else
    const clockid_t clk_id = CLOCK_PROCESS_CPUTIME_ID;
    const char *function = "clock_gettime(CLOCK_PROCESS_CPUTIME_ID)";
#endif

    if (clock_gettime(clk_id, &ts) == 0) {
        if (info) {
            struct timespec res;
            info->implementation = function;
            info->monotonic = 1;
            info->adjustable = 0;
            if (clock_getres(clk_id, &res)) {
                PyErr_SetFromErrno(PyExc_OSError);
                return -1;
            }
            info->resolution = res.tv_sec + res.tv_nsec * 1e-9;
        }

        if (_PyTime_FromTimespec(tp, &ts) < 0) {
            return -1;
        }
        return 0;
    }
#endif

    /* getrusage(RUSAGE_SELF) */
#if defined(HAVE_SYS_RESOURCE_H)
    struct rusage ru;

    if (getrusage(RUSAGE_SELF, &ru) == 0) {
        _PyTime_t utime, stime;

        if (info) {
            info->implementation = "getrusage(RUSAGE_SELF)";
            info->monotonic = 1;
            info->adjustable = 0;
            info->resolution = 1e-6;
        }

        if (_PyTime_FromTimeval(&utime, &ru.ru_utime) < 0) {
            return -1;
        }
        if (_PyTime_FromTimeval(&stime, &ru.ru_stime) < 0) {
            return -1;
        }

        _PyTime_t total = utime + utime;
        *tp = total;
        return 0;
    }
#endif

    /* times() */
#ifdef HAVE_TIMES
    struct tms t;

    if (times(&t) != (clock_t)-1) {
        static long ticks_per_second = -1;

        if (ticks_per_second == -1) {
            long freq;
#if defined(HAVE_SYSCONF) && defined(_SC_CLK_TCK)
            freq = sysconf(_SC_CLK_TCK);
            if (freq < 1) {
                freq = -1;
            }
#elif defined(HZ)
            freq = HZ;
#else
            freq = 60; /* magic fallback value; may be bogus */
#endif

            if (freq != -1) {
                /* check that _PyTime_MulDiv(t, SEC_TO_NS, ticks_per_second)
                   cannot overflow below */
#if LONG_MAX > _PyTime_MAX / SEC_TO_NS
                if ((_PyTime_t)freq > _PyTime_MAX / SEC_TO_NS) {
                    PyErr_SetString(PyExc_OverflowError,
                                    "_SC_CLK_TCK is too large");
                    return -1;
                }
#endif

                ticks_per_second = freq;
            }
        }

        if (ticks_per_second != -1) {
            if (info) {
                info->implementation = "times()";
                info->monotonic = 1;
                info->adjustable = 0;
                info->resolution = 1.0 / (double)ticks_per_second;
            }

            _PyTime_t total;
            total = _PyTime_MulDiv(t.tms_utime, SEC_TO_NS, ticks_per_second);
            total += _PyTime_MulDiv(t.tms_stime, SEC_TO_NS, ticks_per_second);
            *tp = total;
            return 0;
        }
    }
#endif

    /* clock */
    /* Currently, Python 3 requires clock() to build: see issue #22624 */
    return _PyTime_GetClockWithInfo(tp, info);
#endif
}

static PyObject *
time_process_time(PyObject *self, PyObject *unused)
{
    _PyTime_t t;
    if (_PyTime_GetProcessTimeWithInfo(&t, NULL) < 0) {
        return NULL;
    }
    return _PyFloat_FromPyTime(t);
}

PyDoc_STRVAR(process_time_doc,
"process_time() -> float\n\
\n\
Process time for profiling: sum of the kernel and user-space CPU time.");

static PyObject *
time_process_time_ns(PyObject *self, PyObject *unused)
{
    _PyTime_t t;
    if (_PyTime_GetProcessTimeWithInfo(&t, NULL) < 0) {
        return NULL;
    }
    return _PyTime_AsNanosecondsObject(t);
}

PyDoc_STRVAR(process_time_ns_doc,
"process_time() -> int\n\
\n\
Process time for profiling as nanoseconds:\n\
sum of the kernel and user-space CPU time.");


#if defined(MS_WINDOWS)
#define HAVE_THREAD_TIME
static int
_PyTime_GetThreadTimeWithInfo(_PyTime_t *tp, _Py_clock_info_t *info)
{
    HANDLE thread;
    FILETIME creation_time, exit_time, kernel_time, user_time;
    ULARGE_INTEGER large;
    _PyTime_t ktime, utime, t;
    BOOL ok;

    thread =  GetCurrentThread();
    ok = GetThreadTimes(thread, &creation_time, &exit_time,
                        &kernel_time, &user_time);
    if (!ok) {
        PyErr_SetFromWindowsErr(0);
        return -1;
    }

    if (info) {
        info->implementation = "GetThreadTimes()";
        info->resolution = 1e-7;
        info->monotonic = 1;
        info->adjustable = 0;
    }

    large.u.LowPart = kernel_time.dwLowDateTime;
    large.u.HighPart = kernel_time.dwHighDateTime;
    ktime = large.QuadPart;

    large.u.LowPart = user_time.dwLowDateTime;
    large.u.HighPart = user_time.dwHighDateTime;
    utime = large.QuadPart;

    /* ktime and utime have a resolution of 100 nanoseconds */
    t = _PyTime_FromNanoseconds((ktime + utime) * 100);
    *tp = t;
    return 0;
}

#elif defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_PROCESS_CPUTIME_ID)
#define HAVE_THREAD_TIME
static int
_PyTime_GetThreadTimeWithInfo(_PyTime_t *tp, _Py_clock_info_t *info)
{
    struct timespec ts;
    const clockid_t clk_id = CLOCK_THREAD_CPUTIME_ID;
    const char *function = "clock_gettime(CLOCK_THREAD_CPUTIME_ID)";

    if (clock_gettime(clk_id, &ts)) {
        PyErr_SetFromErrno(PyExc_OSError);
        return -1;
    }
    if (info) {
        struct timespec res;
        info->implementation = function;
        info->monotonic = 1;
        info->adjustable = 0;
        if (clock_getres(clk_id, &res)) {
            PyErr_SetFromErrno(PyExc_OSError);
            return -1;
        }
        info->resolution = res.tv_sec + res.tv_nsec * 1e-9;
    }

    if (_PyTime_FromTimespec(tp, &ts) < 0) {
        return -1;
    }
    return 0;
}
#endif

#ifdef HAVE_THREAD_TIME
static PyObject *
time_thread_time(PyObject *self, PyObject *unused)
{
    _PyTime_t t;
    if (_PyTime_GetThreadTimeWithInfo(&t, NULL) < 0) {
        return NULL;
    }
    return _PyFloat_FromPyTime(t);
}

PyDoc_STRVAR(thread_time_doc,
"thread_time() -> float\n\
\n\
Thread time for profiling: sum of the kernel and user-space CPU time.");

static PyObject *
time_thread_time_ns(PyObject *self, PyObject *unused)
{
    _PyTime_t t;
    if (_PyTime_GetThreadTimeWithInfo(&t, NULL) < 0) {
        return NULL;
    }
    return _PyTime_AsNanosecondsObject(t);
}

PyDoc_STRVAR(thread_time_ns_doc,
"thread_time() -> int\n\
\n\
Thread time for profiling as nanoseconds:\n\
sum of the kernel and user-space CPU time.");
#endif


static PyObject *
time_get_clock_info(PyObject *self, PyObject *args)
{
    char *name;
    _Py_clock_info_t info;
    PyObject *obj = NULL, *dict, *ns;
    _PyTime_t t;

    if (!PyArg_ParseTuple(args, "s:get_clock_info", &name)) {
        return NULL;
    }

#ifdef Py_DEBUG
    info.implementation = NULL;
    info.monotonic = -1;
    info.adjustable = -1;
    info.resolution = -1.0;
#else
    info.implementation = "";
    info.monotonic = 0;
    info.adjustable = 0;
    info.resolution = 1.0;
#endif

    if (strcmp(name, "time") == 0) {
        if (_PyTime_GetSystemClockWithInfo(&t, &info) < 0) {
            return NULL;
        }
    }
#ifdef PYCLOCK
    else if (strcmp(name, "clock") == 0) {
        obj = pyclock(&info);
        if (obj == NULL) {
            return NULL;
        }
        Py_DECREF(obj);
    }
#endif
    else if (strcmp(name, "monotonic") == 0) {
        if (_PyTime_GetMonotonicClockWithInfo(&t, &info) < 0) {
            return NULL;
        }
    }
    else if (strcmp(name, "perf_counter") == 0) {
        if (_PyTime_GetPerfCounterWithInfo(&t, &info) < 0) {
            return NULL;
        }
    }
    else if (strcmp(name, "process_time") == 0) {
        if (_PyTime_GetProcessTimeWithInfo(&t, &info) < 0) {
            return NULL;
        }
    }
#ifdef HAVE_THREAD_TIME
    else if (strcmp(name, "thread_time") == 0) {
        if (_PyTime_GetThreadTimeWithInfo(&t, &info) < 0) {
            return NULL;
        }
    }
#endif
    else {
        PyErr_SetString(PyExc_ValueError, "unknown clock");
        return NULL;
    }

    dict = PyDict_New();
    if (dict == NULL) {
        return NULL;
    }

    assert(info.implementation != NULL);
    obj = PyUnicode_FromString(info.implementation);
    if (obj == NULL) {
        goto error;
    }
    if (PyDict_SetItemString(dict, "implementation", obj) == -1) {
        goto error;
    }
    Py_CLEAR(obj);

    assert(info.monotonic != -1);
    obj = PyBool_FromLong(info.monotonic);
    if (obj == NULL) {
        goto error;
    }
    if (PyDict_SetItemString(dict, "monotonic", obj) == -1) {
        goto error;
    }
    Py_CLEAR(obj);

    assert(info.adjustable != -1);
    obj = PyBool_FromLong(info.adjustable);
    if (obj == NULL) {
        goto error;
    }
    if (PyDict_SetItemString(dict, "adjustable", obj) == -1) {
        goto error;
    }
    Py_CLEAR(obj);

    assert(info.resolution > 0.0);
    assert(info.resolution <= 1.0);
    obj = PyFloat_FromDouble(info.resolution);
    if (obj == NULL) {
        goto error;
    }
    if (PyDict_SetItemString(dict, "resolution", obj) == -1) {
        goto error;
    }
    Py_CLEAR(obj);

    ns = _PyNamespace_New(dict);
    Py_DECREF(dict);
    return ns;

error:
    Py_DECREF(dict);
    Py_XDECREF(obj);
    return NULL;
}

PyDoc_STRVAR(get_clock_info_doc,
"get_clock_info(name: str) -> dict\n\
\n\
Get information of the specified clock.");

static void
PyInit_timezone(PyObject *m) {
    /* This code moved from PyInit_time wholesale to allow calling it from
    time_tzset. In the future, some parts of it can be moved back
    (for platforms that don't HAVE_WORKING_TZSET, when we know what they
    are), and the extraneous calls to tzset(3) should be removed.
    I haven't done this yet, as I don't want to change this code as
    little as possible when introducing the time.tzset and time.tzsetwall
    methods. This should simply be a method of doing the following once,
    at the top of this function and removing the call to tzset() from
    time_tzset():

        #ifdef HAVE_TZSET
        tzset()
        #endif

    And I'm lazy and hate C so nyer.
     */
    PyObject *otz0, *otz1;
    tzset();
    PyModule_AddIntConstant(m, "timezone", timezone);
#ifdef HAVE_ALTZONE
    PyModule_AddIntConstant(m, "altzone", altzone);
#elif defined(HAVE_STRUCT_TM_TM_ZONE)
    {
        static const time_t YEAR = (365 * 24 + 6) * 3600;
        time_t t;
        struct tm p;
        long janzone, julyzone;
        t = (time((time_t *)0) / YEAR) * YEAR;
        _PyTime_localtime(t, &p);
        janzone = -p.tm_gmtoff;
        t += YEAR/2;
        _PyTime_localtime(t, &p);
        julyzone = -p.tm_gmtoff;

        // DST is reversed in the southern hemisphere.
        PyModule_AddIntConstant(m, "altzone",
            (janzone < julyzone) ? janzone : julyzone);
    }
#else
    PyModule_AddIntConstant(m, "altzone", timezone-3600);
#endif
    PyModule_AddIntConstant(m, "daylight", daylight);
    otz0 = PyUnicode_DecodeLocale(tzname[0], "surrogateescape");
    otz1 = PyUnicode_DecodeLocale(tzname[1], "surrogateescape");
    PyModule_AddObject(m, "tzname", Py_BuildValue("(NN)", otz0, otz1));
}


static PyMethodDef time_methods[] = {
    {"time",            time_time, METH_NOARGS, time_doc},
    {"time_ns",         time_time_ns, METH_NOARGS, time_ns_doc},
#ifdef PYCLOCK
    {"clock",           time_clock, METH_NOARGS, clock_doc},
#endif
#ifdef HAVE_CLOCK_GETTIME
    {"clock_gettime",   time_clock_gettime, METH_VARARGS, clock_gettime_doc},
    {"clock_gettime_ns",time_clock_gettime_ns, METH_VARARGS, clock_gettime_ns_doc},
#endif
#ifdef HAVE_CLOCK_SETTIME
    {"clock_settime",   time_clock_settime, METH_VARARGS, clock_settime_doc},
    {"clock_settime_ns",time_clock_settime_ns, METH_VARARGS, clock_settime_ns_doc},
#endif
#ifdef HAVE_CLOCK_GETRES
    {"clock_getres",    time_clock_getres, METH_VARARGS, clock_getres_doc},
#endif
#ifdef HAVE_PTHREAD_GETCPUCLOCKID
    {"pthread_getcpuclockid", time_pthread_getcpuclockid, METH_VARARGS, pthread_getcpuclockid_doc},
#endif
    {"sleep",           time_sleep, METH_O, sleep_doc},
    {"gmtime",          time_gmtime, METH_VARARGS, gmtime_doc},
    {"localtime",       time_localtime, METH_VARARGS, localtime_doc},
    {"asctime",         time_asctime, METH_VARARGS, asctime_doc},
    {"ctime",           time_ctime, METH_VARARGS, ctime_doc},
#ifdef HAVE_MKTIME
    {"mktime",          time_mktime, METH_O, mktime_doc},
#endif
#ifdef HAVE_STRFTIME
    {"strftime",        time_strftime, METH_VARARGS, strftime_doc},
#endif
    {"strptime",        time_strptime, METH_VARARGS, strptime_doc},
#ifdef HAVE_WORKING_TZSET
    {"tzset",           time_tzset, METH_NOARGS, tzset_doc},
#endif
    {"monotonic",       time_monotonic, METH_NOARGS, monotonic_doc},
    {"monotonic_ns",    time_monotonic_ns, METH_NOARGS, monotonic_ns_doc},
    {"process_time",    time_process_time, METH_NOARGS, process_time_doc},
    {"process_time_ns", time_process_time_ns, METH_NOARGS, process_time_ns_doc},
#ifdef HAVE_THREAD_TIME
    {"thread_time",     time_thread_time, METH_NOARGS, thread_time_doc},
    {"thread_time_ns",  time_thread_time_ns, METH_NOARGS, thread_time_ns_doc},
#endif
    {"perf_counter",    time_perf_counter, METH_NOARGS, perf_counter_doc},
    {"perf_counter_ns", time_perf_counter_ns, METH_NOARGS, perf_counter_ns_doc},
    {"get_clock_info",  time_get_clock_info, METH_VARARGS, get_clock_info_doc},
    {NULL,              NULL}           /* sentinel */
};


PyDoc_STRVAR(module_doc,
"This module provides various functions to manipulate time values.\n\
\n\
There are two standard representations of time.  One is the number\n\
of seconds since the Epoch, in UTC (a.k.a. GMT).  It may be an integer\n\
or a floating point number (to represent fractions of seconds).\n\
The Epoch is system-defined; on Unix, it is generally January 1st, 1970.\n\
The actual value can be retrieved by calling gmtime(0).\n\
\n\
The other representation is a tuple of 9 integers giving local time.\n\
The tuple items are:\n\
  year (including century, e.g. 1998)\n\
  month (1-12)\n\
  day (1-31)\n\
  hours (0-23)\n\
  minutes (0-59)\n\
  seconds (0-59)\n\
  weekday (0-6, Monday is 0)\n\
  Julian day (day in the year, 1-366)\n\
  DST (Daylight Savings Time) flag (-1, 0 or 1)\n\
If the DST flag is 0, the time is given in the regular time zone;\n\
if it is 1, the time is given in the DST time zone;\n\
if it is -1, mktime() should guess based on the date and time.\n");



static struct PyModuleDef timemodule = {
    PyModuleDef_HEAD_INIT,
    "time",
    module_doc,
    -1,
    time_methods,
    NULL,
    NULL,
    NULL,
    NULL
};

PyMODINIT_FUNC
PyInit_time(void)
{
    PyObject *m;
    m = PyModule_Create(&timemodule);
    if (m == NULL)
        return NULL;

    /* Set, or reset, module variables like time.timezone */
    PyInit_timezone(m);

#ifdef CLOCK_REALTIME
    PyModule_AddIntMacro(m, CLOCK_REALTIME);
#endif
#ifdef CLOCK_MONOTONIC
    PyModule_AddIntMacro(m, CLOCK_MONOTONIC);
#endif
#ifdef CLOCK_MONOTONIC_RAW
    PyModule_AddIntMacro(m, CLOCK_MONOTONIC_RAW);
#endif
#ifdef CLOCK_HIGHRES
    PyModule_AddIntMacro(m, CLOCK_HIGHRES);
#endif
#ifdef CLOCK_PROCESS_CPUTIME_ID
    PyModule_AddIntMacro(m, CLOCK_PROCESS_CPUTIME_ID);
#endif
#ifdef CLOCK_THREAD_CPUTIME_ID
    PyModule_AddIntMacro(m, CLOCK_THREAD_CPUTIME_ID);
#endif
#ifdef CLOCK_PROF
    PyModule_AddIntMacro(m, CLOCK_PROF);
#endif
#ifdef CLOCK_BOOTTIME
    PyModule_AddIntMacro(m, CLOCK_BOOTTIME);
#endif
#ifdef CLOCK_UPTIME
    PyModule_AddIntMacro(m, CLOCK_UPTIME);
#endif

    if (!initialized) {
        if (PyStructSequence_InitType2(&StructTimeType,
                                       &struct_time_type_desc) < 0)
            return NULL;
    }
    Py_INCREF(&StructTimeType);
    PyModule_AddIntConstant(m, "_STRUCT_TM_ITEMS", 11);
    PyModule_AddObject(m, "struct_time", (PyObject*) &StructTimeType);
    initialized = 1;

#if defined(__linux__) && !defined(__GLIBC__)
    struct tm tm;
    const time_t zero = 0;
    if (gmtime_r(&zero, &tm) != NULL)
        utc_string = tm.tm_zone;
#endif

    return m;
}

/* Implement pysleep() for various platforms.
   When interrupted (or when another error occurs), return -1 and
   set an exception; else return 0. */

static int
pysleep(_PyTime_t secs)
{
    _PyTime_t deadline, monotonic;
#ifndef MS_WINDOWS
    struct timeval timeout;
    int err = 0;
#else
    _PyTime_t millisecs;
    unsigned long ul_millis;
    DWORD rc;
    HANDLE hInterruptEvent;
#endif

    deadline = _PyTime_GetMonotonicClock() + secs;

    do {
#ifndef MS_WINDOWS
        if (_PyTime_AsTimeval(secs, &timeout, _PyTime_ROUND_CEILING) < 0)
            return -1;

        Py_BEGIN_ALLOW_THREADS
        err = select(0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &timeout);
        Py_END_ALLOW_THREADS

        if (err == 0)
            break;

        if (errno != EINTR) {
            PyErr_SetFromErrno(PyExc_OSError);
            return -1;
        }
#else
        millisecs = _PyTime_AsMilliseconds(secs, _PyTime_ROUND_CEILING);
        if (millisecs > (double)ULONG_MAX) {
            PyErr_SetString(PyExc_OverflowError,
                            "sleep length is too large");
            return -1;
        }

        /* Allow sleep(0) to maintain win32 semantics, and as decreed
         * by Guido, only the main thread can be interrupted.
         */
        ul_millis = (unsigned long)millisecs;
        if (ul_millis == 0 || !_PyOS_IsMainThread()) {
            Py_BEGIN_ALLOW_THREADS
            Sleep(ul_millis);
            Py_END_ALLOW_THREADS
            break;
        }

        hInterruptEvent = _PyOS_SigintEvent();
        ResetEvent(hInterruptEvent);

        Py_BEGIN_ALLOW_THREADS
        rc = WaitForSingleObjectEx(hInterruptEvent, ul_millis, FALSE);
        Py_END_ALLOW_THREADS

        if (rc != WAIT_OBJECT_0)
            break;
#endif

        /* sleep was interrupted by SIGINT */
        if (PyErr_CheckSignals())
            return -1;

        monotonic = _PyTime_GetMonotonicClock();
        secs = deadline - monotonic;
        if (secs < 0)
            break;
        /* retry with the recomputed delay */
    } while (1);

    return 0;
}
