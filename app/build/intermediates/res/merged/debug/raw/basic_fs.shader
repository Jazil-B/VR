precision mediump float;

uniform sampler2D myTexture;

varying float intensite;
varying vec2 out_texCoord;
void main() {
    gl_FragColor = texture2D(myTexture, out_texCoord) * (0.3 + 1.0 * intensite) * vec4(0.0, 0.5, 1.0, 1.0);
}