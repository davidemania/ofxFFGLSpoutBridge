//******************************************************************
/*
Copyright (c) 2017, Davide Manià - software@cogitamus.it
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the developer nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY DAVIDE MANIA' "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL DAMIAN STEWART BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
//******************************************************************

#include "ofxFFGLSpoutBridge.h"

//******************************************************************
// Init bridge.
// 
// bridgeName: name to be used for texture sharing via Spout.
//  example: name = "PrettySharing" =>
//  we receive from a sender named "PrettySharingFromHost"
//  and cxreate a sender with name "PrettySharingToHost"
//  where Host is the program where the FFGLBridge plugin is running
// 
// width, height: used toi allocate internal fbo. Size is 
// adjustd according to received texture if these values are wrong
//
// flipReceive: vertically flip received texture before storing it
// flipSend: vertically flip fbo texture before sending it
//******************************************************************

void ofxFFGLSpoutBridge::initialize(string bridgeName, int width, int height, bool flipReceive, bool flipSend)
{
	ofDisableArbTex(); // Needed to pass textures to Spout. Without this you only get black. To be investigated...

	frameWidth = width;
	frameHeight = height;

	if (bufferFbo.isAllocated())
	{
		bufferFbo.clear();
	}

	bufferFbo.allocate(width, height, GL_RGBA);

	spoutSenderIsInitialized = spoutReceiverIsInitialized = false;

	// Just in case. Will do nothing if receiver and sender are not active
	spoutReceiver.ReleaseReceiver();
	spoutSender.ReleaseSender();

	flipReceivedTexture = flipReceive;
	flipTextureToSend = flipSend;

	strcpy(spoutSenderName, (bridgeName + "ToHost").c_str());
	strcpy(spoutReceiveFromName, (bridgeName + "FromHost").c_str());

	initialized = true;
}

//******************************************************************
// Receive texture from Spout and draw it to internal fbo
// for further processing
//******************************************************************

void ofxFFGLSpoutBridge::receive()
{
	if (!initialized)
	{
		return;
	}

	initSpout(); // Init must be tried every frame, the bridge with host can go up or down anytime

	if (spoutReceiverIsInitialized)
	{
		// Receive the shared texture to local copy
		//
		//	Success : Returns the sender name, width and height and a local copy of the shared texture
		//	Failure : No sender detected
		//
		// Important - pass the host FBO to restore the binding

		unsigned int id = spoutTexture.getTextureData().textureID;

		if (spoutReceiver.ReceiveTexture(spoutReceiveFromName, receiverWidth, receiverHeight, id, GL_TEXTURE_2D, flipReceivedTexture, 0))
		{
			if (receiverHeight != frameHeight || receiverWidth != frameWidth)
			{
				// Let's adapt our fbo to received frame size
				frameWidth = receiverWidth;
				frameHeight = receiverHeight;

				bufferFbo.clear();
				bufferFbo.allocate(frameWidth, frameHeight, GL_RGBA);
			}

			// draw the shared texture we received to fbo
			bufferFbo.begin();
			ofBackground(0, 0, 0, 0); // needed even if we draw a new frame every time because part of it could be transparent
			spoutTexture.draw(0, 0, frameWidth, frameHeight);
			bufferFbo.end();
		}
		else
		{
			// The sender has closed
			spoutReceiver.ReleaseReceiver();
			spoutReceiverIsInitialized = false;

			ofLogNotice() << "[ofxFFGLSpoutBridge] Release existing receiver (" << spoutReceiveFromName << ")";
		}
	}
	else // we didn't get a texture so let's clear our buffer
	{
		bufferFbo.begin();
		ofBackground(0, 0, 0, 0);
		bufferFbo.end();
	}
}

//******************************************************************
// Send current texture from fbo to host application via Spout
//******************************************************************

void ofxFFGLSpoutBridge::send()
{
	if (!initialized)
	{
		return;
	}

	if (spoutSenderIsInitialized)
	{
		unsigned int id = bufferFbo.getTexture().getTextureData().textureID;
		spoutSender.SendTexture(id, GL_TEXTURE_2D, frameWidth, frameHeight, flipTextureToSend);
	}
}

//******************************************************************
// If Spout links are not active try to initialize them 
//******************************************************************

void ofxFFGLSpoutBridge::initSpout()
{
	//*********************************************************
	// Manage Spout sender initialization and such
	//*********************************************************

	if (!spoutSenderIsInitialized) // create a sender if not initialized yet
	{
		// Create a new sender
		spoutSenderIsInitialized = spoutSender.CreateSender(spoutSenderName, frameWidth, frameHeight);
		if (!spoutSenderIsInitialized)
		{
			ofLogError() << "[ofxFFGLSpoutBridge] Error: could not create Spout sender";
		}
		else
		{
			ofLogNotice() << "[ofxFFGLSpoutBridge] Created new Spout sender (" << spoutSenderName << ")";
		}

		return; // give it one frame to initialize
	}

	//*********************************************************
	// Manage Spout receiver initialization
	//*********************************************************

	if (!spoutReceiverIsInitialized) // create a sender if not initialized yet
	{
		// Create a new receiver
		// CreateReceiver will return true only if it finds a sender running.
		// If a sender name is specified and does not exist it will return false.
		// This also sets the global width and height

		receiverWidth = frameWidth;
		receiverHeight = frameHeight;

		if (spoutReceiver.CreateReceiver(spoutReceiveFromName, receiverWidth, receiverHeight, false))
		{
			spoutTexture.allocate(receiverWidth, receiverHeight, GL_RGBA);

			spoutReceiverIsInitialized = true;

			ofLogNotice() << "[ofxFFGLSpoutBridge] Spout receiver initialized (" << spoutReceiveFromName << ")";
		}

		return; // give it one frame to initialize
	}
}

//******************************************************************
//******************************************************************
