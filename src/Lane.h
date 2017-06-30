#pragma once
#ifndef _LANE_H
#define _LANE_H

#include "Globals.h"

class Lane
{
public:
	void		init();
	void		shut();
	void		draw(const glm::mat4& modelViewProjMtx);
	void		update();

private:
	
};

#endif
