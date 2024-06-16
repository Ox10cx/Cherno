#shader vertex
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 aUV;

out vec2 uv;

void main()
{
   gl_Position = position;
   uv = aUV;
}

#shader fragment
#version 330 core

uniform sampler2D sampler;

in vec2 uv;
out vec4 color;

void main()
{
   color = texture(sampler, uv);
}
