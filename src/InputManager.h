#pragma once
#ifndef _INPUT_MANAGER_H
#define _INPUT_MANAGER_H

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
};

#endif // _INPUT_MANAGER_H
