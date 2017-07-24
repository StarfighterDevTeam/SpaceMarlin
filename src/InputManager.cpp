#include "InputManager.h"

static const sf::Keyboard::Key keyboardUpKey			= sf::Keyboard::Up;
static const sf::Keyboard::Key keyboardDownKey			= sf::Keyboard::Down;
static const sf::Keyboard::Key keyboardLeftKey			= sf::Keyboard::Left;
static const sf::Keyboard::Key keyboardRightKey			= sf::Keyboard::Right;

static const sf::Keyboard::Key keyboardMusicMuted		= sf::Keyboard::M;

static const sf::Keyboard::Key keyboardDebugCamKey		= sf::Keyboard::F1;
static const sf::Keyboard::Key keyboardDebugSlowModeKey	= sf::Keyboard::F2;
static const sf::Keyboard::Key keyboardDebugWireframeKey= sf::Keyboard::F3;
static const sf::Keyboard::Key keyboardDebugCamFrontKey	= sf::Keyboard::Z;
static const sf::Keyboard::Key keyboardDebugCamBackKey	= sf::Keyboard::S;
static const sf::Keyboard::Key keyboardDebugCamLeftKey	= sf::Keyboard::Q;
static const sf::Keyboard::Key keyboardDebugCamRightKey	= sf::Keyboard::D;
static const sf::Keyboard::Key keyboardDebugCamUpKey	= sf::Keyboard::PageUp;
static const sf::Keyboard::Key keyboardDebugCamDownKey	= sf::Keyboard::PageDown;

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
	m_isUpReleased =				false;
	m_wasUpReleasedLastFrame =		false;
	m_isDownReleased =				false;
	m_wasDownReleasedLastFrame =	false;
}

void InputManager::shut()
{
}

void InputManager::update()
{
	m_wasUpReleasedLastFrame = m_isUpReleased;
	m_isUpReleased = !isUpPressed();
	m_wasDownReleasedLastFrame = m_isDownReleased;
	m_isDownReleased = !isDownPressed();
}

bool InputManager::eventIsDebugCamReleased(const sf::Event& evt)
{
	return	(evt.type == sf::Event::KeyReleased && evt.key.code == keyboardDebugCamKey);
}

bool InputManager::eventIsDebugSlowModeReleased(const sf::Event& evt)
{
	return	(evt.type == sf::Event::KeyReleased && evt.key.code == keyboardDebugSlowModeKey);
}

bool InputManager::eventIsDebugWireframeReleased(const sf::Event& evt)
{
	return	(evt.type == sf::Event::KeyReleased && evt.key.code == keyboardDebugWireframeKey);
}

bool InputManager::isMusicMuted(const sf::Event& evt)
{
	return	(evt.type == sf::Event::KeyReleased && evt.key.code == keyboardMusicMuted);
}

bool InputManager::isUpPressed()
{
	return	gData.window->hasFocus() && ((sf::Keyboard::isKeyPressed(keyboardUpKey)) ||
			(!joystickYAxisReversed && sf::Joystick::getAxisPosition(0, sf::Joystick::PovY) > joystickAxisThreshold) ||
			(joystickYAxisReversed && sf::Joystick::getAxisPosition(0, sf::Joystick::PovY) < -joystickAxisThreshold));
}

bool InputManager::isUpTapped()
{
	return m_wasUpReleasedLastFrame && isUpPressed();
}

bool InputManager::isDownPressed()
{
	return	gData.window->hasFocus() && ((sf::Keyboard::isKeyPressed(keyboardDownKey)) ||
			(!joystickYAxisReversed && sf::Joystick::getAxisPosition(0, sf::Joystick::PovY) < -joystickAxisThreshold) ||
			(joystickYAxisReversed && sf::Joystick::getAxisPosition(0, sf::Joystick::PovY) > joystickAxisThreshold));
}

bool InputManager::isDownTapped()
{
	return m_wasDownReleasedLastFrame && isDownPressed();
}

bool InputManager::isLeftPressed()
{
	return gData.window->hasFocus() && (sf::Keyboard::isKeyPressed(keyboardLeftKey) || sf::Joystick::getAxisPosition(0, sf::Joystick::PovX) < -joystickAxisThreshold);
}

bool InputManager::isRightPressed()
{
	return gData.window->hasFocus() && (sf::Keyboard::isKeyPressed(keyboardRightKey) || sf::Joystick::getAxisPosition(0, sf::Joystick::PovX) > joystickAxisThreshold);
}

bool InputManager::isDebugCamFrontPressed()
{
	return gData.window->hasFocus() && (sf::Keyboard::isKeyPressed(keyboardDebugCamFrontKey));
}

bool InputManager::isDebugCamBackPressed()
{
	return gData.window->hasFocus() && (sf::Keyboard::isKeyPressed(keyboardDebugCamBackKey));
}

bool InputManager::isDebugCamLeftPressed()
{
	return gData.window->hasFocus() && (sf::Keyboard::isKeyPressed(keyboardDebugCamLeftKey));
}

bool InputManager::isDebugCamRightPressed()
{
	return gData.window->hasFocus() && (sf::Keyboard::isKeyPressed(keyboardDebugCamRightKey));
}

bool InputManager::isDebugCamUpPressed()
{
	return gData.window->hasFocus() && (sf::Keyboard::isKeyPressed(keyboardDebugCamUpKey));
}

bool InputManager::isDebugCamDownPressed()
{
	return gData.window->hasFocus() && (sf::Keyboard::isKeyPressed(keyboardDebugCamDownKey));
}
