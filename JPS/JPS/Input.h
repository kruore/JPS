#pragma once

#include "Windows.h"

class Input
{
public:
	Input() : isLButtonPress(false), isRButtonPress(false), mousePos{ 0,0 } {};

	POINT mousePos;

	bool isLButtonPress;
	bool isRButtonPress;
	void OnLButtonDown()
	{
		isLButtonPress = true;
	}

	void OnLButtonUp()
	{
		isLButtonPress = false;
	}

	void OnRButtonDown()
	{
		isRButtonPress = true;
	}

	void OnRButtonUp()
	{
		isRButtonPress = false;
	}

	void OnMouseMove(POINT pos)
	{
		mousePos = pos;
	}
};