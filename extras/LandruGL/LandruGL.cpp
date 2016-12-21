
#include "LandruActorVM/Fiber.h"
#include "LandruActorVM/Library.h"
#include "LandruActorVM/VMContext.h"
#include <mutex>
#include <GLFW/glfw3.h>

#define LANDRUGL_EXPORTS
#include "api.h"

using namespace std;
using namespace Landru;

namespace {

    struct OnWindowClosed : public OnEventEvaluator
    {
		explicit OnWindowClosed(GLFWwindow* w, std::shared_ptr<Fiber> f, vector<Instruction> & statements)
			: OnEventEvaluator(f, statements)
			, window(w) {}

        GLFWwindow* window;
    };

    std::vector<GLFWwindow*> sgWindows;
    std::vector<OnWindowClosed> sgOnWindowClosed;

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

	RunState createWindow(FnContext& run)
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
        run.self->push<GLFWwindow*>(window);
		return RunState::Continue;
    }

	RunState windowClosed(FnContext& run)
    {
        vector<Instruction> instr = run.self->back<vector<Instruction>>(-2);
		auto property = run.self->pop<GLFWwindow*>();
//		Wires::Data<GLFWwindow*>* windowData = reinterpret_cast<Wires::Data<GLFWwindow*>*>(property->data.get());
		run.self->popVar(); // drop the instr
//&&& onWindowClosed should be monitoring properties not values of properties
        //record a few things like Fiber, and emplace_back them so that callback can work
	//	GLFWwindow * w = windowData->value();
		if (property)
			sgOnWindowClosed.emplace_back(OnWindowClosed(property, run.vm->fiberPtr(run.self), instr));

		return RunState::Continue;
    }

} // anon



extern "C"
LANDRUGL_API
void landru_gl_init(void* vl)
{
    initGL();
    Landru::Library * lib = reinterpret_cast<Landru::Library*>(vl);
    Landru::Library gl_lib("gl");

    auto glVtable = unique_ptr<Library::Vtable>(new Library::Vtable("gl"));
    glVtable->registerFn("1.0", "createWindow", "ffs", "o", createWindow);
    glVtable->registerFn("1.0", "windowClosed", "o", "o", windowClosed);
    gl_lib.registerVtable(move(glVtable));

    gl_lib.registerFactory("context", [](VMContext&)->std::shared_ptr<Wires::TypedData>
    {
        return std::make_shared<Wires::Data<GLContext>>();
    });

    gl_lib.registerFactory("window", [](VMContext&)->std::shared_ptr<Wires::TypedData>
    {
        return std::make_shared<Wires::Data<GLFWwindow*>>(nullptr);
    });

    lib->libraries.emplace_back(std::move(gl_lib));
}

extern "C"
LANDRUGL_API
void landru_gl_update(double now, VMContext* vm)
{
    bool cullWindows = false;
    do {
        for (auto i = sgWindows.begin(); i != sgWindows.end(); ++i) {
            if (glfwWindowShouldClose(*i)) {
                for (auto j = sgOnWindowClosed.begin(); j != sgOnWindowClosed.end(); ++j) {
					if (j->window == *i) {
						FnContext fn(vm, j->fiber(), nullptr, nullptr);
						auto & instr = j->instructions();
						fn.run(instr);
					}

                    j = sgOnWindowClosed.erase(j);
					if (j == sgOnWindowClosed.end())
						break;
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

extern "C"
LANDRUGL_API
void landru_gl_finish(void* vl)
{
    Landru::Library * lib = reinterpret_cast<Landru::Library*>(vl);
}

extern "C"
LANDRUGL_API
void landru_gl_fiberExpiring(Fiber* f)
{
//    called when Fibers are destroyed so that pending items like onWindowsClosed can be removed
}

extern "C"
LANDRUGL_API
void landru_gl_clearContinuations(Fiber* f, int level)
{
	// called when continuations must be cleared, for example before executing a goto statement
    if (!f)
        sgOnWindowClosed.clear();
    else {
        for (std::vector<OnWindowClosed>::iterator i = sgOnWindowClosed.begin(); i != sgOnWindowClosed.end(); ++i) {
            if (i->fiber() == f) {
                i = sgOnWindowClosed.erase(i);
                if (i == sgOnWindowClosed.end())
                    break;
            }
        }
    }
}

extern "C"
LANDRUGL_API
bool landru_gl_pendingContinuations(Fiber * f)
{
    return sgOnWindowClosed.size() > 0;
}
