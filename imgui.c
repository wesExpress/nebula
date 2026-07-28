#define NK_IMPLEMENTATION
#include "imgui.h"

#include "cglm/cglm.h"

const size_t vertex_buffer_size = sizeof(imgui_vertex) * 10000;
const size_t index_buffer_size  = sizeof(u16) * 10000;

NK_INTERN void clipboard_copy(nk_handle usr, const char *text, int len) 
{
    dm_context *context = usr.ptr;

    dm_window_clipboard_copy(context, text, len);
}

NK_INTERN void clipboard_paste(nk_handle usr, struct nk_text_edit *edit) 
{
    dm_context *context = usr.ptr;

    const char *text = dm_window_clipboard_paste(context);

    if(!text) return;

    nk_textedit_paste(edit, text, nk_strlen(text));
}

bool imgui_init(dm_context *context, imgui_context *imgui_ctx)
{
    nk_init_default(&imgui_ctx->nuklear_context, 0);
    nk_buffer_init_default(&imgui_ctx->commands);

    imgui_ctx->nuklear_context.clip.copy  = clipboard_copy;
    imgui_ctx->nuklear_context.clip.paste = clipboard_paste;

    imgui_ctx->nuklear_context.clip.userdata.ptr = context;

    nk_font_atlas_init_default(&imgui_ctx->font_atlas);

    nk_font_atlas_begin(&imgui_ctx->font_atlas);
        struct nk_font *chicago = nk_font_atlas_add_from_file(&imgui_ctx->font_atlas, "../../assets/fonts/ChicagoFLF.ttf", 14, 0);
        if(!chicago) 
        {
            LOG_FATAL("Could not load Chicago font");
            return false;
        }
    nk_font_atlas_end(&imgui_ctx->font_atlas, nk_handle_ptr(0), &imgui_ctx->null_texture);

    int width, height;
    const void *image = nk_font_atlas_bake(&imgui_ctx->font_atlas, &width, &height, NK_FONT_ATLAS_RGBA32);

    dm_texture2d_desc font_texture_desc = {
        .type=DM_TEXTURE2D_TYPE_SAMPLED,
        .width=width, .height=height,
        .size=sizeof(u32) * width * height,
        .data=(void*)image
    };

    if(!dm_renderer_create_texture(context, font_texture_desc, &imgui_ctx->font_texture)) return false;

    struct nk_color table[NK_COLOR_COUNT];
    table[NK_COLOR_TEXT] = nk_rgba(210, 210, 210, 255);
    table[NK_COLOR_WINDOW] = nk_rgba(57, 67, 71, 215);
    table[NK_COLOR_HEADER] = nk_rgba(51, 51, 56, 220);
    table[NK_COLOR_BORDER] = nk_rgba(46, 46, 46, 255);
    table[NK_COLOR_BUTTON] = nk_rgba(48, 83, 111, 255);
    table[NK_COLOR_BUTTON_HOVER] = nk_rgba(58, 93, 121, 255);
    table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(63, 98, 126, 255);
    table[NK_COLOR_TOGGLE] = nk_rgba(50, 58, 61, 255);
    table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(45, 53, 56, 255);
    table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(48, 83, 111, 255);
    table[NK_COLOR_SELECT] = nk_rgba(57, 67, 61, 255);
    table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(48, 83, 111, 255);
    table[NK_COLOR_SLIDER] = nk_rgba(50, 58, 61, 255);
    table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(48, 83, 111, 245);
    table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
    table[NK_COLOR_PROPERTY] = nk_rgba(50, 58, 61, 255);
    table[NK_COLOR_EDIT] = nk_rgba(50, 58, 61, 225);
    table[NK_COLOR_EDIT_CURSOR] = nk_rgba(210, 210, 210, 255);
    table[NK_COLOR_COMBO] = nk_rgba(50, 58, 61, 255);
    table[NK_COLOR_CHART] = nk_rgba(50, 58, 61, 255);
    table[NK_COLOR_CHART_COLOR] = nk_rgba(48, 83, 111, 255);
    table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
    table[NK_COLOR_SCROLLBAR] = nk_rgba(50, 58, 61, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(48, 83, 111, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
    table[NK_COLOR_TAB_HEADER] = nk_rgba(48, 83, 111, 255);
    nk_style_from_table(&imgui_ctx->nuklear_context, table);
    nk_style_set_font(&imgui_ctx->nuklear_context, &chicago->handle);

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
        .alpha_dst_factor=DM_BLEND_FACTOR_ONE,

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
        .size=vertex_buffer_size,
        .stride=sizeof(imgui_vertex),
    };

    dm_buffer_desc ib_desc = {
        .type=DM_BUFFER_TYPE_INDEX,
        .size=index_buffer_size,
        .stride=sizeof(u16)
    };

    for(u8 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
    {
        if(!dm_renderer_create_buffer(context, scene_desc, &imgui_ctx->scene[i])) return false;
        if(!dm_renderer_create_buffer(context, vb_desc, &imgui_ctx->vb[i]))       return false;
        if(!dm_renderer_create_buffer(context, ib_desc, &imgui_ctx->ib[i]))       return false;

        // TODO: bad using malloc
        imgui_ctx->vertices[i] = calloc(vertex_buffer_size, sizeof(imgui_vertex));
        imgui_ctx->indices[i]  = calloc(index_buffer_size, sizeof(u16));
    }

    dm_sampler_desc sampler_desc = { 0 };
    if(!dm_renderer_create_sampler(context, sampler_desc, &imgui_ctx->sampler)) return false;

    return true;
}

void imgui_update(dm_context *context, imgui_context *imgui_ctx)
{
    // nuklear input
    struct nk_context *nk_context = &imgui_ctx->nuklear_context;

    int mouse_x = dm_get_mouse_x(context);
    int mouse_y = dm_get_mouse_y(context);

    nk_input_begin(nk_context);

    nk_input_key(nk_context, NK_KEY_TEXT_UNDO, 0);
    nk_input_key(nk_context, NK_KEY_TEXT_REDO, 0);
    nk_input_key(nk_context, NK_KEY_TEXT_WORD_LEFT, 0);
    nk_input_key(nk_context, NK_KEY_TEXT_WORD_RIGHT, 0);
    nk_input_key(nk_context, NK_KEY_COPY, 0);
    nk_input_key(nk_context, NK_KEY_CUT, 0);
    nk_input_key(nk_context, NK_KEY_PASTE, 0);

    nk_input_button(nk_context, NK_BUTTON_LEFT,  mouse_x, mouse_y, 0);
    nk_input_button(nk_context, NK_BUTTON_RIGHT, mouse_x, mouse_y, 0);

    if(dm_key_is_pressed(context, DM_KEY_LCTRL))
    {
        if(dm_key_is_pressed(context, DM_KEY_U))     nk_input_key(nk_context, NK_KEY_TEXT_UNDO, 1);
        if(dm_key_is_pressed(context, DM_KEY_R))     nk_input_key(nk_context, NK_KEY_TEXT_REDO, 1);
        if(dm_key_is_pressed(context, DM_KEY_LEFT))  nk_input_key(nk_context, NK_KEY_TEXT_WORD_LEFT, 1);
        if(dm_key_is_pressed(context, DM_KEY_RIGHT)) nk_input_key(nk_context, NK_KEY_TEXT_WORD_RIGHT, 1);
        if(dm_key_is_pressed(context, DM_KEY_C))     nk_input_key(nk_context, NK_KEY_COPY, 1);
        if(dm_key_is_pressed(context, DM_KEY_X))     nk_input_key(nk_context, NK_KEY_CUT, 1);
        if(dm_key_is_pressed(context, DM_KEY_V))     nk_input_key(nk_context, NK_KEY_PASTE, 1);
    }

    if(dm_mouse_button_is_pressed(context, DM_MOUSE_LEFT))  nk_input_button(nk_context, NK_BUTTON_LEFT, mouse_x, mouse_y, 1);
    if(dm_mouse_button_is_pressed(context, DM_MOUSE_RIGHT)) nk_input_button(nk_context, NK_BUTTON_RIGHT, mouse_x, mouse_y, 1);

    nk_input_motion(nk_context, mouse_x, mouse_y);

    nk_input_end(nk_context);

    enum {EASY, HARD};
    static int op = EASY;
    static float value = 0.6f;

    struct nk_context *ctx = &imgui_ctx->nuklear_context;

    if (nk_begin(ctx, "Show", nk_rect(50, 50, 220, 220),
        NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE)) {
        /* fixed widget pixel width */
        nk_layout_row_static(ctx, 30, 80, 1);
        if (nk_button_label(ctx, "button")) {
            /* event handling */
        }

        /* fixed widget window ratio width */
        nk_layout_row_dynamic(ctx, 30, 2);
        if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
        if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;

        /* custom widget pixel width */
        nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
        {
            nk_layout_row_push(ctx, 50);
            nk_label(ctx, "Volume:", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, 110);
            nk_slider_float(ctx, 0, &value, 1.0f, 0.1f);
        }
        nk_layout_row_end(ctx);
    }
    nk_end(ctx);

    // render update
    NK_STORAGE const struct nk_draw_vertex_layout_element vertex_layout[] = {
        { NK_VERTEX_POSITION, NK_FORMAT_FLOAT,              NK_OFFSETOF(imgui_vertex, pos) },
        { NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT,              NK_OFFSETOF(imgui_vertex, uv) },
        { NK_VERTEX_COLOR,    NK_FORMAT_R32G32B32A32_FLOAT, NK_OFFSETOF(imgui_vertex, color) },
        { NK_VERTEX_LAYOUT_END }
    };

    struct nk_convert_config config = { 0 };
    config.vertex_layout = vertex_layout;
    config.vertex_size = sizeof(imgui_vertex);
    config.vertex_alignment = NK_ALIGNOF(imgui_vertex);
    config.global_alpha = 1.0f;
    config.shape_AA = NK_ANTI_ALIASING_ON;
    config.line_AA = NK_ANTI_ALIASING_ON;
    config.circle_segment_count = 22;
    config.curve_segment_count = 22;
    config.arc_segment_count = 22;
    config.tex_null = imgui_ctx->null_texture;

    const u8 current_frame = context->renderer.current_frame;
    struct nk_buffer vb, ib;
    nk_buffer_init_fixed(&vb, imgui_ctx->vertices[current_frame], vertex_buffer_size);
    nk_buffer_init_fixed(&ib, imgui_ctx->indices[current_frame], index_buffer_size);
    nk_convert(nk_context, &imgui_ctx->commands, &vb, &ib, &config);

    dm_render_command_update_buffer(context, imgui_ctx->vb[current_frame], imgui_ctx->vertices[current_frame], vertex_buffer_size);
    dm_render_command_update_buffer(context, imgui_ctx->ib[current_frame], imgui_ctx->indices[current_frame], index_buffer_size);

    //
    mat4 ortho;

    glm_ortho(0, context->window.width, context->window.height, 0, -1.f, 1.f, ortho);

    dm_render_command_update_buffer(context, imgui_ctx->scene[current_frame], ortho, sizeof(ortho));
}

void imgui_shutdown(imgui_context *context)
{
    for(u8 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
    {
        free(context->vertices[i]);
        free(context->indices[i]);
    }

    nk_buffer_free(&context->commands);
    nk_font_atlas_clear(&context->font_atlas);
    nk_free(&context->nuklear_context);
}

void imgui_render(dm_context *context, imgui_context *imgui_ctx, dm_resource render_target)
{
    //
    const u8 current_frame = context->renderer.current_frame;

    dm_resource imgui_resources[] = {
        imgui_ctx->vb[current_frame],
        imgui_ctx->scene[current_frame],
        imgui_ctx->font_texture,
        imgui_ctx->sampler
    };
    
    dm_render_command_begin_rendering(context, render_target, 0, 0, 0, 0, 1, DM_RENDER_LOAD_OP_LOAD, DM_RENDER_STORE_OP_STORE, DM_RENDER_LOAD_OP_LOAD, DM_RENDER_STORE_OP_DONT_CARE);
        dm_render_command_bind_pipeline(context, imgui_ctx->pipeline);
        dm_render_command_bind_index_buffer(context, imgui_ctx->ib[current_frame], 0);
        dm_render_command_push_resources(context, imgui_resources, 4);
        const struct nk_draw_command* cmd;
        uint32_t offset = 0;
        nk_draw_foreach(cmd, &imgui_ctx->nuklear_context, &imgui_ctx->commands)
        {
            if(!cmd->elem_count) continue;

            dm_render_command_draw(context, cmd->elem_count, offset, 1);
            offset += cmd->elem_count;
        }
    dm_render_command_end_rendering(context, render_target);

    nk_clear(&imgui_ctx->nuklear_context);
    nk_buffer_clear(&imgui_ctx->commands);
}
