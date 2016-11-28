
#include "LandruActorVM/Fiber.h"
#include "LandruActorVM/Library.h"
#include "LandruActorVM/VMContext.h"
#include <mutex>
#include <GLFW/glfw3.h>

using namespace std;
using namespace Landru;

namespace {

    std::vector<GLFWwindow*> sgWindows;
    std::vector<GLFWwindow*> sgOnWindowClosed;

    void error_callback(int error, const char* description)
    {
        fprintf(stderr, "Error: %s\n", description);
    }

    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        bool close = key == GLFW_KEY_ESCAPE || key == (GLFW_KEY_LEFT_ALT|GLFW_KEY_F4) || key == (GLFW_KEY_RIGHT_ALT|GLFW_KEY_F4);
        if (close && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        /// @TODO reroute key to landru as a string, like "alt-G" or "alt-g"
    }



    void initGL()
    {
        static std::once_flag once;
        std::call_once(once, []()
        {
            glfwSetErrorCallback(error_callback);
            glfwInit();
            sgWindows = std::vector<GLFWwindow*>();
        });
    }

    class GLContext
    {
    public:
    };

    void createWindow(FnContext& run)
    {
        string title = run.self->pop<string>();
        float height = run.self->pop<float>();
        float width = run.self->pop<float>();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        GLFWwindow* window = glfwCreateWindow((int) width, (int) height, title.c_str(), NULL, NULL);
        glfwSetKeyCallback(window, key_callback);
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
        sgWindows.emplace_back(window);
        run.self->push<Wires::Data<GLFWwindow*>>(window);
    }

    void windowClosed(FnContext& run)
    {
        GLFWwindow* window = run.self->pop<GLFWwindow*>();

        //record a few things like Fiber, and emplace_back them so that callback can work
		sgOnWindowClosed.emplace_back(window);

    }

} // anon

extern "C" void LandruGL_init(void* vl)
{
    initGL();

    Landru::Library * lib = reinterpret_cast<Landru::Library*>(vl);
    auto glVtable = unique_ptr<Library::Vtable>(new Library::Vtable("gl"));

    glVtable->registerFn("1.0", "createWindow", "ffs", "o", createWindow);
    glVtable->registerFn("1.0", "windowClosed", "o", "o", windowClosed);

    lib->registerVtable(move(glVtable));

    lib->registerFactory("gl.context", [](VMContext&)->std::shared_ptr<Wires::TypedData>
    {
        return std::make_shared<Wires::Data<GLContext>>();
    });
}

extern "C" void LandruGL_update(double now)
{
    bool cullWindows = false;
    do {
        for (auto i = sgWindows.begin(); i != sgWindows.end(); ++i) {
            if (glfwWindowShouldClose(*i)) {

                for (auto j = sgOnWindowClosed.begin(); j != sgOnWindowClosed.end(); ++j) {
					if (*j == *i) {
						//callback();
					}

                    j = sgOnWindowClosed.erase(j);
                }

                glfwDestroyWindow(*i);
                sgWindows.erase(i);
                cullWindows = true;
                break;
            }
        }
    } while (cullWindows);

    for (auto w : sgWindows) {
        /// @TODO only render and swap buffers if the window needs redrawing
        /// @TODO render here
        glfwSwapBuffers(w);
    }
    glfwPollEvents();
}

extern "C" void LandruGL_finish(void * vl)
{
    Landru::Library * lib = reinterpret_cast<Landru::Library*>(vl);
}

extern "C" void LandruGL_fiberExpiring(Fiber* f)
{
//    called when Fibers are destroyed so that pending items like onWindowsClosed can be removed
}
