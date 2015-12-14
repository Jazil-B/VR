precision mediump float;

uniform sampler2D myTexture;

//varying float intensite;
varying vec2 out_texCoord;
varying vec3 out_vNormal;
varying vec3 out_vLight;
varying vec3 out_vView;

// 6 arguments, les 2 prmeieres la couleur de la lumière, vecteur N, L et V, et la taille du scintillement)
vec4 calcLight(vec4 diffColor, vec4 specColor, vec3 N, vec3 L, vec3 V, float shininess)
{
	vec3 H = normalize(L + V);
	vec4 diff = max(dot(N, L), 0.0) * diffColor;
	vec4 spec = pow(max(dot(N, H), 0.0), shininess) * specColor;
	return diff + spec;
}

void main() {

    const vec4 whiteColor = vec4(1, 1, 1, 1);
    const float density = 0.001;
    const float LOG2 = 1.442695;

    // Permet d'avoir la distance camera / pixel
    float fogDistance = ((gl_FragCoord.z -20.0)/ gl_FragCoord.w);

    vec4 light = calcLight(whiteColor, whiteColor, out_vNormal, out_vLight, out_vView, 5.0);

    //quantité de gris en fonction de la distance et de la densité
    float fogFactor = 1.0 - clamp(exp2(-density * density * fogDistance * fogDistance * LOG2), 0.0, 1.0);

    //brouillard + lumiere
    //gl_FragColor = mix(texture2D(myTexture, out_texCoord) * light, vec4(0.5, 0.5, 0.5, 1.0), fogFactor);

    // sans le brouillard
    //gl_FragColor = texture2D(myTexture, out_texCoord)*light;

    //brouillard
   gl_FragColor = mix(texture2D(myTexture, out_texCoord), vec4(0.5, 0.5, 0.5, 1.0), fogFactor);

    //  sans le brouillard + lumiere
    //gl_FragColor = texture2D(myTexture, out_texCoord) * light;

    //  sans le brouillard
    gl_FragColor = texture2D(myTexture, out_texCoord) ;


}