#pragma once
#ifndef _LANE_H
#define _LANE_H

#include "Globals.h"

class Camera;

class Lane
{
public:
	void		init();
	void		shut();
	void		draw(const Camera& camera);
	void		update();

private:
	
};

#endif
