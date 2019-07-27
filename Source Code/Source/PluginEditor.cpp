// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*

	Created by William Kalfelz @ Wusik Dot Com (c) 2019

	This project is FREE and Open-Souce. You can use and distribute it as long as you keep the mention above.

	www.Wusik.com

*/
//
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Wusik Pink Image.h"

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
WusikSp22AudioProcessorEditor::WusikSp22AudioProcessorEditor (WusikSp22AudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
	scrollingWaveform = kScrollingWaveformNone;
	waveformEditPosition = waveformEditPositionPrevious = 0;
	waveformFrameSize = 1;
	//
	buttonOptions = Rectangle<int>(350, 0, 86, 60);
	buttonPlay = Rectangle<int>(0, 0, 68, 60);
	buttonStop = Rectangle<int>(64, 0, 46, 60);
	buttonRecord = Rectangle<int>(104, 0, 46, 60);
	//
	valueDisplaySize = Point<int>(80, 24);
	//
	waveformDisplayOffset[kOffsetTop] = 60;
	waveformDisplayOffset[kOffsetBottom] = 92;
	waveformDisplayOffset[kOffsetLeft] = 25;
	waveformDisplayOffset[kOffsetRight] = 22;
	waveformDisplayH = waveformDisplayW = 200;
	//
	waveformThumbOffset[kOffsetTop] = 0;
	waveformThumbOffset[kOffsetBottom] = 84;
	waveformThumbOffset[kOffsetLeft] = 25;
	waveformThumbOffset[kOffsetRight] = 22;
	waveformThumbH = 68;
	waveformThumbW = 200;
	//
	buttons[0] = ImageCache::getFromMemory(BinaryData::Button1_png, BinaryData::Button1_pngSize);
	buttons[1] = ImageCache::getFromMemory(BinaryData::Button2_png, BinaryData::Button2_pngSize);
	buttons[2] = ImageCache::getFromMemory(BinaryData::Button3_png, BinaryData::Button3_pngSize);
	//
	Image tempImage = ImageCache::getFromMemory(BinaryData::Background_png, BinaryData::Background_pngSize);
	WusikPinkImage::stripSquare(tempImage, background);
	//
	Rectangle<int> r = Desktop::getInstance().getDisplays().getMainDisplay().userArea;
	resizeLimits.setSizeLimits(780, 326, r.getWidth(), r.getHeight());	
	addAndMakeVisible(resizer = new ResizableCornerComponent(this, &resizeLimits));
	//
	toolTipWindow = new TooltipWindow(this);
	//
	String xW = readGlobalSettings("Wusik SP22", "Window W");
	if (xW.isEmpty()) xW = "900";
	//
	String xH = readGlobalSettings("Wusik SP22", "Window H");
	if (xH.isEmpty()) xH = "600";
	//
	valueLabel = new WValueLabel;
	addChildComponent(valueLabel);
	//
	WusikKnob* newKnob = new WusikKnob(valueLabel, this, &processor.inputLevelRecording, &buttons[kButton2_Knob], 156, 16, 0.0f, 1.0f, 0.0001f, 26);
	newKnob->setTooltip("Record Detection Level");
	addAndMakeVisible(newKnob);
	//
	newKnob = new WusikKnob(valueLabel, this, &processor.midiEnableRecording, &buttons[kButton1_MIDI_AutoRecord], 191, 6, 0.0f, 1.0f, 1.0f);
	newKnob->setTooltip("Record With MIDI Notes");
	addAndMakeVisible(newKnob);
	//
	newKnob = new WusikKnob(valueLabel, this, &processor.loopedMode, &buttons[kButton3_Loop_OnOff], 244, 6, 0.0f, 1.0f, 1.0f);
	newKnob->setTooltip("Looped Mode");
	newKnob->sampleSize = &processor.sampleLen;
	addAndMakeVisible(newKnob);
	//
	newKnob = new WusikKnob(valueLabel, this, &processor.loopStart, &buttons[kButton2_Knob], 278, 16, 0.0f, float(processor.sampleLen.get()), 1.0f, 26);
	newKnob->setTooltip("Loop Start");
	newKnob->sampleSize = &processor.sampleLen;
	addAndMakeVisible(newKnob);
	//
	newKnob = new WusikKnob(valueLabel, this, &processor.loopEnd, &buttons[kButton2_Knob], 308, 16, 0.0f, float(processor.sampleLen.get()), 1.0f, 26);
	newKnob->setTooltip("Loop End");
	newKnob->sampleSize = &processor.sampleLen;
	addAndMakeVisible(newKnob);
	//
	setSize(xW.getIntValue(), xH.getIntValue());
	centreWithSize(xW.getIntValue(), xH.getIntValue());
	//
	prevGUISize.x = getWidth();
	prevGUISize.y = getHeight();
	//
	createCachedWaveform();
	startTimer(50);
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
WusikSp22AudioProcessorEditor::~WusikSp22AudioProcessorEditor()
{
	toolTipWindow = nullptr;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void WusikSp22AudioProcessorEditor::timerCallback()
{
	stopTimer();
	//
	if (prevGUISize.x != getWidth() || prevGUISize.y != getHeight())
	{
		prevGUISize.x = getWidth();
		prevGUISize.y = getHeight();
		createCachedWaveform();
		repaint();
	}
	//
	if ((processor.isRecording || processor.isPlaying) && processor.sampleLen.get() > 0)
	{
		repaint();
		startTimer(10);
	}
	else startTimer(100);
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void WusikSp22AudioProcessorEditor::paint (Graphics& g)
{
	WusikPinkImage::drawImageSquare(g, background, 0, 0, getWidth(), getHeight());
	//
	if (processor.sampleLen.get() > 0 && processor.sampleLen.get() > waveformDisplayW)
	{
		g.setColour(Colours::black.withAlpha(0.82f));
		g.drawLine(waveformDisplayOffset[kOffsetLeft], waveformDisplayOffset[kOffsetTop] - 1 + waveformDisplayH / 2, waveformDisplayW, waveformDisplayOffset[kOffsetTop] + (waveformDisplayH / 2) + 1, 1.0f);
		//
		if (processor.isRecording)
		{
			int prevPosY[2] = { waveformDisplayH / 2, waveformDisplayH / 2 };
			int samplePos = processor.sampleLen.get() -1;
			//
			for (int xs = 0; xs < waveformDisplayW; xs++)
			{
				g.setColour(Colours::red.withAlpha(0.72f));
				//
				int posY = int(jmap<float>(processor.sampleBuffer.getSample(kLeftChannel, samplePos) * processor.normalizeRecordingValue, -1.0f, 1.0f, float(waveformDisplayH), 0.0f));
				g.drawLine(
					waveformDisplayOffset[kOffsetLeft] + xs, waveformDisplayOffset[kOffsetTop] + prevPosY[kLeftChannel],
					waveformDisplayOffset[kOffsetLeft] + xs + 1, waveformDisplayOffset[kOffsetTop] + posY, 1.0f);
				prevPosY[kLeftChannel] = posY;
				//
				g.setColour(Colours::yellow.withAlpha(0.22f));
				//
				posY = int(jmap<float>(processor.sampleBuffer.getSample(kRightChannel, samplePos) * processor.normalizeRecordingValue, -1.0f, 1.0f, float(waveformDisplayH), 0.0f));
				g.drawLine(
					waveformDisplayOffset[kOffsetLeft] + xs, waveformDisplayOffset[kOffsetTop] + prevPosY[kRightChannel],
					waveformDisplayOffset[kOffsetLeft] + xs + 1, waveformDisplayOffset[kOffsetTop] + posY, 1.0f);
				prevPosY[kRightChannel] = posY;
				//
				samplePos--;
				if (samplePos < 0) samplePos = 0;
			}
		}
		else // Not Recording //
		{
			int loopStart = int(processor.loopStart * float(processor.sampleLen.get()));
			int loopEnd = int(processor.loopEnd * float(processor.sampleLen.get()));
			int samplePos = waveformEditPosition;
			int prevPosY[2] = { waveformDisplayH / 2, waveformDisplayH / 2 };
			//
			for (int xs = 0; xs < waveformDisplayW; xs++)
			{
				g.setColour(Colours::black.withAlpha(0.52f));
				if (processor.loopedMode == 1.0f && loopEnd > 0 && samplePos >= loopStart && samplePos <= loopEnd) g.setColour(Colours::red.withAlpha(0.82f));
				//
				int posY = int(jmap<float>(processor.sampleBuffer.getSample(kLeftChannel, samplePos) * processor.normalizeRecordingValue, -1.0f, 1.0f, float(waveformDisplayH), 0.0f));
				g.drawLine(
					waveformDisplayOffset[kOffsetLeft] + xs, waveformDisplayOffset[kOffsetTop] + prevPosY[kLeftChannel],
					waveformDisplayOffset[kOffsetLeft] + xs + 1, waveformDisplayOffset[kOffsetTop] + posY, 1.0f);
				prevPosY[kLeftChannel] = posY;
				//
				g.setColour(Colours::black.withAlpha(0.22f));
				if (processor.loopedMode == 1.0f && loopEnd > 0 && samplePos >= loopStart && samplePos <= loopEnd) g.setColour(Colours::red.withAlpha(0.42f));
				//
				posY = int(jmap<float>(processor.sampleBuffer.getSample(kRightChannel, samplePos) * processor.normalizeRecordingValue, -1.0f, 1.0f, float(waveformDisplayH), 0.0f));
				g.drawLine(
					waveformDisplayOffset[kOffsetLeft] + xs, waveformDisplayOffset[kOffsetTop] + prevPosY[kRightChannel],
					waveformDisplayOffset[kOffsetLeft] + xs + 1, waveformDisplayOffset[kOffsetTop] + posY, 1.0f);
				prevPosY[kRightChannel] = posY;
				//
				samplePos++;
			}
			//
			if (!cachedWaveform.isNull())
			{
				g.setColour(Colours::black);
				g.drawImage(cachedWaveform, waveformThumbOffset[kOffsetLeft], getHeight() - waveformThumbOffset[kOffsetBottom],
					cachedWaveform.getWidth(), cachedWaveform.getHeight(), 0, 0, cachedWaveform.getWidth(), cachedWaveform.getHeight());
				//
				g.setColour(Colours::yellow.withAlpha(0.22f));
				int xPos = jlimit(0, waveformThumbOffset[kOffsetLeft] + cachedWaveform.getWidth() - 24, waveformThumbOffset[kOffsetLeft] + (waveformEditPosition / waveformFrameSize));
				g.fillRect(xPos, getHeight() - waveformThumbOffset[kOffsetBottom], 24, cachedWaveform.getHeight());
				//
				if (processor.loopedMode == 1.0f && loopEnd > 0)
				{
					int loopS = jmap<int>(loopStart, 0, processor.sampleLen.get(), 0, waveformThumbW);
					int loopE = jmap<int>(loopEnd, 0, processor.sampleLen.get(), 0, waveformThumbW);
					//
					g.setColour(Colours::red.withAlpha(0.22f));
					g.fillRect(waveformThumbOffset[kOffsetLeft] + loopS, getHeight() - waveformThumbOffset[kOffsetBottom], loopE - loopS, cachedWaveform.getHeight());
				}
				//
				if (processor.isPlaying)
				{
					int positionX = jmap<int>(processor.playingPosition.get(), 0, processor.sampleLen.get(), 0, waveformThumbW);
					g.setColour(Colours::red.withAlpha(0.82f));
					g.fillRect(waveformThumbOffset[kOffsetLeft] + positionX, getHeight() - waveformThumbOffset[kOffsetBottom], 2, cachedWaveform.getHeight());
				}
			}
		}
	}
	//
	if (processor.isRecording)
	{
		g.setFont(Font("Verdana", 18, 0));
		g.setColour(Colours::red);
		//
		if (!processor.isRecordingStarted) g.drawText("! Waiting To Record !", getBounds(), Justification::centred);
		else g.drawText("! Recording !", getBounds(), Justification::centred);
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void WusikSp22AudioProcessorEditor::resized()
{
	resizer->setBounds(getWidth() - 16, getHeight() - 16, 16, 16);
	//
	saveGlobalSettings("Wusik SP22", "Window W", String(getWidth()));
	saveGlobalSettings("Wusik SP22", "Window H", String(getHeight()));
	//
	waveformDisplayW = getWidth() - waveformDisplayOffset[kOffsetLeft] - waveformDisplayOffset[kOffsetRight];
	waveformDisplayH = getHeight() - waveformDisplayOffset[kOffsetTop] - waveformDisplayOffset[kOffsetBottom];;
	//
	waveformThumbW = getWidth() - waveformThumbOffset[kOffsetLeft] - waveformThumbOffset[kOffsetRight];
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void WusikSp22AudioProcessorEditor::mouseDown(const MouseEvent& event)
{
	scrollingWaveform = kScrollingWaveformNone;
	//
	if (event.x > (getWidth() - 240) && event.y < 68)
	{
		MessageBox("Wusik SP22 (Free Sampler)\n\nCreated by Williamk (c) www.Wusik.com", "Version: " + String(WPRODUCT_VERSION));
	}
	else if (buttonOptions.contains(event.getPosition()))
	{
		//
	}
	else if (buttonPlay.contains(event.getPosition()))
	{
		processor.isPlaying = processor.isRecording = processor.isRecordingStarted = false;
		processor.playingPosition.set(0);
		processor.isPlaying = true;
		startTimer(10);
	}
	else if (buttonStop.contains(event.getPosition()))
	{
		if (processor.isRecording && processor.isRecordingStarted) createCachedWaveform();
		processor.isPlaying = processor.isRecording = processor.isRecordingStarted = false;
		processor.playingPosition.set(0);
	}
	else if (buttonRecord.contains(event.getPosition()))
	{
		processor.isPlaying = false;
		processor.isRecordingStarted = false;
		processor.isRecording = true;
		processor.playingPosition.set(0);
		processor.clearSampleBuffer();
		startTimer(10);
	}
	else if (event.y <= (getHeight() - waveformDisplayOffset[kOffsetBottom]))
	{
		waveformEditPositionPrevious = waveformEditPosition;
		scrollingWaveform = kScrollingWaveformMain;
	}
	else
	{
		scrollingWaveform = kScrollingWaveformThumb;
		waveformEditPosition = jlimit(0, processor.sampleLen.get() - waveformFrameSize, (event.x - waveformThumbOffset[kOffsetLeft]) * waveformFrameSize);
	}
	//
	repaint();
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void WusikSp22AudioProcessorEditor::mouseDrag(const MouseEvent& event)
{
	if (scrollingWaveform == kScrollingWaveformMain)
	{
		int newPosition = jlimit(0, processor.sampleLen.get() - waveformDisplayW, waveformEditPositionPrevious - event.getDistanceFromDragStartX());
		waveformEditPosition = newPosition;
	}
	else if (scrollingWaveform == kScrollingWaveformThumb)
	{
		waveformEditPosition = jlimit(0, processor.sampleLen.get() - waveformDisplayW, (event.x - waveformThumbOffset[kOffsetLeft]) * waveformFrameSize);
	}
	//
	repaint();
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void WusikSp22AudioProcessorEditor::mouseUp(const MouseEvent& event)
{
	if (buttonOptions.contains(event.getPosition()))
	{
		PopupMenu pm;
		//
		pm.addItem(2, "Export 16 Bits");
		pm.addItem(4, "Export 24 Bits");
		pm.addItem(6, "Export 32 Bits");
		pm.addSeparator();
		pm.addItem(20, "Fade In");
		pm.addItem(22, "Fade Out");
		pm.addItem(24, "Quick Fade In (200 samples)");
		pm.addItem(26, "Quick Fade Out (200 samples)");
		pm.addItem(32, "Auto Normalize", true, processor.normalizeRecording == 1.0f);
		pm.addItem(28, "Crossfade");
		//
		int result = pm.show();
		if (result != 0)
		{
			if (result <= 6) // Export //
			{
				int xBits = 16;
				if (result == 4) xBits = 24;
				if (result == 6) xBits = 32;
				//
				String lastExportPath = readGlobalSettings("Wusik SP22", "Last Export Path");
				if (lastExportPath.isEmpty())
				{
					if (readGlobalSettings("Wusik Engine", "Data Path").isNotEmpty())
					{
						lastExportPath = readGlobalSettings("Wusik Engine", "Data Path");
						lastExportPath = File::addTrailingSeparator(lastExportPath) + "Shared" + File::getSeparatorString() + "SoundSets" + File::getSeparatorString() + "Wusik SP22";
						File(lastExportPath).createDirectory();
					}
				}
				if (lastExportPath.isEmpty()) lastExportPath = File::getSpecialLocation(File::SpecialLocationType::commonDocumentsDirectory).getFullPathName();
				//
				FileChooser saveFile("Export to wav file", lastExportPath + File::getSeparatorString() + lastExportedFilename, "*.wav");
				if (saveFile.browseForFileToSave(true))
				{
					File xFile = saveFile.getResult();
					saveGlobalSettings("Wusik SP22", "Last Export Path", File::addTrailingSeparator(xFile.getParentDirectory().getFullPathName()));
					lastExportedFilename = xFile.getFileName();
					//
					ScopedPointer<AudioFormatWriter> xWriter;
					StringPairArray metaVals = WavAudioFormat::createBWAVMetadata("", "", "", Time::getCurrentTime(), 0, "");
					if (processor.loopedMode == 1.0f)
					{
						metaVals.set("NumSampleLoops", "1");
						metaVals.set("Loop0Start", String(processor.loopStart * float(processor.sampleLen.get())));
						metaVals.set("Loop0End", String(processor.loopEnd * float(processor.sampleLen.get())));
					}
					//
					AudioSampleBuffer tempBuffer(processor.sampleBuffer);
					if (processor.normalizeRecording == 1.0f) tempBuffer.applyGain(processor.normalizeRecordingValue);
					//
					WavAudioFormat xWav;
					ScopedPointer<FileOutputStream> fileWavOutStream = xFile.createOutputStream();
					xWriter = xWav.createWriterFor(fileWavOutStream, processor.recordedSamplerate, 2, xBits, metaVals, 0);
					xWriter->writeFromAudioSampleBuffer(tempBuffer, 0, processor.sampleLen.get());
					fileWavOutStream.release();
					//
					xWriter->flush();
					xWriter = nullptr;
				}
			}
			else if (result == 28) // Crossfade //
			{
				int loopStart = int(processor.loopStart * float(processor.sampleLen.get()));
				int loopEnd = int(processor.loopEnd * float(processor.sampleLen.get()));
				int loopLen = loopEnd - loopStart;
				//
				if (loopLen > 0 && processor.loopedMode == 0.0f || processor.loopEnd == 0.0f || loopStart < loopLen)
				{
					MessageBox("Not Looped", "You haven't set the loop or the loop size doesn't have room for crossfading!");
				}
				else
				{
					AudioSampleBuffer tempBuffer(processor.sampleBuffer);
					processor.sampleBuffer.applyGainRamp(loopStart, loopLen , 1.0f, 0.0f);
					//
					processor.sampleBuffer.addFromWithRamp(kLeftChannel, loopStart, tempBuffer.getReadPointer(kLeftChannel), loopLen, 0.0f, 1.0f);
					processor.sampleBuffer.addFromWithRamp(kRightChannel, loopStart, tempBuffer.getReadPointer(kRightChannel), loopLen, 0.0f, 1.0f);
					//
					createCachedWaveform();
				}
			}
			else if (result == 32) // Normalize //
			{
				if (processor.normalizeRecording == 0.0f) processor.normalizeRecording = 1.0f; else processor.normalizeRecording = 0.0f;
				//
				createCachedWaveform();
			}
			else if (result >= 20) // Fade In / Out //
			{
				if (result == 20) processor.sampleBuffer.applyGainRamp(0, processor.sampleLen.get(), 0.0f, 1.0f); // Fade In //
				else if (result == 22) processor.sampleBuffer.applyGainRamp(0, processor.sampleLen.get(), 1.0f, 0.0f); // Fade Out //
				else if (result == 24) processor.sampleBuffer.applyGainRamp(0, 200, 0.0f, 1.0f); // Quick Fade Int //
				else if (result == 26) processor.sampleBuffer.applyGainRamp(processor.sampleLen.get() - 200, 200, 1.0f, 0.0f); // Quick Fade Out //
				//
				createCachedWaveform();
			}
		}
	}
	//
	repaint();
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void WusikSp22AudioProcessorEditor::mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel)
{
	waveformEditPosition = jlimit(0, processor.sampleLen.get() - waveformDisplayW, waveformEditPosition - int(wheel.deltaY * waveformFrameSize));
	//
	repaint();
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void WusikSp22AudioProcessorEditor::createCachedWaveform()
{
	scrollingWaveform = kScrollingWaveformNone;
	waveformEditPosition = waveformEditPositionPrevious = 0;
	//
	if (processor.sampleLen.get() > 0)
	{
		cachedWaveform = Image(Image::ARGB, waveformThumbW, waveformThumbH, true);
		Graphics g(cachedWaveform);
		//
		g.setColour(Colours::black.withAlpha(0.88f));
		g.drawLine(0, (waveformThumbH / 2) - 1, waveformThumbW, (waveformThumbH / 2) + 1, 1.0f);
		//
		if (processor.normalizeRecording == 0.0f) processor.normalizeRecordingValue = 1.0f;
			else processor.normalizeRecordingValue = 1.0f / processor.sampleBuffer.getMagnitude(0, processor.sampleLen.get());
		//
		if (processor.sampleLen.get() < waveformThumbW)
		{
			waveformFrameSize = 1;
			//
			for (int xs = 0; xs < processor.sampleLen.get(); xs++)
			{
				g.setColour(Colours::black.withAlpha(0.72f));
				int posY = processor.sampleBuffer.getSample(kLeftChannel, xs) * processor.normalizeRecordingValue * float(waveformThumbH / 2);
				g.drawLine(xs, waveformThumbH / 2, xs, (waveformThumbH / 2) + posY, 2.0f);
				//
				g.setColour(Colours::black.withAlpha(0.22f));
				posY = processor.sampleBuffer.getSample(kRightChannel, xs) * processor.normalizeRecordingValue * float(waveformThumbH / 2);
				g.drawLine(xs, waveformThumbH / 2, xs, (waveformThumbH / 2) + posY, 2.0f);
			}
		}
		else
		{
			int position = 0;
			waveformFrameSize = int(float(processor.sampleLen.get()) / float(waveformThumbW));
			//
			for (int xs = 0; xs < (waveformThumbW); xs++)
			{
				float xValue[2] = { 0.0f, 0.0f };
				//
				for (int xf = 0; xf < waveformFrameSize; xf++)
				{
					float tempValue = fabs(processor.sampleBuffer.getSample(kLeftChannel, position) * processor.normalizeRecordingValue);
					if (tempValue > xValue[kLeftChannel]) xValue[kLeftChannel] = tempValue;
					tempValue = fabs(processor.sampleBuffer.getSample(kRightChannel, position) * processor.normalizeRecordingValue);
					if (tempValue > xValue[kRightChannel]) xValue[kRightChannel] = tempValue;
					position++;
				}
				//
				g.setColour(Colours::black.withAlpha(0.72f));
				int posY = xValue[kLeftChannel] * float(waveformThumbH / 2);
				g.drawLine(xs, waveformThumbH / 2, xs, (waveformThumbH / 2) + posY, 2.0f);
				//
				g.setColour(Colours::black.withAlpha(0.22f));
				posY = xValue[kRightChannel] * float(waveformThumbH / 2);
				g.drawLine(xs, waveformThumbH / 2, xs, (waveformThumbH / 2) + posY, 2.0f);
				//
				g.setColour(Colours::black.withAlpha(0.72f));
				posY = -xValue[kLeftChannel] * float(waveformThumbH / 2);
				g.drawLine(xs, waveformThumbH / 2, xs, (waveformThumbH / 2) + posY, 2.0f);
				//
				g.setColour(Colours::black.withAlpha(0.22f));
				posY = -xValue[kRightChannel] * float(waveformThumbH / 2);
				g.drawLine(xs, waveformThumbH / 2, xs, (waveformThumbH / 2) + posY, 2.0f);
			}
		}
	}
}
//
// ------------------------------------------------------------------------------------------------------------------------------ -
// ------------------------------------------------------------------------------------------------------------------------------ -
// ------------------------------------------------------------------------------------------------------------------------------ -
#if !JUCE_WINDOWS 
	#define RG_BASE_FILENAME "~/Library/Preferences/com.Wusik."
#endif
// ------------------------------------------------------------------------------------------------------------------------------ -
String WusikSp22AudioProcessorEditor::readGlobalSettings(String program, String key)
{
	#if JUCE_WINDOWS & !RG_FORCE_TO_FILES
		if (WindowsRegistry::keyExists("HKEY_CURRENT_USER\\Software\\Wusik\\" + program + "\\" + key) &&
			WindowsRegistry::getValue("HKEY_CURRENT_USER\\Software\\Wusik\\" + program + "\\" + key).isNotEmpty())
		{
			return WindowsRegistry::getValue("HKEY_CURRENT_USER\\Software\\Wusik\\" + program + "\\" + key);
		}
	#else
		program = program.replaceCharacter(' ', '.');
		key = key.replaceCharacter(' ', '.');
		//
		File theFile(RG_BASE_FILENAME + program + "." + key + ".configurations");
		if (theFile.existsAsFile())
		{
			ScopedPointer<InputStream> fileStream = theFile.createInputStream();
			static String returnReagGB;
			returnReagGB = fileStream->readString();
			fileStream = nullptr;
			return returnReagGB;
		}
	#endif
	//
	return String();
}

// ------------------------------------------------------------------------------------------------------------------------------ -
void WusikSp22AudioProcessorEditor::saveGlobalSettings(String program, String key, String value)
{
	#if JUCE_WINDOWS & !RG_FORCE_TO_FILES
		WindowsRegistry::setValue("HKEY_CURRENT_USER\\Software\\Wusik\\" + program + "\\" + key, value);
	#else
		program = program.replaceCharacter(' ', '.');
		key = key.replaceCharacter(' ', '.');
		//
		File theFile(RG_BASE_FILENAME + program + "." + key + ".configurations");
		if (theFile.existsAsFile()) theFile.deleteFile();
		ScopedPointer<OutputStream> fileStream = theFile.createOutputStream();
		fileStream->writeString(value);
		fileStream->flush();
		fileStream = nullptr;
	#endif
}

