#include "glfw_app.hpp"
#include "util.hpp"
#include "geometric.hpp"

using namespace avl;

static InputEvent generate_input_event(GLFWwindow * window, InputEvent::Type type, const float2 & cursor, int action)
{
    InputEvent e;
    e.window = window;
    e.type = type;
    e.cursor = cursor;
    e.action = action;
    e.mods = 0;
    
    glfwGetWindowSize(window, &e.windowSize.x, &e.windowSize.y);

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) | glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT)) e.mods |= GLFW_MOD_SHIFT;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) | glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL)) e.mods |= GLFW_MOD_CONTROL;
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) | glfwGetKey(window, GLFW_KEY_RIGHT_ALT)) e.mods |= GLFW_MOD_ALT;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SUPER) | glfwGetKey(window, GLFW_KEY_RIGHT_SUPER)) e.mods |= GLFW_MOD_SUPER;

    return e;
}

static void s_error_callback(int error, const char * description)
{
    std::cerr << description << std::endl;
}

#define CATCH_CURRENT app->exceptions.push_back(std::current_exception())

GLFWApp::GLFWApp(int width, int height, const std::string title, int glfwSamples, bool usingImgui)
{
    if (!glfwInit())
        ::exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    glfwWindowHint(GLFW_SAMPLES, glfwSamples);

    glfwWindowHint(GLFW_SRGB_CAPABLE, true);
    
#if defined(ANVIL_PLATFORM_OSX)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
    
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 0);
    
    glfwSetErrorCallback(s_error_callback);
    
    window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);

    if (!window) 
    {
        ANVIL_ERROR("Failed to open GLFW window");
        glfwTerminate();
        ::exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    
    ANVIL_INFO("GL_VERSION =  " << (char *)glGetString(GL_VERSION));
    ANVIL_INFO("GL_VENDOR =   " << (char *)glGetString(GL_VENDOR));
    ANVIL_INFO("GL_RENDERER = " << (char *)glGetString(GL_RENDERER));
   
#if defined(ANVIL_PLATFORM_WINDOWS)
    glewExperimental = GL_TRUE;
    if (GLenum err = glewInit()) 
       throw std::runtime_error(std::string("glewInit() failed - ") + (const char *)glewGetErrorString(err));
    ANVIL_INFO("GLEW_VERSION = " << (char *)glewGetString(GLEW_VERSION));
#endif
    
    glfwSetWindowUserPointer(window, this);

    glfwSetWindowRefreshCallback(window, [](GLFWwindow * window)
    {
        //auto app = (GLFWApp *)(glfwGetWindowUserPointer(window)); try { app->on_draw();} catch(...) { CATCH_CURRENT; }
    });
    
    glfwSetWindowFocusCallback  (window, [](GLFWwindow * window, int focused)
    {
        auto app = (GLFWApp *)(glfwGetWindowUserPointer(window)); try { app->on_window_focus(!!focused);} catch(...) { CATCH_CURRENT; }
    });
    
    glfwSetWindowSizeCallback (window, [](GLFWwindow * window, int width, int height)
    {
        auto app = (GLFWApp *)(glfwGetWindowUserPointer(window)); try { app->on_window_resize({width, height});} catch(...) { CATCH_CURRENT; }
    });
    
    glfwSetCharCallback (window, [](GLFWwindow * window, unsigned int codepoint)
    {
        auto app = (GLFWApp *)(glfwGetWindowUserPointer(window)); try { app->consume_character(codepoint);} catch(...) { CATCH_CURRENT; }
    });
    
    glfwSetKeyCallback (window, [](GLFWwindow * window, int key, int, int action, int)
    {
        auto app = (GLFWApp *)(glfwGetWindowUserPointer(window)); try { app->consume_key(key, action);} catch(...) { CATCH_CURRENT; }
    });
    
    glfwSetMouseButtonCallback (window, [](GLFWwindow * window, int button, int action, int)
    {
        auto app = (GLFWApp *)(glfwGetWindowUserPointer(window)); try { app->consume_mousebtn(button, action);} catch(...) { CATCH_CURRENT; }
    });
    
    glfwSetCursorPosCallback (window, [](GLFWwindow * window, double xpos, double ypos)
    {
        auto app = (GLFWApp *)(glfwGetWindowUserPointer(window)); try { app->consume_cursor(xpos, ypos);} catch(...) { CATCH_CURRENT; }
    });
    
    glfwSetScrollCallback (window, [](GLFWwindow * window, double deltaX, double deltaY)
    {
        auto app = (GLFWApp *)(glfwGetWindowUserPointer(window)); try { app->consume_scroll(deltaX, deltaY);} catch(...) { CATCH_CURRENT; }
    });
    
	/*    
    glfwSetDropCallback (window, [](GLFWwindow * window, int count, const char * names[])
    {
        auto app = (GLFWApp *)(glfwGetWindowUserPointer(window)); try { app->on_drop({names, names+count});} catch(...) { CATCH_CURRENT; }
    });
	*/
}

GLFWApp::~GLFWApp() 
{
    if (window) glfwDestroyWindow(window);
}

void GLFWApp::preprocess_input(InputEvent & event)
{
    if (event.type == InputEvent::MOUSE)
    {
        if (event.is_mouse_down()) isDragging = true;
        else if (event.is_mouse_up()) isDragging = false;
    }
    event.drag = isDragging;
    on_input(event);
}

void GLFWApp::consume_character(uint32_t codepoint)
{
    auto e = generate_input_event(window, InputEvent::CHAR, get_cursor_position(), 0);
    e.value[0] = codepoint;
    preprocess_input(e);
}

void GLFWApp::consume_key(int key, int action)
{
    auto e = generate_input_event(window, InputEvent::KEY, get_cursor_position(), action);
    e.value[0] = key;
    preprocess_input(e);
}

void GLFWApp::consume_mousebtn(int button, int action)
{
    auto e = generate_input_event(window, InputEvent::MOUSE, get_cursor_position(), action);
    e.value[0] = button;
    preprocess_input(e);
}

void GLFWApp::consume_cursor(double xpos, double ypos)
{
    auto e = generate_input_event(window, InputEvent::CURSOR, {(float)xpos, (float)ypos}, 0);
    preprocess_input(e);
}

void GLFWApp::consume_scroll(double deltaX, double deltaY)
{
    auto e = generate_input_event(window, InputEvent::SCROLL, get_cursor_position(), 0);
    e.value[0] = (float) deltaX;
    e.value[1] = (float) deltaY;
    preprocess_input(e);
}

void GLFWApp::main_loop() 
{
    auto t0 = std::chrono::high_resolution_clock::now();
    
    while (!glfwWindowShouldClose(window)) 
    {
        glfwPollEvents();
        
        for (auto & e : exceptions) 
            on_uncaught_exception(e);

        try
        {
            auto t1 = std::chrono::high_resolution_clock::now();
            auto timestep = std::chrono::duration<float>(t1 - t0).count();
            t0 = t1;
            
            elapsedFrames++;
            fpsTime += timestep;

            if (fpsTime > 0.5f)
            {
                fps = elapsedFrames / fpsTime;
                elapsedFrames = 0;
                fpsTime = 0;
            }

            UpdateEvent e;
            e.elapsed_s = glfwGetTime();
            e.timestep_ms = timestep;
            e.framesPerSecond = fps;
            e.elapsedFrames = elapsedFrames;

            on_update(e);
            on_draw();
        }
        catch(...)
        {
            on_uncaught_exception(std::current_exception());
        }
    }
}

void GLFWApp::on_uncaught_exception(std::exception_ptr e)
{
    rethrow_exception(e);
}

float2 GLFWApp::get_cursor_position() const
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    return {(float)xpos, (float)ypos};
}

void GLFWApp::exit()
{
    glfwSetWindowShouldClose(window, 1);
}

namespace avl
{
    int main(int argc, char * argv[]) 
    {
        int returnCode = EXIT_FAILURE;
        try
        {
            if (!glfwInit()) 
                throw std::runtime_error("glfwInit() failed.");
            returnCode = main(argc, argv);
        }
        catch (const std::exception & e) 
        { 
            ANVIL_ERROR("[Fatal] Caught exception: \n" << e.what());
        }
        catch (...) 
        { 
            ANVIL_ERROR("[Fatal] Caught unknown exception.");
        }

        glfwTerminate();
        return returnCode;
    }
}

int get_current_monitor(GLFWwindow * window)
{
    int numberOfMonitors;
    GLFWmonitor** monitors = glfwGetMonitors(&numberOfMonitors);
    
    int xW, yW;
    glfwGetWindowPos(window, &xW, &yW);
    
    for (int iC = 0; iC < numberOfMonitors; iC++)
    {
        int xM, yM;
        glfwGetMonitorPos(monitors[iC], &xM, &yM);
        const GLFWvidmode * desktopMode = glfwGetVideoMode(monitors[iC]);
        Bounds2D monitorRect {(float) xM, (float) yM, (float)xM + desktopMode->width, (float)yM + desktopMode->height};
        if (monitorRect.contains((float) xW, (float) yW))
            return iC;
    }
    return 0;
}

int2 get_screen_size(GLFWwindow * window)
{
    int numMonitors;
    GLFWmonitor** monitors = glfwGetMonitors(&numMonitors);
    if (numMonitors > 0)
    {
        int currentMonitor = get_current_monitor(window);
        const GLFWvidmode * desktopMode = glfwGetVideoMode(monitors[currentMonitor]);
        if (desktopMode)
            return {desktopMode->width, desktopMode->height};
        else
            return {0, 0};
    }
    return {0, 0};
}

#if defined (ANVIL_PLATFORM_WINDOWS)

#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>
#include <shellapi.h>

namespace avl
{
    std::string windows_to_utf8(const std::wstring & string)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conversion;
        return conversion.to_bytes(string);
    }

    std::wstring utf8_to_windows(const std::string & string)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conversion;
        return conversion.from_bytes(string);
    }
}

void GLFWApp::enter_fullscreen(GLFWwindow * window, int2 & windowedSize, int2 & windowedPos)
{
    glfwGetWindowSize(window, &windowedSize.x, &windowedSize.y);
    glfwGetWindowPos(window, &windowedPos.x, &windowedPos.y);

    HWND hwnd = glfwGetWin32Window(window);
    SetWindowLong(hwnd, GWL_EXSTYLE, 0);
    SetWindowLong(hwnd, GWL_STYLE, WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

    int32_t newScreenWidth = get_screen_size(window).x;
    int32_t newScreenHeight = get_screen_size(window).y;

    int monitorCount;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    int currentMonitor = get_current_monitor(window);
    int xpos, ypos;
    glfwGetMonitorPos(monitors[currentMonitor], &xpos, &ypos);

    SetWindowPos(hwnd, HWND_TOPMOST, xpos, ypos, (int) newScreenWidth, (int) newScreenHeight, SWP_SHOWWINDOW);
}

void GLFWApp::exit_fullscreen(GLFWwindow * window, const int2 & windowedSize, const int2 & windowedPos)
{
    HWND hwnd = glfwGetWin32Window(window);
    DWORD EX_STYLE = WS_EX_OVERLAPPEDWINDOW;
    DWORD STYLE = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_SIZEBOX;
        
    ChangeDisplaySettings(0, 0);
    SetWindowLong(hwnd, GWL_EXSTYLE, EX_STYLE);
    SetWindowLong(hwnd, GWL_STYLE, STYLE);
    SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

    // glfw/os bug: window shrinks by 4 pixels in x and y
    glfwSetWindowPos(window, windowedPos.x - 2, windowedPos.y - 2);
    glfwSetWindowSize(window, windowedSize.x + 4, windowedSize.y + 4);
}

#elif defined(ANVIL_PLATFORM_OSX)

#define GLFW_EXPOSE_NATIVE_COCOA
#define GLFW_EXPOSE_NATIVE_NSGL
#include "GLFW/glfw3native.h"

void GLFWApp::enter_fullscreen(GLFWwindow * window, int2 & windowedSize, int2 & windowedPos)
{
    glfwGetWindowSize(window, &windowedSize.x, &windowedSize.y);
    glfwGetWindowPos(window, &windowedPos.x, &windowedPos.y);
    
    [NSApp setPresentationOptions:NSApplicationPresentationHideMenuBar | NSApplicationPresentationHideDock];
    NSWindow * cocoaWindow = glfwGetCocoaWindow(window);
    [cocoaWindow setStyleMask:NSBorderlessWindowMask];
    
    int32_t newScreenWidth = get_screen_size(window).x;
    int32_t newScreenHeight = get_screen_size(window).y;
    
    int monitorCount;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    int currentMonitor = get_current_monitor(window);
    int xpos, ypos;
    glfwGetMonitorPos(monitors[currentMonitor], &xpos, &ypos);
    
    glfwSetWindowSize(window, newScreenWidth, newScreenHeight);
    glfwSetWindowPos(window, windowedPos.x, windowedPos.y);

    [cocoaWindow makeFirstResponder:cocoaWindow.contentView];
}

void GLFWApp::exit_fullscreen(GLFWwindow * window, const int2 & windowedSize, const int2 & windowedPos)
{
    [NSApp setPresentationOptions:NSApplicationPresentationDefault];
    NSWindow * cocoaWindow = glfwGetCocoaWindow(window);
    [cocoaWindow setStyleMask:NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask];
    [cocoaWindow makeFirstResponder:cocoaWindow.contentView];
    glfwSetWindowPos(window, windowedPos.x, windowedPos.y);
    glfwSetWindowSize(window, windowedSize.x, windowedSize.y);
}

#endif // end platform specific