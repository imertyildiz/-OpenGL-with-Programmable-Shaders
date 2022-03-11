#version 430

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTex;

uniform vec3 lightPosition;
uniform vec3 cameraPosition;

uniform mat4 ProjectionMatrix;
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 NormalMatrix;
uniform mat4 MVP;

uniform sampler2D TexColor;
uniform sampler2D TexGrey;
uniform float textureOffset;

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

float getHeight(in vec2 xy){
    vec4 val = texture(TexGrey, xy);
    float height = val.r;
    return height * heightFactor;
}

void main()
{
    
    // Calculate texture coordinate based on data.TexCoord
    // vec2 textureCoordinate = vec2(0, 0);
    // vec4 texColor = texture(TexGrey, textureCoordinate);

    // get texture value, compute height
    // compute normal vector


   // set gl_Position variable correctly to give the transformed vertex position
    data.TexCoord = VertexTex;
    data.TexCoord.x += textureOffset * (1.0 / 500.0);
    
    data.Normal = (transpose(inverse(ModelMatrix))*vec4(VertexNormal.xyz, 1.0f)).xyz;
    vec3 heightoff = VertexNormal* getHeight(vec2(mod(data.TexCoord.x + textureOffset,1),data.TexCoord.y));
    vec3 calculated_pos = VertexPosition + heightoff;
    CameraVector = normalize(cameraPosition - (ModelMatrix *vec4(calculated_pos.xyz, 1.0f)).xyz);
    LightVector = normalize(lightPosition - (ModelMatrix *vec4(calculated_pos.xyz, 1.0f)).xyz);
    
    gl_Position = MVP * vec4(calculated_pos.xyz, 1.0f); // this is a placeholder. It does not correctly set the position
}