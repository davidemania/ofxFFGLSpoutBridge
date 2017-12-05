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

#pragma once

#include "ofMain.h"
#include "Spout.h"

//******************************************************************
// ofxFFGLSpoutBridge
// This class manages the bridge between FFGL host via Spout
//******************************************************************

class ofxFFGLSpoutBridge
{
public:
	ofxFFGLSpoutBridge() { initialized = false; };
	virtual ~ofxFFGLSpoutBridge() {};

	void initialize(string bridgeName, int width, int height, bool flipReceive = false, bool flipSend = false);

	void receive();
	void send();

	ofFbo& getFbo() { return bufferFbo; }

private:
	int frameWidth, frameHeight;

	SpoutSender spoutSender;
	SpoutReceiver spoutReceiver;

	ofTexture spoutTexture;
	bool flipReceivedTexture, flipTextureToSend;

	char spoutSenderName[256];
	char spoutReceiveFromName[256];

	unsigned int receiverWidth, receiverHeight;

	bool spoutSenderIsInitialized, spoutReceiverIsInitialized;
	bool initialized;
	void initSpout();

	ofFbo bufferFbo;
};
