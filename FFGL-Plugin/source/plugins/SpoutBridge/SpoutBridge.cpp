//**********************************************************************************
//
// SpoutBridge.cpp
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

#include <FFGL.h>
#include <FFGLLib.h>

#include "SpoutBridge.h"
#include "../../lib/ffgl/utilities/utilities.h"

#define FFPARAM_MOVE_X  (0)
#define FFPARAM_MOVE_Y	 (1)
#define FFPARAM_ROTATE	 (2)
#define FFPARAM_SHARING_NAME (3)

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Plugin information
////////////////////////////////////////////////////////////////////////////////////////////////////

static CFFGLPluginInfo PluginInfo(
	FFGLSpoutBridge::CreateInstance,		// Create method
	"DMSB",								// Plugin unique ID
	"A_FFGLSpoutBridge",					// Plugin name
	1,						   			// API major version number 													
	500,								// API minor version number
	1,									// Plugin major version number
	000,								// Plugin minor version number
	FF_EFFECT,							// Plugin type
	"Spout send & receive test",		// Plugin description
	"By Davide Manià 2017 - software@cogitamus.it"				// About
);

FFGLSpoutBridge::FFGLSpoutBridge()
	:CFreeFrameGLPlugin(),
	m_initResources(1),
	m_inputTextureLocation(-1),
	m_BrightnessLocation(-1),
	transmitSocket(IpEndpointName(OSC_ADDRESS, OSC_PORT))
{
	// Input properties
	SetMinInputs(1);
	SetMaxInputs(1);

	// Parameters
	SetParamInfo(FFPARAM_MOVE_X, "Move X", FF_TYPE_STANDARD, 0.5f);
	moveX = 0.5f;

	SetParamInfo(FFPARAM_MOVE_Y, "Move Y", FF_TYPE_STANDARD, 0.5f);
	moveY = 0.5f;

	SetParamInfo(FFPARAM_ROTATE, "Rotate", FF_TYPE_STANDARD, 0.5f);
	rotate = 0.5f;

	SetParamInfo(FFPARAM_SHARING_NAME, "Name", FF_TYPE_TEXT, defaultName);
	strcpy(currentName, defaultName);

	spoutSenderIsInitialized = spoutReceiverIsInitialized = false;

	receivedTexture = 0;      // only used for memoryshare mode
	receiverWidth = receiverHeight = -1;

	sharingNameHasChanged = false;

	strcpy(spoutName, defaultName);
	
	strcpy(spoutSenderName, spoutName);
	strcat(spoutSenderName, "FromHost");

	strcpy(spoutReceiverName, spoutName);
	strcat(spoutReceiverName, "ToHost");

	sprintf(debugBuffer, "SpoutBridge plugin started");
	OutputDebugString(debugBuffer);
}

FFGLSpoutBridge::~FFGLSpoutBridge()
{
	// OpenGL context required
	// ReleaseSender does nothing if there is no sender
	if (wglGetCurrentContext())
	{
		if (spoutSenderIsInitialized)
		{
			spoutSender.ReleaseSender();
		}

		// ReleaseReceiver does nothing if there is no receiver
		spoutReceiver.ReleaseReceiver();
		if (receivedTexture != 0)
		{
			glDeleteTextures(1, &receivedTexture);
		}

		receivedTexture = 0;
	}
}

FFResult FFGLSpoutBridge::InitGL(const FFGLViewportStruct *vp)
{
	m_initResources = 0;

	sprintf(debugBuffer, "InitGL done");
	OutputDebugString(debugBuffer);

	return FF_SUCCESS;
}

FFResult FFGLSpoutBridge::DeInitGL()
{
	m_shader.FreeGLResources();

	// OpenGL context required
	if (wglGetCurrentContext())
		if (spoutSenderIsInitialized) spoutSender.ReleaseSender();

	spoutSenderIsInitialized = false;
	
	return FF_SUCCESS;
}

//**********************************************************************************
// The main function, draws every frame
//**********************************************************************************

FFResult FFGLSpoutBridge::ProcessOpenGL(ProcessOpenGLStruct *pGL)
{
	// We need a texture to process
	if (pGL->numInputTextures < 1) return FF_FAIL;
	if (pGL->inputTextures[0] == NULL) return FF_FAIL;

	FFGLTextureStruct &InputTexture = *(pGL->inputTextures[0]);

	// get the max s,t that correspond to the width, height
	// of the used portion of the allocated texture space
	FFGLTexCoords maxCoords = GetMaxGLTexCoords(InputTexture);

	// Draw the incomimg texture now whether a sender has initialized or not
	//DrawFFGLtexture(InputTexture.Handle, maxCoords);
	//return FF_SUCCESS;

	//*********************************************************
	// Manage Spout sender initialization and such
	//*********************************************************

	if (!spoutSenderIsInitialized) // create a sender if not initialized yet
	{
		// Set global width and height so any change can be tested
		m_Width = (unsigned int)InputTexture.Width;
		m_Height = (unsigned int)InputTexture.Height;

		// Create a new sender
		spoutSenderIsInitialized = spoutSender.CreateSender(spoutSenderName, m_Width, m_Height);
		if (!spoutSenderIsInitialized)
		{
			sprintf(debugBuffer, "Error: could not create Spout sender");
			OutputDebugString(debugBuffer);
		}
		else
		{
			sprintf(debugBuffer, "Created new Spout sender[%s]\n", spoutSenderName);
			OutputDebugString(debugBuffer);
		}

		return FF_SUCCESS; // give it one frame to initialize
	}
	else if (m_Width != (unsigned int)InputTexture.Width || // Has the texture size or sharing name changed ?
		     m_Height != (unsigned int)InputTexture.Height ||
			 sharingNameHasChanged)
	{
		sprintf(debugBuffer, "Release existing sender [%s]\n", spoutSenderName);
		OutputDebugString(debugBuffer);

		// Release existing sender
		spoutSender.ReleaseSender();
		spoutSenderIsInitialized = false;
		sharingNameHasChanged = false;
		return FF_SUCCESS; // return for initialization on the next frame
	}

	// Render the Freeframe texture into the shared texture
	// Important - pass the FFGL host FBO to restore the binding because Spout uses a local fbo
	spoutSender.DrawToSharedTexture(InputTexture.Handle, GL_TEXTURE_2D, m_Width, m_Height, (float)maxCoords.s, (float)maxCoords.t, 1.0f, false, pGL->HostFBO);

	//*********************************************************
	// Manage Spout receiver initialization
	//*********************************************************

	if (!spoutReceiverIsInitialized) // create a sender if not initialized yet
	{
		// Set global width and height so any change can be tested
		//m_Width = (unsigned int)InputTexture.Width;
		//m_Height = (unsigned int)InputTexture.Height;

		// Create a new receiver
		// CreateReceiver will return true only if it finds a sender running.
		// If a sender name is specified and does not exist it will return false.
		// This also sets the global width and height
		
		if (spoutReceiver.CreateReceiver(spoutReceiverName, receiverWidth, receiverHeight, false))
		{
			initReceivedTexture(); // Initialize a texture
			spoutReceiverIsInitialized = true;

			sprintf(debugBuffer, "Spout receiver initialized [%s]", spoutReceiverName);
			OutputDebugString(debugBuffer);
		}

		return FF_SUCCESS; // give it one frame to initialize
	}
	else
	{
		// Receive the shared texture to local copy
		//
		//	Success : Returns the sender name, width and height and a local copy of the shared texture
		//	Failure : No sender detected
		//
		// Important - pass the host FBO to restore the binding

		unsigned int width = receiverWidth, height = receiverHeight;

		if (spoutReceiver.ReceiveTexture(spoutReceiverName, width, height, receivedTexture, GL_TEXTURE_2D, false, pGL->HostFBO))
		{
			// draw the shared texture
			DrawReceivedTexture(receivedTexture, GL_TEXTURE_2D, m_Width, m_Height);
			//DrawFFGLtexture(receivedTexture, maxCoords);
			//DrawReceivedTexture(InputTexture.Handle, GL_TEXTURE_2D, m_Width, m_Height);
		}
		else 
		{
			// The sender has closed
			spoutReceiver.ReleaseReceiver();
			spoutReceiverIsInitialized = false;

			sprintf(debugBuffer, "Release existing receiver [%s]\n", spoutReceiverName);
			OutputDebugString(debugBuffer);
		}
	}

	return FF_SUCCESS;
}

//**********************************************************************************
// Updates a text parameter with the value from host
// Triggered every frame (so it seems)
//**********************************************************************************

FFResult FFGLSpoutBridge::SetTextParameter(unsigned int dwIndex, const char *value)
{
	switch (dwIndex)
	{
	case FFPARAM_SHARING_NAME:
		if (strcmp(spoutName, value) != 0)
		{
			strcpy(spoutName, value);

			sprintf(debugBuffer, "Sharing name changed -> %s", spoutName);
			OutputDebugString(debugBuffer);

			strcpy(spoutSenderName, spoutName);
			strcat(spoutSenderName, "FromHost");

			strcpy(spoutReceiverName, spoutName);
			strcat(spoutReceiverName, "ToHost");

			sharingNameHasChanged = true;
		}
		break;
	}

	return FF_SUCCESS;
}

//**********************************************************************************
// Updates host text parameter with the given value
// - untested -
//**********************************************************************************

char* FFGLSpoutBridge::GetTextParameter(unsigned int dwIndex)
{
	switch (dwIndex)
	{
	case FFPARAM_SHARING_NAME:
		return spoutName;
	}
}

//**********************************************************************************
//**********************************************************************************

float FFGLSpoutBridge::GetFloatParameter(unsigned int dwIndex)
{
	float retValue = 0.0;

	switch (dwIndex)
	{
	case FFPARAM_MOVE_X:
		retValue = moveX;
		return retValue;
	case FFPARAM_MOVE_Y:
		retValue = moveY;
		return retValue;
	case FFPARAM_ROTATE:
		retValue = rotate;
		return retValue;
	default:
		return retValue;
	}
}

//**********************************************************************************
//**********************************************************************************

FFResult FFGLSpoutBridge::SetFloatParameter(unsigned int dwIndex, float value)
{
	switch (dwIndex)
	{
	case FFPARAM_MOVE_X:
		moveX = value;
		break;
	case FFPARAM_MOVE_Y:
		moveY = value;
		break;
	case FFPARAM_ROTATE:
		rotate = value;
		break;
	default:
		return FF_FAIL;
	}

	// send parameters data via OSC to client application
	osc::OutboundPacketStream packet(oscBuffer, OUTPUT_BUFFER_SIZE);

	packet << osc::BeginBundleImmediate
		<< osc::BeginMessage("/test1")
		<< true << 23 << moveX << moveY << rotate << "hello" << osc::EndMessage
		<< osc::BeginMessage("/test2")
		<< true << 24 << (float)10.8 << "world" << osc::EndMessage
		<< osc::EndBundle;

	transmitSocket.Send(packet.Data(), packet.Size());

	return FF_SUCCESS;
}

//**********************************************************************************
//**********************************************************************************

void FFGLSpoutBridge::DrawFFGLtexture(GLuint TextureHandle, FFGLTexCoords maxCoords)
{
	GLfloat tex_coords[] = {
		0.0, 0.0,
		0.0, (float)maxCoords.t,
		(float)maxCoords.s, (float)maxCoords.t,
		(float)maxCoords.s, 0.0 };

	GLfloat verts[] = {
		-1.0, -1.0,
		-1.0,  1.0,
		1.0,  1.0,
		1.0, -1.0 };

	glPushMatrix();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, TextureHandle); // bind the FFGL texture

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, verts);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

//**********************************************************************************
// Initialize a local texture
//**********************************************************************************

void FFGLSpoutBridge::initReceivedTexture()
{
	if (receivedTexture != 0) 
	{
		glDeleteTextures(1, &receivedTexture);
		receivedTexture = 0;
	}

	glGenTextures(1, &receivedTexture);
	glBindTexture(GL_TEXTURE_2D, receivedTexture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
}

//**********************************************************************************
//**********************************************************************************

void FFGLSpoutBridge::DrawReceivedTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height)
{
	GLfloat tex_coords[] =
	{
		0.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
		1.0, 0.0
	};

	GLfloat verts[] =
	{
		-1.0, -1.0,
		-1.0,  1.0,
		1.0,  1.0,
		1.0, -1.0
	};

	glPushMatrix();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, TextureID); // bind the FFGL texture

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, verts);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	return;
}

//**********************************************************************************
//**********************************************************************************

