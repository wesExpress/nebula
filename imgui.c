#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#include "Nuklear/nuklear.h"

typedef struct imgui_context_t
{
    struct nk_context *nuklear_context;
    struct nk_font_atlas font_atlas;
    struct nk_buffer commands;
    struct nk_draw_null_texture null_texture;

    int max_vertex_buffer, int_max_index_buffer;
} imgui_context;

#include "imgui.h"


