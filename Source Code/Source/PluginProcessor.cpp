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
WusikSp22AudioProcessor::WusikSp22AudioProcessor() : midiMessage(0xf4, 0.0)
{
	sampPos = 0;
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
}
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
WusikSp22AudioProcessor::~WusikSp22AudioProcessor()
{
}
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void WusikSp22AudioProcessor::clearSampleBuffer()
{
	sampleBuffer.setSize(2, int(lastSampleRate * 34.0), false, false, true); // 34 Seconds (last second is not used for recording, but to have a space for drawing //
	sampleBuffer.clear();
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
	xStream.writeInt64(sampleLen.get());
	//
	if (sampleLen.get() > 0)
	{
		for (int xs = 0; xs < sampleLen.get(); xs++)
		{
			xStream.writeFloat(sampleBuffer.getSample(kLeftChannel, xs));
			xStream.writeFloat(sampleBuffer.getSample(kRightChannel, xs));
		}
	}
}
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void WusikSp22AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	clearSampleBuffer();
	//
	isPlaying = false;
	playingPosition.set(0.0);
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
	sampleLen.set(xStream.readInt64());
	//
	sampleBuffer.setSize(2, int(recordedSamplerate * 32.0), false, false, true); // 32 Seconds //
	sampleBuffer.clear();
	//
	if (sampleLen.get() > 0)
	{
		for (int xs = 0; xs < sampleLen.get(); xs++)
		{
			sampleBuffer.setSample(kLeftChannel, xs, xStream.readFloat());
			sampleBuffer.setSample(kRightChannel, xs, xStream.readFloat());
		}
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
					if (midiMessage.isNoteOn())
					{
						clearSampleBuffer();
						endRecordingCounter = 0;
						isRecordingStarted = true;
						sampleLen.set(0);
						recordedSamplerate = lastSampleRate;
					}
					else
					{
						isRecordingStarted = isRecording = false;
						//
						if (getActiveEditor() != nullptr)
						{
							((WusikSp22AudioProcessorEditor*)getActiveEditor())->createCachedWaveform();
							getActiveEditor()->repaint();
						}
					}
				}
				//
				if (!isRecording)
				{
					if (midiMessage.isNoteOn())
					{
						playingPosition.set(0.0);
						playingRate = 1.0;
						isPlaying = true;
						//
						if (!fixedPitchPlayback)
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
						playingPosition.set(0.0);
						isPlaying = false;
					}
				}
			}
		}
		//
		if (isRecording)
		{
			if (isRecordingStarted)
			{
				sampleBuffer.setSample(kLeftChannel, sampleLen.get(), buffer.getSample(kLeftChannel, sampleBufferPos));
				sampleBuffer.setSample(kRightChannel, sampleLen.get(), buffer.getSample(kRightChannel, sampleBufferPos));
				//
				if (sampleLen.get() < (sampleBuffer.getNumSamples() - lastSampleRate)) // To give a second of extra data when drawing //
				{
					sampleLen.set(sampleLen.get() + 1);
					//
					if (fabs(buffer.getSample(kLeftChannel, sampleBufferPos)) <= inputLevelRecording || fabs(buffer.getSample(kRightChannel, sampleBufferPos)) <= inputLevelRecording) endRecordingCounter++; else endRecordingCounter = 0;
					if (midiEnableRecording == 0.0f && endRecordingCounter > int(lastSampleRate * 4.0f))
					{
						isRecordingStarted = isRecording = false;
						sampleLen.set(sampleLen.get() - int(lastSampleRate * 4.0f));
						//
						if (getActiveEditor() != nullptr)
						{
							((WusikSp22AudioProcessorEditor*)getActiveEditor())->createCachedWaveform();
							getActiveEditor()->repaint();
						}
					}
				}
				else
				{
					isRecordingStarted = isRecording = false;
					//
					if (getActiveEditor() != nullptr)
					{
						((WusikSp22AudioProcessorEditor*)getActiveEditor())->createCachedWaveform();
						getActiveEditor()->repaint();
					}
				}
			}
			else
			{
				if (midiEnableRecording == 0.0f && (fabs(buffer.getSample(kLeftChannel, sampleBufferPos)) > inputLevelRecording || fabs(buffer.getSample(kRightChannel, sampleBufferPos)) > inputLevelRecording))
				{
					clearSampleBuffer();
					endRecordingCounter = 0;
					isRecordingStarted = true;
					sampleLen.set(0);
					recordedSamplerate = lastSampleRate;
				}
			}
		}
		else if (isPlaying)
		{
			if (playingPosition.get() > double(sampleLen.get()))
			{
				playingPosition.set(0.0);
				isPlaying = false;
			}
			else
			{
				if (playbackAntialias)
				{
					int intPosition = int(playingPosition.get());
					float fractionalPosition = playingPosition.get() - float(intPosition);
					//
					buffer.setSample(kLeftChannel, sampleBufferPos, normalizeRecordingValue *	hermite(fractionalPosition,
																									sampleBuffer.getSample(kLeftChannel, intPosition),
																									sampleBuffer.getSample(kLeftChannel, intPosition + 1),
																									sampleBuffer.getSample(kLeftChannel, intPosition + 2),
																									sampleBuffer.getSample(kLeftChannel, intPosition + 3)));
					//
					buffer.setSample(kRightChannel, sampleBufferPos, normalizeRecordingValue *	hermite(fractionalPosition,
																									sampleBuffer.getSample(kRightChannel, intPosition),
																									sampleBuffer.getSample(kRightChannel, intPosition + 1),
																									sampleBuffer.getSample(kRightChannel, intPosition + 2),
																									sampleBuffer.getSample(kRightChannel, intPosition + 3)));
				}
				else
				{
					buffer.setSample(kLeftChannel, sampleBufferPos, normalizeRecordingValue * sampleBuffer.getSample(kLeftChannel, int(playingPosition.get())));
					buffer.setSample(kRightChannel, sampleBufferPos, normalizeRecordingValue *sampleBuffer.getSample(kRightChannel, int(playingPosition.get())));
				}
				//
				playingPosition.set(playingPosition.get() + playingRate);
				//
				if (loopedMode == 1.0f)
				{
					if (playingPosition.get() >= double(loopEnd * float(sampleLen.get())))
					{
						playingPosition.set(double(loopStart * float(sampleLen.get())));
					}
				}
			}
		}
	}
}