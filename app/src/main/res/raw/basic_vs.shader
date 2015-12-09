uniform mat4 mmat, vmat;
uniform mat4 tinv_mmat;
uniform mat4 projmat;
uniform vec3 lum_pos;

attribute vec3 vPosition;
attribute vec2 vTexture;
attribute vec3 vNormal;

//varying float intensite;
varying vec2 out_texCoord;
varying vec3 out_vNormal;
varying vec3 out_vLight;

//lumière speculaire
varying vec3 out_vView;

float myClamp(float v, float min, float max) {
    if(v > max) return max;
    if(v < min) return min;
    return v;
}

void main() {
//    vec4 nnorm = tinv_mmat * vec4(vNormal, 0.0);
//    vec3 L = normalize(mpos.xyz - lum_pos);
//    intensite = myClamp(dot(vec4(-L, 1), nnorm), 0.0, 1.0);
//    gl_Position = projmat * vmat * mmat * vec4(vPosition, 1.0);

    // changer 0,0,0 par la position du joueur
    vec3 camPosition = vec3(0, 0, 0);
    vec4 vertexPosition = mmat * vec4(vPosition, 1.0);

    gl_Position = projmat * vmat * vertexPosition;

    out_texCoord = vTexture;
    out_vNormal = normalize(vec3(tinv_mmat * vec4(vNormal, 0.0)));
    out_vLight = normalize(lum_pos - vertexPosition.xyz);
    out_vView = normalize(vertexPosition.xyz - camPosition);

}