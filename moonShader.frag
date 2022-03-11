#version 430

in Data
{
    vec3 Position;
    vec3 Normal;
    vec2 TexCoord;
} data;
in vec3 LightVector;
in vec3 CameraVector;

uniform vec3 lightPosition;
uniform sampler2D TexColor;
uniform sampler2D MoonTexColor;
uniform sampler2D TexGrey;
uniform float textureOffset;

out vec4 FragColor;

vec3 ambientReflectenceCoefficient = vec3(0.5f, 0.5f, 0.5f);
vec3 ambientLightColor = vec3(0.6f, 0.6f, 0.6f);
vec3 specularReflectenceCoefficient= vec3(1.0f, 1.0f, 1.0f);
vec3 specularLightColor = vec3(1.0f, 1.0f, 1.0f);
float SpecularExponent = 10;
vec3 diffuseLightColor = vec3(1.0f, 1.0f, 1.0f);


void main()
{
    // Calculate texture coordinate based on data.TexCoord
    vec2 textureCoordinate = vec2(mod(data.TexCoord.x + textureOffset*2,1),data.TexCoord.y);
    vec4 textureColor = texture(MoonTexColor, textureCoordinate);
    vec3 diffuseReflectenceCoefficient= vec3(textureColor.x, textureColor.y, textureColor.z );

    vec3 ambient = (ambientReflectenceCoefficient * ambientLightColor);
    // float dotProd = max(dot(data.Normal, LightVector), 0.0f);    
    float dotProd = clamp(dot(data.Normal, LightVector),0,1);
    vec3 diffuse = dotProd * (diffuseLightColor * diffuseReflectenceCoefficient);
    // vec3 reflection = reflect(-normalize(LightVector-0.5),data.Normal);
    vec3 reflection = normalize(LightVector + CameraVector);
    // float dotProd2 = pow(max(dot(reflection, normalize(CameraVector)),0.0f),SpecularExponent);
    float dotProd2 = clamp(dot(data.Normal, reflection),0,1);
    vec3 spec = pow(dotProd2,SpecularExponent) * (specularLightColor * specularReflectenceCoefficient);
    
    //FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    FragColor = vec4(clamp(textureColor.xyz * (ambient + diffuse + spec),0.0,1.0), 1.0f);

}