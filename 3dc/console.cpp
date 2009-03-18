/*
	Console system. Heavily influenced by Quake3's code :)
*/

#include <string>
#include <vector>
#include <sstream>

#include "console.h"
#include "d3_func.h"

extern "C" 
{
	#include "avp_menugfx.hpp"
}

#define CHAR_WIDTH	15
#define CHAR_HEIGHT	15

#define ONE_FIXED	65536

char buf[100];

struct Console 
{
	bool isActive;

	int xPos;
	int yPos;
	int lines;
	int lineWidth;

	int indent;

	int width;
	int height;

	int time;
	int destinationY;

	std::vector<std::string> text;
	std::string	inputLine;
};

//extern Console console;
Console console;

void Con_AddLine(std::string temp)
{
	console.text.push_back(temp);
}

void Con_Init()
{
	console.isActive = false;
	console.xPos = 0;
	console.yPos = 0;
	console.width = 640;

	console.lines = 240 / CHAR_HEIGHT;
	console.lineWidth = 640 / CHAR_WIDTH;

	console.height = console.lines * CHAR_HEIGHT;
	console.indent = CHAR_WIDTH / 2;

	sprintf(buf, "console height: %d num lines: %d\n", console.lines * CHAR_HEIGHT, console.lines);
	OutputDebugString(buf);

	Con_AddLine("Blah");
	Con_AddLine("here is another line");
	Con_AddLine("Initialisation failed! stuff broke horribly :(");
	Con_AddLine("fooooooooooood!");
	Con_AddLine("testing");
	Con_AddLine("1");
	Con_AddLine("2");
	Con_AddLine("big long test string");

//	LoadConsoleFont();

	OutputDebugString("console initialised\n");
}

void Con_ProcessCommand()
{
	/* if nothing typed, don't bother doing anything */
	if (console.inputLine.length() == 0) return;

	std::string temp;
	std::stringstream stream(console.inputLine.c_str());
//	std::istream stream(&console.inputLine);

	getline(stream, temp, ' ');

	OutputDebugString(temp.c_str());

	/* grab the command itself (first word before space) */

}

bool prevEnter = false;
bool prevBackspace = false;

/* handle user pressing enter key. make this less horrible */
void Con_Key_Enter(bool state)
{
//	if (prevEnter == false && state == true)
	{
		Con_ProcessCommand();
		OutputDebugString("console ENTER\n");
	}
	prevEnter = state;
}

void Con_Key_Backspace(bool state)
{
	Con_RemoveTypedChar();
}

void Con_Toggle()
{
	return;
	OutputDebugString("console toggle\n");
	console.isActive = !console.isActive;
	console.time = 250;

	if (console.height > 0) console.destinationY = 0;
	else console.destinationY = console.lines * CHAR_HEIGHT;
}

bool Con_IsActive()
{
	return console.isActive;
}

void Con_CheckResize()
{
/*
	if (console.width != ScreenDescriptorBlock.SDB_Width)
	{
		console.width = ScreenDescriptorBlock.SDB_Width;
	}
*/
}

void Con_AddTypedChar(char c)
{
	if(!console.isActive) return;

	if (c == 0x08)
	{
		Con_Key_Backspace(true);
		return;
	}

	if (c == 13)
	{	
		Con_Key_Enter(true);
		return;
	}
	sprintf(buf, "Con_AddTypedChar: %c\n", c);
	OutputDebugString(buf);

	console.inputLine.push_back(c);
}

void Con_RemoveTypedChar()
{
	if (console.inputLine.length())
	{
		console.inputLine.resize(console.inputLine.length() - 1, 1);
	}
}

void Con_Draw()
{
	int charWidth = 0;

	/* is console moving to a new position? */
	if (console.destinationY > console.height)
	{
		console.height+=CHAR_HEIGHT;
	}
	else if (console.destinationY < console.height)
	{
		console.height-=CHAR_HEIGHT;
	}

	DrawQuad(console.xPos, console.yPos, console.width, console.height, D3DCOLOR_ARGB(255, 80, 160, 120));

	DrawQuad(console.xPos, console.yPos+console.height, console.width, 2, D3DCOLOR_ARGB(255, 128, 160, 25));

	int charCount = 0;

	/* draw input cusor */
	charWidth = RenderSmallChar('>', console.indent, console.height - CHAR_HEIGHT, ONE_FIXED, ONE_FIXED, ONE_FIXED, ONE_FIXED);
	RenderSmallChar('_', console.indent + charWidth, console.height - CHAR_HEIGHT, ONE_FIXED, ONE_FIXED, ONE_FIXED, ONE_FIXED);

	int rows = console.text.size() - 1;

	int y = console.height - CHAR_HEIGHT * 2;

	int lines = console.lines;
	int xOffset = 0;

	for (int i = rows; i >= 0; i--, y-=CHAR_HEIGHT)
	{
		xOffset = 0;
		charWidth = 0;

		for(int j = 0; j < console.text[i].length(); j++)
		{
			//if ((j * CHAR_WIDTH) > console.lineWidth) break;
			charWidth = RenderSmallChar(console.text[i].at(j), console.indent + /*(xOffset * j)*/xOffset, y, ONE_FIXED, ONE_FIXED / 2, ONE_FIXED, ONE_FIXED);
			xOffset+=charWidth;
		}
	}

	xOffset = CHAR_WIDTH * 2;
	charWidth = 0;

	for(int j = 0; j < console.inputLine.length(); j++)
	{
		//if ((j * CHAR_WIDTH) > console.lineWidth) break;
		charWidth = RenderSmallChar(console.inputLine.at(j), console.indent + /*(xOffset * j)*/xOffset, console.height - CHAR_HEIGHT, ONE_FIXED, ONE_FIXED, ONE_FIXED, ONE_FIXED);
		xOffset+=charWidth;
	}
}