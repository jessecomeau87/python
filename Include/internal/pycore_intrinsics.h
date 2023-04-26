
/* Unary Functions: */

#define INTRINSIC_PRINT 1
#define INTRINSIC_IMPORT_STAR 2
#define INTRINSIC_STOPITERATION_ERROR 3
#define INTRINSIC_ASYNC_GEN_WRAP 4
#define INTRINSIC_UNARY_POSITIVE 5
#define INTRINSIC_LIST_TO_TUPLE 6
#define INTRINSIC_TYPEVAR 7
#define INTRINSIC_PARAMSPEC 8
#define INTRINSIC_TYPEVARTUPLE 9
#define INTRINSIC_SUBSCRIPT_GENERIC 10
#define INTRINSIC_TYPEALIAS 11

#define MAX_INTRINSIC_1 11


/* Binary Functions: */

#define INTRINSIC_PREP_RERAISE_STAR 1
#define INTRINSIC_SET_CLASS_DICT    2
#define INTRINSIC_TYPEVAR_WITH_BOUND 3
#define INTRINSIC_TYPEVAR_WITH_CONSTRAINTS 4
#define INTRINSIC_SET_FUNCTION_TYPE_PARAMS 5

#define MAX_INTRINSIC_2 5


typedef PyObject *(*instrinsic_func1)(PyThreadState* tstate, PyObject *value);
typedef PyObject *(*instrinsic_func2)(PyThreadState* tstate, PyObject *value1, PyObject *value2);

extern const instrinsic_func1 _PyIntrinsics_UnaryFunctions[];
extern const instrinsic_func2 _PyIntrinsics_BinaryFunctions[];

