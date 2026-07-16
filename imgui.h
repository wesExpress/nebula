#ifndef __IMGUI_H__
#define __IMGUI_H__

#include <stdint.h> 

typedef struct imgui_vertex_t
{
    float pos[2];
    float uv[2];
    float color[4];
} imgui_vertex;

typedef uint32_t imgui_index;

typedef struct imgui_context_t
{
    imgui_vertex *vertices;
    imgui_index  *indices;

    int vertex_count, index_count;
} imgui_context;

void imgui_begin(imgui_context *context, int x, int y, int w, int h);
void imgui_end(imgui_context *context);

#endif
