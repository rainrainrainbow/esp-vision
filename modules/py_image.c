/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "py_image.h"

#include "esp_err.h"

#include "py/runtime.h"

#include "preview.h"

#ifdef NO_QSTR
#define IMAGE_PIXFORMAT_BINARY    (0)
#define IMAGE_PIXFORMAT_GRAYSCALE (0)
#define IMAGE_PIXFORMAT_RGB565    (0)
#define IMAGE_PIXFORMAT_JPEG      (0)
#else
#define IMAGE_PIXFORMAT_BINARY    PIXFORMAT_BINARY
#define IMAGE_PIXFORMAT_GRAYSCALE PIXFORMAT_GRAYSCALE
#define IMAGE_PIXFORMAT_RGB565    PIXFORMAT_RGB565
#define IMAGE_PIXFORMAT_JPEG      PIXFORMAT_JPEG
#endif

typedef struct {
    mp_obj_base_t base;
    image_t img;
    mp_obj_t backing;
} esp_vision_image_obj_t;

static void image_raise_esp_err(const char *prefix, esp_err_t err)
{
    mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("%s: %s"), prefix, esp_err_to_name(err));
}

static int image_clamp_u8(int value)
{
    if (value < 0) {
        return 0;
    }
    if (value > 255) {
        return 255;
    }
    return value;
}

static image_t *image_get_drawable(mp_obj_t self_in)
{
    image_t *img = esp_vision_image_get_image(self_in);
    if ((img->pixels == NULL) || (img->w <= 0) || (img->h <= 0)) {
        mp_raise_ValueError(MP_ERROR_TEXT("image has no pixel buffer"));
    }
    if ((img->pixfmt != IMAGE_PIXFORMAT_BINARY) &&
            (img->pixfmt != IMAGE_PIXFORMAT_GRAYSCALE) &&
            (img->pixfmt != IMAGE_PIXFORMAT_RGB565)) {
        mp_raise_ValueError(MP_ERROR_TEXT("image format is not drawable"));
    }
    return img;
}

static int image_color_from_obj(image_t *img, mp_obj_t color_in)
{
    if ((color_in == MP_OBJ_NULL) || (color_in == mp_const_none)) {
        return -1;
    }

    if (mp_obj_is_int(color_in)) {
        int color = mp_obj_get_int(color_in);
        switch (img->pixfmt) {
        case IMAGE_PIXFORMAT_BINARY:
            return color != 0;
        case IMAGE_PIXFORMAT_GRAYSCALE:
            return image_clamp_u8(color);
        case IMAGE_PIXFORMAT_RGB565:
            return color & 0xffff;
        default:
            return color;
        }
    }

    size_t len = 0;
    mp_obj_t *items = NULL;
    mp_obj_get_array(color_in, &len, &items);
    if (len != 3) {
        mp_raise_ValueError(MP_ERROR_TEXT("color tuple must have 3 items"));
    }

    int r = image_clamp_u8(mp_obj_get_int(items[0]));
    int g = image_clamp_u8(mp_obj_get_int(items[1]));
    int b = image_clamp_u8(mp_obj_get_int(items[2]));

    switch (img->pixfmt) {
    case IMAGE_PIXFORMAT_BINARY:
        return COLOR_RGB888_TO_Y(r, g, b) > 127;
    case IMAGE_PIXFORMAT_GRAYSCALE:
        return COLOR_RGB888_TO_Y(r, g, b);
    case IMAGE_PIXFORMAT_RGB565:
        return COLOR_R8_G8_B8_TO_RGB565(r, g, b);
    default:
        return -1;
    }
}

image_t *esp_vision_image_get_image(mp_obj_t obj)
{
    if (!mp_obj_is_type(obj, &esp_vision_image_type)) {
        mp_raise_TypeError(MP_ERROR_TEXT("expected image"));
    }

    esp_vision_image_obj_t *self = MP_OBJ_TO_PTR(obj);
    return &self->img;
}

mp_obj_t esp_vision_image_new(int32_t width,
                              int32_t height,
                              uint32_t pixfmt,
                              uint8_t *pixels,
                              mp_obj_t backing)
{
    esp_vision_image_obj_t *self = mp_obj_malloc(esp_vision_image_obj_t, &esp_vision_image_type);
    self->img.w = width;
    self->img.h = height;
    self->img.pixfmt = pixfmt;
    self->img.size = 0;
    self->img._raw = NULL;
    self->img.pixels = pixels;
    self->backing = backing;
    return MP_OBJ_FROM_PTR(self);
}

static void image_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    (void)kind;
    image_t *img = esp_vision_image_get_image(self_in);
    mp_printf(print, "<image %dx%d pixfmt=%u size=%u>",
              (int)img->w,
              (int)img->h,
              (unsigned int)img->pixfmt,
              (unsigned int)image_size(img));
}

static mp_obj_t image_width(mp_obj_t self_in)
{
    image_t *img = esp_vision_image_get_image(self_in);
    return mp_obj_new_int(img->w);
}
static MP_DEFINE_CONST_FUN_OBJ_1(image_width_obj, image_width);

static mp_obj_t image_height(mp_obj_t self_in)
{
    image_t *img = esp_vision_image_get_image(self_in);
    return mp_obj_new_int(img->h);
}
static MP_DEFINE_CONST_FUN_OBJ_1(image_height_obj, image_height);

static mp_obj_t image_format(mp_obj_t self_in)
{
    image_t *img = esp_vision_image_get_image(self_in);
    return mp_obj_new_int_from_uint(img->pixfmt);
}
static MP_DEFINE_CONST_FUN_OBJ_1(image_format_obj, image_format);

static mp_obj_t image_size_obj_fun(mp_obj_t self_in)
{
    image_t *img = esp_vision_image_get_image(self_in);
    return mp_obj_new_int_from_uint(image_size(img));
}
static MP_DEFINE_CONST_FUN_OBJ_1(image_size_obj, image_size_obj_fun);

static mp_obj_t image_flush(mp_obj_t self_in)
{
    image_t *img = esp_vision_image_get_image(self_in);

    esp_err_t ret = esp_vision_preview_flush(img);
    if (ret != ESP_OK) {
        image_raise_esp_err("preview flush failed", ret);
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(image_flush_obj, image_flush);

static mp_obj_t image_bytearray(mp_obj_t self_in)
{
    image_t *img = esp_vision_image_get_image(self_in);
    size_t size = image_size(img);
    if ((img->pixels == NULL) || (size == 0)) {
        mp_raise_ValueError(MP_ERROR_TEXT("image has no pixel buffer"));
    }
    return mp_obj_new_bytearray_by_ref(size, img->pixels);
}
static MP_DEFINE_CONST_FUN_OBJ_1(image_bytearray_obj, image_bytearray);

static mp_obj_t image_draw_line(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_x0, ARG_y0, ARG_x1, ARG_y1, ARG_color, ARG_thickness };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x0, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y0, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_x1, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y1, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_color, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_thickness, MP_ARG_INT, {.u_int = 1} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    image_t *img = image_get_drawable(pos_args[0]);
    int color = image_color_from_obj(img, args[ARG_color].u_obj);
    imlib_draw_line(img,
                    args[ARG_x0].u_int,
                    args[ARG_y0].u_int,
                    args[ARG_x1].u_int,
                    args[ARG_y1].u_int,
                    color,
                    args[ARG_thickness].u_int);
    return pos_args[0];
}
static MP_DEFINE_CONST_FUN_OBJ_KW(image_draw_line_obj, 1, image_draw_line);

static mp_obj_t image_draw_rectangle(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_x, ARG_y, ARG_w, ARG_h, ARG_color, ARG_thickness, ARG_fill };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_w, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_h, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_color, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_thickness, MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_fill, MP_ARG_BOOL, {.u_bool = false} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    image_t *img = image_get_drawable(pos_args[0]);
    int color = image_color_from_obj(img, args[ARG_color].u_obj);
    imlib_draw_rectangle(img,
                         args[ARG_x].u_int,
                         args[ARG_y].u_int,
                         args[ARG_w].u_int,
                         args[ARG_h].u_int,
                         color,
                         args[ARG_thickness].u_int,
                         args[ARG_fill].u_bool);
    return pos_args[0];
}
static MP_DEFINE_CONST_FUN_OBJ_KW(image_draw_rectangle_obj, 1, image_draw_rectangle);

static mp_obj_t image_draw_circle(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_x, ARG_y, ARG_r, ARG_color, ARG_thickness, ARG_fill };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_r, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_color, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_thickness, MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_fill, MP_ARG_BOOL, {.u_bool = false} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    image_t *img = image_get_drawable(pos_args[0]);
    int color = image_color_from_obj(img, args[ARG_color].u_obj);
    imlib_draw_circle(img,
                      args[ARG_x].u_int,
                      args[ARG_y].u_int,
                      args[ARG_r].u_int,
                      color,
                      args[ARG_thickness].u_int,
                      args[ARG_fill].u_bool);
    return pos_args[0];
}
static MP_DEFINE_CONST_FUN_OBJ_KW(image_draw_circle_obj, 1, image_draw_circle);

static mp_obj_t image_draw_cross(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_x, ARG_y, ARG_color, ARG_size, ARG_thickness };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_color, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_size, MP_ARG_INT, {.u_int = 5} },
        { MP_QSTR_thickness, MP_ARG_INT, {.u_int = 1} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    image_t *img = image_get_drawable(pos_args[0]);
    int color = image_color_from_obj(img, args[ARG_color].u_obj);
    int x = args[ARG_x].u_int;
    int y = args[ARG_y].u_int;
    int size = args[ARG_size].u_int;
    int thickness = args[ARG_thickness].u_int;
    imlib_draw_line(img, x - size, y, x + size, y, color, thickness);
    imlib_draw_line(img, x, y - size, x, y + size, color, thickness);
    return pos_args[0];
}
static MP_DEFINE_CONST_FUN_OBJ_KW(image_draw_cross_obj, 1, image_draw_cross);

static mp_obj_t image_draw_arrow(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_x0, ARG_y0, ARG_x1, ARG_y1, ARG_color, ARG_size, ARG_thickness };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x0, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y0, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_x1, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y1, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_color, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_size, MP_ARG_INT, {.u_int = 10} },
        { MP_QSTR_thickness, MP_ARG_INT, {.u_int = 1} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    image_t *img = image_get_drawable(pos_args[0]);
    int color = image_color_from_obj(img, args[ARG_color].u_obj);
    int x0 = args[ARG_x0].u_int;
    int y0 = args[ARG_y0].u_int;
    int x1 = args[ARG_x1].u_int;
    int y1 = args[ARG_y1].u_int;
    int size = args[ARG_size].u_int;
    int thickness = args[ARG_thickness].u_int;
    int dx = x1 - x0;
    int dy = y1 - y0;

    imlib_draw_line(img, x0, y0, x1, y1, color, thickness);
    if ((dx == 0) && (dy == 0)) {
        return pos_args[0];
    }

    float length = fast_sqrtf((dx * dx) + (dy * dy));
    float ux = dx / length;
    float uy = dy / length;
    float vx = -uy;
    float vy = ux;
    int a0x = fast_roundf(x1 - (size * ux) + (size * vx * 0.5f));
    int a0y = fast_roundf(y1 - (size * uy) + (size * vy * 0.5f));
    int a1x = fast_roundf(x1 - (size * ux) - (size * vx * 0.5f));
    int a1y = fast_roundf(y1 - (size * uy) - (size * vy * 0.5f));
    imlib_draw_line(img, x1, y1, a0x, a0y, color, thickness);
    imlib_draw_line(img, x1, y1, a1x, a1y, color, thickness);
    return pos_args[0];
}
static MP_DEFINE_CONST_FUN_OBJ_KW(image_draw_arrow_obj, 1, image_draw_arrow);

static mp_obj_t image_draw_string(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_x, ARG_y, ARG_text, ARG_color, ARG_scale };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_text, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_color, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_scale, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    image_t *img = image_get_drawable(pos_args[0]);
    int color = image_color_from_obj(img, args[ARG_color].u_obj);
    float scale = (args[ARG_scale].u_obj == MP_OBJ_NULL) ? 1.0f : mp_obj_get_float(args[ARG_scale].u_obj);
    if (scale <= 0.0f) {
        mp_raise_ValueError(MP_ERROR_TEXT("scale must be > 0"));
    }

    imlib_draw_string(img,
                      args[ARG_x].u_int,
                      args[ARG_y].u_int,
                      mp_obj_str_get_str(args[ARG_text].u_obj),
                      color,
                      scale,
                      0,
                      0,
                      true,
                      0,
                      false,
                      false,
                      0,
                      false,
                      false);
    return pos_args[0];
}
static MP_DEFINE_CONST_FUN_OBJ_KW(image_draw_string_obj, 1, image_draw_string);

static mp_obj_t image_unary_op(mp_unary_op_t op, mp_obj_t self_in)
{
    image_t *img = esp_vision_image_get_image(self_in);

    switch (op) {
    case MP_UNARY_OP_LEN:
        return mp_obj_new_int_from_uint(image_size(img));
    default:
        return MP_OBJ_NULL;
    }
}

static const mp_rom_map_elem_t image_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_width), MP_ROM_PTR(&image_width_obj) },
    { MP_ROM_QSTR(MP_QSTR_height), MP_ROM_PTR(&image_height_obj) },
    { MP_ROM_QSTR(MP_QSTR_format), MP_ROM_PTR(&image_format_obj) },
    { MP_ROM_QSTR(MP_QSTR_size), MP_ROM_PTR(&image_size_obj) },
    { MP_ROM_QSTR(MP_QSTR_flush), MP_ROM_PTR(&image_flush_obj) },
    { MP_ROM_QSTR(MP_QSTR_bytearray), MP_ROM_PTR(&image_bytearray_obj) },
    { MP_ROM_QSTR(MP_QSTR_draw_line), MP_ROM_PTR(&image_draw_line_obj) },
    { MP_ROM_QSTR(MP_QSTR_draw_rectangle), MP_ROM_PTR(&image_draw_rectangle_obj) },
    { MP_ROM_QSTR(MP_QSTR_draw_circle), MP_ROM_PTR(&image_draw_circle_obj) },
    { MP_ROM_QSTR(MP_QSTR_draw_cross), MP_ROM_PTR(&image_draw_cross_obj) },
    { MP_ROM_QSTR(MP_QSTR_draw_arrow), MP_ROM_PTR(&image_draw_arrow_obj) },
    { MP_ROM_QSTR(MP_QSTR_draw_string), MP_ROM_PTR(&image_draw_string_obj) },
};
static MP_DEFINE_CONST_DICT(image_locals_dict, image_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    esp_vision_image_type,
    MP_QSTR_Image,
    MP_TYPE_FLAG_NONE,
    print, image_print,
    unary_op, image_unary_op,
    locals_dict, &image_locals_dict
);

static const mp_rom_map_elem_t image_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_image) },
    { MP_ROM_QSTR(MP_QSTR_Image), MP_ROM_PTR(&esp_vision_image_type) },
    { MP_ROM_QSTR(MP_QSTR_BINARY), MP_ROM_INT(IMAGE_PIXFORMAT_BINARY) },
    { MP_ROM_QSTR(MP_QSTR_GRAYSCALE), MP_ROM_INT(IMAGE_PIXFORMAT_GRAYSCALE) },
    { MP_ROM_QSTR(MP_QSTR_RGB565), MP_ROM_INT(IMAGE_PIXFORMAT_RGB565) },
    { MP_ROM_QSTR(MP_QSTR_JPEG), MP_ROM_INT(IMAGE_PIXFORMAT_JPEG) },
};
static MP_DEFINE_CONST_DICT(image_module_globals, image_module_globals_table);

const mp_obj_module_t mp_module_image = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *) &image_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_image, mp_module_image);
