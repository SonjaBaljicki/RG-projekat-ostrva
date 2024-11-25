#version 330 core
layout(location = 0) in vec2 aPos;  // Ulazne koordinate
uniform float scaleY;               // Faktor skaliranja za gornji vertex

void main()
{
    // Provera da li je vertex "gornji" (Y == maksimalni Y)
    vec2 position = aPos;
    if (aPos.y > 0.0)  // Pretpostavljamo da je gornji vertex onaj sa pozitivnom Y koordinatom
    {
        position.y *= scaleY;  // Skaliraj samo Y koordinatu gornjeg vertex-a
    }
    gl_Position = vec4(position, 0.0, 1.0);  // Postavi novu poziciju
}
