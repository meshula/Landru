
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

	struct OnWindowResized : public OnEventEvaluator
	{
		explicit OnWindowResized(GLFWwindow* w, std::shared_ptr<Fiber> f, vector<Instruction> & statements)
			: OnEventEvaluator(f, statements)
			, window(w) {}

		GLFWwindow* window;

		static void window_resized(GLFWwindow* w, int width, int height);
	};

    std::vector<GLFWwindow*> sgWindows;
    std::vector<OnWindowClosed> sgOnWindowClosed;
	std::vector<OnWindowResized> sgOnWindowResized;

	struct PendingResize
	{
		GLFWwindow * window;
		float width, height;
	};
	std::vector<PendingResize> sgResizes;

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

	void OnWindowResized::window_resized(GLFWwindow* w, int width, int height)
	{
		for (auto & i : sgOnWindowResized) {
			if (i.window == w) {
				sgResizes.push_back({ w, (float) width, (float) height });
			}
		}
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
        glfwSwapInterval(0);
        sgWindows.emplace_back(window);
        run.self->push<GLFWwindow*>(window);
		return RunState::Continue;
    }

	RunState windowClosed(FnContext& run)
    {
        vector<Instruction> instr = run.self->back<vector<Instruction>>(-2);
		auto property = run.self->pop<GLFWwindow*>();
		run.self->popVar(); // drop the instr
		if (property)
			sgOnWindowClosed.emplace_back(OnWindowClosed(property, run.vm->fiberPtr(run.self), instr));

		return RunState::Continue;
    }

	RunState windowResized(FnContext& run)
	{
		vector<Instruction> instr = run.self->back<vector<Instruction>>(-2);
		auto property = run.self->pop<GLFWwindow*>();
		run.self->popVar(); // drop the instr

		if (property) {
			glfwSetWindowSizeCallback(property, OnWindowResized::window_resized);
			sgOnWindowResized.emplace_back(OnWindowResized(property, run.vm->fiberPtr(run.self), instr));
		}

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
    glVtable->registerFn("1.0", "windowClosed", "o", "", windowClosed);
	glVtable->registerFn("1.0", "windowResized", "o", "ff", windowResized);
	gl_lib.registerVtable(move(glVtable));

    gl_lib.registerFactory("context", []()->std::shared_ptr<Wires::TypedData>
    {
        return std::make_shared<Wires::Data<GLContext>>();
    });

    gl_lib.registerFactory("window", []()->std::shared_ptr<Wires::TypedData>
    {
        return std::make_shared<Wires::Data<GLFWwindow*>>(nullptr);
    });

    lib->libraries.emplace_back(std::move(gl_lib));
}

extern "C"
LANDRUGL_API
RunState landru_gl_update(double now, VMContext* vm)
{
    bool cullWindows;
    do {
		cullWindows = false;
        for (auto i = sgWindows.begin(); i != sgWindows.end(); ++i) {
            if (glfwWindowShouldClose(*i)) {
                for (auto j = sgOnWindowClosed.begin(); j != sgOnWindowClosed.end(); ++j) {
					if (j->window == *i) {
						FnContext fn(vm, j->fiber(), nullptr);
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

	for (auto i : sgResizes) {
		for (auto & j : sgOnWindowResized) {
			if (j.window == i.window) {
				FnContext fn(vm, j.fiber(), nullptr);
				j.fiber()->push<float>(i.height);
				j.fiber()->push<float>(i.width);
				auto & instr = j.instructions();
				fn.run(instr);
			}
		}
	}
	sgResizes.clear();

    for (auto w : sgWindows) {
        /// @TODO only render and swap buffers if the window needs redrawing
        /// @TODO render here
        glfwSwapBuffers(w);
    }
    glfwPollEvents();

	return RunState::Continue;
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
    else
	{
		for (std::vector<OnWindowClosed>::iterator i = sgOnWindowClosed.begin(); i != sgOnWindowClosed.end(); ++i) {
            if (i->fiber() == f) {
                i = sgOnWindowClosed.erase(i);
                if (i == sgOnWindowClosed.end())
                    break;
            }
        }
		for (std::vector<OnWindowResized>::iterator i = sgOnWindowResized.begin(); i != sgOnWindowResized.end(); ++i) {
			if (i->fiber() == f) {
				i = sgOnWindowResized.erase(i);
				if (i == sgOnWindowResized.end())
					break;
			}
		}
	}
}

extern "C"
LANDRUGL_API
bool landru_gl_pendingContinuations(Fiber * f)
{
	return sgOnWindowClosed.size() > 0 || sgOnWindowResized.size() > 0;
}
