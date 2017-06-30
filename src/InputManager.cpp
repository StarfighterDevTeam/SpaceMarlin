#include "Globals.h"
#include "InputManager.h"

static const sf::Keyboard::Key keyboardUpKey		= sf::Keyboard::Up;
static const sf::Keyboard::Key keyboardDownKey		= sf::Keyboard::Down;
static const sf::Keyboard::Key keyboardLeftKey		= sf::Keyboard::Left;
static const sf::Keyboard::Key keyboardRightKey		= sf::Keyboard::Right;

#ifdef SFML_RPI
	static const bool joystickYAxisReversed = true;
#else
	static const bool joystickYAxisReversed = false;
#endif
static const float joystickAxisThreshold = 10.f;
static const int joystickButtonValidate = 2;
static const int joystickButtonBonus	= 0;	// TODO: not tested
static const int joystickButtonFight	= 3;
static const int joystickButtonLottery	= 3;
static const int joystickButtonJump		= 1;

void InputManager::init()
{
}

void InputManager::shut()
{
}

void InputManager::update()
{
}

bool InputManager::isUpPressed()
{
	return	(sf::Keyboard::isKeyPressed(keyboardUpKey)) ||
			(!joystickYAxisReversed && sf::Joystick::getAxisPosition(0, sf::Joystick::PovY) > joystickAxisThreshold) ||
			(joystickYAxisReversed && sf::Joystick::getAxisPosition(0, sf::Joystick::PovY) < -joystickAxisThreshold);
}

bool InputManager::isDownPressed()
{
	return	(sf::Keyboard::isKeyPressed(keyboardDownKey)) ||
			(!joystickYAxisReversed && sf::Joystick::getAxisPosition(0, sf::Joystick::PovY) < -joystickAxisThreshold) ||
			(joystickYAxisReversed && sf::Joystick::getAxisPosition(0, sf::Joystick::PovY) > joystickAxisThreshold);
}

bool InputManager::isLeftPressed()
{
	return sf::Keyboard::isKeyPressed(keyboardLeftKey) || sf::Joystick::getAxisPosition(0, sf::Joystick::PovX) < -joystickAxisThreshold;
}

bool InputManager::isRightPressed()
{
	return sf::Keyboard::isKeyPressed(keyboardRightKey) || sf::Joystick::getAxisPosition(0, sf::Joystick::PovX) > joystickAxisThreshold;
}
