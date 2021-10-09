#define PY_SSIZE_T_CLEAN
#include <Python.h>

struct Node {
    uint64_t value;
    PyObject * payload;
};

struct Heap {
    PyObject_HEAD
    uint64_t limit;
    uint64_t count;
    uint64_t value;
};

PyTypeObject * Heap_type;

Node * get_nodes(Heap * heap) {
    return (Node *)&heap[1];
}

Heap * meth_heap(PyObject * self, PyObject * vargs, PyObject * kwargs) {
    static char * keywords[] = {"limit", NULL};

    uint64_t limit;

    if (!PyArg_ParseTupleAndKeywords(vargs, kwargs, "k", keywords, &limit)) {
        return NULL;
    }

    Heap * res = PyObject_NewVar(Heap, Heap_type, limit);
    res->limit = limit;
    res->count = 0;
    res->value = 0;
    return res;
}

PyObject * Heap_meth_push(Heap * self, PyObject ** args, Py_ssize_t nargs) {
    if (nargs != 2 || !PyLong_CheckExact(args[1])) {
        return NULL;
    }

    Py_INCREF(args[0]);
    Node * const nodes = get_nodes(self);
    int idx = self->count++;
    nodes[idx] = {self->value + PyLong_AsUnsignedLongLong(args[1]), args[0]};
    while (idx) {
        int parent = (idx - 1) / 2;
        if (nodes[parent].value <= nodes[idx].value) {
            break;
        }
        const Node temp = nodes[parent];
        nodes[parent] = nodes[idx];
        nodes[idx] = temp;
        idx = parent;
    }
    Py_RETURN_NONE;
}

PyObject * Heap_meth_update(Heap * self, PyObject * arg) {
    if (!PyLong_CheckExact(arg)) {
        return NULL;
    }

    self->value += PyLong_AsUnsignedLongLong(arg);
    Py_RETURN_NONE;
}

PyObject * Heap_meth_pop(Heap * self) {
    Node * nodes = get_nodes(self);
    if (self->count == 0 || nodes[0].value > self->value) {
        Py_RETURN_NONE;
    }
    PyObject * res = nodes[0].payload;
    const int last = --self->count;
    nodes[0] = nodes[last];
    int idx = 0;
    while (true) {
        const int left = idx * 2 + 1;
        const int right = left + 1;
        if (left >= last) {
            break;
        }
        int child = left;
        if (right < last && nodes[left].value > nodes[right].value) {
            child = right;
        }
        if (nodes[child].value >= nodes[idx].value) {
            break;
        }
        const Node temp = nodes[child];
        nodes[child] = nodes[idx];
        nodes[idx] = temp;
        idx = child;
    }
    return res;
}

void default_dealloc(PyObject * self) {
    Py_TYPE(self)->tp_free(self);
}

PyMethodDef Heap_methods[] = {
    {"update", (PyCFunction)Heap_meth_update, METH_O, 0},
    {"push", (PyCFunction)Heap_meth_push, METH_FASTCALL, 0},
    {"pop", (PyCFunction)Heap_meth_pop, METH_NOARGS, 0},
    {0},
};

PyType_Slot Heap_slots[] = {
    {Py_tp_methods, Heap_methods},
    {Py_tp_dealloc, (void *)default_dealloc},
    {},
};

PyType_Spec Heap_spec = {"msgheap.Heap", sizeof(Heap), sizeof(Node), Py_TPFLAGS_DEFAULT, Heap_slots};

PyMethodDef module_methods[] = {
    {"heap", (PyCFunction)meth_heap, METH_VARARGS | METH_KEYWORDS, 0},
    {0},
};

PyModuleDef module_def = {PyModuleDef_HEAD_INIT, "msgheap", 0, -1, module_methods, 0, 0, 0, 0};

extern "C" PyObject * PyInit_msgheap() {
    PyObject * module = PyModule_Create(&module_def);
    Heap_type = (PyTypeObject *)PyType_FromSpec(&Heap_spec);
    return module;
}
