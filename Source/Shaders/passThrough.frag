R"(

#version 430 core

in vec4 colorIn;

layout(location = 0) out vec4 color;

void main() {
  color = colorIn;
}

)"
