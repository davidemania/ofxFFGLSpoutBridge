//**********************************************************************************
//
// SpoutBridge.h
//
// Send a texture from host to an OpenFrameworks application (or whatever :)
// and get back updated picture to send to host
//
// Parameters are sent and received via TCP/IP socket (OSC protocol)
//
// Davide Manià, December 2017
// software@cogitamus.it
//
// To read debug messages from plugin dll use the "DebugView" tool
// https://docs.microsoft.com/it-it/sysinternals/downloads/debugview
//
// FreeFrame is an open-source cross-platform real-time video effects plugin system.
// It provides a framework for developing video effects plugins and hosts on Windows, 
// Linux and Mac OSX. 
// 
// FreeFrameGL (FFGL) is an extension to the FreeFrame spec to support video processing
// with OpenGL on Windows, Linux, and Mac OSX.
//
// Copyright (c) 2002, 2003, 2004, 2006 www.freeframe.org. All rights reserved. 
//
// Spout SDK Copyright(c) 2014 - 2017, Lynn Jarvis. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, 
//	are permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materials provided with the
//    distribution.
//  * Neither the name of FreeFrame nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//
//	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
//	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
//	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
//	IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
//	INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
//	BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
//	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
//	OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
//	OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
//	OF THE POSSIBILITY OF SUCH DAMAGE. 
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <FFGLShader.h>
#include "FFGLPluginSDK.h"
#include <string>

//#include "FFGLExtensions.h" // can't use these if using Glew 31.12.13
#include "FFGLLib.h"
#include "Spout.h"
#include "osc/OscOutboundPacketStream.h"
#include "ip/UdpSocket.h"

#define OSC_ADDRESS "127.0.0.1"
#define OSC_DEFAULT_PORT "7251"

#define OUTPUT_BUFFER_SIZE 1024

class FFGLSpoutBridge : public CFreeFrameGLPlugin
{
public:
	FFGLSpoutBridge();
	~FFGLSpoutBridge();

	///////////////////////////////////////////////////
	// FreeFrame plugin methods
	///////////////////////////////////////////////////

	FFResult SetFloatParameter(unsigned int dwIndex, float value) override;
	float GetFloatParameter(unsigned int index) override;

	FFResult SetTextParameter(unsigned int dwIndex, const char *value) override;
	char* GetTextParameter(unsigned int dwIndex) override;

	FFResult ProcessOpenGL(ProcessOpenGLStruct* pGL) override;
	FFResult InitGL(const FFGLViewportStruct *vp);
	FFResult DeInitGL();

	///////////////////////////////////////////////////
	// Factory method
	///////////////////////////////////////////////////

	static FFResult __stdcall CreateInstance(CFreeFrameGLPlugin **ppOutInstance)
	{
		*ppOutInstance = new FFGLSpoutBridge();
		if (*ppOutInstance != NULL)
			return FF_SUCCESS;
		return FF_FAIL;
	}

protected:
	// Parameters
	float moveX;
	float moveY;
	float rotate;
	char currentName[256];

	int m_initResources;

	char defaultName[256] = "FFGLSpoutBridge";

	FFGLShader m_shader;
	GLint m_inputTextureLocation;
	GLint m_BrightnessLocation;

    // Spout stuff

	//FFGLExtensions m_extensions; // can't use these if using Glew 31.12.13

	GLint m_maxCoordsLocation;
	GLuint receivedTexture;

	SpoutSender spoutSender;
	SpoutReceiver spoutReceiver;

	unsigned int m_Width, m_Height;

	char spoutName[256];
	char spoutSenderName[270];
	char spoutReceiverName[270];
	bool sharingNameHasChanged;

	unsigned int receiverWidth, receiverHeight;

	bool spoutSenderIsInitialized, spoutReceiverIsInitialized;

	void DrawFFGLtexture(GLuint TextureHandle, FFGLTexCoords maxCoords);

	void initReceivedTexture();
	void DrawReceivedTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height);

	char debugBuffer[512];
	//char spoutSharingName[512];

	UdpTransmitSocket* transmitSocket;
	char oscBuffer[OUTPUT_BUFFER_SIZE];
	int currentOscPort;
};
