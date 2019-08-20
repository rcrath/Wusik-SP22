# Welcome to Wusik SP22
This product is FREE and also Open-Source.
Current Version 1.0.4

------------------------------------------------------

The Source Code requires JUCE 5 latest version.

https://github.com/WeAreROLI/JUCE

------------------------------------------------------

To record something click the record button next to the play and stop buttons on the left. They are, from left to right: Play, Stop and Record. Next is the audio detection level and the MIDI Note trigger for recording. After recording something you can use MIDI Notes to play it back or click the Play button. To export the recording to a wav file, click on Options and select one of the Export options.

------------------------------------------------------

August 20 2019 - Version 1.0.4 

	- Added the option to record very large audios into RAM. By default the code has a 60 seconds buffer, when the audio is larger it will start adding 60 seconds buffers into an array so you can record up to several minutes. Only limited by your RAM and OS type (64/32 bits). Audio shorter than 60 seconds can use the key pitch to change the pitch of the playback, larger audios will always play without any pitch changes.

------------------------------------------------------

Created by WilliamK @ www.Wusik.com (c) 2019