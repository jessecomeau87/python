#ifndef Py_AST_H
#define Py_AST_H
#ifdef __cplusplus
extern "C" {
#endif

PyAPI_FUNC(int) PyAST_Validate(mod_ty);
PyAPI_FUNC(mod_ty) PyAST_FromNode(
    const node *n,
    PyCompilerFlags *flags,
    const char *filename,       /* decoded from the filesystem encoding */
    PyArena *arena);
PyAPI_FUNC(mod_ty) PyAST_FromNodeObject(
    const node *n,
    PyCompilerFlags *flags,
    PyObject *filename,
    PyArena *arena);
PyAPI_FUNC(PyObject *) PyAST_UnicodeFromAstExpr(
    expr_ty e,
    int omit_parens,
    int omit_string_brackets);

#ifdef __cplusplus
}
#endif
#endif /* !Py_AST_H */
