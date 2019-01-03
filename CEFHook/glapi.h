#pragma once
#include <Windows.h>
#include <gl\GL.h>
#include "glext.h"
void *GetAnyGLFuncAddress(const char *name);
void initGL();
#define GLFUNC(returnType, convention, name, ...)         \
	typedef returnType (convention*name##Fn)(__VA_ARGS__); \
	extern name##Fn name;

GLFUNC(void, __stdcall, BL_glBindTexture, unsigned int target, unsigned int texture);
GLFUNC(void, __stdcall, BL_glGetTexLevelParameteriv, unsigned int target, int level, unsigned int pname, int *params);
GLFUNC(void, __stdcall, BL_glTexImage2D, unsigned int target, int level, int internalFormat, int width, int height, int border, unsigned int format, unsigned int type, const void* pixels);
GLFUNC(unsigned int, __stdcall, BL_glGetError, void);
GLFUNC(void, __stdcall, BL_glTexParameteri, unsigned int, unsigned int, int);
GLFUNC(void, __stdcall, BL_glEnable, unsigned int);
GLFUNC(void, __stdcall, BL_glTexSubImage2D, unsigned int, int, int, int, int, int, unsigned int, unsigned int, void*);
GLFUNC(const char*, __stdcall, BL_glGetString, unsigned int);
GLFUNC(void, __stdcall, BL_glGenerateMipmap, unsigned int);
GLFUNC(PROC, WINAPI, BL_wglGetProcAddress, LPCSTR bleh);
GLFUNC(void, , BL_glGenBuffers, GLsizei n, GLuint* buf);
GLFUNC(void, , BL_glGenBuffersARB, GLsizei n, GLuint *buffers);
GLFUNC(void, , BL_glBindBuffer, int target, unsigned int bufferName);
GLFUNC(void, , BL_glBindBufferARB, int target, unsigned int bufferName);
GLFUNC(void, , BL_glDeleteBuffers, GLsizei n, const GLuint* bufs);
GLFUNC(void, , BL_glDeleteBuffersARB, GLsizei n, const GLuint* bufs);
GLFUNC(void, , BL_glBufferData, unsigned int target, GLsizeiptr size, const void *data, unsigned int usage);
GLFUNC(void, , BL_glBufferSubData, GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
GLFUNC(void, , BL_glBufferDataARB, unsigned int target, GLsizeiptr size, const void *data, unsigned int usage);
GLFUNC(void, , BL_glBufferSubDataARB, GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
extern char* glVersion;
extern unsigned int glMajor;