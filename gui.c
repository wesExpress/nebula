#include "gui.h"

#include "cglm/cglm.h"

static bool demo = true;

bool gui_init(dm_context *context, gui_context *gui_ctx)
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
        .shaders[DM_RASTER_SHADER_STAGE_VERTEX]   = vertex_shader,
        .shaders[DM_RASTER_SHADER_STAGE_FRAGMENT] = fragment_shader,
        .depth=false,
        .culling=DM_CULL_NONE,
        .primitive_type=DM_PRIMITIVE_TRIANGLE_LIST,
        .fill=DM_FILL_FULL,
        .winding=DM_WINDING_COUNTERCLOCKWISE,
        .blend=true,
        .color_blend_op=DM_BLEND_OP_ADD,
        .color_src_factor=DM_BLEND_FACTOR_SRC_ALPHA,
        .color_dst_factor=DM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .alpha_blend_op=DM_BLEND_OP_ADD,
        .alpha_src_factor=DM_BLEND_FACTOR_ONE,
        .alpha_dst_factor=DM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
    };

    if(!dm_renderer_create_raster_pipeline(context, pipe_desc, &gui_ctx->resources.pipeline)) return false;

    dm_sampler_desc linear_desc = {
        .min=DM_SAMPLER_FILTER_LINEAR,
        .mag=DM_SAMPLER_FILTER_LINEAR,
        .mip=DM_SAMPLER_FILTER_LINEAR
    };

    dm_sampler_desc nearest_desc = {
        .min=DM_SAMPLER_FILTER_NEAREST,
        .mag=DM_SAMPLER_FILTER_NEAREST,
        .mip=DM_SAMPLER_FILTER_NEAREST
    };

    if(!dm_renderer_create_sampler(context, linear_desc, &gui_ctx->resources.linear_sampler))  return false;
    if(!dm_renderer_create_sampler(context, linear_desc, &gui_ctx->resources.nearest_sampler)) return false;

    dm_buffer_desc vb_desc = {
        .type=DM_BUFFER_TYPE_VERTEX,
        .size=sizeof(ImDrawVert) * GUI_MAX_VERTICES,
        .stride=sizeof(ImDrawVert)
    };
    dm_buffer_desc ib_desc = {
        .type=DM_BUFFER_TYPE_INDEX,
        .size=sizeof(ImDrawIdx) * GUI_MAX_INDICES,
        .stride=sizeof(ImDrawIdx)
    };
    dm_buffer_desc cb_desc = {
        .type=DM_BUFFER_TYPE_STORAGE,
        .size=sizeof(mat4),
    };

    for(u8 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
    {
        if(!dm_renderer_create_buffer(context, vb_desc, &gui_ctx->resources.vb[i]))    return false;
        if(!dm_renderer_create_buffer(context, ib_desc, &gui_ctx->resources.ib[i]))    return false;
        if(!dm_renderer_create_buffer(context, cb_desc, &gui_ctx->resources.scene[i])) return false;
    }

    ImGuiIO *io = ImGui_GetIO();

    return true;
}

void gui_new_frame(dm_context *context, gui_context *gui_ctx)
{
    ImGui_NewFrame();

    ImGuiIO *io = ImGui_GetIO();
    ImGui_ShowDemoWindow(&demo);

    static float f = 0.0f;
    static int counter = 0;
    static ImColor clear_color;

    ImGui_Begin("Hello, world!", NULL, 0);              // Create a window called "Hello, world!" and append into it.

    ImGui_Text("This is some useful text.");            // Display some text (you can use a format strings too)
    ImGui_Checkbox("Demo Window", &demo);               // Edit bools storing our window open/close state
    ImGui_Checkbox("Another Window", &demo);

    ImGui_SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
    ImGui_ColorEdit3("clear color", (float*)&clear_color, 0); // Edit 3 floats representing a color

    if (ImGui_Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
        counter++;
    ImGui_SameLine();
    ImGui_Text("counter = %d", counter);

    ImGui_Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
    ImGui_End();
}

bool gui_create_texture(dm_context *context, ImTextureData *tex, dm_resource *resource)
{
    dm_texture2d_desc desc = {
        .type=DM_TEXTURE2D_TYPE_SAMPLED,
        .format=DM_TEXTURE2D_FORMAT_R8G8B8A8_UNORM,
        .width=tex->Width,
        .height=tex->Height,
        .data=tex->Pixels,
    };

    if(!dm_renderer_create_texture(context, desc, resource))         return false;
    if(!dm_renderer_upload_resources_to_heap(context, &resource, 1)) return false;

    ImTextureData_SetStatus(tex, ImTextureStatus_OK);

    return true;
}

void gui_update_texture(dm_context *context, ImTextureData *tex, dm_resource resource)
{
    dm_render_command_update_texture(context, resource, tex->Pixels, tex->Width * tex->Height * tex->BytesPerPixel);

    ImTextureData_SetStatus(tex, ImTextureStatus_OK);
}

bool gui_end_frame(dm_context *context, gui_context *gui_ctx)
{
    const u8 current_frame = context->renderer.current_frame;

    ImGui_Render();
    ImDrawData *draw_data = ImGui_GetDrawData();
    if(draw_data->CmdLists.Size==0) return true;

    if(draw_data->Textures)
    {
        for(u32 i=0; i<draw_data->Textures->Size; i++)
        {
            ImTextureData *tex = *draw_data->Textures[i].Data;
            if(tex->Status == ImTextureStatus_WantCreate)
            {
                if(!gui_create_texture(context, tex, &gui_ctx->resources.texture)) return false;
            }
            //else if(tex->Status == ImTextureStatus_WantUpdates) gui_update_texture(context, tex, gui_ctx->resources.texture);
        }
    }

    size_t index_offset = 0;
    size_t vertex_offset = 0;

    dm_resource vb = gui_ctx->resources.vb[current_frame];
    dm_resource ib = gui_ctx->resources.ib[current_frame];

    for(u32 i=0; i<draw_data->CmdListsCount; i++)
    {
        ImDrawList *list = draw_data->CmdLists.Data[i];

        size_t vb_size = list->VtxBuffer.Size * sizeof(ImDrawVert);
        size_t ib_size = list->IdxBuffer.Size * sizeof(ImDrawIdx);

        dm_render_command_update_buffer(context, vb, list->VtxBuffer.Data, vb_size, vertex_offset);
        dm_render_command_update_buffer(context, ib, list->IdxBuffer.Data, ib_size, index_offset);

        vertex_offset += vb_size;
        index_offset  += ib_size;
    }

    // projection matrix
    mat4 ortho;
    glm_ortho(0, context->window.width, context->window.height,0, 0.f,1.f, ortho);

    dm_render_command_update_buffer(context, gui_ctx->resources.scene[current_frame], ortho, sizeof(ortho), 0);

    return true;
}

void gui_render(dm_context *context, gui_context *gui_ctx)
{
    const u8 current_frame = context->renderer.current_frame;

    ImDrawData *draw_data = ImGui_GetDrawData();
    if(draw_data->CmdLists.Size == 0) return;

    dm_resource resources[] = {
        gui_ctx->resources.vb[current_frame],
        gui_ctx->resources.scene[current_frame],
        gui_ctx->resources.texture,
        gui_ctx->resources.linear_sampler
    };

    int width = context->window.width;
    int height = context->window.height;

    ImVec2 clip_off = draw_data->DisplayPos;
    ImVec2 clip_scale = draw_data->FramebufferScale;

    u32 index_offset = 0;
    u32 vertex_offset = 0;

    dm_render_command_set_viewport(context, 0, 0, width, height, 0, 1.f);
    dm_render_command_set_scissor(context, 0, 0, width, height);
    dm_render_command_bind_pipeline(context, gui_ctx->resources.pipeline);
    dm_render_command_push_resources(context, resources, 4);
    dm_render_command_bind_index_buffer(context, gui_ctx->resources.ib[current_frame], index_offset);

    for(u32 i=0; i<draw_data->CmdListsCount; i++)
    {
        ImDrawList *list = draw_data->CmdLists.Data[i];

        for(u32 j=0; j<list->CmdBuffer.Size; j++)
        {
            const ImDrawCmd *cmd = &list->CmdBuffer.Data[j];
            if(cmd->ElemCount == 0) continue;

            size_t cmd_vertex_offset = vertex_offset + cmd->VtxOffset;
#ifdef DM_METAL
            size_t cmd_index_offset  = index_offset  + cmd->IdxOffset * sizeof(ImDrawIdx);
#else
            size_t cmd_index_offset  = index_offset  + cmd->IdxOffset;
#endif

            ImVec2 clip_min = { 
                (cmd->ClipRect.x-clip_off.x) * clip_scale.x, 
                (cmd->ClipRect.y-clip_off.y) * clip_scale.y 
            };
            ImVec2 clip_max = { 
                (cmd->ClipRect.z-clip_off.x) * clip_scale.x, 
                (cmd->ClipRect.w-clip_off.y) * clip_scale.y
            };

            if(clip_min.x < 0.f) clip_min.x = 0.f;
            if(clip_min.y < 0.f) clip_min.y = 0.f;
            if(clip_max.x > (float)width) clip_max.x = (float)width;
            if(clip_max.y > (float)height) clip_max.y = (float)height;
            if(clip_max.x <= clip_min.x || clip_max.y <= clip_min.y) continue;

            int scissor_width  = clip_max.x - clip_min.x;
            int scissor_height = clip_max.y - clip_min.y;
            dm_render_command_set_scissor(context, clip_min.x, clip_min.y, scissor_width, scissor_height);

            dm_render_command_draw(context, cmd->ElemCount, cmd_index_offset, 1, cmd_vertex_offset);
        }

        vertex_offset += (size_t)list->VtxBuffer.Size;
#ifdef DM_METAL
        index_offset  += (size_t)list->IdxBuffer.Size * sizeof(ImDrawIdx);
#else
        index_offset  += (size_t)list->IdxBuffer.Size;
#endif
    }
}
