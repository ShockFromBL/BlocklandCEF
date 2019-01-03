#include <Windows.h>
#include "glapi.h"

#pragma comment(lib, "Opengl32.lib")

BL_glBindTextureFn BL_glBindTexture;
BL_glGetTexLevelParameterivFn BL_glGetTexLevelParameteriv;
BL_glTexImage2DFn BL_glTexImage2D;
BL_glGetErrorFn BL_glGetError;
BL_glTexParameteriFn BL_glTexParameteri;
BL_glEnableFn BL_glEnable;
BL_glTexSubImage2DFn BL_glTexSubImage2D;
BL_glGetStringFn BL_glGetString;
BL_glGenerateMipmapFn BL_glGenerateMipmap;
BL_wglGetProcAddressFn BL_wglGetProcAddress;
BL_glGenBuffersFn BL_glGenBuffers;
BL_glBindBufferFn BL_glBindBuffer;
BL_glDeleteBuffersFn BL_glDeleteBuffers;
BL_glBufferDataFn BL_glBufferData;
BL_glBufferSubDataFn BL_glBufferSubData;
BL_glGenBuffersARBFn BL_glGenBuffersARB;
BL_glBindBufferARBFn BL_glBindBufferARB;
BL_glDeleteBuffersARBFn BL_glDeleteBuffersARB;
BL_glBufferDataARBFn BL_glBufferDataARB;
BL_glBufferSubDataARBFn BL_glBufferSubDataARB;
char* glVersion;
unsigned int glMajor;


void *GetAnyGLFuncAddress(const char *name)
{
	void* p = nullptr;
	if (BL_wglGetProcAddress) {
		p = (void*)BL_wglGetProcAddress(name);
	}
	else {
		p = (void *)wglGetProcAddress(name);
	}
	if (p == 0 ||
		(p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
		(p == (void*)-1))
	{
		HMODULE module = LoadLibraryA("opengl32.dll");
		p = (void *)GetProcAddress(module, name);
	}

	return p;
}
void initGL() {
	HMODULE module = LoadLibraryA("opengl32.dll");
	BL_wglGetProcAddress = (BL_wglGetProcAddressFn)GetProcAddress(module, "wglGetProcAddress");
	BL_glBindTexture = (BL_glBindTextureFn)GetAnyGLFuncAddress("glBindTexture");
	BL_glGetTexLevelParameteriv = (BL_glGetTexLevelParameterivFn)GetAnyGLFuncAddress("glGetTexLevelParameteriv");
	BL_glTexImage2D = (BL_glTexImage2DFn)GetAnyGLFuncAddress("glTexImage2D");
	BL_glGetError = (BL_glGetErrorFn)GetAnyGLFuncAddress("glGetError");
	BL_glTexParameteri = (BL_glTexParameteriFn)GetAnyGLFuncAddress("glTexParameteri");
	BL_glEnable = (BL_glEnableFn)GetAnyGLFuncAddress("glEnable");
	BL_glTexSubImage2D = (BL_glTexSubImage2DFn)GetAnyGLFuncAddress("glTexSubImage2D");
	BL_glGetString = (BL_glGetStringFn)GetAnyGLFuncAddress("glGetString"); // GL_VERSION
	BL_glGenerateMipmap = (BL_glGenerateMipmapFn)GetAnyGLFuncAddress("glGenerateMipmaps");
	BL_glGenBuffers = (BL_glGenBuffersFn)GetAnyGLFuncAddress("glGenBuffers");
	BL_glBindBuffer = (BL_glBindBufferFn)GetAnyGLFuncAddress("glBindBuffer");
	BL_glDeleteBuffers = (BL_glDeleteBuffersFn)GetAnyGLFuncAddress("glDeleteBuffers");
	BL_glBufferData = (BL_glBufferDataFn)GetAnyGLFuncAddress("glBufferData");
	BL_glBufferSubData = (BL_glBufferSubDataFn)GetAnyGLFuncAddress("glBufferSubData");
	BL_glGenBuffersARB = (BL_glGenBuffersARBFn)GetAnyGLFuncAddress("glGenBuffersARB");
	BL_glBindBufferARB = (BL_glBindBufferARBFn)GetAnyGLFuncAddress("glBindBufferARB");
	BL_glDeleteBuffersARB = (BL_glDeleteBuffersARBFn)GetAnyGLFuncAddress("glDeleteBuffersARB");
	BL_glBufferDataARB = (BL_glBufferDataARBFn)GetAnyGLFuncAddress("glBufferDataARB");
	BL_glBufferSubDataARB = (BL_glBufferSubDataARBFn)GetAnyGLFuncAddress("glBufferSubDataARB");
	glMajor = 0;
}