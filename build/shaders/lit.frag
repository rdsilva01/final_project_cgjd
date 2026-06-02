#version 410 core
in vec3 FragPos;
in vec3 Normal;
out vec4 FragColor;

uniform vec3  solidColor;
uniform vec3  lightPos;
uniform vec3  viewPos;
uniform float alpha;     // 1.0 opaque, <1.0 for translucent overlap view

void main() {
    // Two-sided shading: a face seen from behind (e.g. through a translucent
    // hull) should still be lit, so use abs(dot) for the diffuse term.
    vec3 N = normalize(Normal);
    vec3 L = normalize(lightPos - FragPos);
    float diff = abs(dot(N, L));

    vec3 V = normalize(viewPos - FragPos);
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), 32.0) * 0.25;

    float ambient = 0.30;
    vec3 color = solidColor * (ambient + 0.70 * diff) + vec3(spec);
    FragColor = vec4(color, alpha);
}
