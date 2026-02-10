#include "py/runtime.h"
#include "py/obj.h"

static void * flush_buf = NULL;
static void (*flush_fun)(int, int, int, int, void*) = NULL;

void set_flush_buf(void * buf){
    flush_buf = buf;
}

void set_flush_callback(void (*cb)(int, int, int, int, void*)){
    flush_fun = cb;
}

void call_flush_callback(int x1, int y1, int x2, int y2, void * data_bytes){
    if (flush_fun != NULL) {
        flush_fun(x1, y1, x2, y2, data_bytes);
    }
}

static mp_obj_t cpyswap_get_flush_buf(void) {
    if (flush_buf == NULL) {
        return mp_const_none;
    }
    return mp_obj_new_int_from_uint((mp_uint_t)(uintptr_t)flush_buf);
}
static MP_DEFINE_CONST_FUN_OBJ_0(cpyswap_get_flush_buf_obj, cpyswap_get_flush_buf);

static mp_obj_t cpyswap_call_flush_callback(size_t n_args, const mp_obj_t *args) {
    // if (n_args != 5) {
    //     mp_raise_TypeError(MP_ERROR_TEXT(T"call_flush_callback() expects 5 arguments"));
    // }
    
    int x1 = mp_obj_get_int(args[0]);
    int y1 = mp_obj_get_int(args[1]);
    int x2 = mp_obj_get_int(args[2]);
    int y2 = mp_obj_get_int(args[3]);
    
    // 处理data_bytes参数（bytes对象或None）
    void *data_bytes = NULL;
    if (args[4] != mp_const_none) {
        mp_buffer_info_t bufinfo;
        // 获取bytes对象的缓冲区信息
        mp_get_buffer_raise(args[4], &bufinfo, MP_BUFFER_READ);
        data_bytes = bufinfo.buf;  // 获取数据指针
        // 如果需要长度，可以使用 bufinfo.len
    }
    
    call_flush_callback(x1, y1, x2, y2, data_bytes);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cpyswap_call_flush_callback_obj, 5, 5, cpyswap_call_flush_callback);

static const mp_rom_map_elem_t cpyswap_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_cpyswap) },
    { MP_ROM_QSTR(MP_QSTR_get_flush_buf), MP_ROM_PTR(&cpyswap_get_flush_buf_obj) },
    { MP_ROM_QSTR(MP_QSTR_call_flush_callback), MP_ROM_PTR(&cpyswap_call_flush_callback_obj) },
};
static MP_DEFINE_CONST_DICT(cpyswap_module_globals, cpyswap_module_globals_table);

const mp_obj_module_t cpyswap_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&cpyswap_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_cpyswap, cpyswap_user_cmodule);