#version 330 core
layout(location = 0) in vec2 aPos; 
uniform float scaleY;              

void main()
{
    vec2 position = aPos;
    if (aPos.y > 0.0)  
    {
        position.y *= scaleY; 
    }
    gl_Position = vec4(position, 0.0, 1.0);
}