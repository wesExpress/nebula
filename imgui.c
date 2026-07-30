#include "imgui.h"
#include "microui/microui.h"

#include "cglm/cglm.h"

#include <stdlib.h>

int text_width(mu_Font font, const char *str, int len)
{
    //LOG_TRACE("MICROUI: TEXT WIDTH (%s)", str);
    return 0;
}

int text_height(mu_Font font)
{
    //LOG_TRACE("MICROUI: TEXT HEIGHT");
    return 0;
}

void imgui_clip(imgui_frame_data *frame_data, mu_Rect rect)
{
    LOG_TRACE("MICROUI: CLIP");
    LOG_TRACE("         (%d, %d), (%d, %d)", rect.x, rect.y, rect.w, rect.h);
}

void imgui_draw_rect(imgui_frame_data *frame_data, mu_Rect rect, mu_Color color)
{
    if(frame_data->vertex_count + 4 >= IMGUI_MAX_VERTICES) return;

    float x = rect.x;
    float y = rect.y;
    float w = rect.w;
    float h = rect.h;
    float r = color.r / 255.f;
    float g = color.g / 255.f;
    float b = color.b / 255.f;
    float a = color.a / 255.f;

    imgui_vertex v0 = {
        .pos={x+0,y+0},
        .color={r,g,b,a}
    };
    imgui_vertex v1 = {
        .pos={x+w,y},
        .color={r,g,b,a}
    };
    imgui_vertex v2 = {
        .pos={x+w,y+h},
        .color={r,g,b,a}
    };
    imgui_vertex v3 = {
        .pos={x+0,y+h},
        .color={r,g,b,a}
    };

    frame_data->vertices[frame_data->vertex_count+0] = v0;
    frame_data->vertices[frame_data->vertex_count+1] = v1;
    frame_data->vertices[frame_data->vertex_count+2] = v2;
    frame_data->vertices[frame_data->vertex_count+3] = v3;
    
    frame_data->indices[frame_data->index_count+0] = frame_data->vertex_count+0;
    frame_data->indices[frame_data->index_count+1] = frame_data->vertex_count+1;
    frame_data->indices[frame_data->index_count+2] = frame_data->vertex_count+2;

    frame_data->indices[frame_data->index_count+3] = frame_data->vertex_count+2;
    frame_data->indices[frame_data->index_count+4] = frame_data->vertex_count+3;
    frame_data->indices[frame_data->index_count+5] = frame_data->vertex_count+0;

    frame_data->vertex_count += 4;
    frame_data->index_count  += 6;
}

#if 0
void draw_frame(mu_Context *ctx, mu_Rect rect, int colorid)
{
    LOG_TRACE("MICRIUI: DRAW FRAME");
}
#endif

bool imgui_init(dm_context *context, imgui_context *imgui_ctx)
{
    dm_raster_shader vertex_shader = {
        .path="../../assets/shaders/imgui_vertex",
        .entry="v_main"
    };

    dm_raster_shader fragment_shader = {
        .path="../../assets/shaders/imgui_fragment",
        .entry="f_main"
    };

    dm_raster_pipe_desc pipe_desc = {
        .shaders[DM_RASTER_SHADER_STAGE_VERTEX]=vertex_shader,
        .shaders[DM_RASTER_SHADER_STAGE_FRAGMENT]=fragment_shader,
        .depth=false,

        .blend=true,
        .color_blend_op=DM_BLEND_OP_ADD,
        .color_src_factor=DM_BLEND_FACTOR_SRC_ALPHA,
        .color_dst_factor=DM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .alpha_blend_op=DM_BLEND_OP_ADD,
        .alpha_src_factor=DM_BLEND_FACTOR_SRC_ALPHA,
        .alpha_dst_factor=DM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,

        .winding=DM_WINDING_COUNTERCLOCKWISE,
        .culling=DM_CULL_NONE,
        .fill=DM_FILL_FULL,
        .primitive_type=DM_PRIMITIVE_TRIANGLE_LIST
    };

    if(!dm_renderer_create_raster_pipeline(context, pipe_desc, &imgui_ctx->pipeline)) return false;

    mat4 ortho; 
    glm_mat4_identity(ortho);

    dm_buffer_desc scene_desc = {
        .type=DM_BUFFER_TYPE_STORAGE,
        .size=sizeof(mat4),
        .stride=sizeof(mat4),
        .data=ortho
    };

    dm_buffer_desc vb_desc = {
        .type=DM_BUFFER_TYPE_VERTEX,
        .size=sizeof(imgui_vertex) * IMGUI_MAX_VERTICES,
        .stride=sizeof(imgui_vertex),
    };

    dm_buffer_desc ib_desc = {
        .type=DM_BUFFER_TYPE_INDEX,
        .size=sizeof(imgui_index) * IMGUI_MAX_INDICES,
        .stride=sizeof(imgui_index)
    };

    for(u8 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
    {
        if(!dm_renderer_create_buffer(context, scene_desc, &imgui_ctx->scene[i])) return false;
        if(!dm_renderer_create_buffer(context, vb_desc, &imgui_ctx->vb[i]))       return false;
        if(!dm_renderer_create_buffer(context, ib_desc, &imgui_ctx->ib[i]))       return false;
    }

    dm_sampler_desc sampler_desc = { 0 };
    if(!dm_renderer_create_sampler(context, sampler_desc, &imgui_ctx->sampler)) return false;

    // try mucroui instead
    imgui_ctx->mu_ctx = malloc(sizeof(mu_Context));
    mu_init(imgui_ctx->mu_ctx);
    imgui_ctx->mu_ctx->text_width = text_width;
    imgui_ctx->mu_ctx->text_height = text_height;
    //imgui_ctx->mu_ctx->draw_frame = draw_frame;

    return true;
}

void imgui_update(dm_context *context, imgui_context *imgui_ctx)
{
    int mouse_x = dm_get_mouse_x(context);
    int mouse_y = dm_get_mouse_y(context);

    const u8 current_frame = context->renderer.current_frame;

    //
    mat4 ortho;

    glm_ortho(0, context->window.width, context->window.height, 0, -1.f, 1.f, ortho);

    dm_render_command_update_buffer(context, imgui_ctx->scene[current_frame], ortho, sizeof(ortho));

    // mu stuff
    if(dm_key_is_pressed(context, DM_KEY_LSHIFT)) mu_input_keydown(imgui_ctx->mu_ctx, MU_KEY_SHIFT);
    else if(dm_key_just_released(context, DM_KEY_LSHIFT)) mu_input_keyup(imgui_ctx->mu_ctx, MU_KEY_SHIFT);

    if(dm_key_is_pressed(context, DM_KEY_LCTRL)) mu_input_keydown(imgui_ctx->mu_ctx, MU_KEY_CTRL);
    else if(dm_key_just_released(context, DM_KEY_LCTRL)) mu_input_keyup(imgui_ctx->mu_ctx, MU_KEY_CTRL);

    if(dm_key_is_pressed(context, DM_KEY_LALT)) mu_input_keydown(imgui_ctx->mu_ctx, MU_KEY_ALT);
    else if(dm_key_just_released(context, DM_KEY_LALT)) mu_input_keyup(imgui_ctx->mu_ctx, MU_KEY_ALT);

    if(dm_key_is_pressed(context, DM_KEY_BACKSPACE)) mu_input_keydown(imgui_ctx->mu_ctx, MU_KEY_BACKSPACE);
    else if(dm_key_just_released(context, DM_KEY_BACKSPACE)) mu_input_keyup(imgui_ctx->mu_ctx, MU_KEY_BACKSPACE);

    if(dm_key_is_pressed(context, DM_KEY_ENTER)) mu_input_keydown(imgui_ctx->mu_ctx, MU_KEY_RETURN);
    else if(dm_key_just_released(context, DM_KEY_ENTER)) mu_input_keyup(imgui_ctx->mu_ctx, MU_KEY_RETURN);

    if(dm_mouse_button_is_pressed(context , DM_MOUSE_LEFT)) mu_input_mousedown(imgui_ctx->mu_ctx, mouse_x, mouse_y, MU_MOUSE_LEFT);
    else if(dm_mouse_button_just_released(context, DM_MOUSE_LEFT)) mu_input_mouseup(imgui_ctx->mu_ctx, mouse_x, mouse_y, MU_MOUSE_LEFT);

    if(dm_mouse_button_is_pressed(context , DM_MOUSE_RIGHT)) mu_input_mousedown(imgui_ctx->mu_ctx, mouse_x, mouse_y, MU_MOUSE_RIGHT);
    else if(dm_mouse_button_just_released(context, DM_MOUSE_RIGHT)) mu_input_mouseup(imgui_ctx->mu_ctx, mouse_x, mouse_y, MU_MOUSE_RIGHT);

    if(dm_mouse_button_is_pressed(context , DM_MOUSE_MIDDLE)) mu_input_mousedown(imgui_ctx->mu_ctx, mouse_x, mouse_y, MU_MOUSE_MIDDLE);
    else if(dm_mouse_button_just_released(context, DM_MOUSE_MIDDLE)) mu_input_mouseup(imgui_ctx->mu_ctx, mouse_x, mouse_y, MU_MOUSE_MIDDLE);

    mu_input_mousemove(imgui_ctx->mu_ctx, mouse_x, mouse_y);

    //
    mu_Context *ctx = imgui_ctx->mu_ctx;

    mu_begin(imgui_ctx->mu_ctx);

    if (mu_begin_window(ctx, "My Window", mu_rect(10, 10, 140, 86))) {
  mu_layout_row(ctx, 2, (int[]) { 60, -1 }, 0);

  mu_label(ctx, "First:");
  if (mu_button(ctx, "Button1")) {
    printf("Button1 pressed\n");
  }

  mu_label(ctx, "Second:");
  if (mu_button(ctx, "Button2")) {
    mu_open_popup(ctx, "My Popup");
  }

  if (mu_begin_popup(ctx, "My Popup")) {
    mu_label(ctx, "Hello world!");
    mu_end_popup(ctx);
  }

  mu_end_window(ctx);
}

    mu_end(imgui_ctx->mu_ctx);

    mu_Command *mu_cmd = NULL;
    while(mu_next_command(imgui_ctx->mu_ctx, &mu_cmd))
    {
        switch(mu_cmd->type)
        {
            case MU_COMMAND_JUMP:
                //LOG_TRACE("JUMP");
                break;
            case MU_COMMAND_CLIP: imgui_clip(&imgui_ctx->frame_data[current_frame], mu_cmd->clip.rect); break;
            case MU_COMMAND_RECT: imgui_draw_rect(&imgui_ctx->frame_data[current_frame], mu_cmd->rect.rect, mu_cmd->rect.color); break;
            case MU_COMMAND_TEXT:
                //LOG_TRACE("TEXT");
                break;
            case MU_COMMAND_ICON:
                //LOG_TRACE("ICON");
                break;

            default:
                break;
        }
    }

    //
    dm_render_command_update_buffer(context, imgui_ctx->vb[current_frame], imgui_ctx->frame_data[current_frame].vertices, sizeof(imgui_vertex) * IMGUI_MAX_VERTICES);
    dm_render_command_update_buffer(context, imgui_ctx->ib[current_frame], imgui_ctx->frame_data[current_frame].indices, sizeof(u32) * IMGUI_MAX_INDICES);
}

void imgui_shutdown(imgui_context *context)
{
    free(context->mu_ctx);
}

void imgui_render(dm_context *context, imgui_context *imgui_ctx, dm_resource render_target)
{
    //
    const u8 current_frame = context->renderer.current_frame;

    dm_resource imgui_resources[] = {
        imgui_ctx->vb[current_frame],
        imgui_ctx->scene[current_frame],
        //imgui_ctx->font_texture,
        //imgui_ctx->sampler
    };
    
    dm_render_command_begin_rendering(context, render_target, 0, 0, 0, 0, 1, DM_RENDER_LOAD_OP_LOAD, DM_RENDER_STORE_OP_STORE, DM_RENDER_LOAD_OP_LOAD, DM_RENDER_STORE_OP_DONT_CARE);
        dm_render_command_bind_pipeline(context, imgui_ctx->pipeline);
        dm_render_command_bind_index_buffer(context, imgui_ctx->ib[current_frame], 0);
        dm_render_command_push_resources(context, imgui_resources, 2);
        dm_render_command_draw(context, imgui_ctx->frame_data[current_frame].index_count, 0, 1);


    dm_render_command_end_rendering(context, render_target);

    //
    imgui_ctx->frame_data[current_frame].vertex_count = 0;
    imgui_ctx->frame_data[current_frame].index_count  = 0;
}
