#ifndef VolViz_GLFW_h
#define VolViz_GLFW_h

#include "config.h"
#include "GL.h"

#include <functional>
#include <string>

namespace VolViz {
namespace GL {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
/// RAII wrapper for GLFW
class GLFW {
public:
  /// Crates a GLFW context associated with a hidden window
  inline GLFW()
      : GLFW("Volume Visualizer", VOLVIZ_DEFAULT_WINDOW_WIDTH,
             VOLVIZ_DEFAULT_WINDOW_HEIGHT) {}

  GLFW(std::string title, std::size_t width, std::size_t height);
  GLFW(GLFW const &) = delete;
  GLFW(GLFW &&rhs);

  ~GLFW();

  /// Hides the current window
  void hide() noexcept;
  /// Unhides the current window
  void show() noexcept;

  /// Returns true if the window is hidden
  bool isHidden() const noexcept;

  /// Makes the window's conext current
  void makeCurrent() noexcept;

  inline operator bool() const noexcept {
    return !glfwWindowShouldClose(window);
  }

  inline void swapBuffers() const noexcept { glfwSwapBuffers(window); }

  inline void waitEvents() const noexcept { glfwWaitEvents(); }

  /// Returns the width of the window
  std::size_t width() const noexcept;
  /// Returns the height of the window
  std::size_t height() const noexcept;

  /// keyboard input handler
  std::function<void(int, int, int, int)> keyInputHandler;
  /// window resize callback
  std::function<void(std::size_t, std::size_t)> windowResizeCallback;
  /// scroll wheel input handler
  std::function<void(double, double)> scrollWheelInputHandler;
  /// mouse button callback
  std::function<void(int, int, int)> mouseButtonCallback;
  /// mouse move callback
  std::function<void(double, double)> mouseMoveCallback;

private:
  GLFWwindow *window = nullptr;
};
#pragma clang diagnostic pop

} // namespace GL
} // namespace VolViz

#endif // VolViz_GLFW_h
