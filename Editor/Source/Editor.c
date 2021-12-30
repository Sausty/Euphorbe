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
    E_FreeResource(editor_state.mesh);
    E_FreeBuffer(editor_state.material_settings);
    E_FreeResource(editor_state.metallic_roughness_texture);
    E_FreeResource(editor_state.albedo_texture);
    E_CleanRenderGraph(editor_state.graph, &editor_state.execute_info);

    E_RendererShutdown();
    E_FreeWindow(editor_state.window);
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
    editor_state.running = 1;

    // Initialise Euphorbe
    E_RendererInitSettings settings = { 0 };
    settings.log_found_layers = 0;
    settings.log_renderer_events = 1;
    settings.enable_debug = 1;
    settings.gui_should_clear = 1;

    editor_state.window = E_CreateWindow(1280, 720, "Euphorbe Editor");
    E_RendererInit(editor_state.window, settings);

    E_TimerInit();

    EditorCameraInit(&editor_state.camera);
}

void EditorInitialiseRenderState()
{
    editor_state.execute_info.width = editor_state.window->width;
    editor_state.execute_info.height = editor_state.window->height;

    editor_state.graph = E_CreateRenderGraph();
    editor_state.geometry_node = CreateGeometryNode();
    editor_state.hdr_node = CreateHDRNode();
    editor_state.final_blit_node = CreateFinalBlitNode();

    E_RenderGraphConnectNodes(editor_state.geometry_node, GeometryNodeOutput_Color, editor_state.hdr_node, HDRNodeInput_Geometry);
    E_RenderGraphConnectNodes(editor_state.hdr_node, HDRNodeOutput_Color, editor_state.final_blit_node, FinalBlitNodeInput_ImageIn);

    E_BuildRenderGraph(editor_state.graph, &editor_state.execute_info, editor_state.final_blit_node);
}

void EditorInitialiseTexturedMesh()
{
    editor_state.material_buffer[0] = 1.0f; // Has albedo texture
    editor_state.material_buffer[1] = 1.0f; // Has roughness texture
    editor_state.material_buffer[2] = 1.0f; // Enable blending
    editor_state.material_buffer[3] = 0.0f; // Don't flip UVs

    editor_state.albedo_texture = E_LoadResource("Assets/Textures/Suzanne_BaseColor.png", E_ResourceTypeTexture);
    editor_state.metallic_roughness_texture = E_LoadResource("Assets/Textures/Suzanne_MetallicRoughness.png", E_ResourceTypeTexture);

    editor_state.mesh = E_LoadResource("Assets/Models/Suzanne.gltf", E_ResourceTypeMesh);
    editor_state.material_instance = E_CreateMaterialInstance(GetGeometryNodeMaterial(editor_state.geometry_node));

    editor_state.material_settings = E_CreateUniformBuffer(sizeof(vec4));
    E_SetBufferData(editor_state.material_settings, &editor_state.material_buffer, sizeof(vec4));

    E_MaterialInstanceWriteBuffer(editor_state.material_instance, 0, editor_state.material_settings, sizeof(vec4));
    E_MaterialInstanceWriteImage(editor_state.material_instance, 1, editor_state.albedo_texture->as.image);
    E_MaterialInstanceWriteImage(editor_state.material_instance, 2, editor_state.metallic_roughness_texture->as.image);

    editor_state.execute_info.drawables[0].mesh = editor_state.mesh->as.mesh;
    editor_state.execute_info.drawables[0].material_instance = editor_state.material_instance;
    editor_state.execute_info.drawable_count++;
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

        editor_state.execute_info.width = viewport_panel.viewport_size[0];
        editor_state.execute_info.height = viewport_panel.viewport_size[0];

        E_ResizeRenderGraph(editor_state.graph, &editor_state.execute_info);
        EditorCameraResize(&editor_state.camera, viewport_panel.viewport_size[0], viewport_panel.viewport_size[1]);
    }
}

void EditorBeginRender()
{
    f64 start = EditorBeginProfiling();

    E_RendererBegin();

    editor_state.perf.begin_render = EditorEndProfiling(start);
}

void EditorEndRender()
{
    f64 start = EditorBeginProfiling();

    E_RendererEnd();
    
    editor_state.perf.end_render = EditorEndProfiling(start);
}

void EditorDraw()
{
    f64 start = EditorBeginProfiling();

    E_SetBufferData(editor_state.material_settings, &editor_state.material_buffer, sizeof(vec4));
    
    glm_mat4_copy(editor_state.camera.view, editor_state.execute_info.view);
    glm_mat4_copy(editor_state.camera.proj, editor_state.execute_info.projection);
    E_ExecuteRenderGraph(editor_state.graph, &editor_state.execute_info);

    editor_state.perf.execute_render_graph = EditorEndProfiling(start);
}

void EditorDrawGUI()
{
    f64 start = EditorBeginProfiling();

    EditorCreateDockspace();

    E_LogDraw();
    DrawViewportPanel(editor_state.final_blit_node->outputs[0], &editor_state.is_viewport_focused);

    // Material Viewer
    {
        igBegin("Material Viewer", NULL, ImGuiWindowFlags_AlwaysAutoResize);
        igCheckbox("Enable Albedo", &editor_state.material_buffer[0]);
        igCheckbox("Enable Metallic Roughness", &editor_state.material_buffer[1]);
        igCheckbox("Enable Blending", &editor_state.material_buffer[2]);
        igCheckbox("Flip UVs", &editor_state.material_buffer[3]);
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
            igText("Triangle: %d", editor_state.mesh->as.mesh->total_tri_count);
            igText("Vertices: %d", editor_state.mesh->as.mesh->total_vertex_count);
            igText("Indices: %d", editor_state.mesh->as.mesh->total_index_count);
            igTreePop();
        }
        igEnd();
    }

    // Render Graph Viewer
    {
        igBegin("Render Graph Viewer", NULL, ImGuiWindowFlags_None);
        GeometryNodeDrawGUI(editor_state.geometry_node);
        HDRNodeDrawGUI(editor_state.hdr_node);
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
    if (editor_state.is_viewport_focused)
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
