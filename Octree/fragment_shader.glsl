R"(
#version 330 core

// blinn-phong shading: brought from http://www.cs.toronto.edu/~jacobson/phong-demo/
// https://en.wikipedia.org/wiki/Blinn%E2%80%93Phong_reflection_model#OpenGL_Shading_Language_code_sample

in vec3 normalInterp;  // Surface normal
in vec3 vertPos;       // Vertex position

// Material
uniform float shininess;
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;

// light
const vec3 lightPos = vec3(1.0, 1.0, 1.0);
const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const float lightPower = 40.0;
const float screenGamma = 2.2; // Assume the monitor is calibrated to the sRGB color space

// Interpolated values from the vertex shaders
// in vec3 fragmentColor;

out vec3 color;

void main() {
  vec3 normal = normalize(normalInterp);
  vec3 lightDir = lightPos - vertPos;
  float distance = length(lightDir);
  distance = distance * distance;
  lightDir = normalize(lightDir);

  float lambertian = max(dot(lightDir, normal), 0.0);
  float specular = 0.0;

  if (lambertian > 0.0) {

    vec3 viewDir = normalize(-vertPos);

    // this is blinn phong
    vec3 halfDir = normalize(lightDir + viewDir);
    float specAngle = max(dot(halfDir, normal), 0.0);
    specular = pow(specAngle, shininess);
       
    // this is phong (for comparison)
    // if (mode == 2) {
    //  vec3 reflectDir = reflect(-lightDir, normal);
    //  specAngle = max(dot(reflectDir, viewDir), 0.0);
    // note that the exponent is different here
    //  specular = pow(specAngle, shininess/4.0);
    //}
  }
  vec3 colorLinear = ambientColor +
                     diffuseColor * lambertian * lightColor * lightPower / distance +
                     specularColor * specular * lightColor * lightPower / distance;
  // apply gamma correction (assume ambientColor, diffuseColor and specColor
  // have been linearized, i.e. have no gamma correction in them)
  color = pow(colorLinear, vec3(1.0 / screenGamma));

  // use the gamma corrected color in the fragment
  // gl_FragColor = vec4(colorGammaCorrected, 1.0);

  // Output color = color specified in the vertex shader,
  // interpolated between all 3 surrounding vertices
  // color = fragmentColor;
}
)"