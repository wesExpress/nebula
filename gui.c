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

    return true;
}

void gui_new_frame(dm_context *context, gui_context *gui_ctx)
{
    ImGuiIO *io = ImGui_GetIO();
    ImGui_NewFrame();
    ImGui_ShowDemoWindow(&demo);

    static float f = 0.0f;
    static int counter = 0;

    ImGui_Begin("Hello, world!", NULL, 0);                          // Create a window called "Hello, world!" and append into it.

    ImGui_Text("This is some useful text.");               // Display some text (you can use a format strings too)
    ImGui_Checkbox("Demo Window", &demo);      // Edit bools storing our window open/close state
    ImGui_Checkbox("Another Window", &demo);

    ImGui_SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
    //ImGui_ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

    if (ImGui_Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
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
    tex->Status = ImTextureStatus_OK;

    if(!dm_renderer_create_texture(context, desc, resource))         return false;
    if(!dm_renderer_upload_resources_to_heap(context, &resource, 1)) return false;

    return true;
}

void gui_update_texture(dm_context *context, ImTextureData *tex, dm_resource resource)
{
    for(u32 j=0; j<tex->Updates.Size; j++)
    {
        ImTextureRect rect = tex->Updates.Data[j];
        dm_render_command_update_texture(context, resource, tex->Pixels, rect.x, rect.y, rect.w, rect.h);
    }

    tex->Status = ImTextureStatus_OK;
}

bool gui_end_frame(dm_context *context, gui_context *gui_ctx)
{
    const u8 current_frame = context->renderer.current_frame;

    ImGui_EndFrame();
    ImGui_Render();
    ImDrawData *draw_data = ImGui_GetDrawData();

    if(draw_data->Textures)
    {
        for(u32 i=0; i<draw_data->Textures->Size; i++)
        {
            ImTextureData *tex = *draw_data->Textures[i].Data;
            switch(tex->Status)
            {
                default:
                case ImTextureStatus_OK: continue;

                case ImTextureStatus_WantCreate:  gui_create_texture(context, tex, &gui_ctx->resources.texture); break;
                case ImTextureStatus_WantUpdates: gui_update_texture(context, tex, gui_ctx->resources.texture); break;
                case ImTextureStatus_WantDestroy:
                    LOG_INFO("DESTROY");
                break;
            }
        }
    }

    size_t index_offset = 0;
    size_t vertex_offset = 0;

    for(u32 i=0; i<draw_data->CmdListsCount; i++)
    {
        ImDrawList *list = draw_data->CmdLists.Data[i];

        dm_render_command_update_buffer(context, gui_ctx->resources.vb[current_frame], list->VtxBuffer.Data, list->VtxBuffer.Size * sizeof(ImDrawVert), vertex_offset);
        dm_render_command_update_buffer(context, gui_ctx->resources.ib[current_frame], list->IdxBuffer.Data, list->IdxBuffer.Size * sizeof(ImDrawIdx), index_offset);

        vertex_offset += (size_t)list->VtxBuffer.Size * sizeof(ImDrawVert);
        index_offset  += (size_t)list->IdxBuffer.Size * sizeof(ImDrawIdx);
    }

    // projection matrix
    mat4 ortho;
    glm_ortho(0, context->window.width, context->window.height, 0, -1.f,1.f, ortho);

    dm_render_command_update_buffer(context, gui_ctx->resources.scene[current_frame], ortho, sizeof(ortho), 0);

    return true;
}

void gui_render(dm_context *context, gui_context *gui_ctx, dm_resource render_target)
{
    const u8 current_frame = context->renderer.current_frame;

    ImDrawData *draw_data = ImGui_GetDrawData();

    dm_resource resources[] = {
        gui_ctx->resources.vb[current_frame],
        gui_ctx->resources.scene[current_frame],
        gui_ctx->resources.texture,
        gui_ctx->resources.linear_sampler
    };

    u32 index_offset = 0;

    int width  = draw_data->DisplaySize.x ;
    int height = draw_data->DisplaySize.y ;

    ImVec2 clip_off = draw_data->DisplayPos;

    dm_render_command_begin_rendering(context, render_target, 0, 0, 0, 1, 1, DM_RENDER_LOAD_OP_LOAD, DM_RENDER_STORE_OP_STORE, DM_RENDER_LOAD_OP_DONT_CARE, DM_RENDER_STORE_OP_DONT_CARE);
        dm_render_command_set_viewport(context, 0, 0, width, height, 0, 1.f);
        dm_render_command_set_scissor(context, 0, 0, width, height);
        dm_render_command_bind_pipeline(context, gui_ctx->resources.pipeline);
        dm_render_command_push_resources(context, resources, 4);
        dm_render_command_bind_index_buffer(context, gui_ctx->resources.ib[current_frame], 0);

        for(u32 i=0; i<draw_data->CmdListsCount; i++)
        {
            ImDrawList *list = draw_data->CmdLists.Data[i];

            for(u32 j=0; j<list->CmdBuffer.Size; j++)
            {
                const ImDrawCmd *cmd = &list->CmdBuffer.Data[j];
                if(cmd->ElemCount == 0) continue;

                ImVec2 clip_min = { cmd->ClipRect.x, cmd->ClipRect.y };
                ImVec2 clip_max = { cmd->ClipRect.z, cmd->ClipRect.w };
                if(clip_min.x < 0.f) clip_min.x = 0.f;
                if(clip_min.y < 0.f) clip_min.y = 0.f;
                if(clip_max.x > (float)width) clip_min.x = (float)width;
                if(clip_max.y > (float)height) clip_min.y = (float)height;
                if(clip_max.x <= clip_min.x || clip_max.y <= clip_min.y) continue;

                dm_render_command_set_scissor(context, clip_min.x, clip_min.y, clip_max.x-clip_min.x, clip_max.y-clip_min.y);

                dm_render_command_draw(context, cmd->ElemCount, index_offset, 1);
            }

            index_offset += (size_t)list->IdxBuffer.Size * sizeof(ImDrawIdx);
        }

    dm_render_command_end_rendering(context, render_target);
}
