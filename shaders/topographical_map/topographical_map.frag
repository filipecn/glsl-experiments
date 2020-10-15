#version 440 core
layout(location = 0) out vec4 fragColor;

in vec3 fNormal;
in vec3 fPosition;
in vec2 fUV;

struct Light {
  vec3 direction; // directional light direction
  vec3 point; // point light position
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct Material {
  vec3 kAmbient;
  vec3 kDiffuse;
  vec3 kSpecular;
  float shininess;
};

layout(location = 1) uniform Light light;
layout(location = 6) uniform Material material;
layout(location = 10) uniform vec3 cameraPosition;
layout(location = 14) uniform vec2 screenResolution;

const int levelCount = 10;
const float maxHeight = 1.0;
const float minHeight = -1.0;


float map3(in float value, in float start1, in float stop1, in float start2, in float stop2, in float v) {
  float b = start2;
  float c = stop2 - start2;
  float t = value - start1;
  float d = stop1 - start1;
  float p = v;
  float r = 0.0;
    t /= d/2;
    if (t < 1) return c/2*pow(t, p) + b;
    r = c/2 * (2 - pow(2 - t, p)) + b;
  return r;
}

vec4 hsv_to_rgb(float h, float s, float v, float a)
{
	float c = v * s;
	h = mod((h * 6.0), 6.0);
	float x = c * (1.0 - abs(mod(h, 2.0) - 1.0));
	vec4 color;

	if (0.0 <= h && h < 1.0) {
		color = vec4(c, x, 0.0, a);
	} else if (1.0 <= h && h < 2.0) {
		color = vec4(x, c, 0.0, a);
	} else if (2.0 <= h && h < 3.0) {
		color = vec4(0.0, c, x, a);
	} else if (3.0 <= h && h < 4.0) {
		color = vec4(0.0, x, c, a);
	} else if (4.0 <= h && h < 5.0) {
		color = vec4(x, 0.0, c, a);
	} else if (5.0 <= h && h < 6.0) {
		color = vec4(c, 0.0, x, a);
	} else {
		color = vec4(0.0, 0.0, 0.0, a);
	}

	color.rgb += v - c;

	return color;
}

vec3 hsbValueScale(in float hue, in float i) {
  vec3 scale = vec3(0.0);
  int n = levelCount;
  
  float saturation = 1;
  /* Vary the brightness regardless of value number */
  float brightness = map3(i, 0, n - 1, 1, 0, 1.6);
  /* Increase saturation only in the first half */
  if (i < n/2)
    saturation = map3(i, 0, n/2 - 1, 0, 1, 1.6);
  scale = hsv_to_rgb(hue, saturation, brightness, 1.0).rgb;
  
  return scale;
}


const float N = 10.0; // grid ratio
float gridTextureGradBox( in float p, in float ddx, in float ddy )
{
	// filter kernel
    float w = max(abs(ddx), abs(ddy)) + 0.01;

	// analytic (box) filtering
    float a = p + 0.5*w;                        
    float b = p - 0.5*w;           
    float i = (floor(a)+min(fract(a)*N,1.0)-
              floor(b)-min(fract(b)*N,1.0))/(N*w);
    //pattern
    return 1 - i;
}


void main() {


vec3 N = normalize(fNormal);
vec3 V = normalize(cameraPosition - fPosition);
vec2 uv = fUV;

float level = (fPosition.y - minHeight)/(maxHeight - minHeight) * levelCount;
vec3 col = hsbValueScale(0.36, levelCount - floor(level)).rgb;

float thickness = 0.04;
float line = 1 - step(thickness, fract(level));


float ddx_uvw = dFdx( line ); 
float ddy_uvw = dFdy( line ); 
col += vec3(line);
//col += gridTextureGradBox( line, ddx_uvw, ddy_uvw );




// gamma correction	
col = pow( col, vec3(0.4545) );
fragColor = vec4(col,1.0);
}




