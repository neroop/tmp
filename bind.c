#include "Python.h"

// 重要：避免使用废弃的接口
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION

#include "numpy/arrayobject.h"
#include "obj_c.h"
#include <stdio.h>

PyObject* hello(PyObject* self, PyObject* args)
{
    PyObject* arg1 = NULL;
    if (!PyArg_ParseTuple(args, "O!", &PyArray_Type, &arg1)) {
        return NULL;
    }
    PyObject* arr = PyArray_FROM_OTF(arg1, NPY_UINT8, NPY_ARRAY_IN_ARRAY);
    if (arr == NULL) {
        printf("NULL arr\n");
        Py_XDECREF(arr);
        return NULL;
    }

    int nd = PyArray_NDIM((PyArrayObject*) arr);
    npy_intp* p = PyArray_SHAPE((PyArrayObject*) arr);
    printf("dim %d\n", nd);
    unsigned long long k = 1;
    for (int i = 0; i < nd; ++i) {
        printf("sharp[%d]=%ld\n", i, p[i]);
        k *= p[i];
    }

    uint8_t* d = PyArray_DATA((PyArrayObject*) arr);
    for (unsigned long long i = 0; i < k; ++i) {
        printf("%d ", d[i]);
    }
    d[0] = 123;
    printf("\n");

    Py_DECREF(arr);
    Py_RETURN_NONE;
}

PyObject* hello2(PyObject* self)
{
    struct Demo* d = constructDemo();
    destroyDemo(d);
    Py_RETURN_NONE;
}

PyMethodDef helloworld_funcs[] = {
    {
        "hello",
        (PyCFunction) hello,
        METH_VARARGS,
        "hello"
    },

    {
        "hello2",
        (PyCFunction) hello2,
        METH_NOARGS,
        "hello2"
    },

    {NULL} // 重要：缺少会段错误
};

PyModuleDef helloworld_mod = {
    PyModuleDef_HEAD_INIT,
    "helloworld",
    "hello doc",
    -1,
    helloworld_funcs,
    NULL,
    NULL,
    NULL,
    NULL
};

// https://docs.python.org/3/extending/newtypes_tutorial.html
// https://www.oracle.com/technical-resources/articles/it-infrastructure/mixing-c-and-cplusplus.html

PyMODINIT_FUNC PyInit_helloworld(void)
{
    // 重要：必须有这个导入 https://numpy.org/doc/stable/user/c-info.how-to-extend.html#required-subroutine
    import_array();
    return PyModule_Create(&helloworld_mod);
}
