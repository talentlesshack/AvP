/*

  Platform specific project specific C functions

*/

#include "3dc.h"
#include "module.h"
#include "inline.h"

#include "gameplat.h"
#include "gamedef.h"

#include "dynblock.h"
#include "dynamics.h"
#define UseLocalAssert FALSE
#include "ourasert.h"

/*
	Externs from pc\io.c
*/

extern int InputMode;
extern unsigned char KeyboardInput[];
extern unsigned char DebouncedKeyboardInput[MAX_NUMBER_OF_INPUT_KEYS];

extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;

extern  unsigned char KeyASCII;

// Prototypes

int IDemandFireWeapon(void);
int IDemandNextWeapon(void);
int IDemandPreviousWeapon(void);


// Functions



void catpathandextension(char* dst, char* src)
{
	int len = lstrlen(dst);

	if ((len > 0 && (dst[len-1] != '\\' && dst[len-1] != '/')) && *src != '.')
		{
			lstrcat(dst,"\\");
		}

    lstrcat(dst,src);

/*
	The second null here is to support the use
	of SHFileOperation, which is a Windows 95
	addition that is uncompilable under Watcom
	with ver 10.5, but will hopefully become
	available later...
*/
    len = lstrlen(dst);
    dst[len+1] = 0;

}


/* game platform definition of the Mouse Mode*/

int MouseMode = MouseVelocityMode;

/*

  Real PC control functions

*/

int IDemandLookUp(void)
{
	return FALSE;
}


int IDemandLookDown(void)
{
	return FALSE;
}

int IDemandTurnLeft(void)
{
	InputMode = Digital;
	if(KeyboardInput[KEY_LEFT] || KeyboardInput[KEY_JOYSTICK_BUTTON_15])
		return TRUE;
	return FALSE;
}


int IDemandTurnRight(void)
{
	InputMode = Digital;
	if(KeyboardInput[KEY_RIGHT] || KeyboardInput[KEY_JOYSTICK_BUTTON_16])
		return TRUE;
	return FALSE;
}


int IDemandGoForward(void)
{
	InputMode = Digital;
	if(KeyboardInput[KEY_UP] || KeyboardInput[KEY_JOYSTICK_BUTTON_13])
		return TRUE;
	return FALSE;
}


int IDemandGoBackward(void)
{
	InputMode = Digital;
	if(KeyboardInput[KEY_DOWN] || KeyboardInput[KEY_JOYSTICK_BUTTON_14])
		return TRUE;
	return FALSE;
}

int IDemandJump(void)
{
	InputMode = Digital;
        if(KeyboardInput[KEY_CAPS])
		return TRUE;
	return FALSE;
}

int IDemandCrouch(void)
{
	InputMode = Digital;
        if(KeyboardInput[KEY_Z])
		return TRUE;
	return FALSE;
}

int IDemandSelect(void)
{
	InputMode = Digital;
    
    if (KeyboardInput[KEY_CR]) 
		return TRUE;
    if (KeyboardInput[KEY_SPACE]) 
		return TRUE;
	if (DebouncedKeyboardInput[KEY_JOYSTICK_BUTTON_2])
		return TRUE; // bjd - A button?
	else 
		return FALSE;
}

int IDemandStop(void)
{
	return FALSE;
}


int IDemandFaster(void)
{
	InputMode = Digital;
	if(KeyboardInput[KEY_LEFTSHIFT])
		return TRUE;
	return FALSE;
}


int IDemandSideStep(void)
{
	InputMode = Digital;
	if(KeyboardInput[KEY_LEFTALT])
		return TRUE;
	return FALSE;
}

int IDemandPickupItem(void)
{
	InputMode = Digital;
	if(KeyboardInput[KEY_P])
		return TRUE;
	return FALSE;
}

int IDemandDropItem(void)
{
	InputMode = Digital;
	if(KeyboardInput[KEY_D])
		return TRUE;
	return FALSE;
}

int IDemandMenu(void)
{
	InputMode = Digital;
	if(KeyboardInput[KEY_M])
		return TRUE;
	return FALSE;
}

int IDemandOperate(void)
{
	InputMode = Digital;
	if(KeyboardInput[KEY_SPACE])
		return TRUE;
	return FALSE;
}



int IDemandFireWeapon(void)
{
	InputMode = Digital;
        if(KeyboardInput[KEY_CR])
		return TRUE;
	return FALSE;
}

/* KJL 11:29:12 10/07/96 - added by me */
int IDemandPreviousWeapon(void)
{
	InputMode = Digital;
   	if(KeyboardInput[KEY_1]) return TRUE;
    else return FALSE;
}
int IDemandNextWeapon(void)
{
	InputMode = Digital;
   	if(KeyboardInput[KEY_2]) return TRUE;
    else return FALSE;
}

int IDemandChangeEnvironment()
{
	InputMode = Digital;

	if(KeyboardInput[KEY_F1])
		return 0;
	else if(KeyboardInput[KEY_F2])
		return 1;
	else if(KeyboardInput[KEY_F3])
		return 2;
	else if(KeyboardInput[KEY_F4])
		return 3;
	else if(KeyboardInput[KEY_F5])
		return 4;
	else
		return(-1);
}






#if 0
/*KJL***************************************
*           HUD MAP DISPLAY CODE           *
***************************************KJL*/
#include "hud_map.h"
static unsigned int GreyColour;
static unsigned int WhiteColour;
static unsigned int RedColour;
extern int ScanDrawMode;
extern int NumVertices;
extern int ZBufferMode;

void PlatformSpecificInitHUDMap(void)
{
	if (ScanDrawMode != ScanDrawDirectDraw)
	{
		GreyColour = /*GetSingleColourForPrimary(*/0x007f7f7f/*)*/;
		WhiteColour = /*GetSingleColourForPrimary(*/0x00ffffff/*)*/;
		RedColour =  /*GetSingleColourForPrimary(*/0x007f0000/*)*/;
	}
	else
	{
		extern unsigned char TestPalette[];
		GreyColour = NearestColour(31,31,31, TestPalette);
		WhiteColour = NearestColour(63,63,63, TestPalette);
		RedColour = NearestColour(63,0,0, TestPalette);
	}
}
void DrawHUDMapLine(VECTOR2D *vertex1, VECTOR2D *vertex2, enum MAP_COLOUR_ID colourID)
{
	unsigned int colourIndex;
	switch(colourID)
	{
		default:
		case MAP_COLOUR_WHITE:
		{
			colourIndex = WhiteColour;
			break;
		}	
		
		case MAP_COLOUR_GREY:
		{
			colourIndex = GreyColour;
			break;
		}	
		
		case MAP_COLOUR_RED:
		{
			colourIndex = RedColour;
			break;
		}	

	}

	if (ScanDrawMode != ScanDrawDirectDraw)
	{
		if (ZBufferOn!=ZBufferMode)
		{
			DirectWriteD3DLine(vertex1,vertex2,colourIndex);
			/* 
			The offset by 24 is a bodge until Microsoft work out 
			what the fuck's going on...
			*/ /* Neal's comment, not mine... :)  KJL */
			if ((ScanDrawMode != ScanDrawDirectDraw) && (NumVertices > (MaxD3DVertices-24))) 
			{
				UnlockExecuteBufferAndPrepareForUse();
				ExecuteBuffer();
				LockExecuteBuffer();
			}
		}
		else D3D_Line(vertex1,vertex2,colourIndex);

	}
	else
	{
	  	Draw_Line_VMType_8(vertex1,vertex2,colourIndex);
  	}
}


void PlatformSpecificEnteringHUDMap(void)
{
	/* this is here to make sure that the right colours
	   are chosen from the palette */
	PlatformSpecificInitHUDMap(); 
}

void PlatformSpecificExitingHUDMap(void)
{
}

/*KJL***************************************
*           HUD MAP DISPLAY CODE           *
***************************************KJL*/
#endif




/* KJL 15:53:52 05/04/97 - 
Loaders/Unloaders for language internationalization code in language.c */

char *LoadTextFile(char *filename)
{
	char *bufferPtr;
	long int save_pos, size_of_file;
	FILE *fp;

	fp = fopen(filename,"rb");
	
	if (!fp) goto error;

	save_pos=ftell(fp);
	fseek(fp,0L,SEEK_END);
	size_of_file=ftell(fp);
	
	bufferPtr = AllocateMem(size_of_file);
	if (!bufferPtr)
	{
		memoryInitialisationFailure = 1;
		goto error;
	}

	fseek(fp,save_pos,SEEK_SET);

	
	if (!fread(bufferPtr, size_of_file,1,fp))
	{
		fclose(fp);
		goto error;
	}
			
	fclose(fp);
	return bufferPtr;
	
error:
	{
		/* error whilst trying to load file */
		textprint("Error! Can not load file %s.\n",filename);
		LOCALASSERT(0);
		return 0;
	}
}


void UnloadTextFile(char *filename, char *bufferPtr)
{
	if (bufferPtr) DeallocateMem(bufferPtr);
}
