// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*

	Created by William Kalfelz @ Wusik Dot Com (c) 2019

	This project is FREE and Open-Souce. You can use and distribute it as long as you keep the mention above.

	www.Wusik.com

*/
//
#include "PluginProcessor.h"
#include "PluginEditor.h"

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
WusikKnob::WusikKnob(WValueLabel* _valueLabel, WusikSp22AudioProcessorEditor* _processorEditor, float* _theValue, Image *_theImage, int _X, int _Y, float _min, float _max, float _rate, int _frameHeight)
	: theImage(_theImage), processorEditor(_processorEditor), hasMoved(false), valueLabel(_valueLabel), initialValue(0.0f), min(_min), max(_max), rate(_rate), theValue(_theValue), isOnOff(false), sampleSize(nullptr)
{
	setComponentID("WusikKnob");
	//
	if (min == 0.0f && max == 1.0f && rate == 1.0f)
	{
		frameHeight = theImage->getHeight() / 2;
		numberOfFrames = 2.0f;
		isOnOff = true;
	}
	else
	{
		frameHeight = _frameHeight;
		numberOfFrames = theImage->getHeight() / frameHeight;
	}
	//
	setBounds(_X, _Y, theImage->getWidth(), frameHeight);
	//
	setWantsKeyboardFocus(true);
	addKeyListener(this);
}
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
String WusikKnob::getText()
{
	if (sampleSize != nullptr) max = *sampleSize;
	if (rate == 1.0f) return String(int(*theValue * max)); else return String(*theValue, 4);
}
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void WusikKnob::paint(Graphics& g)
{
	int xvalue = float(*theValue) * (numberOfFrames - 1);
	g.drawImage(*theImage, 0, 0, getWidth(), getHeight(), 0, xvalue * frameHeight, getWidth(), frameHeight);
}
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void WusikKnob::mouseExit(const MouseEvent& e)
{
	valueLabel->setVisible(false);
}
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void WusikKnob::mouseDown(const MouseEvent& e)
{
	hasMoved = false;
	//
	initialValue = *theValue;
	//
	if (isOnOff)
	{
		if (*theValue > 0.0f) *theValue = 0.0f; else *theValue = 1.0f;
		repaint();
	}
	//
	if (sampleSize != nullptr) processorEditor->repaint();
}
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void WusikKnob::mouseEnter(const MouseEvent& e)
{
	valueLabel->setVisible(false);
	valueLabel->setBounds(0, 0, processorEditor->valueDisplaySize.getX(), processorEditor->valueDisplaySize.getY());
	//
	if (isOnOff) return;
	//
	if (sampleSize != nullptr) max = *sampleSize;
	//
	int pX = getX();
	int pY = getY();
	//
	//
	valueLabel->setTopLeftPosition(
		Point<int>(pX + (getWidth() / 2) - (processorEditor->valueDisplaySize.getX() / 2), pY + getHeight()));
	//
	valueLabel->rectangleBorder = 2.0f;
	valueLabel->rectangleColour = Colours::black;
	valueLabel->textColour = Colours::white;
	valueLabel->textFont = Font("Verdana", 12, 0);
	valueLabel->background = Colours::black.withAlpha(0.82f);
	valueLabel->setText(getText());
	valueLabel->setVisible(true);
	valueLabel->toFront(false);
}
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void WusikKnob::mouseDrag(const MouseEvent& e)
{
	hasMoved = true;
	if (isOnOff) return;
	//
	float multiplyer = 1.0f;
	if (e.mods.isShiftDown() || e.mods.isRightButtonDown()) multiplyer = 0.01f;
	//
	float newValue = jlimit(0.0f, 1.0f, initialValue - (float(e.getDistanceFromDragStartY() * multiplyer * 0.006f)));
	//
	*theValue = newValue;
	//
	valueLabel->setText(getText());
	//
	repaint();
	if (sampleSize != nullptr) processorEditor->repaint();
}
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void WusikKnob::mouseUp(const MouseEvent& e)
{
}
//
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool WusikKnob::keyPressed(const KeyPress &key, Component* component)
{
	if (isOnOff)
	{
		if (sampleSize != nullptr) processorEditor->repaint();
		return false;
	}
	float newValue = *theValue;
	//
	if (sampleSize != nullptr) max = *sampleSize;
	//
	float theRate = 0.01f;
	if (key.getModifiers().isShiftDown()) theRate = 0.001f;
	if (rate == 1.0f)
	{
		theRate = 1.0f / fabs(max - min);
	}
	//
	if (key.getKeyCode() == KeyPress::upKey) newValue += theRate;
	else if (key.getKeyCode() == KeyPress::downKey) newValue -= theRate;
	//
	newValue = jlimit(min, max, newValue);
	*theValue = newValue;
	//
	valueLabel->setText(getText());
	//	
	repaint();
	if (sampleSize != nullptr) processorEditor->repaint();
	return true;
}