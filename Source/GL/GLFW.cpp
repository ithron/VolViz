#include "GLFW.h"

#include "Error.h"

#include <algorithm>
#include <cassert>

namespace VolViz {
namespace Private_ {
namespace GL {

namespace {
std::vector<std::string> queryExtensions() {
  std::vector<std::string> extensions;

  GLint nExt = 0;
  glGetIntegerv(GL_NUM_EXTENSIONS, &nExt);
  assertGL("Failed to query number of available extenions");

  for (auto i = 0; i < nExt; ++i) {
    auto const ext = reinterpret_cast<char const *>(
        glGetStringi(GL_EXTENSIONS, static_cast<GLuint>(i)));
    assert(ext && "Failed to query extension");
    extensions.emplace_back(ext);
  }

  return extensions;
}
} // namespace

GLFW::GLFW(std::string title, std::size_t width, std::size_t height) {
  if (!glfwInit()) throw(std::runtime_error("Failed to init GLFW"));

  glfwWindowHint(GLFW_VISIBLE, false);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
#if defined(__APPLE__)
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#else
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#endif
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  if (!(window =
            glfwCreateWindow(static_cast<int>(width), static_cast<int>(height),
                             title.c_str(), nullptr, nullptr))) {
    throw std::runtime_error("Failed to create GLFW window");
  }

  glfwSetWindowUserPointer(window, this);

  // setup key callback handler
  glfwSetKeyCallback(
      window, [](GLFWwindow *win, int key, int scancode, int action, int mode) {
        auto ptr = glfwGetWindowUserPointer(win);
        assert(ptr != nullptr && "Invalid user pointer");
        auto *const self = static_cast<GLFW *>(ptr);
        if (self->keyInputHandler)
          self->keyInputHandler(key, scancode, action, mode);
      });
  // setup window resize callback
  glfwSetWindowSizeCallback(window, [](GLFWwindow *win, int w, int h) {
    auto ptr = glfwGetWindowUserPointer(win);
    assert(ptr != nullptr && "Invalid user pointer");
    auto *const self = static_cast<GLFW *>(ptr);
    if (self->windowResizeCallback)
      self->windowResizeCallback(static_cast<std::size_t>(w),
                                 static_cast<std::size_t>(h));
  });

  // setup scroll wheel input
  glfwSetScrollCallback(window, [](GLFWwindow *win, double x, double y) {
    auto ptr = glfwGetWindowUserPointer(win);
    assert(ptr != nullptr && "Invalid user pointer");
    auto *const self = static_cast<GLFW *>(ptr);
    if (self->scrollWheelInputHandler) self->scrollWheelInputHandler(x, y);
  });

  // setup mouse button callback
  glfwSetMouseButtonCallback(window, [](GLFWwindow *win, int b, int a, int m) {
    auto ptr = glfwGetWindowUserPointer(win);
    assert(ptr != nullptr && "Invalid user pointer");
    auto *const self = static_cast<GLFW *>(ptr);
    if (self->mouseButtonCallback) self->mouseButtonCallback(b, a, m);
  });

  // setup mouse move callback
  glfwSetCursorPosCallback(window, [](GLFWwindow *win, double x, double y) {
    auto ptr = glfwGetWindowUserPointer(win);
    assert(ptr != nullptr && "Invalid user pointer");
    auto *const self = static_cast<GLFW *>(ptr);
    if (self->mouseMoveCallback) self->mouseMoveCallback(x, y);
  });

  makeCurrent();
  
  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    throw std::runtime_error("Failed to load OpenGL functions");
  }
  
  supportedExtensions_ = queryExtensions();
  glfwSwapInterval(1);
}

GLFW::GLFW(GLFW &&rhs) : window(rhs.window) { rhs.window = nullptr; }

GLFW::~GLFW() {
  if (window) glfwDestroyWindow(window);
  glfwTerminate();
}

void GLFW::hide() noexcept { glfwHideWindow(window); }

void GLFW::show() noexcept { glfwShowWindow(window); }

bool GLFW::isHidden() const noexcept {
  return glfwGetWindowAttrib(window, GLFW_VISIBLE) == 0;
}

void GLFW::makeCurrent() noexcept { glfwMakeContextCurrent(window); }

void GLFW::detachContext() noexcept { glfwMakeContextCurrent(nullptr); }

std::size_t GLFW::width() const noexcept {
  int w, h;
  glfwGetWindowSize(window, &w, &h);
  if (!isHidden()) return static_cast<std::size_t>(w);
  return VOLVIZ_DEFAULT_WINDOW_WIDTH;
}

std::size_t GLFW::height() const noexcept {
  int w, h;
  glfwGetWindowSize(window, &w, &h);
  if (!isHidden()) return static_cast<std::size_t>(h);
  return VOLVIZ_DEFAULT_WINDOW_HEIGHT;
}

bool GLFW::supportsExtension(std::string name) const noexcept {
  return std::find(supportedExtensions_.begin(), supportedExtensions_.end(),
                   name) != supportedExtensions_.end();
}
} // namespace GL
} // namespace Private_
} // namespace VolViz
