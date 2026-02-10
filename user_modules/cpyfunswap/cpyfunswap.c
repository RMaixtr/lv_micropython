// moddynfunc.c - 简化版动态函数注册模块

#include "py/runtime.h"
#include "py/obj.h"
#include "py/mphal.h"

// 注册的函数元数据（仅保存必要信息）
typedef struct _mp_obj_dynfunc_t {
    mp_obj_base_t base;
    void (*func)(char *);    // 统一的函数签名：void func(char *)
    const char *name;        // 函数名
} mp_obj_dynfunc_t;

static const mp_obj_type_t dynfunc_type;
static mp_obj_dict_t *dynfunc_registry = NULL;  // 使用dict自动扩容

// ========== C端API：注册函数 ==========

// 注册函数（供C代码调用）
void mp_dynfunc_register(const char *name, void (*func)(char *)) {
    // 延迟初始化注册表（初始大小0，自动扩容）
    if (dynfunc_registry == NULL) {
        dynfunc_registry = mp_obj_new_dict(0);
    }
    
    // 创建并填充dynfunc对象
    mp_obj_dynfunc_t *o = m_new_obj(mp_obj_dynfunc_t);
    o->base.type = &dynfunc_type;
    o->func = func;
    o->name = name;
    
    // 存入字典：{name: dynfunc_obj}
    mp_obj_dict_store(dynfunc_registry,
                     mp_obj_new_str(name, strlen(name)),
                     MP_OBJ_FROM_PTR(o));
}

// ========== Python端：dynfunc对象 ==========

// 打印对象信息
static void dynfunc_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    mp_obj_dynfunc_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "<dynfunc '%s' @%p>", self->name, self->func);
}

// 调用函数
static mp_obj_t dynfunc_call(mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_obj_dynfunc_t *self = MP_OBJ_TO_PTR(self_in);
    
    // 参数检查
    if (n_kw != 0) {
        mp_raise_TypeError(MP_ERROR_TEXT("no keyword arguments allowed"));
    }
    if (n_args != 1) {
        mp_raise_TypeError(MP_ERROR_TEXT("expected 1 string argument"));
    }
    
    // 获取字符串并创建可修改的副本（C函数可能需要修改）
    const char *str = mp_obj_str_get_str(args[0]);
    size_t len = strlen(str);
    char *buf = m_new(char, len + 1);
    memcpy(buf, str, len + 1);
    
    // 调用C函数
    self->func(buf);
    
    // 清理并返回None
    m_del(char, buf, len + 1);
    return mp_const_none;
}

// dynfunc类型定义
static MP_DEFINE_CONST_OBJ_TYPE(
    dynfunc_type,
    MP_QSTR_dynfunc,
    MP_TYPE_FLAG_NONE,
    print, dynfunc_print,
    call, dynfunc_call
    );

// ========== 模块方法 ==========

// dynfunc.get(name) - 获取注册的函数
static mp_obj_t mod_dynfunc_get(mp_obj_t name_in) {
    if (dynfunc_registry == NULL) {
        mp_raise_ValueError(MP_ERROR_TEXT("no registered functions"));
    }
    
    const char *name = mp_obj_str_get_str(name_in);
    mp_obj_t obj = mp_obj_dict_get(dynfunc_registry, mp_obj_new_str(name, strlen(name)));
    
    if (obj == MP_OBJ_NULL) {
        mp_raise_ValueError(MP_ERROR_TEXT("function not registered"));
    }
    return obj;
}
static MP_DEFINE_CONST_FUN_OBJ_1(mod_dynfunc_get_obj, mod_dynfunc_get);


// ========== 模块定义 ==========

static const mp_rom_map_elem_t mp_module_dynfunc_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_dynfunc) },
    { MP_ROM_QSTR(MP_QSTR_get), MP_ROM_PTR(&mod_dynfunc_get_obj) },
};

static MP_DEFINE_CONST_DICT(mp_module_dynfunc_globals, mp_module_dynfunc_globals_table);

const mp_obj_module_t mp_module_dynfunc = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_dynfunc_globals,
};

MP_REGISTER_MODULE(MP_QSTR_dynfunc, mp_module_dynfunc);

// ========== 使用示例 ==========

/* 
// 1. 在任意C文件中定义函数
void board_handler(char *str) {
    printf("C: Received string: %s\n", str);
    // 可修改str内容
}

// 2. 在初始化时注册（如 main.c 或 board init）
void board_init(void) {
    mp_dynfunc_register("board_handler", board_handler);
}

// 3. Python中使用
import dynfunc

handler = dynfunc.get("board_handler")
handler("hello from Python")  # 输出: C: Received string: hello from Python

# 列出所有函数
print(dynfunc.list())  # ['board_handler']
*/