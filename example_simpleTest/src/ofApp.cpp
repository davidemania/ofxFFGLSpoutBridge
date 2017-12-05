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

#include "ofApp.h"

//******************************************************************
//******************************************************************

void ofApp::setup()
{
	ofSetWindowTitle("FFGLSpoutBridge example");

	spoutBridge.initialize("FFGLSpoutBridge", ofGetWidth(), ofGetHeight());

	font.load("Arial", 45);
	currentHue = 0.0;
}

//******************************************************************
//******************************************************************

void ofApp::update()
{

}

//******************************************************************
//******************************************************************

void ofApp::draw()
{
	spoutBridge.receive();

	// Now we have the frame from host application in an fbo
	// let's draw something over it and send it back.

	ofFbo& sharedFbo = spoutBridge.getFbo();
	sharedFbo.begin();

		// Put your magic here :)

		string msg = "OF-FFGL Spout Bridge Example";
		static ofRectangle box = font.getStringBoundingBox(msg, 0, 0);

		float x = (sharedFbo.getWidth() - box.width) / 2;
		float y = (sharedFbo.getHeight() - box.height) / 2;

		x += ofSignedNoise(ofGetElapsedTimef()) * sharedFbo.getHeight() / 4;
		y += ofSignedNoise(ofGetElapsedTimef() + 1) * sharedFbo.getWidth() / 4;

		if ((currentHue+= 0.1) >= 256)
		{
			currentHue = 0;
		}

		ofPushStyle();
		ofSetColor(ofColor().fromHsb(currentHue, 192, 255));
		font.drawString(msg, x, y);
		ofPopStyle();

	sharedFbo.end();
	
	sharedFbo.draw(0, 0);

	spoutBridge.send();
}

//******************************************************************
//******************************************************************

void ofApp::keyPressed(int key)
{

}

//******************************************************************
//******************************************************************
