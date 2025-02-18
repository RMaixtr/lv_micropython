// Include MicroPython API.
#include "py/runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


STATIC mp_obj_t rpopen(mp_obj_t arg_1_obj) {
    size_t arg_1_len;
    const char* arg_1 = mp_obj_str_get_data(arg_1_obj, &arg_1_len);

    char* ret_ptr = strdup("err");

    FILE* pipe = popen(arg_1, "r");
    if (pipe == NULL) {
        return mp_obj_new_str(ret_ptr, 3);
    }

    char buffer[128];
    size_t total_size = 0;
    size_t buffer_size = sizeof(buffer);

    while (fgets(buffer, buffer_size, pipe) != NULL) {
        size_t len = strlen(buffer);
        char* new_buffer = realloc(ret_ptr, total_size + len + 1);
        if (new_buffer == NULL) {
            pclose(pipe);
            return mp_obj_new_str(ret_ptr, 3);
        }
        ret_ptr = new_buffer;
        memcpy(ret_ptr + total_size, buffer, len);
        total_size += len;
    }

    int status = pclose(pipe);
    if (status == -1) {
        return mp_obj_new_str(ret_ptr, 3);
    }

    return mp_obj_new_str(ret_ptr, total_size);
}
MP_DEFINE_CONST_FUN_OBJ_1(rpopen_obj, rpopen);

// Define all properties of the module.
// Table entries are key/value pairs of the attribute name (a string)
// and the MicroPython object reference.
// All identifiers and strings are written as MP_QSTR_xxx and will be
// optimized to word-sized integers by the build system (interned strings).
STATIC const mp_rom_map_elem_t example_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_cexample) },
    { MP_ROM_QSTR(MP_QSTR_rpopen), MP_ROM_PTR(&rpopen_obj) },
};
STATIC MP_DEFINE_CONST_DICT(example_module_globals, example_module_globals_table);

// Define module object.
const mp_obj_module_t example_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&example_module_globals,
};

// Register the module to make it available in Python.
MP_REGISTER_MODULE(MP_QSTR_cmodule, example_user_cmodule);
