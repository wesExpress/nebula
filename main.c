#include "DarkMatter/dm.h"
#include <stdio.h>

int main(void)
{
    dm_handle test = { 0 };

    printf("Hello world\n");

    dm_arena arena = { 0 };
    dm_arena_create(&arena, DM_MEGABYTE);

    size_t context_offset;
    dm_context *context = dm_arena_alloc(&arena, sizeof(dm_context), &context_offset);
    if(!dm_init(context, 500, 500, "nebula", 0)) return 1;

    dm_handle swapchain;
    dm_render_target_desc desc = {
        .type=DM_RENDER_TARGET_TYPE_SWAPCHAIN,
        .color_attachment.load_op=DM_RENDER_ATTACHMENT_LOAD_OP_CLEAR,
        .color_attachment.store_op=DM_RENDER_ATTACHMENT_STORE_OP_STORE,
        .depth_attachment.load_op=DM_RENDER_ATTACHMENT_LOAD_OP_LOAD,
        .depth_attachment.store_op=DM_RENDER_ATTACHMENT_STORE_OP_STORE
    };
    if(!dm_renderer_create_render_target(context, desc, &swapchain)) return 1;

    while(dm_is_running(context))
    {
        dm_update(context);

        /*********
         * RENDER
         **********/
        if(!dm_render_begin(context)) break;

        dm_render_command_begin_rendering(context, swapchain, 0.1f, 0.1f, 0.5f, 1.f, 1.f);
        dm_render_command_end_rendering(context, swapchain);

        if(!dm_render_end(context)) break;
    }

    dm_shutdown(context);

    //
    dm_arena_detroy(&arena);

    return 0;
}
