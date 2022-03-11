#version 430


layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTex;

uniform vec3 lightPosition;
uniform vec3 cameraPosition;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 NormalMatrix;
uniform mat4 MVP;
uniform mat4 ModelMatrix;

uniform sampler2D TexColor;
uniform sampler2D TexGrey;
uniform float textureOffset;
uniform float orbitDegree;

uniform float heightFactor;
uniform float imageWidth;
uniform float imageHeight;

out Data
{
    vec3 Position;
    vec3 Normal;
    vec2 TexCoord;
} data;


out vec3 LightVector;// Vector from Vertex to Light;
out vec3 CameraVector;// Vector from Vertex to Camera;


void main()
{
    data.TexCoord = VertexTex;
    data.TexCoord.x += textureOffset * (1.0 / 500.0);
    //data.Normal = VertexNormal;
    data.Normal = (transpose(inverse(ModelMatrix))*vec4(VertexNormal.xyz, 1.0f)).xyz;
    // vec3 calculated_pos = rotMatrix * VertexPosition;
    mat3 rotMatrix = mat3(cos(orbitDegree), -sin(orbitDegree), 0.0, sin(orbitDegree), cos(orbitDegree), 0.0, 0.0,0.0,1.0);
    vec3 calculated_pos = rotMatrix * VertexPosition;
    CameraVector = normalize(cameraPosition - (ModelMatrix *vec4(calculated_pos.xyz, 1.0f)).xyz);
    LightVector = normalize(lightPosition - (ModelMatrix *vec4(calculated_pos.xyz, 1.0f)).xyz);
    

    // get orbitDegree value, compute new x, y coordinates
    
    //data.Position = (MVP * vec4(calculated_pos.xyz, 1.0f)).xyz; 
    gl_Position = MVP * vec4(calculated_pos.xyz, 1.0f);
    

    // there won't be height in moon shader
 
    // set gl_Position variable correctly to give the transformed vertex position

    //gl_Position = vec4(0,0,0,0); // this is a placeholder. It does not correctly set the position
}