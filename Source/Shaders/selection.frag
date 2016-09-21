R"(

#version 410 core

uniform uint index;

layout(location = 0) out uint gIndex;

void main() {
  gIndex = index;
}

)"
