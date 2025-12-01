#ifdef GL_ES
precision highp float;
#endif
uniform vec2 m_plane_center;
uniform float m_zoomCount;
uniform vec2 m_pixel_size;
uniform int MAX_ITER;
uniform float m_aspectRatio;
//uniform sampler2D paletteTexture;
uniform vec2 m_plane_size;
const float PI = 6.28318;

uniform vec3 offset;
uniform vec3 amp;
uniform vec3 freq;
uniform vec3 phase;



vec3 palette(in float t, in vec3 a, in vec3 b, in vec3 c, in vec3 d)
{
    // a + b * cos( 2 * PI * (c * t + d) )
    return a + b * cos( PI * (c * t * d) );
}
// this website generates cool color params https://dev.thi.ng/gradients/
//vec3 a = vec3(0.5, 0.5, 0.5);
//vec3 b = vec3(0.5, 0.5, 0.5);
//vec3 g = vec3(1.0, 1.0, 1.0);
//vec3 d = vec3(0.80, 0.90, 0.30);
//offset
vec3 a = vec3(0.5, 0.5, 0.5);
//Amplitude
vec3 b = vec3(0.5, 0.5, 0.5);
//frequency
vec3 g = vec3(1.0 , 1.0, 0.5);
//phase
vec3 d = vec3(0.80, 0.9, 0.30);


void main() {


    vec2 frag = vec2(gl_FragCoord.x, m_pixel_size.y - gl_FragCoord.y);

    float zoomFactor = pow(1.0, m_zoomCount);

    vec2 c = m_plane_center + (frag / m_pixel_size - 0.5) * m_plane_size;

    vec2 z = vec2(0.0);
    int i;
    for (i = 0; i < MAX_ITER; ++i) {
        float x = (z.x * z.x - z.y * z.y) + c.x;
        float y = (2.0 * z.x * z.y) + c.y;
        z = vec2(x,y);
        if (dot(z,z) > 4.0) break;
    }

//    if (i >= MAX_ITER - 1) {
//        gl_FragColor = vec4(vec3(0, 0, 0), 1.0);
//    } else {
        //int colorIndex = i - (i / 16) * 16;// remainder in [0, 15]


        //reduces pixilation
        float smooth_iter = float(i) - log2(log2(dot(z,z))) + 4.0;
        float t = float(i) / float(MAX_ITER);

        //vec3 color = texture2D(paletteTexture, vec2(t,0.5)).rgb;

        //vec3 color = palette(t,a,b,g,d);
        vec3 color = palette(t,offset,amp,freq,phase);

        //color = pow(color, vec3(1.0 / 2.2));
        // make coords at MAX_ITER black
        // better than if statement i < MAX_ITER returns 0.0 if equal returns 1.0;
        float isInside = step(float(MAX_ITER) - 0.5, float(i));
        // color * (1 - isInside) + black * isInside
        color = mix(color, vec3(0.0, 0.0, 0.0), isInside);


        gl_FragColor = vec4(color, 1.0);
   // }
}