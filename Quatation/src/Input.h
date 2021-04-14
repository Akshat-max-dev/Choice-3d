#pragma once
#ifndef INPUT_H_
#define INPUT_H_
#include "cpch.h"

#include <glm/glm.hpp>

#include "Choice.h"

namespace choice
{
	namespace Key
	{
		enum : uint16_t
		{
			//From glfw3.h
			SPACE             = 32,
			APOSTROPHE        = 39,  /* ' */
			COMMA             = 44,  /* , */
			MINUS             = 45,  /* - */
			PERIOD            = 46,  /* . */
			SLASH             = 47,  /* / */
			NUM0              = 48,
			NUM1              = 49,
			NUM2              = 50,
			NUM3              = 51,
			NUM4              = 52,
			NUM5              = 53,
			NUM6              = 54,
			NUM7              = 55,
			NUM8              = 56,
			NUM9              = 57,
			SEMICOLON         = 59,  /* ; */
			EQUAL             = 61,  /* = */
			A                 = 65,
			B                 = 66,
			C                 = 67,
			D                 = 68,
			E                 = 69,
			F                 = 70,
			G                 = 71,
			H                 = 72,
			I                 = 73,
			J                 = 74,
			K                 = 75,
			L                 = 76,
			M                 = 77,
			N                 = 78,
			O                 = 79,
			P                 = 80,
			Q                 = 81,
			R                 = 82,
			S                 = 83,
			T                 = 84,
			U                 = 85,
			V                 = 86,
			W                 = 87,
			X                 = 88,
			Y                 = 89,
			Z                 = 90,
			LEFTBRACKET       = 91,  /* [ */
			BACKSLASH         = 92,  /* \ */
			RIGHTBRACKET      = 93,  /* ] */
			GRAVEACCENT       = 96,  /* ` */
			WORLD1            = 161, /* non-US #1 */
			WORLD2            = 162, /* non-US #2 */

			/* Function keys */
			ESCAPE            = 256,
			ENTER             = 257,
			TAB               = 258,
			BACKSPACE         = 259,
			INSERT            = 260,
			KDELETE           = 261,
			RIGHT             = 262,
			LEFT              = 263,
			DOWN              = 264,
			UP                = 265,
			PAGEUP            = 266,
			PAGEDOWN          = 267,
			HOME              = 268,
			END               = 269,
			CAPSLOCK          = 280,
			SCROLLLOCK        = 281,
			NUMLOCK           = 282,
			PRINTSCREEN       = 283,
			PAUSE             = 284,
			F1                = 290,
			F2                = 291,
			F3                = 292,
			F4                = 293,
			F5                = 294,
			F6                = 295,
			F7                = 296,
			F8                = 297,
			F9                = 298,
			F10               = 299,
			F11               = 300,
			F12               = 301,
			F13               = 302,
			F14               = 303,
			F15               = 304,
			F16               = 305,
			F17               = 306,
			F18               = 307,
			F19               = 308,
			F20               = 309,
			F21               = 310,
			F22               = 311,
			F23               = 312,
			F24               = 313,
			F25               = 314,
			KP0               = 320,
			KP1               = 321,
			KP2               = 322,
			KP3               = 323,
			KP4               = 324,
			KP5               = 325,
			KP6               = 326,
			KP7               = 327,
			KP8               = 328,
			KP9               = 329,
			KPDECIMAL         = 330,
			KPDIVIDE          = 331,
			KPMULTIPLY        = 332,
			KPSUBTRACT        = 333,
			KPADD             = 334,
			KPENTER           = 335,
			KPEQUAL           = 336,
			LEFTSHIFT         = 340,
			LEFTCONTROL       = 341,
			LEFTALT           = 342,
			LEFTSUPER         = 343,
			RIGHTSHIFT        = 344,
			RIGHTCONTROL      = 345,
			RIGHTALT          = 346,
			RIGHTSUPER        = 347,
			MENU              = 348
		};
	}

	namespace Mouse
	{
		enum : uint16_t
		{
			BUTTON1        = 0,
			BUTTON2        = 1,
			BUTTON3        = 2,
			BUTTON4        = 3,
			BUTTON5        = 4,
			BUTTON6        = 5,
			BUTTON7        = 6,
			BUTTON8        = 7,
			BUTTONLAST     = BUTTON8,
			BUTTONLEFT     = BUTTON1,
			BUTTONRIGHT    = BUTTON2,
			BUTTONMIDDLE   = BUTTON3
		};
	}

	class Input
	{
	public:
		static bool IsKeyPressed(int key)
		{
			auto action = glfwGetKey(Choice::Instance()->GetWindow()->GetWindow(), key);
			return action == GLFW_PRESS;
		}

		static bool IsButtonPressed(int button)
		{
			auto action = glfwGetMouseButton(Choice::Instance()->GetWindow()->GetWindow(), button);
			return action == GLFW_PRESS;
		}

		static float GetMouseX() { return GetMousePosition().x; }
		static float GetMouseY() { return GetMousePosition().y; }
	private:
		static glm::vec2 GetMousePosition()
		{
			double xpos, ypos;
			glfwGetCursorPos(Choice::Instance()->GetWindow()->GetWindow(), &xpos, &ypos);
			return{ (float)xpos, (float)ypos };
		}
	};
}
#endif