#include "GLApplication.h"

// Every app needs some globals. No need to get fancy. 
// Bite the bullet and call them what they are.
OWENGINE_API GlobalSettings* globals = nullptr;

#include <algorithm>

#include "../Helpers/ErrorHandling.h"
#include "../Helpers/Logger.h"
#include "../Helpers/LogStream.h"

#include "SaveAndRestore.h"
#include "GlobalSettings.h"
#include "Movie.h"
#include "Camera.h"

#pragma comment( lib, "glfw3.lib" )
#pragma comment (lib, "OpenGL32.lib")
#pragma comment (lib, "freetype.lib")

static GLApplication* hackForErrorCallback = nullptr;
void GLAPIENTRY error_callback(int error, const char* description);
void GLAPIENTRY debugMessageCallback(GLenum source, GLenum type, GLuint id,
						GLenum severity, GLsizei length, const GLchar *message, 
						const void *userParam);

GLApplication::GLApplication(UserInput* ui)
	:mUserInput(ui)
{
}

GLApplication::~GLApplication()
{
	if (hackForErrorCallback == this)
		hackForErrorCallback = nullptr;
}

void GLApplication::init(Movie* movie, UserInput* ui, MacroRecorder* recorder, 
						SaveAndRestore* saveRestore, Camera* camera)
{
	globals->mPhysicalWindowSize = saveRestore->physicalWindowSize();
	if (glfwInit())
	{
		// https://antongerdelan.net/opengl/glcontext2.html
		// You'll find a list of all the key codes and other input handling commands 
		// at http://www.glfw.org/docs/latest/group__input.html. 
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

		glfwWindowHint(GLFW_SAMPLES, 4); // anti- aliasing. 16 for really high

#ifdef SECOND_MONITOR_WINDOW
		int count;
		GLFWmonitor** mons = glfwGetMonitors(&count);
		GLFWmonitor* mon = mons[1];
		const GLFWvidmode* mode = glfwGetVideoMode(mon);

		mWindow = glfwCreateWindow(mScrWidth, mScrHeight, windowTitle, mon, NULL);
		glfwSetWindowMonitor(window, NULL, -1000, 200, mScrWidth, mScrHeight, mode->refreshRate);
#else
		mWindow = glfwCreateWindow(globals->mPhysicalWindowSize.x,
			globals->mPhysicalWindowSize.y,
						movie->windowTitle().c_str(), NULL, NULL);
		if (!mWindow)
		{
			throw NMSException("Could not create OpenGL window.");
		}
		glfwSetWindowMonitor(mWindow, NULL, 2000, 200, 
			globals->mPhysicalWindowSize.x,
			globals->mPhysicalWindowSize.y, GLFW_DONT_CARE);
#endif
		glfwMakeContextCurrent(mWindow);
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
			throw NMSException("gladLoadGLLoader failed.");
		//glEnable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// tell GL to only draw onto a pixel if the shape is closer to the viewer
		glEnable(GL_DEPTH_TEST); // enable depth-testing
		glDepthFunc(GL_LEQUAL); // depth-testing interprets a smaller value as "closer"

		// mFrameBufferSize and mWindowSize may differ. See:
		// (https://stackoverflow.com/questions/45796287/screen-coordinates-to-world-coordinates)

		globals->mLogger->log_gl_params();
		enableCallbacks();
		try
		{
			movie->init(this, ui, recorder);
			mUserInput->init(this);
			// We set the window size before the callbacks were hooked up.
			onFrameBufferResizeCallback(mWindow, 
				globals->mPhysicalWindowSize.x,
				globals->mPhysicalWindowSize.y);
		}

		catch (const std::exception& ex)
		{
			std::cout << ex.what() << " press any key\n";
			int ch;
			std::cin >> ch;
			exit(-1);
		}
	}
	else
	{
		throw NMSException("glfwInit() failed");
	}
	camera->bindResize(mUserInput);
}

void GLApplication::run(Movie* movie)
{
	movie->preRun();
	movie->run(mUserInput, mWindow);
	glfwDestroyWindow(mWindow);
	glfwTerminate();
}

void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity,
	GLsizei length, const char *message, const void *userParam)
{}

void GLApplication::enableCallbacks()
{
	hackForErrorCallback = this;
	glfwSetWindowUserPointer(mWindow, this);

	glfwSetErrorCallback([](int error, const char* description) {
		error_callback(error, description);
	});

	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		// https://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-ARB_debug_output.pdf
		// https://learnopengl.com/In-Practice/Debugging
		// https://www.khronos.org/opengl/wiki/Debug_Output
		// initialize debug output 
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
//		GLuint ignore = { 131185 }; // NVidia telling us that a buffer was successfully created.
//		int count = sizeof(ignore) / sizeof(GLuint);
		glDebugMessageCallback(debugMessageCallback, nullptr);
		// Does not seem to be working
//		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
//				GL_DONT_CARE, count, &ignore, GL_FALSE);
	}
	
	glfwSetWindowCloseCallback(mWindow, [](GLFWwindow* window) {
		auto pointer = reinterpret_cast<GLApplication*>(glfwGetWindowUserPointer(window));
		pointer->onCloseCallback(window);
	});

	glfwSetFramebufferSizeCallback(mWindow, [](GLFWwindow* window, int width, int height) {
		auto pointer = reinterpret_cast<GLApplication*>(glfwGetWindowUserPointer(window));
		pointer->onFrameBufferResizeCallback(window, width, height);
	});

	glfwSetKeyCallback(mWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		auto pointer = reinterpret_cast<GLApplication*>(glfwGetWindowUserPointer(window));
		pointer->onKeyPressCallback(window, key, scancode, action, mods);
	});

	glfwSetCharCallback(mWindow, [](GLFWwindow* window, unsigned int scancode) {
		auto pointer = reinterpret_cast<GLApplication*>(glfwGetWindowUserPointer(window));
		pointer->onCharCallback(window, scancode);
	});

	glfwSetMouseButtonCallback(mWindow, [](GLFWwindow* window, int button, int action, int mods) {
		auto pointer = reinterpret_cast<GLApplication*>(glfwGetWindowUserPointer(window));
		pointer->onPointingDeviceCallback(window, button, action, mods);
	});

	glfwSetCursorPosCallback(mWindow, [](GLFWwindow* window, double x, double y) {
		auto pointer = reinterpret_cast<GLApplication*>(glfwGetWindowUserPointer(window));
		pointer->onPointingDevicePositionCallback(window, x, y);
	});
}

void GLApplication::onFrameBufferResizeCallback(GLFWwindow* window, 
								int width, int height)
{
	if (width == 0 || height == 0)
	{
		// Also happens when minimised
		globals->minimised(true);
		return;
	}
	else
	{
		globals->minimised(false);
	}
	// The following is need for Mouse Clicks
	mFrameBuffer = glm::ivec2(width, height);
	int w, h;
	glfwGetWindowSize(window, &w, &h);
	mWindowSize.x = w;
	mWindowSize.y = h;

	glViewport(0, 0, width, height);
	globals->physicalWindowSize({ width, height });
	mUserInput->windowResize(window, glm::ivec2(width, height));
}

void GLApplication::onPointingDeviceCallback(GLFWwindow* window, 
										int button, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		// https://stackoverflow.com/questions/45796287/screen-coordinates-to-world-coordinates
		// https://stackoverflow.com/questions/11277501/how-to-recover-view-space-position-given-view-space-depth-value-and-ndc-xy?noredirect=1&lq=1
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		glm::vec2 screen_pos = glm::vec2(xpos, ypos);
		glm::vec2 pixel_pos
			= screen_pos * glm::vec2(mFrameBuffer.x, mFrameBuffer.y) /
			glm::vec2(mWindowSize.x, mWindowSize.y);
		// shift to GL's center convention
		pixel_pos = pixel_pos + glm::vec2(0.5f, 0.5f);

		glm::vec3 pos = glm::vec3(pixel_pos.x, mWindowSize.y - pixel_pos.y, 0.0f);

		glReadPixels(static_cast<GLint>(xpos), static_cast<GLint>(ypos), 1, 1,
			GL_DEPTH_COMPONENT, GL_FLOAT, &pos.z);

		mUserInput->pointingDevice(window, button, action, mods, pos);
	}
}

void GLApplication::onPointingDevicePositionCallback(GLFWwindow* window, double x, double y)
{
	globals->mPointingDevicePosition.x = static_cast<glm::vec2::value_type>(x);
	globals->mPointingDevicePosition.y = static_cast<glm::vec2::value_type>(y);
	mUserInput->cursorPosition(window, x, y);
}

void GLApplication::onCharCallback(GLFWwindow* OW_UNUSED(window), unsigned int codepoint)
{
	mUserInput->keyboard(codepoint, 0, 0, 0, 0);
}

void GLApplication::onKeyPressCallback(GLFWwindow* OW_UNUSED(window), 
								int key, int scancode, int action, int mods)
{
	mUserInput->keyboard(0, key, scancode, action, mods);
}

void GLApplication::onCloseCallback(GLFWwindow* window)
{
	mUserInput->closeWindow(window);
}

static std::string sourceAsString(GLenum source)
{
	std::string retval;
	switch (source)
	{
		case GL_DEBUG_SOURCE_API:             retval = "API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   retval = "Window System"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: retval = "Shader Compiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     retval = "Third Party"; break;
		case GL_DEBUG_SOURCE_APPLICATION:     retval = "Application"; break;
		case GL_DEBUG_SOURCE_OTHER:           retval = "Other"; break;
	}
	return retval;
}

static std::string typeAsString(GLenum type)
{
	std::string retval;
	switch (type)
	{
		case GL_DEBUG_TYPE_ERROR:               retval = "Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: retval = "Deprecated Behaviour"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  retval = "Undefined Behaviour"; break;
		case GL_DEBUG_TYPE_PORTABILITY:         retval = "Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:         retval = "Performance"; break;
		case GL_DEBUG_TYPE_MARKER:              retval = "Marker"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          retval = "Push Group"; break;
		case GL_DEBUG_TYPE_POP_GROUP:           retval = "Pop Group"; break;
		case GL_DEBUG_TYPE_OTHER:               retval = "Other"; break;
	}
	return retval;
}

static std::string severityAsString(GLenum severity)
{
	std::string retval;
	switch (severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:         retval = "Severity: high"; break;
		case GL_DEBUG_SEVERITY_MEDIUM:       retval = "Severity: medium"; break;
		case GL_DEBUG_SEVERITY_LOW:          retval = "Severity: low"; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: retval = "Severity: notification"; break;
	}
	return retval;
}

void GLApplication::onDebugMessageCallback(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length,
	const GLchar *message, const void *userParam)
{
	LogStream(LogStreamLevel::Warning) 
		<< "\nGLFW Debug Callback:\n"
		<< "Source [" << sourceAsString(source) << "]\n"
		<< "Type [" << typeAsString(type) << "]\n"
		<< "Id [" << id << "]\n"
		<< "Severity [" << severityAsString(severity) << "]\n"
		<< "Message: " << message << "\n";
}

void debugMessageCallback(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length, const GLchar *message,
	const void *userParam)
{
	if (id == 131185)
		return;
	if (hackForErrorCallback)
		hackForErrorCallback->onDebugMessageCallback(source, type, id,
			severity, length, message,
			userParam);
	else
	{
		LogStream(LogStreamLevel::Warning)
			<< "GLFW Debug Callback (hack not working):\n"
			<< "Source [" << sourceAsString(source) << "]\n"
			<< "Type [" << typeAsString(type) << "]\n"
			<< "Id [" << id << "]\n"
			<< "Severity [" << severityAsString(severity) << "]\n"
			<< "Message: " << message;
	}
}

void error_callback(int error, const char* description)
{
	// https://github.com/Polytonic/Glitter
	if (hackForErrorCallback)
		hackForErrorCallback->errorReporting(error, description);
	else
	{
		LogStream(LogStreamLevel::Warning) << "GLFW ERROR: code ["
			<< error << "] msg [" << description << "]";
	}
}

void GLApplication::errorReporting(int error, const char* description)
{
	LogStream(LogStreamLevel::Warning) << "GLFW ERROR: code [" << error
		<< "] msg [" << description << "]";
}

/*
Unused Callbacks

GLFWAPI GLFWerrorfun glfwSetErrorCallback(GLFWerrorfun callback);
GLFWAPI GLFWwindowposfun glfwSetWindowPosCallback(GLFWwindow* window, GLFWwindowposfun callback);
GLFWAPI GLFWwindowsizefun glfwSetWindowSizeCallback(GLFWwindow* window, GLFWwindowsizefun callback);

GLFWAPI GLFWwindowrefreshfun glfwSetWindowRefreshCallback(GLFWwindow* window, GLFWwindowrefreshfun callback);
GLFWAPI GLFWwindowfocusfun glfwSetWindowFocusCallback(GLFWwindow* window, GLFWwindowfocusfun callback);
GLFWAPI GLFWwindowiconifyfun glfwSetWindowIconifyCallback(GLFWwindow* window, GLFWwindowiconifyfun callback);
GLFWAPI GLFWwindowmaximizefun glfwSetWindowMaximizeCallback(GLFWwindow* window, GLFWwindowmaximizefun callback);

GLFWAPI GLFWcursorposfun glfwSetCursorPosCallback(GLFWwindow* window, GLFWcursorposfun callback);
GLFWAPI GLFWcursorenterfun glfwSetCursorEnterCallback(GLFWwindow* window, GLFWcursorenterfun callback);
GLFWAPI GLFWscrollfun glfwSetScrollCallback(GLFWwindow* window, GLFWscrollfun callback);


GLFWAPI GLFWdropfun glfwSetDropCallback(GLFWwindow* window, GLFWdropfun callback);
GLFWAPI GLFWmonitorfun glfwSetMonitorCallback(GLFWmonitorfun callback);
*/
