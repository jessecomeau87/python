/* Implementation helper: a struct that looks like a tuple.  See timemodule
   and posixmodule for example uses. */

#include "Python.h"
#include "structmember.h"

static const char visible_length_key[] = "n_sequence_fields";
static const char real_length_key[] = "n_fields";
static const char unnamed_fields_key[] = "n_unnamed_fields";

/* Fields with this name have only a field index, not a field name.
   They are only allowed for indices < n_visible_fields. */
char *PyStructSequence_UnnamedField = "unnamed field";
_Py_IDENTIFIER(n_sequence_fields);
_Py_IDENTIFIER(n_fields);
_Py_IDENTIFIER(n_unnamed_fields);

#define VISIBLE_SIZE(op) Py_SIZE(op)
#define VISIBLE_SIZE_TP(tp) PyLong_AsSsize_t( \
                      _PyDict_GetItemId((tp)->tp_dict, &PyId_n_sequence_fields))

#define REAL_SIZE_TP(tp) PyLong_AsSsize_t( \
                      _PyDict_GetItemId((tp)->tp_dict, &PyId_n_fields))
#define REAL_SIZE(op) REAL_SIZE_TP(Py_TYPE(op))

#define UNNAMED_FIELDS_TP(tp) PyLong_AsSsize_t( \
                      _PyDict_GetItemId((tp)->tp_dict, &PyId_n_unnamed_fields))
#define UNNAMED_FIELDS(op) UNNAMED_FIELDS_TP(Py_TYPE(op))


PyObject *
PyStructSequence_New(PyTypeObject *type)
{
    PyStructSequence *obj;
    Py_ssize_t size = REAL_SIZE_TP(type), i;

    obj = PyObject_GC_NewVar(PyStructSequence, type, size);
    if (obj == NULL)
        return NULL;
    /* Hack the size of the variable object, so invisible fields don't appear
     to Python code. */
    Py_SIZE(obj) = VISIBLE_SIZE_TP(type);
    for (i = 0; i < size; i++)
        obj->ob_item[i] = NULL;

    return (PyObject*)obj;
}

void
PyStructSequence_SetItem(PyObject* op, Py_ssize_t i, PyObject* v)
{
    PyStructSequence_SET_ITEM(op, i, v);
}

PyObject*
PyStructSequence_GetItem(PyObject* op, Py_ssize_t i)
{
    return PyStructSequence_GET_ITEM(op, i);
}

static void
structseq_dealloc(PyStructSequence *obj)
{
    Py_ssize_t i, size;

    size = REAL_SIZE(obj);
    for (i = 0; i < size; ++i) {
        Py_XDECREF(obj->ob_item[i]);
    }
    PyObject_GC_Del(obj);
}

/*[clinic input]
class structseq "PyStructSequence *" "NULL"
[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=9d781c6922c77752]*/

#include "clinic/structseq.c.h"

/*[clinic input]
@classmethod
structseq.__new__ as structseq_new
    sequence as arg: object
    dict: object = NULL
[clinic start generated code]*/

static PyObject *
structseq_new_impl(PyTypeObject *type, PyObject *arg, PyObject *dict)
/*[clinic end generated code: output=baa082e788b171da input=9b44810243907377]*/
{
    PyObject *ob;
    PyStructSequence *res = NULL;
    Py_ssize_t len, min_len, max_len, i, n_unnamed_fields;

    arg = PySequence_Fast(arg, "constructor requires a sequence");

    if (!arg) {
        return NULL;
    }

    if (dict && !PyDict_Check(dict)) {
        PyErr_Format(PyExc_TypeError,
                     "%.500s() takes a dict as second arg, if any",
                     type->tp_name);
        Py_DECREF(arg);
        return NULL;
    }

    len = PySequence_Fast_GET_SIZE(arg);
    min_len = VISIBLE_SIZE_TP(type);
    max_len = REAL_SIZE_TP(type);
    n_unnamed_fields = UNNAMED_FIELDS_TP(type);

    if (min_len != max_len) {
        if (len < min_len) {
            PyErr_Format(PyExc_TypeError,
                "%.500s() takes an at least %zd-sequence (%zd-sequence given)",
                type->tp_name, min_len, len);
            Py_DECREF(arg);
            return NULL;
        }

        if (len > max_len) {
            PyErr_Format(PyExc_TypeError,
                "%.500s() takes an at most %zd-sequence (%zd-sequence given)",
                type->tp_name, max_len, len);
            Py_DECREF(arg);
            return NULL;
        }
    }
    else {
        if (len != min_len) {
            PyErr_Format(PyExc_TypeError,
                         "%.500s() takes a %zd-sequence (%zd-sequence given)",
                         type->tp_name, min_len, len);
            Py_DECREF(arg);
            return NULL;
        }
    }

    res = (PyStructSequence*) PyStructSequence_New(type);
    if (res == NULL) {
        Py_DECREF(arg);
        return NULL;
    }
    for (i = 0; i < len; ++i) {
        PyObject *v = PySequence_Fast_GET_ITEM(arg, i);
        Py_INCREF(v);
        res->ob_item[i] = v;
    }
    for (; i < max_len; ++i) {
        if (dict && (ob = PyDict_GetItemString(
            dict, type->tp_members[i-n_unnamed_fields].name))) {
        }
        else {
            ob = Py_None;
        }
        Py_INCREF(ob);
        res->ob_item[i] = ob;
    }

    Py_DECREF(arg);
    return (PyObject*) res;
}


static PyObject *
structseq_repr(PyStructSequence *obj)
{
    /* buffer and type size were chosen well considered. */
#define REPR_BUFFER_SIZE 512
#define TYPE_MAXSIZE 100

    PyTypeObject *typ = Py_TYPE(obj);
    Py_ssize_t i;
    int removelast = 0;
    Py_ssize_t len;
    char buf[REPR_BUFFER_SIZE];
    char *endofbuf, *pbuf = buf;

    /* pointer to end of writeable buffer; safes space for "...)\0" */
    endofbuf= &buf[REPR_BUFFER_SIZE-5];

    /* "typename(", limited to  TYPE_MAXSIZE */
    len = strlen(typ->tp_name) > TYPE_MAXSIZE ? TYPE_MAXSIZE :
                            strlen(typ->tp_name);
    strncpy(pbuf, typ->tp_name, len);
    pbuf += len;
    *pbuf++ = '(';

    for (i=0; i < VISIBLE_SIZE(obj); i++) {
        PyObject *val, *repr;
        const char *cname, *crepr;

        cname = typ->tp_members[i].name;
        if (cname == NULL) {
            PyErr_Format(PyExc_SystemError, "In structseq_repr(), member %d name is NULL"
                         " for type %.500s", i, typ->tp_name);
            return NULL;
        }
        val = PyStructSequence_GET_ITEM(obj, i);
        repr = PyObject_Repr(val);
        if (repr == NULL)
            return NULL;
        crepr = PyUnicode_AsUTF8(repr);
        if (crepr == NULL) {
            Py_DECREF(repr);
            return NULL;
        }

        /* + 3: keep space for "=" and ", " */
        len = strlen(cname) + strlen(crepr) + 3;
        if ((pbuf+len) <= endofbuf) {
            strcpy(pbuf, cname);
            pbuf += strlen(cname);
            *pbuf++ = '=';
            strcpy(pbuf, crepr);
            pbuf += strlen(crepr);
            *pbuf++ = ',';
            *pbuf++ = ' ';
            removelast = 1;
            Py_DECREF(repr);
        }
        else {
            strcpy(pbuf, "...");
            pbuf += 3;
            removelast = 0;
            Py_DECREF(repr);
            break;
        }
    }
    if (removelast) {
        /* overwrite last ", " */
        pbuf-=2;
    }
    *pbuf++ = ')';
    *pbuf = '\0';

    return PyUnicode_FromString(buf);
}

static PyObject *
structseq_reduce(PyStructSequence* self, PyObject *Py_UNUSED(ignored))
{
    PyObject* tup = NULL;
    PyObject* dict = NULL;
    PyObject* result;
    Py_ssize_t n_fields, n_visible_fields, n_unnamed_fields, i;

    n_fields = REAL_SIZE(self);
    n_visible_fields = VISIBLE_SIZE(self);
    n_unnamed_fields = UNNAMED_FIELDS(self);
    tup = PyTuple_New(n_visible_fields);
    if (!tup)
        goto error;

    dict = PyDict_New();
    if (!dict)
        goto error;

    for (i = 0; i < n_visible_fields; i++) {
        Py_INCREF(self->ob_item[i]);
        PyTuple_SET_ITEM(tup, i, self->ob_item[i]);
    }

    for (; i < n_fields; i++) {
        const char *n = Py_TYPE(self)->tp_members[i-n_unnamed_fields].name;
        if (PyDict_SetItemString(dict, n, self->ob_item[i]) < 0)
            goto error;
    }

    result = Py_BuildValue("(O(OO))", Py_TYPE(self), tup, dict);

    Py_DECREF(tup);
    Py_DECREF(dict);

    return result;

error:
    Py_XDECREF(tup);
    Py_XDECREF(dict);
    return NULL;
}

static PyMethodDef structseq_methods[] = {
    {"__reduce__", (PyCFunction)structseq_reduce, METH_NOARGS, NULL},
    {NULL, NULL}
};

static void count_members(PyStructSequence_Desc *desc, Py_ssize_t *n_members, Py_ssize_t *n_unnamed_members) {
    Py_ssize_t i;

    *n_unnamed_members = 0;
    for (i = 0; desc->fields[i].name != NULL; ++i)
        if (desc->fields[i].name == PyStructSequence_UnnamedField)
            (*n_unnamed_members)++;
        (*n_members) = i;
}

static int initialize_structseq_dict(PyStructSequence_Desc *desc, PyObject* dict) {
    PyObject *v;
    Py_ssize_t n_members, n_unnamed_members;

#define SET_DICT_FROM_SIZE(key, value)                          \
    do {                                                        \
        v = PyLong_FromSsize_t(value);                          \
        if (v == NULL)                                          \
            return -1;                                          \
        if (PyDict_SetItemString(dict, key, v) < 0) {           \
            Py_DECREF(v);                                       \
            return -1;                                          \
        }                                                       \
        Py_DECREF(v);                                           \
    } while (0)

    count_members(desc, &n_members, &n_unnamed_members);
    SET_DICT_FROM_SIZE(visible_length_key, desc->n_in_sequence);
    SET_DICT_FROM_SIZE(real_length_key, n_members);
    SET_DICT_FROM_SIZE(unnamed_fields_key, n_unnamed_members);
    return 0;
}

static PyMemberDef* initialize_members(PyStructSequence_Desc *desc) {
    PyMemberDef* members;
    Py_ssize_t n_members, n_unnamed_members, i, k;

    count_members(desc, &n_members, &n_unnamed_members);
    members = PyMem_NEW(PyMemberDef, n_members-n_unnamed_members + 1);
    if (members == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    for (i = k = 0; i < n_members; ++i) {
        if (desc->fields[i].name == PyStructSequence_UnnamedField)
            continue;
        members[k].name = desc->fields[i].name;
        members[k].type = T_OBJECT;
        members[k].offset = offsetof(PyStructSequence, ob_item)
          + i * sizeof(PyObject*);
        members[k].flags = READONLY;
        members[k].doc = desc->fields[i].doc;
        k++;
    }
    members[k].name = NULL;
    return members;
}

int
PyStructSequence_InitType2(PyTypeObject *type, PyStructSequence_Desc *desc)
{
    PyMemberDef* members;

#ifdef Py_TRACE_REFS
    /* if the type object was chained, unchain it first
       before overwriting its storage */
    if (type->ob_base.ob_base._ob_next) {
        _Py_ForgetReference((PyObject*)type);
    }
#endif

    Py_REFCNT(type) = 1;
    type->tp_name = desc->name;
    type->tp_basicsize = sizeof(PyStructSequence) - sizeof(PyObject *);
    type->tp_itemsize = sizeof(PyObject *);
    type->tp_dealloc = (destructor)structseq_dealloc;
    type->tp_repr = (reprfunc)structseq_repr;
    type->tp_doc = desc->doc;
    type->tp_base = &PyTuple_Type;
    type->tp_methods = structseq_methods;
    type->tp_new = structseq_new;
    type->tp_flags = Py_TPFLAGS_DEFAULT;

    members = initialize_members(desc);
    if (members == NULL)
        return -1;
    type->tp_members = members;

    if (PyType_Ready(type) < 0)
        return -1;
    Py_INCREF(type);

    if (initialize_structseq_dict(desc, type->tp_dict) < 0)
        return -1;

    return 0;
}

void
PyStructSequence_InitType(PyTypeObject *type, PyStructSequence_Desc *desc)
{
    (void)PyStructSequence_InitType2(type, desc);
}

PyTypeObject*
PyStructSequence_NewType(PyStructSequence_Desc *desc)
{
    PyObject* type;
    PyObject* bases;
    PyMemberDef* members;

    PyType_Spec* spec = PyMem_NEW(PyType_Spec, 1);
    spec->name = desc->name;
    spec->basicsize = sizeof(PyStructSequence) - sizeof(PyObject *);
    spec->itemsize = sizeof(PyObject *);
    spec->flags = Py_TPFLAGS_DEFAULT;
    spec->slots = PyMem_NEW(PyType_Slot, 6);

    spec->slots[0].slot = Py_tp_dealloc;
    spec->slots[0].pfunc = (destructor)structseq_dealloc;

    spec->slots[1].slot = Py_tp_repr;
    spec->slots[1].pfunc = (reprfunc)structseq_repr;

    spec->slots[2].slot = Py_tp_doc;
    spec->slots[2].pfunc = (void*)desc->doc;

    spec->slots[3].slot = Py_tp_methods;
    spec->slots[3].pfunc = structseq_methods;

    spec->slots[4].slot = Py_tp_new;
    spec->slots[4].pfunc = structseq_new;

    members = initialize_members(desc);
    if (members == NULL)
        return NULL;
    spec->slots[5].slot = Py_tp_members;
    spec->slots[5].pfunc = members;

    spec->slots[6].slot = 0;
    spec->slots[6].pfunc = 0;

    bases = PyTuple_Pack(1, &PyTuple_Type);
    type = PyType_FromSpecWithBases(spec, bases);
    if (type == NULL)
        return NULL;
    Py_INCREF(type);

    if (initialize_structseq_dict(desc, ((PyTypeObject *)type)->tp_dict) < 0)
        return NULL;

    return type;
}

int _PyStructSequence_Init(void)
{
    if (_PyUnicode_FromId(&PyId_n_sequence_fields) == NULL
        || _PyUnicode_FromId(&PyId_n_fields) == NULL
        || _PyUnicode_FromId(&PyId_n_unnamed_fields) == NULL)
        return -1;

    return 0;
}
