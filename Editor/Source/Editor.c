#include "Editor.h"

EditorData editor_state;

void EditorInit()
{
    EditorInitialiseWindow();
    EditorInitialiseRenderState();
    EditorInitialiseTexturedMesh();
    EditorLaunch();
}

void EditorCleanup()
{
    E_RendererWait();

    E_FreeMaterialInstance(editor_state.material_instance);

    E_FreeMesh(editor_state.mesh);
    E_FreeResource(editor_state.normal_texture);
    E_FreeResource(editor_state.metallic_roughness_texture);
    E_FreeResource(editor_state.albedo_texture);

    E_FreeBuffer(editor_state.material_settings);
    E_FreeBuffer(editor_state.transform_buffer);
  
    E_CleanRenderGraph(editor_state.graph, &editor_state.execute_info);
    E_FreePipelineStatistics(editor_state.stats);

    E_FreeDefaultSamplers();

    E_RendererShutdown();
    E_FreeWindow(editor_state.window);

    E_FreeCVarSystem(&editor_state.cvar_sys);
}

void EditorUpdate()
{
    while (E_IsWindowOpen(editor_state.window) && editor_state.running)
    {
        f32 time = E_TimerGetTime();
        f32 dt = time - editor_state.last_frame;
        editor_state.last_frame = time;

        EditorAssureViewportSize();

        EditorBeginRender();

        EditorDraw();
        EditorDrawGUI();

        EditorEndRender();

        if (editor_state.is_viewport_focused)
            EditorCameraProcessMouse(&editor_state.camera, dt);
        if (editor_state.is_viewport_focused)
            EditorUpdateCameraInput(dt);
        EditorCameraUpdate(&editor_state.camera, dt);

        E_WindowUpdate(editor_state.window);
    }
}

void EditorResize(i32 width, i32 height)
{
    E_RendererResize(width, height);
    E_ResizeRenderGraph(editor_state.graph, &editor_state.execute_info);
}

//

void EditorInitialiseWindow()
{
    E_CreateCVarSystem("Assets/Configs/DefaultConfig.toml", &editor_state.cvar_sys);

    editor_state.running = 1;
    editor_state.execute_info.cvar_table_ptr = &editor_state.cvar_sys;
    editor_state.mesh_shader_enabled = E_GetCVar(&editor_state.cvar_sys, "enable_mesh_shaders").u.b;

    // Initialise Euphorbe
    E_RendererInitSettings settings = { 0 };
    settings.log_found_layers = E_GetCVar(&editor_state.cvar_sys, "log_found_layers").u.b;
    settings.log_renderer_events = E_GetCVar(&editor_state.cvar_sys, "log_renderer_events").u.b;
    settings.enable_debug = E_GetCVar(&editor_state.cvar_sys, "enable_debug").u.b;
    settings.gui_should_clear = E_GetCVar(&editor_state.cvar_sys, "gui_should_clear").u.b;
    settings.enable_vsync = E_GetCVar(&editor_state.cvar_sys, "enable_vsync").u.b;

    editor_state.window = E_CreateWindow(1280, 720, "Euphorbe Engine | Renderer: <Vulkan>", E_GetCVar(&editor_state.cvar_sys, "dark_mode").u.b);
    E_RendererInit(editor_state.window, settings);
    E_InitDefaultSamplers();

    E_TimerInit();
    editor_state.stats = E_CreatePipelineStatistics();

    EditorCameraInit(&editor_state.camera);
}

void EditorInitialiseRenderState()
{
    editor_state.execute_info.width = editor_state.window->width;
    editor_state.execute_info.height = editor_state.window->height;

    editor_state.graph = E_CreateRenderGraph();
    editor_state.geometry_node = CreateGeometryNode();
    editor_state.fxaa_node = CreateFXAANode();
    editor_state.tonemapping_node = CreateTonemappingNode();
    editor_state.final_blit_node = CreateFinalBlitNode();

    E_RenderGraphConnectNodes(editor_state.geometry_node, GeometryNodeOutput_Color, editor_state.fxaa_node, FXAANodeInput_Color);
    E_RenderGraphConnectNodes(editor_state.fxaa_node, FXAANodeOutput_ImageOut, editor_state.tonemapping_node, TonemappingNodeInput_Geometry);
    E_RenderGraphConnectNodes(editor_state.tonemapping_node, TonemappingNodeOutput_Color, editor_state.final_blit_node, FinalBlitNodeInput_ImageIn);

    E_BuildRenderGraph(editor_state.graph, &editor_state.execute_info, editor_state.final_blit_node);
}

void EditorInitialiseTexturedMesh()
{
    memset(&editor_state.execute_info.drawables[0], 0, sizeof(E_Drawable));

    editor_state.material_buffer[0] = 1; // Has albedo texture
    editor_state.material_buffer[1] = 1; // Has roughness texture
    editor_state.material_buffer[2] = 1; // Has normal texture
    editor_state.material_buffer[3] = 0; // Do not draw meshlets

    editor_state.albedo_texture = E_LoadResource("Assets/Textures/DamagedHelmet_Albedo.jpg", E_ResourceTypeTexture);
    editor_state.metallic_roughness_texture = E_LoadResource("Assets/Textures/DamagedHelmet_MetallicRoughness.jpg", E_ResourceTypeTexture);
    editor_state.normal_texture = E_LoadResource("Assets/Textures/DamagedHelmet_Normal.jpg", E_ResourceTypeTexture);

    editor_state.mesh = E_LoadMesh(GetGeometryNodeMaterial(editor_state.geometry_node)->as.material, "Assets/Models/DamagedHelmet.gltf");

    editor_state.transform_buffer = E_CreateUniformBuffer(sizeof(editor_state.execute_info.drawables[0].transform));
    E_SetBufferData(editor_state.transform_buffer, &editor_state.execute_info.drawables[0].transform, sizeof(editor_state.execute_info.drawables[0].transform));

    if (editor_state.mesh_shader_enabled)
        editor_state.material_instance = E_CreateMaterialInstance(GetGeometryNodeMaterial(editor_state.geometry_node)->as.material, 1);
    else
        editor_state.material_instance = E_CreateMaterialInstance(GetGeometryNodeMaterial(editor_state.geometry_node)->as.material, 0);

    editor_state.material_settings = E_CreateUniformBuffer(sizeof(vec4));
    E_SetBufferData(editor_state.material_settings, &editor_state.material_buffer, sizeof(vec4));

    E_MaterialInstanceWriteBuffer(editor_state.material_instance, 0, editor_state.transform_buffer, sizeof(editor_state.execute_info.drawables[0].transform));
    E_MaterialInstanceWriteBuffer(editor_state.material_instance, 1, editor_state.material_settings, sizeof(vec4));
    E_MaterialInstanceWriteSampler(editor_state.material_instance, 2, E_LinearSampler);
    E_MaterialInstanceWriteSampledImage(editor_state.material_instance, 3, editor_state.albedo_texture->as.image);
    E_MaterialInstanceWriteSampledImage(editor_state.material_instance, 4, editor_state.metallic_roughness_texture->as.image);
    E_MaterialInstanceWriteSampledImage(editor_state.material_instance, 5, editor_state.normal_texture->as.image);

    editor_state.execute_info.drawables[0].mesh = editor_state.mesh;
    editor_state.execute_info.drawables[0].material_instance = editor_state.material_instance;
    glm_mat4_identity(editor_state.execute_info.drawables[0].transform);
    glm_rotate(editor_state.execute_info.drawables[0].transform, glm_rad(90.0f), (vec3) { 1.0f, 0.0f, 0.0f });
    editor_state.execute_info.drawable_count++;

    // Initialise directional light
    glm_vec4_fill(editor_state.execute_info.point_lights[0].color, 10.0f);
    
    editor_state.execute_info.point_lights[0].position[0] = 2.5f;
    editor_state.execute_info.point_lights[0].position[1] = -0.5f;
    editor_state.execute_info.point_lights[0].position[2] = 3.7f;
}

void EditorLaunch()
{
    InitViewportPanel(editor_state.window->width, editor_state.window->height);
    E_WindowSetResizeCallback(editor_state.window, EditorResize);
    E_WindowSetScrollCallback(editor_state.window, EditorScroll);
    E_LaunchWindow(editor_state.window);
}

void EditorAssureViewportSize()
{
    if (viewport_panel.viewport_size[0] != (f32)editor_state.execute_info.width || viewport_panel.viewport_size[1] != (f32)editor_state.execute_info.height && viewport_panel.viewport_size[0] > 0 && viewport_panel.viewport_size[1] > 0)
    {
        E_RendererWait();

        editor_state.execute_info.width = (i32)viewport_panel.viewport_size[0];
        editor_state.execute_info.height = (i32)viewport_panel.viewport_size[0];

        E_ResizeRenderGraph(editor_state.graph, &editor_state.execute_info);
        EditorCameraResize(&editor_state.camera, (i32)viewport_panel.viewport_size[0], (i32)viewport_panel.viewport_size[1]);
    }
}

void EditorBeginRender()
{
    f64 start = EditorBeginProfiling();

    E_RendererBegin();
    E_CommandBuffer* cmd_buf = E_GetSwapchainCommandBuffer();
    E_ResetPipelineQuery(cmd_buf, editor_state.stats);
    E_BeginPipelineQuery(cmd_buf, editor_state.stats);

    editor_state.perf.begin_render = EditorEndProfiling(start);
}

void EditorEndRender()
{
    f64 start = EditorBeginProfiling();

    E_CommandBuffer* cmd_buf = E_GetSwapchainCommandBuffer();
    E_EndPipelineQuery(cmd_buf, editor_state.stats);

    E_RendererEnd();
    E_GetQueryResults(editor_state.stats);
    E_RendererPresent();

    editor_state.perf.end_render = EditorEndProfiling(start);
}

void EditorDraw()
{
    f64 start = EditorBeginProfiling();

    E_SetBufferData(editor_state.transform_buffer, &editor_state.execute_info.drawables[0].transform, sizeof(editor_state.execute_info.drawables[0].transform));
    E_SetBufferData(editor_state.material_settings, &editor_state.material_buffer, sizeof(vec4));
    
    glm_mat4_copy(editor_state.camera.view, editor_state.execute_info.view);
    glm_mat4_copy(editor_state.camera.proj, editor_state.execute_info.projection);
    glm_vec3_copy(editor_state.camera.position, editor_state.execute_info.camera_position);
    E_ExecuteRenderGraph(editor_state.graph, &editor_state.execute_info);

    editor_state.perf.execute_render_graph = EditorEndProfiling(start);
}

void EditorDrawGUI()
{
    f64 start = EditorBeginProfiling();

    EditorCreateDockspace();

    E_LogDraw();
    DrawViewportPanel(editor_state.final_blit_node->outputs[0], &editor_state.is_viewport_focused, &editor_state.is_viewport_hovered);

    // Material Viewer
    {
        igBegin("Material Viewer", NULL, ImGuiWindowFlags_AlwaysAutoResize);
        igCheckbox("Enable Albedo", (bool*)& editor_state.material_buffer[0]);
        igCheckbox("Enable Metallic Roughness", (bool*)&editor_state.material_buffer[1]);
        igCheckbox("Enable Normal Map", (bool*)&editor_state.material_buffer[2]);
        igCheckbox("Draw Meshlets", (bool*)&editor_state.material_buffer[3]);
        igEnd();
    }

    // Performance panel
    {
        igBegin("Performance Viewer", NULL, ImGuiWindowFlags_None);
        f64 editor_update = editor_state.perf.begin_render + editor_state.perf.end_render + editor_state.perf.execute_render_graph + editor_state.perf.draw_gui;

        b32 perf_open = igTreeNodeEx_Str("Performance Timer", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding);
        if (perf_open)
        {
            igText("EditorBeginRender: %g ms", editor_state.perf.begin_render);
            igText("EditorEndRender: %g ms", editor_state.perf.end_render);
            igText("EditorDraw: %g ms", editor_state.perf.execute_render_graph);
            igText("EditorDrawGUI: %g ms", editor_state.perf.draw_gui);
            igText("EditorUpdate: %g ms", editor_update);
            igTreePop();
        }

        b32 render_stats = igTreeNodeEx_Str("Renderer Stats", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding);
        if (render_stats)
        {
            E_RendererDrawRendererStats();
            E_DrawPipelineStatisticsGUI(editor_state.stats);
            igTreePop();
        }

        b32 memory_info = igTreeNodeEx_Str("Memory Stats", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding);
        if (memory_info)
        {
            E_RendererDrawMemoryUsageGUI();
            igTreePop();
        }

        b32 gpu_info = igTreeNodeEx_Str("Graphics Card Info", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding);
        if (gpu_info)
        {
            E_RendererDrawGraphicsCardInfo();
            igTreePop();
        }

        igEnd();
    }

    // Light Panel
    {
        igBegin("Light Manager", NULL, ImGuiWindowFlags_None);
        igDragFloat4("Light Color", editor_state.execute_info.point_lights[0].color, 0.1f, 0.1f, 0.0f, "%.1f", ImGuiSliderFlags_None);
        igDragFloat4("Light Position", editor_state.execute_info.point_lights[0].position, 0.1f, 0.1f, 0.0f, "%.1f", ImGuiSliderFlags_None);
        igEnd();
    }

    // Render Graph Viewer
    {
        igBegin("Scene Renderer", NULL, ImGuiWindowFlags_None);
        GeometryNodeDrawGUI(editor_state.geometry_node);
        FXAANodeDrawGUI(editor_state.fxaa_node);
        TonemappingNodeDrawGUI(editor_state.tonemapping_node);
        igEnd();
    }

    EditorDestroyDockspace();

    editor_state.perf.draw_gui = EditorEndProfiling(start);
}

void EditorCreateDockspace()
{
    E_BeginGUI();

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    ImGuiViewport* viewport = igGetMainViewport();
    igSetNextWindowPos(viewport->Pos, ImGuiCond_None, (ImVec2) { 0.0f, 0.0f });
    igSetNextWindowSize(viewport->Size, ImGuiCond_None);
    igSetNextWindowViewport(viewport->ID);
    igPushStyleVar_Float(ImGuiStyleVar_WindowRounding, 0.0f);
    igPushStyleVar_Float(ImGuiStyleVar_WindowBorderSize, 0.0f);
    igPushStyleVar_Vec2(ImGuiStyleVar_WindowPadding, (ImVec2) { 0.0f, 0.0f });
    igBegin("Dockspace", NULL, window_flags);
    igPopStyleVar(3);

    igDockSpace(igGetID_Str("MyDockSpace"), (ImVec2) { 0.0f, 0.0f }, ImGuiDockNodeFlags_None, NULL);

    if (igBeginMenuBar())
    {
        if (igBeginMenu("File", 1))
        {
            if (igMenuItem_Bool("Quit", "Ctrl+Q", 0, 1))
                editor_state.running = 0;
            igEndMenu();
        }

        igEndMenuBar();
    }
}

void EditorDestroyDockspace()
{
    igEnd();

    E_EndGUI();
}

void EditorScroll(f32 scroll)
{
    if (editor_state.is_viewport_focused && editor_state.is_viewport_hovered)
        EditorCameraOnMouseScroll(&editor_state.camera, scroll);
}

f64 EditorBeginProfiling()
{
    return clock();
}

void EditorUpdateCameraInput(f32 dt)
{
    EditorCameraProcessInput(&editor_state.camera, dt);
}

f64 EditorEndProfiling(f64 start)
{
    clock_t end = clock();
    return ((f64)(end - start));
}
