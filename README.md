FFGLSpoutBridge
=====================================

Introduction
------------
With this addon, toigether with the related FFGL-SpoutBridge plugin (included in this repo, in the FFGL-Plugin directory), it is possible to use any openFrameworks application as a plugin for any software adopting the FFGL standard. I only tested it with Resolume Arena (a demo version can be downloaded here https://resolume.com/download/files?file=Resolume_Arena_6_0_1_Installer.exe) but it should work with other hosts too.

Since integrating openFrameworks inside the plugin code has proven to be a hard task (I managed to make it work - sort of - with older OF versions but the code never was stable enough and it had bugs nobody managed to fix) the approach used here is different. Not the most elegant thing I could imagine, but it works. The plugin itself sends the texture it gets from host to the OF app via Spout texture sharing, the app gets it, implements arbitrary code and then sends the updated texture to the plugin that finally gives itback to host.

The plugin can implement parameters (only float ones in this version, but it is easy enough to extend the process to different types), their values are sent to the OF application every frame with an OSC packet.

The process looks a little cumbersome but the back-and-forward path is very fast and in practice there is no noticeable delay while implementing it in Arena (or any other VJ software).

License
-------
State which license you offer your addon under. openFrameworks is distributed under the [MIT License](https://en.wikipedia.org/wiki/MIT_License), and you might consider using this for your repository. By default, `license.md` contains a copy of the MIT license to which you can add your name and the year.

Installation
------------
Any steps necessary to install your addon. Optimally, this means just dropping the folder into the `openFrameworks/addons/` folder.

Dependencies
------------
What other addons are needed to properly work with this one?

Compatibility
------------
Which versions of OF does this addon work with?

Known issues
------------
Any specific (and long-living) issues/limitations you want to mention? For bugs, etc. you should use the issue tracker of your addon's repository

Version history
------------
It make sense to include a version history here (newest releases first), describing new features and changes to the addon. Use [git tags](http://learn.github.com/p/tagging.html) to mark release points in your repo, too!

### Version 0.1 (Date):
Describe relevant changes etc.


