// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*

	Created by William Kalfelz @ Wusik Dot Com (c) 2019

	This project is FREE and Open-Souce. You can use and distribute it as long as you keep the mention above.

	www.Wusik.com

*/
//
#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
//
#define WPRODUCT_VERSION "V1.0.4"
#define WBUFFER_TIME_IN_SECONDS 60.0
//
enum
{
	kLeftChannel,
	kRightChannel,
	kPlayAudio = 0,
	kStopAudio
};
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
class WRecordThread : public Thread
{
public:
	WRecordThread(void* _owner) : Thread("Wusik Recording"), owner(_owner) { };
	void run();
	void* owner;
};
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
class WusikSp22AudioProcessor : public AudioProcessor
{
public:
	WusikSp22AudioProcessor();
    ~WusikSp22AudioProcessor();
    //
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
	void releaseResources() override { };
    #ifndef JucePlugin_PreferredChannelConfigurations
		bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
	#endif
	void getStateInformation(MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;
    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;
    AudioProcessorEditor* createEditor() override;
	bool hasEditor() const override { return true; };
	const String getName() const override { return JucePlugin_Name; }
	bool acceptsMidi() const { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
	int getNumPrograms() override { return 1; };
	int getCurrentProgram() override { return 0; };
	void setCurrentProgram(int index) override { };
	const String getProgramName(int index) override { return String(); };
	void changeProgramName(int index, const String& newName) override { };
	void startRecording();
	void finishRecording();
	void doAudio(int what);
	//
	float hermite(float x, float y0, float y1, float y2, float y3)
	{
		// 4-point, 3rd-order Hermite (x-form)
		float c0 = y1;
		float c1 = 0.5f * (y2 - y0);
		float c3 = (1.5f * (y1 - y2)) + (0.5f * (y3 - y0));
		float c2 = (y0 - (y1 + c1)) - c3;
		//
		return (((((c3 * x) + c2) * x) + c1) * x) + c0;
	}
	//
	float lastSampleRate;
	//
	float inputLevelRecording;
	float midiEnableRecording;
	float loopedMode;
	float loopStart;
	float loopEnd;
	bool isRecording, isRecordingStarted, isPlaying;
	int endRecordingCounter;
	float recordedSamplerate;
	float normalizeRecording;
	float normalizeRecordingValue;
	float autoStop;
	double playingPosition;
	double playingRate;
	MidiMessage midiMessage;
	int sampPos;
	bool fixedPitchPlayback, playbackAntialias;
	//
	WRecordThread recordingThread;
	OwnedArray<AudioSampleBuffer> sampleBuffer;
	int sampleLen, sampleLenRecording, whichSampleBufferRecording;
	ScopedPointer<AudioSampleBuffer> tempBufferToAdd;
	bool readyToAddTempBuffer;
	AudioSampleBuffer* recordingBufferToShow;
	//
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WusikSp22AudioProcessor)
};