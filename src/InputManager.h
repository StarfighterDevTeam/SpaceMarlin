#pragma once
#ifndef _INPUT_MANAGER_H
#define _INPUT_MANAGER_H

#include "Globals.h"

class InputManager
{
public:
	void	init();
	void	shut();
	void	update();

	static bool eventIsDebugCamReleased(const sf::Event& evt);

	bool	isUpPressed();
	bool	isDownPressed();
	bool	isLeftPressed();
	bool	isRightPressed();

	bool	isDebugCamFrontPressed();
	bool	isDebugCamBackPressed();
	bool	isDebugCamLeftPressed();
	bool	isDebugCamRightPressed();
	bool	isDebugCamUpPressed();
	bool	isDebugCamDownPressed();
};

#endif // _INPUT_MANAGER_H
