%module MLX90640
%include "stdint.i"

%{
int setup(int fps);
void cleanup();
float * get_frame();
%}

%typemap(out) float *get_frame %{
    $result = PyList_New(768);
    for (int i = 0; i < 768; ++i) {
        PyList_SetItem($result, i, PyFloat_FromDouble($1[i]));
    }
%}

int setup(int fps);
void cleanup();
float * get_frame();