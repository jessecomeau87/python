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

// XXX Order by frequency of use?

struct _Py_global_strings {
    struct {
        STR(empty, "")
        STR(newline, "\n")  // nl
        STR(dot, ".")

        STR(comma_sep, ", ")  // comma_id

        STR(br_open, "{")
        STR(br_close, "}")
        STR(br_dbl_open, "{{")
        STR(br_dbl_close, "}}")

        STR(anon_dictcomp, "<dictcomp>")
        STR(anon_genexpr, "<genexpr>")
        STR(anon_lambda, "<lambda>")
        STR(anon_listcomp, "<listcomp>")
        STR(anon_module, "<module>")
        STR(anon_setcomp, "<setcomp>")
        STR(anon_string, "<string>")
        STR(dot_locals, ".<locals>")

        STR(replace_inf, "1eNN")
        //STR(replace_inf, ['1', 'e', 1 + DBL_MAX_10_EXP])

		STR(latin1_0, "\x00")
		STR(latin1_1, "\x01")
		STR(latin1_2, "\x02")
		STR(latin1_3, "\x03")
		STR(latin1_4, "\x04")
		STR(latin1_5, "\x05")
		STR(latin1_6, "\x06")
		STR(latin1_7, "\x07")
		STR(latin1_8, "\x08")
		STR(latin1_9, "\x09")
		STR(latin1_10, "\x0A")
		STR(latin1_11, "\x0B")
		STR(latin1_12, "\x0C")
		STR(latin1_13, "\x0D")
		STR(latin1_14, "\x0E")
		STR(latin1_15, "\x0F")
		STR(latin1_16, "\x10")
		STR(latin1_17, "\x11")
		STR(latin1_18, "\x12")
		STR(latin1_19, "\x13")
		STR(latin1_20, "\x14")
		STR(latin1_21, "\x15")
		STR(latin1_22, "\x16")
		STR(latin1_23, "\x17")
		STR(latin1_24, "\x18")
		STR(latin1_25, "\x19")
		STR(latin1_26, "\x1A")
		STR(latin1_27, "\x1B")
		STR(latin1_28, "\x1C")
		STR(latin1_29, "\x1D")
		STR(latin1_30, "\x1E")
		STR(latin1_31, "\x1F")
		STR(latin1_32, "\x20")
		STR(latin1_33, "\x21")
		STR(latin1_34, "\x22")
		STR(latin1_35, "\x23")
		STR(latin1_36, "\x24")
		STR(latin1_37, "\x25")
		STR(latin1_38, "\x26")
		STR(latin1_39, "\x27")
		STR(latin1_40, "\x28")
		STR(latin1_41, "\x29")
		STR(latin1_42, "\x2A")
		STR(latin1_43, "\x2B")
		STR(latin1_44, "\x2C")
		STR(latin1_45, "\x2D")
		STR(latin1_46, "\x2E")
		STR(latin1_47, "\x2F")
		STR(latin1_48, "\x30")
		STR(latin1_49, "\x31")
		STR(latin1_50, "\x32")
		STR(latin1_51, "\x33")
		STR(latin1_52, "\x34")
		STR(latin1_53, "\x35")
		STR(latin1_54, "\x36")
		STR(latin1_55, "\x37")
		STR(latin1_56, "\x38")
		STR(latin1_57, "\x39")
		STR(latin1_58, "\x3A")
		STR(latin1_59, "\x3B")
		STR(latin1_60, "\x3C")
		STR(latin1_61, "\x3D")
		STR(latin1_62, "\x3E")
		STR(latin1_63, "\x3F")
		STR(latin1_64, "\x40")
		STR(latin1_65, "\x41")
		STR(latin1_66, "\x42")
		STR(latin1_67, "\x43")
		STR(latin1_68, "\x44")
		STR(latin1_69, "\x45")
		STR(latin1_70, "\x46")
		STR(latin1_71, "\x47")
		STR(latin1_72, "\x48")
		STR(latin1_73, "\x49")
		STR(latin1_74, "\x4A")
		STR(latin1_75, "\x4B")
		STR(latin1_76, "\x4C")
		STR(latin1_77, "\x4D")
		STR(latin1_78, "\x4E")
		STR(latin1_79, "\x4F")
		STR(latin1_80, "\x50")
		STR(latin1_81, "\x51")
		STR(latin1_82, "\x52")
		STR(latin1_83, "\x53")
		STR(latin1_84, "\x54")
		STR(latin1_85, "\x55")
		STR(latin1_86, "\x56")
		STR(latin1_87, "\x57")
		STR(latin1_88, "\x58")
		STR(latin1_89, "\x59")
		STR(latin1_90, "\x5A")
		STR(latin1_91, "\x5B")
		STR(latin1_92, "\x5C")
		STR(latin1_93, "\x5D")
		STR(latin1_94, "\x5E")
		STR(latin1_95, "\x5F")
		STR(latin1_96, "\x60")
		STR(latin1_97, "\x61")
		STR(latin1_98, "\x62")
		STR(latin1_99, "\x63")
		STR(latin1_100, "\x64")
		STR(latin1_101, "\x65")
		STR(latin1_102, "\x66")
		STR(latin1_103, "\x67")
		STR(latin1_104, "\x68")
		STR(latin1_105, "\x69")
		STR(latin1_106, "\x6A")
		STR(latin1_107, "\x6B")
		STR(latin1_108, "\x6C")
		STR(latin1_109, "\x6D")
		STR(latin1_110, "\x6E")
		STR(latin1_111, "\x6F")
		STR(latin1_112, "\x70")
		STR(latin1_113, "\x71")
		STR(latin1_114, "\x72")
		STR(latin1_115, "\x73")
		STR(latin1_116, "\x74")
		STR(latin1_117, "\x75")
		STR(latin1_118, "\x76")
		STR(latin1_119, "\x77")
		STR(latin1_120, "\x78")
		STR(latin1_121, "\x79")
		STR(latin1_122, "\x7A")
		STR(latin1_123, "\x7B")
		STR(latin1_124, "\x7C")
		STR(latin1_125, "\x7D")
		STR(latin1_126, "\x7E")
		STR(latin1_127, "\x7F")
		STR(latin1_128, "\x80")
		STR(latin1_129, "\x81")
		STR(latin1_130, "\x82")
		STR(latin1_131, "\x83")
		STR(latin1_132, "\x84")
		STR(latin1_133, "\x85")
		STR(latin1_134, "\x86")
		STR(latin1_135, "\x87")
		STR(latin1_136, "\x88")
		STR(latin1_137, "\x89")
		STR(latin1_138, "\x8A")
		STR(latin1_139, "\x8B")
		STR(latin1_140, "\x8C")
		STR(latin1_141, "\x8D")
		STR(latin1_142, "\x8E")
		STR(latin1_143, "\x8F")
		STR(latin1_144, "\x90")
		STR(latin1_145, "\x91")
		STR(latin1_146, "\x92")
		STR(latin1_147, "\x93")
		STR(latin1_148, "\x94")
		STR(latin1_149, "\x95")
		STR(latin1_150, "\x96")
		STR(latin1_151, "\x97")
		STR(latin1_152, "\x98")
		STR(latin1_153, "\x99")
		STR(latin1_154, "\x9A")
		STR(latin1_155, "\x9B")
		STR(latin1_156, "\x9C")
		STR(latin1_157, "\x9D")
		STR(latin1_158, "\x9E")
		STR(latin1_159, "\x9F")
		STR(latin1_160, "\xA0")
		STR(latin1_161, "\xA1")
		STR(latin1_162, "\xA2")
		STR(latin1_163, "\xA3")
		STR(latin1_164, "\xA4")
		STR(latin1_165, "\xA5")
		STR(latin1_166, "\xA6")
		STR(latin1_167, "\xA7")
		STR(latin1_168, "\xA8")
		STR(latin1_169, "\xA9")
		STR(latin1_170, "\xAA")
		STR(latin1_171, "\xAB")
		STR(latin1_172, "\xAC")
		STR(latin1_173, "\xAD")
		STR(latin1_174, "\xAE")
		STR(latin1_175, "\xAF")
		STR(latin1_176, "\xB0")
		STR(latin1_177, "\xB1")
		STR(latin1_178, "\xB2")
		STR(latin1_179, "\xB3")
		STR(latin1_180, "\xB4")
		STR(latin1_181, "\xB5")
		STR(latin1_182, "\xB6")
		STR(latin1_183, "\xB7")
		STR(latin1_184, "\xB8")
		STR(latin1_185, "\xB9")
		STR(latin1_186, "\xBA")
		STR(latin1_187, "\xBB")
		STR(latin1_188, "\xBC")
		STR(latin1_189, "\xBD")
		STR(latin1_190, "\xBE")
		STR(latin1_191, "\xBF")
		STR(latin1_192, "\xC0")
		STR(latin1_193, "\xC1")
		STR(latin1_194, "\xC2")
		STR(latin1_195, "\xC3")
		STR(latin1_196, "\xC4")
		STR(latin1_197, "\xC5")
		STR(latin1_198, "\xC6")
		STR(latin1_199, "\xC7")
		STR(latin1_200, "\xC8")
		STR(latin1_201, "\xC9")
		STR(latin1_202, "\xCA")
		STR(latin1_203, "\xCB")
		STR(latin1_204, "\xCC")
		STR(latin1_205, "\xCD")
		STR(latin1_206, "\xCE")
		STR(latin1_207, "\xCF")
		STR(latin1_208, "\xD0")
		STR(latin1_209, "\xD1")
		STR(latin1_210, "\xD2")
		STR(latin1_211, "\xD3")
		STR(latin1_212, "\xD4")
		STR(latin1_213, "\xD5")
		STR(latin1_214, "\xD6")
		STR(latin1_215, "\xD7")
		STR(latin1_216, "\xD8")
		STR(latin1_217, "\xD9")
		STR(latin1_218, "\xDA")
		STR(latin1_219, "\xDB")
		STR(latin1_220, "\xDC")
		STR(latin1_221, "\xDD")
		STR(latin1_222, "\xDE")
		STR(latin1_223, "\xDF")
		STR(latin1_224, "\xE0")
		STR(latin1_225, "\xE1")
		STR(latin1_226, "\xE2")
		STR(latin1_227, "\xE3")
		STR(latin1_228, "\xE4")
		STR(latin1_229, "\xE5")
		STR(latin1_230, "\xE6")
		STR(latin1_231, "\xE7")
		STR(latin1_232, "\xE8")
		STR(latin1_233, "\xE9")
		STR(latin1_234, "\xEA")
		STR(latin1_235, "\xEB")
		STR(latin1_236, "\xEC")
		STR(latin1_237, "\xED")
		STR(latin1_238, "\xEE")
		STR(latin1_239, "\xEF")
		STR(latin1_240, "\xF0")
		STR(latin1_241, "\xF1")
		STR(latin1_242, "\xF2")
		STR(latin1_243, "\xF3")
		STR(latin1_244, "\xF4")
		STR(latin1_245, "\xF5")
		STR(latin1_246, "\xF6")
		STR(latin1_247, "\xF7")
		STR(latin1_248, "\xF8")
		STR(latin1_249, "\xF9")
		STR(latin1_250, "\xFA")
		STR(latin1_251, "\xFB")
		STR(latin1_252, "\xFC")
		STR(latin1_253, "\xFD")
		STR(latin1_254, "\xFE")
		STR(latin1_255, "\xFF")
    } literals;

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
        ID(append)
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
        ID(default)
        ID(defaultaction)
        ID(difference_update)
        ID(displayhook)
        ID(enable)
        ID(encode)
        ID(encoding)
        ID(end_lineno)
        ID(end_offset)
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
        ID(ignore)
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

#define _Py_GET_GLOBAL_STRING(NAME) \
     (&_Py_SINGLETON(strings.literals.$ ## NAME._ascii.ob_base))


#ifdef __cplusplus
}
#endif
#endif /* !Py_INTERNAL_GLOBAL_STRINGS_H */
