FFGLSpoutBridge
=====================================

Introduction
------------
With this addon, together with the related FFGL-SpoutBridge plugin (included in this repo, in the FFGL-Plugin directory), it is possible to use an openFrameworks application as a plugin for any software adopting the FFGL standard. I only tested it with Resolume Arena (a demo version can be downloaded here https://resolume.com/download/files?file=Resolume_Arena_6_0_1_Installer.exe) but it should work with other hosts too.

Since integrating openFrameworks inside the plugin code has proven to be a hard task (I managed to make it work - sort of - with older OF versions but the code never was stable enough and it had bugs nobody managed to fix) the approach used here is different. Not the most elegant thing I could imagine, but it works. The plugin itself sends the texture it gets from host to the OF app via Spout texture sharing, the app gets it, implements arbitrary code and then sends the updated texture to the plugin that finally gives it back to host for further processing.

The plugin can implement parameters (only float ones in this version, but it is easy enough to extend the process to different types), their values are sent to the OF application every frame with an OSC packet.

The process looks a little cumbersome but the back-and-forward path is very fast and in practice there is no noticeable delay while implementing it in Arena (or any other VJ software).

The plugin has two text parameters, to set the name to be used for texture sharing and the OSC port. In the example client app it is possible to set this values pressing "n" and "p". Of course the settings in host and client must match for sharing to work. Using different names should make it possible to run more instances of the plugin at the same time, each linked to its client application.

License
-------
The code for this addon and for the included FFGL-Plugin is offered like openFrameworks itself under the [MIT License](https://en.wikipedia.org/wiki/MIT_License). Read `license.md` for details.

Installation
------------
To install ofxFFGLSpoutBridge addon just drop the folder into `openFrameworks/addons/`, then use project builder or Visual Studio OF plugin to add it to your project.

The FFGL plugin solution is located in FFGL-Plugin/build/windows directory. It includes the Spout SDK (again) and a version of the FFGL source library). To generate the plugin DLL you can use the "build current solution" menu, or press CTRL-SHIFT-B. The solution has a post build phase that copies the DLL in the vfx directory of Resolume Arena (it it is installed in its standard location), change it if you use a different host program. A copy of the DLL is also written to the build/binaries directory.

The source code is located in FFGL-Plugin/source/plugins/SpoutBridge

An alternative is to just press the "build and run" green triangle, the plugin gets built, the dll copied in the appropriate place and the host application (Arena) is run afterwards. This approach is useful for debugging, as you can set breakpoints in code ad look into it while the plugin is running. Sweet :)

If you don't like this approach (or can't make it work) you can use a different one to debug the plugin: in SpoutBridge.cpp there are a few calls to "OutputDebugString()", the output can be read using the "DebugView" tool, that can be downloaded here for free https://docs.microsoft.com/it-it/sysinternals/downloads/debugview
The debug output does not go to DebugView if Arena is started from Visual Studio, so you have to choose one of the two approaches.

Dependencies
------------
Since there appear to be different Spout and FFGL libraries around I tried to make the addon completely self contained. Everything here should compile out of the box with Visual Studio 2017 community edition (make sure you selected the Windows SDK while installing it). If this is not the case for you drop a message to the author.

The example openFrameworks application uses the spout SDK (included here in `libs/spoutSDK`). If you write your application in apps/myApps copy the directory in your source tree (or leave it where it is but make sure you addd the SDK files to your project).

The example also needs the ofxOsc addon, that should work out of the box since it is a core one. If you get errors with the compiler complaining about missing headers remove from the project the `ofxOsc/libs/oscpack/src/ip/posix` directory.

Compatibility
------------
This addon has been tested on Windows 10, Visual Studio 2017 Community edition and OpenFrameworks 0.9.8

An OSX version using Syphon would be interesting, but at the moment I don't have time to investigate its feasibility.

Known issues
------------
The OSC link currently is only directed from Host to Client. It would be easy enough to add support for the other direction, but it's not there yet.

Version history
------------

### Version 0.1 (December 6, 2017):
First draft.


