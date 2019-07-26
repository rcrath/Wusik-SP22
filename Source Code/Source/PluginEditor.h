// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*

	Created by William Kalfelz @ Wusik Dot Com (c) 2019

	This project is FREE and Open-Souce. You can use and distribute it as long as you keep the mention above.

	www.Wusik.com

*/
//
#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
//
enum
{
	kButton1_MIDI_AutoRecord,
	kButton2_Knob,
	kButton3_Loop_OnOff,
	kMAXBUTTONS,
	//
	kOffsetTop = 0,
	kOffsetBottom,
	kOffsetLeft,
	kOffsetRight,
	//
	kScrollingWaveformNone = 0,
	kScrollingWaveformMain,
	kScrollingWaveformThumb
};
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#define MessageBox(dd,ds) AlertWindow::showMessageBox(AlertWindow::NoIcon, dd,ds)
#define ShowValue(value) AlertWindow::showMessageBox(AlertWindow::NoIcon, "","" + String::formatted("%d",value))
#define ShowValueF(value) AlertWindow::showMessageBox(AlertWindow::NoIcon, "","" + String::formatted("%f",value))
#define ConfirmBox(a,b) AlertWindow::showOkCancelBox(AlertWindow::NoIcon, a, b)
#define Alert MessageBox("","Alert!")
#define slash File::getSeparatorString()
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
class WValueLabel : public Component
{
public:
	void paint(Graphics& g)
	{
		g.fillAll(background);
		//
		g.setColour(rectangleColour);
		g.drawRect(0, 0, getWidth(), getHeight(), rectangleBorder);
		//
		g.setFont(textFont);
		g.setColour(textColour);
		g.drawText(text, Rectangle<int>(0, 0, getWidth(), getHeight()), Justification::centred);
	};
	//
	void setText(String _text) 
	{ 
		text = _text;
		repaint(); 
	};
	//
	String text;
	Colour background;
	Colour rectangleColour;
	Colour textColour;
	Font textFont;
	int rectangleBorder;
};
//
class WusikSp22AudioProcessorEditor;
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
class WusikKnob : public Component, public SettableTooltipClient, public KeyListener
{
public:
	WusikKnob(WValueLabel* _valueLabel, WusikSp22AudioProcessorEditor* _processorEditor, float* _theValue, Image *_theImage, int _X, int _Y, float _min, float _max, float _rate, int _frameHeight = 0);
	~WusikKnob() { removeKeyListener(this); };
	//
	void paint(Graphics& g);
	void mouseExit(const MouseEvent& e);
	void mouseDown(const MouseEvent& e);
	void mouseEnter(const MouseEvent& e);
	void mouseDrag(const MouseEvent& e);
	void mouseUp(const MouseEvent& e);
	bool keyPressed(const KeyPress &key, Component* component);
	String getText();
	//
	WValueLabel* valueLabel;
	WusikSp22AudioProcessorEditor* processorEditor;
	Image *theImage;
	float numberOfFrames;
	int frameHeight;
	bool hasMoved;
	float* theValue;
	float initialValue;
	float min, max, rate;
	bool isOnOff;
	Atomic<int>* sampleSize;
};
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
class WusikSp22AudioProcessorEditor  : public AudioProcessorEditor, public Timer
{
public:
    WusikSp22AudioProcessorEditor (WusikSp22AudioProcessor&);
    ~WusikSp22AudioProcessorEditor();
    //
    void paint (Graphics&) override;
    void resized() override;
	static String readGlobalSettings(String program, String key);
	static void saveGlobalSettings(String program, String key, String value);
	void mouseDown(const MouseEvent& event);
	void mouseDrag(const MouseEvent& event);
	void mouseUp(const MouseEvent& event);
	void mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel);
	void timerCallback();
	void createCachedWaveform();
	//
	ScopedPointer<TooltipWindow> toolTipWindow;
	Image background[9];
	Image buttons[kMAXBUTTONS];
	ResizableCornerComponent* resizer;
	ComponentBoundsConstrainer resizeLimits;
	WValueLabel* valueLabel;
	//
	Rectangle<int> buttonOptions, buttonPlay, buttonRecord, buttonStop;
	Point<int> valueDisplaySize;
	int waveformDisplayOffset[4], waveformDisplayW, waveformDisplayH, waveformThumbOffset[4], waveformThumbW, waveformThumbH;
	Image cachedWaveform;
	int waveformEditPosition, waveformFrameSize, waveformEditPositionPrevious;
	int scrollingWaveform;
	String lastExportedFilename;
	//
private:
    WusikSp22AudioProcessor& processor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WusikSp22AudioProcessorEditor)
};
