#include "WindowsWindow.h"

#include <Euphorbe/Core/Log.h>
#include <Euphorbe/Graphics/Renderer.h>

#include <cimgui.h>
#include <generator/cimgui_template.h>
#include <vulkan/vulkan.h>
#include <generator/output/cimgui_impl.h>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
        return 1;

    switch (msg)
    {
    case WM_CREATE:
    {
        LPCREATESTRUCT create_struct = (LPCREATESTRUCT)lparam;
        E_WindowsWindow* window = (E_WindowsWindow*)create_struct->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);

        break;
    }
    case WM_DESTROY:
    {
        E_WindowsWindow* window = (E_WindowsWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        window->is_open = 0;
        break;
    }

    case WM_SIZE:
    {
        i32 width = LOWORD(lparam);
        i32 height = HIWORD(lparam);
        E_WindowsWindow* window = (E_WindowsWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        *window->width_pointer = width;
        *window->height_pointer = height;

        if (window->resize_callback != NULL)
            window->resize_callback(width, height);

        break;
    }

    case WM_MOUSEWHEEL:
    {
        E_WindowsWindow* window = (E_WindowsWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        f32 scroll = (f32)GET_WHEEL_DELTA_WPARAM(wparam) / (f32)WHEEL_DELTA;

        if (window->scroll_callback != NULL)
            window->scroll_callback(scroll);

        break;
    }

    default:
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    return 0;
}

E_WindowsWindow* E_CreateWindowsWindow(i32* width, i32* height, const char* title, b32 dark_mode)
{
    E_WindowsWindow* result = malloc(sizeof(E_WindowsWindow));
    if (result)
    {
        result->width_pointer = width;
        result->height_pointer = height;
        result->resize_callback = NULL;

        WNDCLASSA window_class = { 0 };
        window_class.lpfnWndProc = WinProc;
        window_class.hInstance = GetModuleHandleA(NULL);
        window_class.lpszClassName = "EuphorbeWindowClass";

        RegisterClassA(&window_class);

        result->hwnd = CreateWindowA(window_class.lpszClassName,
            title,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            *width, *height,
            NULL, NULL, window_class.hInstance,
            result);

        if (!result->hwnd)
            E_LogError("Failed to create HWND!");

        result->is_open = 1;

        if (result->hwnd)
        {
            RECT rect;
            GetClientRect(result->hwnd, &rect);
            *width = rect.right - rect.left;
            *height = rect.bottom - rect.top;
        }

        igCreateContext(NULL);

        ImGuiIO* io = igGetIO();
        //io->FontDefault = ImFontAtlas_AddFontFromFileTTF(io->Fonts, "Assets/Fonts/Consolas.ttf", 16.0f, NULL, NULL);

        ImGuiStyle* style = igGetStyle();
        io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        if (dark_mode)
            igStyleColorsDark(NULL);
        else
            igStyleColorsLight(NULL);

        ImGui_ImplWin32_Init(result->hwnd);

        return result;
    }

    return NULL;
}

void E_LaunchWindowsWindow(E_WindowsWindow* window)
{
    ShowWindow(window->hwnd, SW_MAXIMIZE);

    RECT rect;
    GetClientRect(window->hwnd, &rect);
    *window->width_pointer = rect.right - rect.left;
    *window->height_pointer = rect.bottom - rect.top;
}

void E_FreeWindowsWindow(E_WindowsWindow* window)
{
    ImGui_ImplWin32_Shutdown();

    DestroyWindow(window->hwnd);
    free(window);
}

void E_UpdateWindowsWindow(E_WindowsWindow* window)
{
    MSG msg;
    while (PeekMessageA(&msg, window->hwnd, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}