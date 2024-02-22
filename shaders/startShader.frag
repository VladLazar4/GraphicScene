#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

uniform	vec3 lightSun;
uniform	vec3 lightColorSun;
uniform	vec3 lightColorFar;
uniform	vec3 lightColorFelinar;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

uniform bool fogON;
uniform float fogDensity;

uniform mat4 view;

uniform bool faruriON;
uniform bool felinarON;
uniform vec3 lightPosFelinar;
uniform vec3 lightPosFar1;
uniform vec3 lightPosFar2;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;


float computeShadow()
{
	//perform perspective divide
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	//tranform from [-1,1] range to [0,1] range
	normalizedCoords = normalizedCoords * 0.5 + 0.5;

	//get closest depth value from lights perspective
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;

	//get depth of current fragment from lights perspective
	float currentDepth = normalizedCoords.z;

	float bias = 0.005f;
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	if (normalizedCoords.z > 1.0f)
		return 0.0f;

	return shadow;
}

void computeLightComponents()
{		
	vec3 cameraPosEye = vec3(-38.7511f, 5.55983f, 0.22628f);
	
	//transform normal
	vec3 normalEye = normalize(fNormal);	
	
	//compute light direction
	vec3 lightSunN = normalize(lightSun);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
		
	//compute ambient light
	ambient = ambientStrength * lightColorSun;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightSunN), 0.0f) * lightColorSun;
	
	//compute specular light
	vec3 reflection = reflect(-lightSunN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColorSun;
}



float ambientPoint = 0.000001f;
uniform float constant;
float quadratic = 1.75;
float linear = 2.225f;

vec3 computePointLight(vec4 lightPosEye, vec3 lightColor)
{
	vec3 cameraPosEye = vec3(0.0f);
	vec3 normalEye = normalize(fNormal);
	vec3 lightDirN = normalize(lightPosEye.xyz - fPosEye.xyz);
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
	vec3 ambient = ambientPoint * lightColor;
	vec3 diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	vec3 halfVector = normalize(lightDirN + viewDirN);
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
	vec3 specular = specularStrength * specCoeff * lightColor;
	float distance = length(lightPosEye.xyz - fPosEye.xyz);
	float att = 1.0f / (constant + linear * distance + quadratic * distance * distance);
	return (ambient + diffuse + specular) * att * vec3(2.0f,2.0f,2.0f);
}


vec3 spotlightDir = vec3(0.0f, -0.005f, 0.0f);
float spotlightInnerCutOff = cos(radians(12.5f));
float spotlightOuterCutOff = cos(radians(17.5f));
float spotlightConstant = 1.0f;
float spotlightLinear = 0.09f;
float spotlightQuadratic = 0.032f;

vec3 computeSpotlight(vec4 lightPosEye)
{
    vec3 cameraPosEye = vec3(0.0f);
    vec3 normalEye = normalize(fNormal);
    vec3 lightDirN = normalize(lightPosEye.xyz - fPosEye.xyz);
    vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
    
    float cosTheta = dot(-lightDirN, normalize(spotlightDir));
    float spotlightEffect = smoothstep(spotlightOuterCutOff, spotlightInnerCutOff, cosTheta);
    
    vec3 ambient = ambientPoint * spotlightEffect * lightColorFar;
    vec3 diffuse = max(dot(normalEye, lightDirN), 0.0f) * spotlightEffect * lightColorFar;
    
    vec3 halfVector = normalize(lightDirN + viewDirN);
    vec3 reflection = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
    vec3 specular = specularStrength * specCoeff * spotlightEffect * lightColorFar;

    float distance = length(lightPosEye.xyz - fPosEye.xyz);
    float att = 1.0f / (spotlightConstant + spotlightLinear * distance + spotlightQuadratic * distance * distance);
    
    return (ambient + diffuse + specular) * att * vec3(2.0f, 2.0f, 2.0f);
}

float computeFog()
{
	 float fragmentDistance = length(fPosEye);
	 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

	 return clamp(fogFactor, 0.0f, 1.0f);
}

void main() 
{
	computeLightComponents();
	
	vec3 baseColor = vec3(0.9f, 0.35f, 0.0f);
	
	ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;

	float shadow = computeShadow();
	vec3 color = min((ambient + (1.0f - shadow) * diffuse) + (1.0f - shadow) * specular, 1.0f);

	if (faruriON == true){
		vec4 lightPosEyeFar1 = view * vec4(lightPosFar1, 1.0f);
		vec4 lightPosEyeFar2 = view * vec4(lightPosFar2, 1.0f);
		
		color += computePointLight(lightPosEyeFar1, lightColorFar);
		color += computePointLight(lightPosEyeFar2, lightColorFar);

		//color += computeSpotlight(lightPosFar1);
		//color += computeSpotlight(lightPosFar);
	}

	if(felinarON == true){
		vec4 lightPosEyeFelinar = view * vec4(lightPosFelinar, 1.0f);
		color += computePointLight(lightPosEyeFelinar, lightColorFelinar);
	}

	vec4 diffuseColor = texture(diffuseTexture, fTexCoords);

	float fogFactor = computeFog();
	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);

	if (fogON == true){
		fColor = mix(fogColor, vec4(color, 1.0f), fogFactor);
	}
	else{
		fColor =  vec4(color, 1.0f);	
	}
}