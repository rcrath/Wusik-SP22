// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*

	Created by William Kalfelz @ Wusik Dot Com (c) 2019

	This project is FREE and Open-Souce. You can use and distribute it as long as you keep the mention above.

	www.Wusik.com

*/
//
#include "PluginProcessor.h"
#include "PluginEditor.h"
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
WusikSp22AudioProcessor::WusikSp22AudioProcessor() : midiMessage(0xf4, 0.0), recordingThread(this)
{
	sampPos = 0;
	if (WusikSp22AudioProcessorEditor::readGlobalSettings("Wusik SP22", "AutoStop").compare("Y") == 0) autoStop = true; else autoStop = false;
	normalizeRecording = 1.0f;
	normalizeRecordingValue = 1.0f;
	inputLevelRecording = 0.1f;
	midiEnableRecording = 0.0f;
	loopedMode = 0.0f;
	loopStart = 0.0f;
	loopEnd = 1.0f;
	endRecordingCounter = 0;
	recordedSamplerate = 44100.0f;
	lastSampleRate = 44100.0f;
	isRecording = false;
	isRecordingStarted = false;
	isPlaying = false;
	fixedPitchPlayback = false;
	playbackAntialias = true;
	playingRate = 1.0;
	sampleLenRecording = 0;
	whichSampleBufferRecording = 0;
	readyToAddTempBuffer = false;
	recordingBufferToShow = nullptr;
}
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
WusikSp22AudioProcessor::~WusikSp22AudioProcessor()
{
	recordingThread.signalThreadShouldExit();
	recordingThread.notify();
	while (recordingThread.isThreadRunning()) { Thread::sleep(10); }
	//
	sampleBuffer.clear();
}
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void WusikSp22AudioProcessor::startRecording()
{
	if (isRecordingStarted)
	{
		finishRecording();
		return;
	}
	//
	sampleBuffer.clear();
	//
	sampleBuffer.add(new AudioSampleBuffer);
	sampleBuffer.getLast()->setSize(2, int(lastSampleRate * WBUFFER_TIME_IN_SECONDS));
	sampleBuffer.getLast()->clear();
	//
	sampleBuffer.add(new AudioSampleBuffer);
	sampleBuffer.getLast()->setSize(2, int(lastSampleRate * WBUFFER_TIME_IN_SECONDS));
	sampleBuffer.getLast()->clear();
	//
	recordingBufferToShow = sampleBuffer.getFirst();
	tempBufferToAdd = nullptr;
	readyToAddTempBuffer = false;
	endRecordingCounter = 0;
	isRecording = true;
	sampleLen = 0;
	sampleLenRecording = 0;
	recordedSamplerate = lastSampleRate;
	whichSampleBufferRecording = 0;
	//
	recordingThread.startThread(4);
}
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void WusikSp22AudioProcessor::finishRecording()
{
	recordingThread.signalThreadShouldExit();
	recordingThread.notify();
	//
	isRecordingStarted = isRecording = false;
	recordingBufferToShow = nullptr;
	//
	if (getActiveEditor() != nullptr)
	{
		((WusikSp22AudioProcessorEditor*)getActiveEditor())->createCachedWaveform();
		getActiveEditor()->repaint();
	}
}
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void WRecordThread::run()
{
	WusikSp22AudioProcessor* processor = (WusikSp22AudioProcessor*)owner;
	//
	while (!currentThreadShouldExit())
	{
		wait(-1);
		if (currentThreadShouldExit()) break;
		//
		processor->tempBufferToAdd = new AudioSampleBuffer;
		processor->tempBufferToAdd->setSize(2, int(processor->lastSampleRate * WBUFFER_TIME_IN_SECONDS));
		processor->tempBufferToAdd->clear();
		processor->readyToAddTempBuffer = true;
	}
}
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
AudioProcessorEditor* WusikSp22AudioProcessor::createEditor()
{
    return new WusikSp22AudioProcessorEditor (*this);
}
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void WusikSp22AudioProcessor::getStateInformation (MemoryBlock& destData)
{
	MemoryOutputStream xStream(destData, false);
	//
	xStream.writeInt(1); // Version
	//
	xStream.writeFloat(midiEnableRecording);
	xStream.writeFloat(loopedMode);
	xStream.writeFloat(inputLevelRecording);
	xStream.writeFloat(loopStart);
	xStream.writeFloat(loopEnd);
	xStream.writeFloat(recordedSamplerate);
	xStream.writeFloat(normalizeRecording);
	xStream.writeFloat(normalizeRecordingValue);
	//
	if (sampleLen > 0)
	{
		int theSamples = sampleLen;
		//
		if (sampleLen < sampleBuffer[0]->getNumSamples()) xStream.writeInt64(sampleLen);
		else
		{
			xStream.writeInt64(sampleBuffer[0]->getNumSamples());
			theSamples = sampleBuffer[0]->getNumSamples();
		}
		//
		for (int xs = 0; xs < theSamples; xs++)
		{
			xStream.writeFloat(sampleBuffer[0]->getSample(kLeftChannel, xs));
			xStream.writeFloat(sampleBuffer[0]->getSample(kRightChannel, xs));
		}
	}
	else xStream.writeInt64(0);
}
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void WusikSp22AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	isPlaying = false;
	playingPosition = 0.0;
	//
	MemoryInputStream xStream(data, sizeInBytes, false);
	int xVersion = xStream.readInt();
	//
	midiEnableRecording = xStream.readFloat();
	loopedMode = xStream.readFloat();
	inputLevelRecording = xStream.readFloat();
	loopStart = xStream.readFloat();
	loopEnd = xStream.readFloat();
	recordedSamplerate = xStream.readFloat();
	normalizeRecording = xStream.readFloat();
	normalizeRecordingValue = xStream.readFloat();
	sampleLen = xStream.readInt64();
	//
	sampleBuffer.clear();
	//
	if (sampleLen > 0)
	{
		sampleBuffer.add(new AudioSampleBuffer);
		sampleBuffer.getLast()->setSize(2, int(lastSampleRate * WBUFFER_TIME_IN_SECONDS));
		sampleBuffer.getLast()->clear();
		//
		for (int xs = 0; xs < sampleLen; xs++)
		{
			sampleBuffer.getLast()->setSample(kLeftChannel, xs, xStream.readFloat());
			sampleBuffer.getLast()->setSample(kRightChannel, xs, xStream.readFloat());
		}
	}
	else
	{
		sampleBuffer.add(new AudioSampleBuffer);
		sampleBuffer.getLast()->setSize(2, 0, false, false, false);

	}
	//
	if (getActiveEditor() != nullptr)
	{
		((WusikSp22AudioProcessorEditor*)getActiveEditor())->createCachedWaveform();
		getActiveEditor()->repaint();
	}
}
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WusikSp22AudioProcessor();
}
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void WusikSp22AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	lastSampleRate = sampleRate;
	if (sampleRate != lastSampleRate)
	{
		// !!!! //
	}
}
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void WusikSp22AudioProcessor::doAudio(int what)
{
	if (isRecordingStarted)
	{
		finishRecording();
		return;
	}
	//
	if (what == kPlayAudio)
	{
		playingPosition = 0.0;
		playingRate = 1.0;
		isPlaying = true;
	}
	else
	{
		playingPosition = 0.0;
		isPlaying = false;
	}
}
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
void WusikSp22AudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
	MidiBuffer::Iterator midiIterator(midiMessages);
	//
	for (int sampleBufferPos = 0; sampleBufferPos < buffer.getNumSamples(); sampleBufferPos++)
	{
		// Process MIDI Notes and other MIDI stuff //
		midiIterator.setNextSamplePosition(sampleBufferPos);
		while (midiIterator.getNextEvent(midiMessage, sampPos) && sampPos == sampleBufferPos)
		{
			if (midiMessage.isNoteOnOrOff())
			{
				if (midiEnableRecording == 1.0f && isRecording)
				{
					if (midiMessage.isNoteOn()) startRecording(); else finishRecording();
				}
				//
				if (!isRecording)
				{
					if (midiMessage.isNoteOn())
					{
						doAudio(kPlayAudio);
						//
						if (!fixedPitchPlayback && sampleLen < sampleBuffer[0]->getNumSamples())
						{
							double cyclesPerSecond = MidiMessage::getMidiNoteInHertz(midiMessage.getNoteNumber());
							double angleDelta = cyclesPerSecond / lastSampleRate;
							double baseRate = double(lastSampleRate) / MidiMessage::getMidiNoteInHertz(60);
							playingRate = angleDelta * baseRate;
							if (playingRate < 0.0f) playingRate = 0.0f;
						}
					}
					else
					{
						doAudio(kStopAudio);
					}
				}
			}
		}
		//
		if (isRecording)
		{
			if (isRecordingStarted)
			{
				if (readyToAddTempBuffer)
				{
					readyToAddTempBuffer = false;
					sampleBuffer.add(tempBufferToAdd.release());
				}
				//
				int theSize = sampleBuffer.size();
				sampleBuffer[whichSampleBufferRecording]->setSample(kLeftChannel, sampleLenRecording, buffer.getSample(kLeftChannel, sampleBufferPos));
				sampleBuffer[whichSampleBufferRecording]->setSample(kRightChannel, sampleLenRecording, buffer.getSample(kRightChannel, sampleBufferPos));
				//
				sampleLenRecording++;
				sampleLen++;
				//
				if (sampleLenRecording >= sampleBuffer[whichSampleBufferRecording]->getNumSamples())
				{
					whichSampleBufferRecording++;
					sampleLenRecording = 0;
					recordingBufferToShow = sampleBuffer[whichSampleBufferRecording];
					recordingThread.notify();
				}
				//
				if (autoStop)
				{
					if (fabs(buffer.getSample(kLeftChannel, sampleBufferPos)) <= inputLevelRecording || fabs(buffer.getSample(kRightChannel, sampleBufferPos)) <= inputLevelRecording) endRecordingCounter++; else endRecordingCounter = 0;
					if (midiEnableRecording == 0.0f && endRecordingCounter > int(lastSampleRate * 4.0f)) finishRecording();
				}
			}
			else
			{
				if (midiEnableRecording == 0.0f && (fabs(buffer.getSample(kLeftChannel, sampleBufferPos)) > inputLevelRecording || fabs(buffer.getSample(kRightChannel, sampleBufferPos)) > inputLevelRecording))
				{
					isRecordingStarted = true;
				}
			}
		}
		else if (isPlaying)
		{
			if (playingPosition > double(sampleLen))
			{
				playingPosition = 0.0;
				isPlaying = false;
			}
			else
			{
				if (playbackAntialias && playingRate != 1.0)
				{
					int intPosition = int(playingPosition);
					float fractionalPosition = playingPosition - float(intPosition);
					//
					buffer.setSample(kLeftChannel, sampleBufferPos, normalizeRecordingValue *	hermite(fractionalPosition,
																									sampleBuffer[0]->getSample(kLeftChannel, intPosition),
																									sampleBuffer[0]->getSample(kLeftChannel, intPosition + 1),
																									sampleBuffer[0]->getSample(kLeftChannel, intPosition + 2),
																									sampleBuffer[0]->getSample(kLeftChannel, intPosition + 3)));
					//
					buffer.setSample(kRightChannel, sampleBufferPos, normalizeRecordingValue *	hermite(fractionalPosition,
																									sampleBuffer[0]->getSample(kRightChannel, intPosition),
																									sampleBuffer[0]->getSample(kRightChannel, intPosition + 1),
																									sampleBuffer[0]->getSample(kRightChannel, intPosition + 2),
																									sampleBuffer[0]->getSample(kRightChannel, intPosition + 3)));
				}
				else
				{
					int xWhich = 0;
					int xPosition = 0;
					//
					if (sampleLen < sampleBuffer[0]->getNumSamples())
					{
						xPosition = int(playingPosition);
					}
					else
					{
						xPosition = int(playingPosition) % sampleBuffer[0]->getNumSamples();
						xWhich = int(playingPosition / double(sampleBuffer[0]->getNumSamples()));
					}
					//
					buffer.setSample(kLeftChannel, sampleBufferPos, normalizeRecordingValue * sampleBuffer[xWhich]->getSample(kLeftChannel, xPosition));
					buffer.setSample(kRightChannel, sampleBufferPos, normalizeRecordingValue *sampleBuffer[xWhich]->getSample(kRightChannel, xPosition));
				}
				//
				playingPosition += playingRate;
				//
				if (loopedMode == 1.0f)
				{
					if (playingPosition >= double(loopEnd * float(sampleLen)))
					{
						playingPosition = double(loopStart * float(sampleLen));
					}
				}
			}
		}
	}
}