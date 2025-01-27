#pragma once

#include <Euphorbe/Core/Log.h>
#include <Euphorbe/Platform/Window.h>
#include <cglm/cglm.h>

#include "Image.h"
#include "Material.h"
#include "Buffer.h"

typedef struct E_CommandBuffer E_CommandBuffer;

typedef struct E_RendererInitSettings E_RendererInitSettings;
struct E_RendererInitSettings
{
    b32 log_found_layers; // Default: true
    b32 log_renderer_events; // Default: true
    b32 enable_debug; // Default: false
    b32 gui_should_clear; // If viewport is in a gui window
    b32 enable_vsync; // Enable vsync
};

typedef struct E_ClearValue E_ClearValue;
struct E_ClearValue
{
    f32 r;
    f32 g;
    f32 b;
    f32 a;
    f32 depth;
    u32 stencil;
};

typedef struct E_ImageAttachment E_ImageAttachment;
struct E_ImageAttachment
{
    E_Image* image;
    E_ImageLayout layout;
    E_ClearValue clear_value;
};

// Light info
typedef struct E_PointLight E_PointLight;
struct E_PointLight
{
    vec4 position;
    vec4 color;
};

// Renderer stats 
typedef struct E_RendererStats E_RendererStats;
struct E_RendererStats
{
    u32 total_vertex_count;
    u32 total_index_count;
    u32 total_triangle_count;
    u32 total_meshlet_count;
    u32 total_draw_calls;
};

extern E_RendererStats E_CurrentRendererStatistics;

void E_RendererInit(E_Window* window, E_RendererInitSettings settings);
void E_RendererShutdown();

void E_RendererBegin();
void E_RendererEnd();
void E_RendererPresent();
void E_RendererWait();

void E_RendererDrawMemoryUsageGUI();
void E_RendererDrawRendererStats();
void E_RendererDrawGraphicsCardInfo();

E_Image* E_GetSwapchainImage();
u32 E_GetSwapchainImageIndex();
E_CommandBuffer* E_GetSwapchainCommandBuffer();

//
void E_BeginGUI();
void E_EndGUI();

void E_RendererResize(i32 width, i32 height);