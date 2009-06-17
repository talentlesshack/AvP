#include "3dc.h"
#include "module.h"
#include "platform.h"
#include "kshape.h"
#include "progress_bar.h"
#include "chnktexi.h"
#include "awtexld.h"
#include "ffstdio.h"
#include "inline.h"
#include "gamedef.h"
#include "psnd.h"

extern "C"
{
#include "language.h"
#include "d3d_render.h"

extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;
extern unsigned char DebouncedGotAnyKey;

extern void MinimalNetCollectMessages(void);
extern void NetSendMessages(void);
extern void RenderGrabbedScreen(void);

extern void ThisFramesRenderingHasBegun(void);
extern void ThisFramesRenderingHasFinished(void);

extern IMAGEHEADER ImageHeaderArray[];

extern int AAFontImageNumber;
extern int FadingGameInAfterLoading;
extern void RenderBriefingText(int centreY, int brightness);
};

static int CurrentPosition=0;
static int BarLeft;
static int BarRight;
static int BarTop;
static int BarBottom;

static const char* Loading_Image_Name="Menus\\Loading.rim";
static const char* Loading_Bar_Empty_Image_Name="Menus\\Loadingbar_empty.rim";
static const char* Loading_Bar_Full_Image_Name="Menus\\Loadingbar_full.rim";

RECT LoadingBarEmpty_DestRect;
RECT LoadingBarEmpty_SrcRect;
RECT LoadingBarFull_DestRect;
RECT LoadingBarFull_SrcRect;

D3DTEXTURE LoadingBarFullTexture;
D3DTEXTURE LoadingBarEmptyTexture;

AvPTexture *image = 0;
AvPTexture *LoadingBarEmpty;
AvPTexture *LoadingBarFull;
AvPTexture *aa_font;

int fullbarHeight, fullbarWidth, emptybarHeight, emptybarWidth;

void Start_Progress_Bar()
{
	AAFontImageNumber = CL_LoadImageOnce("Common\\aa_font.RIM",LIO_D3DTEXTURE|LIO_RELATIVEPATH|LIO_RESTORABLE);
	
	/* load other graphics */
	{
		char buffer[100];
		CL_GetImageFileName(buffer, 100,Loading_Bar_Empty_Image_Name, LIO_RELATIVEPATH);
		
		//see if graphic can be found in fast file
		unsigned int fastFileLength;
		void const * pFastFileData = ffreadbuf(buffer,&fastFileLength);
		
		if(pFastFileData)
		{
			//load from fast file
			LoadingBarEmpty = AwCreateTexture//Surface
							(
								"pxf",
								pFastFileData,
								fastFileLength,
								0
							);
		}
		else
		{
			//load graphic from rim file
			LoadingBarEmpty = AwCreateTexture//Surface
							(
								"sf",
								buffer,
								0
							);
		}
		// create d3d texture here
		LoadingBarEmptyTexture = CreateD3DTexturePadded((AvPTexture*)LoadingBarEmpty, &emptybarHeight, &emptybarWidth);
	}
	{
		char buffer[100];
		CL_GetImageFileName(buffer, 100,Loading_Bar_Full_Image_Name, LIO_RELATIVEPATH);
		
		//see if graphic can be found in fast file
		unsigned int fastFileLength;
		void const * pFastFileData = ffreadbuf(buffer,&fastFileLength);
		
		if(pFastFileData)
		{
			//load from fast file
			LoadingBarFull = AwCreateTexture//Surface
							(
								"pxf",
								pFastFileData,
								fastFileLength,
								0
							);
		}
		else
		{
			//load graphic from rim file
			LoadingBarFull = AwCreateTexture//Surface
							(
								"sf",
								buffer,
								0
							);
		}
		
		LoadingBarFullTexture = CreateD3DTexturePadded((AvPTexture*)LoadingBarFull, &fullbarHeight, &fullbarWidth);
	}
/*	
	//set progress bar dimensions
	BarLeft=ScreenDescriptorBlock.SDB_Width/6;
	BarRight=(ScreenDescriptorBlock.SDB_Width*5)/6;
//	BarTop=(ScreenDescriptorBlock.SDB_Height*19)/22;
	BarTop = ((ScreenDescriptorBlock.SDB_Height - ScreenDescriptorBlock.SDB_SafeZoneHeightOffset)) * 19/22;
//	BarBottom=(ScreenDescriptorBlock.SDB_Height*21)/22;
	BarBottom=((ScreenDescriptorBlock.SDB_Height - ScreenDescriptorBlock.SDB_SafeZoneHeightOffset)*21)/22;
*/	
	//load background image for bar
	char buffer[100];
	CL_GetImageFileName(buffer, 100,Loading_Image_Name, LIO_RELATIVEPATH);
	
	//see if graphic can be found in fast file
	unsigned int fastFileLength;
	void const * pFastFileData = ffreadbuf(buffer,&fastFileLength);
	
	if(pFastFileData)
	{
		//load from fast file
		image = AwCreateTexture//Surface
						(
							"pxf",
							pFastFileData,
							fastFileLength,
							0
						);
	}
	else
	{
		//load graphic from rim file
		image = AwCreateTexture//Surface
						(
							"sf",
							buffer,
							0
						);
	}
	#if 0
	if(image)
	{
		//draw background image
		lpDDSBack->Blt(0,image,0,DDBLT_WAIT,0);
	}
	else
	{
		//failed to load background graphic , make do with black background
		ColourFillBackBuffer(0);
	}
	#endif

	//RenderGrabbedScreen();
	//draw initial progress bar

	LoadingBarEmpty_SrcRect.left=0;
	LoadingBarEmpty_SrcRect.right=639;
	LoadingBarEmpty_SrcRect.top=0;
	LoadingBarEmpty_SrcRect.bottom=39;
	LoadingBarEmpty_DestRect.left=0;
	LoadingBarEmpty_DestRect.right=ScreenDescriptorBlock.SDB_Width-1;
	LoadingBarEmpty_DestRect.top=((ScreenDescriptorBlock.SDB_Height - ScreenDescriptorBlock.SDB_SafeZoneHeightOffset) *11)/12;
	LoadingBarEmpty_DestRect.bottom=(ScreenDescriptorBlock.SDB_Height - ScreenDescriptorBlock.SDB_SafeZoneHeightOffset)-1;

	{
		ThisFramesRenderingHasBegun();

		DrawProgressBar(LoadingBarEmpty_SrcRect, LoadingBarEmpty_DestRect, LoadingBarEmptyTexture, LoadingBarEmpty->width, LoadingBarEmpty->height, emptybarWidth, emptybarHeight);

		RenderBriefingText(ScreenDescriptorBlock.SDB_Height/2, ONE_FIXED);

		ThisFramesRenderingHasFinished();

		FlipBuffers();	
	}

	CurrentPosition=0;
}

void Set_Progress_Bar_Position(int pos)
{
//	AAFontImageNumber = CL_LoadImageOnce("Common\\aa_font.RIM",LIO_D3DTEXTURE|LIO_RELATIVEPATH|LIO_RESTORABLE);
//	int NewPosition=((BarRight-BarLeft)*pos)/PBAR_LENGTH;
	int NewPosition = DIV_FIXED(pos,PBAR_LENGTH);
	if(NewPosition>CurrentPosition)
	{

		CurrentPosition=NewPosition;
//		ColourFillBackBufferQuad(GetSingleColourForPrimary(0xff0000),BarLeft,BarTop,BarLeft+CurrentPosition,BarBottom);
		LoadingBarFull_SrcRect.left=0;
		LoadingBarFull_SrcRect.right=MUL_FIXED(639,NewPosition);
		LoadingBarFull_SrcRect.top=0;
		LoadingBarFull_SrcRect.bottom=39;
		LoadingBarFull_DestRect.left=0;
		LoadingBarFull_DestRect.right=MUL_FIXED(ScreenDescriptorBlock.SDB_Width-1,NewPosition);
		LoadingBarFull_DestRect.top=((ScreenDescriptorBlock.SDB_Height - ScreenDescriptorBlock.SDB_SafeZoneHeightOffset) *11)/12;
		LoadingBarFull_DestRect.bottom=(ScreenDescriptorBlock.SDB_Height - ScreenDescriptorBlock.SDB_SafeZoneHeightOffset)-1;
		
		ThisFramesRenderingHasBegun();

		// need to render the empty bar here again. As we're not blitting anymore, 
		// the empty bar will only be rendered for one frame.
		DrawProgressBar(LoadingBarEmpty_SrcRect, LoadingBarEmpty_DestRect, LoadingBarEmptyTexture, LoadingBarEmpty->width, LoadingBarEmpty->height, emptybarWidth, emptybarHeight);

		// also need this here again, or else the text disappears!
		RenderBriefingText(ScreenDescriptorBlock.SDB_Height/2, ONE_FIXED);

//		FlipBuffers();

		// now render the green percent loaded overlay
		DrawProgressBar(LoadingBarFull_SrcRect, LoadingBarFull_DestRect, LoadingBarFullTexture, LoadingBarFull->width, LoadingBarFull->height, fullbarWidth, fullbarHeight);
//		if (LoadingBarFull) lpDDSBack->Blt(&LoadingBarFull_DestRect,LoadingBarFull,&LoadingBarFull_SrcRect,DDBLT_WAIT,0);
		
		ThisFramesRenderingHasFinished();
		
		FlipBuffers();	
		/*
		If this is a network game , then check the received network messages from 
		time to time (~every second).
		Has nothing to do with the progress bar , but this is a convenient place to
		do the check.
		*/
		
		if(AvP.Network != I_No_Network)
		{
			static int LastSendTime;
			int time=GetTickCount();
			if(time-LastSendTime>1000 || time<LastSendTime)
			{
				//time to check our messages 
				LastSendTime=time;
				MinimalNetCollectMessages();
				//send messages , mainly  needed so that the host will send the game description
				//allowing people to join while the host is loading
				NetSendMessages();
			}
		}
	}
}

extern "C"
{

void Game_Has_Loaded(void)
{
	extern int NormalFrameTime;
	extern void RenderStringCentred(char *stringPtr, int centreX, int y, int colour);

	SoundSys_StopAll();
	SoundSys_Management();

	int f = 65536;
	ResetFrameCounter();

	do
	{
		CheckForWindowsMessages();
		ReadUserInput();
	
//		FlipBuffers();
		ThisFramesRenderingHasBegun();
/*
		ColourFillBackBufferQuad
		(
			0,
			0,
			(ScreenDescriptorBlock.SDB_Height*11)/12,
			ScreenDescriptorBlock.SDB_Width-1,
			ScreenDescriptorBlock.SDB_Height-1
		);
*/
		if (f)
		{
			LoadingBarFull_SrcRect.left=0;
			LoadingBarFull_SrcRect.right=639;
			LoadingBarFull_SrcRect.top=0;
			LoadingBarFull_SrcRect.bottom=39;
			LoadingBarFull_DestRect.left=MUL_FIXED(ScreenDescriptorBlock.SDB_Width-1,(ONE_FIXED-f)/2);
			LoadingBarFull_DestRect.right=MUL_FIXED(ScreenDescriptorBlock.SDB_Width-1,f)+LoadingBarFull_DestRect.left;

			int h = MUL_FIXED((ScreenDescriptorBlock.SDB_Height)/24,ONE_FIXED-f);
			LoadingBarFull_DestRect.top=((ScreenDescriptorBlock.SDB_Height - ScreenDescriptorBlock.SDB_SafeZoneHeightOffset) *11)/12+h;
			LoadingBarFull_DestRect.bottom=(ScreenDescriptorBlock.SDB_Height - ScreenDescriptorBlock.SDB_SafeZoneHeightOffset)-1-h;
			
			// also need this here again, or else the text disappears!
			RenderBriefingText(ScreenDescriptorBlock.SDB_Height/2, ONE_FIXED);

			DrawProgressBar(LoadingBarFull_SrcRect, LoadingBarFull_DestRect, LoadingBarFullTexture, LoadingBarFull->width, LoadingBarFull->height, fullbarWidth, fullbarHeight);
// bjd			if (LoadingBarFull) lpDDSBack->Blt(&LoadingBarFull_DestRect,LoadingBarFull,&LoadingBarFull_SrcRect,DDBLT_WAIT,0);
			f-=NormalFrameTime;
			if (f<0) f=0;
		}
		{
//			extern void ThisFramesRenderingHasBegun(void);
//			ThisFramesRenderingHasBegun();
		}
		RenderStringCentred(GetTextString(TEXTSTRING_INGAME_PRESSANYKEYTOCONTINUE), ScreenDescriptorBlock.SDB_Width/2, ((ScreenDescriptorBlock.SDB_Height - ScreenDescriptorBlock.SDB_SafeZoneHeightOffset)*23)/24-9, 0xffffffff);
//		{
			/* after this call, no more graphics can be drawn until the next frame */
//			extern void ThisFramesRenderingHasFinished(void);
			ThisFramesRenderingHasFinished();
//		}

		FlipBuffers();	
		FrameCounterHandler();

		/* If in a network game then we may as well check the network messages while waiting*/
		if(AvP.Network != I_No_Network)
		{
			MinimalNetCollectMessages();
			//send messages , mainly  needed so that the host will send the game description
			//allowing people to join while the host is loading
			NetSendMessages();
		}
	}

	while(!DebouncedGotAnyKey);

	FadingGameInAfterLoading=ONE_FIXED;

	if (image)
	{
		ReleaseAvPTexture(image);
	}

	if (LoadingBarEmpty) 
	{
		ReleaseAvPTexture(LoadingBarEmpty);
	}
	if (LoadingBarFull)
	{
		ReleaseAvPTexture(LoadingBarFull);
	}

	SAFE_RELEASE(LoadingBarEmptyTexture);
	SAFE_RELEASE(LoadingBarFullTexture);
}


};