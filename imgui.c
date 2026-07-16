#include "imgui.h"

#define R 0.1f
#define G 0.3f
#define B 0.6f
#define A 0.8f

void imgui_begin(imgui_context *context, int x, int y, int w, int h)
{
    const int left = x;
    const int right = x + w;
    const int top = y;
    const int bottom = y + h;

    const imgui_vertex top_left = {
        .pos={ left,top },
        .uv={ 0,0 },
        .color={ R,G,B,A, }
    };
    const imgui_vertex bottom_left = {
        .pos={ left,bottom},
        .uv={ 0,1 },
        .color={ R,G,B,A }
    };
    const imgui_vertex bottom_right = {
        .pos={ right,bottom},
        .uv={ 1,1 },
        .color={ R,G,B,A }
    };
    const imgui_vertex top_right = {
        .pos={ right,top},
        .uv={ 1,0 },
        .color={ R,G,B,A }
    };
    const imgui_index tl_i = context->vertex_count + 0;
    const imgui_index bl_i = context->vertex_count + 1;
    const imgui_index br_i = context->vertex_count + 2;
    const imgui_index tr_i = context->vertex_count + 3;

    context->vertices[context->vertex_count++] = top_left;
    context->vertices[context->vertex_count++] = bottom_left;
    context->vertices[context->vertex_count++] = bottom_right;
    context->vertices[context->vertex_count++] = top_right;
    
    context->indices[context->index_count++] = tl_i;
    context->indices[context->index_count++] = bl_i;
    context->indices[context->index_count++] = br_i;

    context->indices[context->index_count++] = tr_i;
    context->indices[context->index_count++] = tl_i;
    context->indices[context->index_count++] = br_i;
}

void imgui_end(imgui_context *context)
{
}

