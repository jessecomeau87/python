#ifndef Py_INTERNAL_GLOBAL_STRINGS_H
#define Py_INTERNAL_GLOBAL_STRINGS_H
#ifdef __cplusplus
extern "C" {
#endif

#ifndef Py_BUILD_CORE
#  error "this header requires Py_BUILD_CORE define"
#endif

// The data structure & init here are inspired by Tools/scripts/deepfreeze.py.

// All field names generated by STR() have an extra leading dollar sign.
// This helps avoid collisions with keywords, etc.

#define STR(NAME, LITERAL) \
    struct { \
        PyASCIIObject _ascii; \
        uint8_t _data[_Py_STRING_LENGTH(LITERAL)]; \
    } $ ## NAME;
#define ID(NAME) \
    STR(NAME, #NAME)

struct _Py_global_strings {
    struct {
        ID(Py_Repr)
        ID(TextIOWrapper)
        ID(WarningMessage)
        ID(_)
        ID(__IOBase_closed)
        ID(__abs__)
        ID(__abstractmethods__)
        ID(__add__)
        ID(__aenter__)
        ID(__aexit__)
        ID(__aiter__)
        ID(__all__)
        ID(__and__)
        ID(__anext__)
        ID(__annotations__)
        ID(__await__)
        ID(__bases__)
        ID(__bool__)
        ID(__build_class__)
        ID(__builtins__)
        ID(__bytes__)
        ID(__call__)
        ID(__class__)
        ID(__class_getitem__)
        ID(__classcell__)
        ID(__complex__)
        ID(__contains__)
        ID(__copy__)
        ID(__del__)
        ID(__delattr__)
        ID(__delete__)
        ID(__delitem__)
        ID(__dict__)
        ID(__dir__)
        ID(__divmod__)
        ID(__doc__)  // docstr
        ID(__enter__)
        ID(__eq__)
        ID(__exit__)
        ID(__file__)
        ID(__float__)
        ID(__floordiv__)
        ID(__format__)
        ID(__fspath__)
        ID(__ge__)
        ID(__get__)
        ID(__getattr__)
        ID(__getattribute__)
        ID(__getitem__)
        ID(__getnewargs__)
        ID(__getnewargs_ex__)
        ID(__getstate__)
        ID(__gt__)
        ID(__hash__)
        ID(__iadd__)
        ID(__iand__)
        ID(__ifloordiv__)
        ID(__ilshift__)
        ID(__imatmul__)
        ID(__imod__)
        ID(__import__)
        ID(__imul__)
        ID(__index__)
        ID(__init__)
        ID(__init_subclass__)
        ID(__instancecheck__)
        ID(__int__)
        ID(__invert__)
        ID(__ior__)
        ID(__ipow__)
        ID(__irshift__)
        ID(__isabstractmethod__)
        ID(__isub__)
        ID(__iter__)
        ID(__itruediv__)
        ID(__ixor__)
        ID(__le__)
        ID(__len__)
        ID(__length_hint__)
        ID(__loader__)
        ID(__lshift__)
        ID(__lt__)
        ID(__ltrace__)
        ID(__main__)
        ID(__matmul__)
        ID(__missing__)
        ID(__mod__)
        ID(__module__)
        ID(__mro_entries__)
        ID(__mul__)
        ID(__name__)
        ID(__ne__)
        ID(__neg__)
        ID(__new__)
        ID(__newobj__)
        ID(__newobj_ex__)
        ID(__next__)
        ID(__or__)
        ID(__package__)
        ID(__path__)
        ID(__pos__)
        ID(__pow__)
        ID(__prepare__)
        ID(__qualname__)
        ID(__radd__)
        ID(__rand__)
        ID(__rdivmod__)
        ID(__reduce__)
        ID(__repr__)
        ID(__reversed__)
        ID(__rfloordiv__)
        ID(__rlshift__)
        ID(__rmatmul__)
        ID(__rmod__)
        ID(__rmul__)
        ID(__ror__)
        ID(__round__)
        ID(__rpow__)
        ID(__rrshift__)
        ID(__rshift__)
        ID(__rsub__)
        ID(__rtruediv__)
        ID(__rxor__)
        ID(__set__)
        ID(__set_name__)
        ID(__setattr__)
        ID(__setitem__)
        ID(__setstate__)
        ID(__sizeof__)
        ID(__slotnames__)
        ID(__slots__)
        ID(__spec__)
        ID(__str__)
        ID(__sub__)
        ID(__subclasscheck__)
        ID(__subclasshook__)
        ID(__truediv__)
        ID(__trunc__)
        ID(__warningregistry__)
        ID(__xor__)
        ID(_abc_impl)
        ID(_blksize)
        ID(_bootstrap)
        ID(_dealloc_warn)
        ID(_finalizing)
        ID(_find_and_load)
        ID(_fix_up_module)
        ID(_get_sourcefile)
        ID(_handle_fromlist)
        ID(_initializing)
        ID(_is_text_encoding)
        ID(_lock_unlock_module)
        ID(_showwarnmsg)
        ID(_shutdown)
        ID(_slotnames)
        ID(_strptime_time)
        ID(_warn_unawaited_coroutine)
        ID(_xoptions)
        ID(big)
        ID(buffer)
        ID(builtins)
        ID(c_call)
        ID(c_exception)
        ID(c_return)
        ID(call)
        ID(clear)
        ID(close)
        ID(closed)
        ID(code)
        ID(copy)
        ID(copyreg)
        ID(decode)
        ID(defaultaction)
        ID(difference_update)
        ID(displayhook)
        ID(enable)
        ID(encode)
        ID(encoding)
        ID(errors)
        ID(excepthook)
        ID(exception)
        ID(extend)
        ID(filename)
        ID(fileno)
        ID(fillvalue)
        ID(filters)
        ID(flush)
        ID(get)
        ID(get_source)
        ID(getattr)
        ID(getpreferredencoding)
        ID(getstate)
        ID(imp)
        ID(importlib)
        ID(inf)
        ID(intersection_update)
        ID(isatty)
        ID(items)
        ID(iter)
        ID(keys)
        ID(last_traceback)
        ID(last_type)
        ID(last_value)
        ID(line)
        ID(lineno)
        ID(little)
        ID(locale)
        ID(match)
        ID(metaclass)
        ID(mode)
        ID(mro)
        ID(msg)
        ID(n_fields)
        ID(n_sequence_fields)
        ID(n_unnamed_fields)
        ID(name)
        ID(newlines)
        ID(offset)
        ID(onceregistry)
        ID(opcode)
        ID(open)
        ID(parent)
        ID(partial)
        ID(path)
        ID(peek)
        ID(print_file_and_line)
        ID(ps1)
        ID(ps2)
        ID(raw)
        ID(read)
        ID(read1)
        ID(readable)
        ID(readall)
        ID(readinto)
        ID(readinto1)
        ID(readline)
        ID(reload)
        ID(replace)
        ID(reset)
        ID(return)
        ID(reversed)
        ID(seek)
        ID(seekable)
        ID(send)
        ID(setstate)
        ID(sort)
        ID(st_mode)
        ID(stderr)
        ID(stdin)
        ID(stdout)
        ID(strict)
        ID(struct_rusage)
        ID(symmetric_difference_update)
        ID(tell)
        ID(text)
        ID(threading)
        ID(throw)
        ID(truncate)
        ID(unraisablehook)
        ID(update)
        ID(values)
        ID(version)
        ID(warnings)
        ID(warnoptions)
        ID(writable)
        ID(write)
        ID(zipimporter)
    } identifiers;
};

#undef ID
#undef STR


#define _Py_GET_GLOBAL_IDENTIFIER(NAME) \
     (&_Py_SINGLETON(strings.identifiers.$ ## NAME._ascii.ob_base))


#ifdef __cplusplus
}
#endif
#endif /* !Py_INTERNAL_GLOBAL_STRINGS_H */
