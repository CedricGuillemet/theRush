#version 120
uniform vec2 ArcpFrame;
uniform vec4 ArcpFrameOpt;
varying vec2 Apos;
varying vec4 AposPos;
uniform sampler2D colorSourceTexture;
#define FXAA_LINEAR 1
#define FXAA_GLSL_120 1
#define FXAA_PC_CONSOLE 1
%s
void main() 
{ 
	gl_FragColor = FxaaPixelShader(
    Apos,
    AposPos,
    colorSourceTexture,
    ArcpFrame,
    ArcpFrameOpt 
	);
}