
extern "C"
{
	#include "3dc.h"
	#include "inline.h"
	#include "fmv.h"
	#include "AvP_Menus.h"
	#include "avp_intro.h"
	extern int NormalFrameTime;
	extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;
	extern unsigned char GotAnyKey;
	extern unsigned char DebouncedGotAnyKey;

	extern AVPMENUGFX AvPMenuGfxStorage[];
extern void DirectReadKeyboard(void);

extern void ThisFramesRenderingHasBegun(void);
extern void ThisFramesRenderingHasFinished(void);

static int IntroHasAlreadyBeenPlayed = 1;


void Show_CopyrightInfo(void);
void Show_Presents(void);
void Show_ARebellionGame(void);
void Show_AvPLogo(void);
extern void ShowSplashScreens(void);
extern void Show_WinnerScreen(void);
extern void PlayBinkedFMV(char *filenamePtr);
extern void DrawMainMenusBackdrop(void);
extern void FadedScreen(int alpha);

extern void DrawFadeQuad(int topX, int topY, int alpha);

void WeWantAnIntro(void)
{
	IntroHasAlreadyBeenPlayed = 0;
}


extern void PlayIntroSequence(void)
{
	if (IntroHasAlreadyBeenPlayed)
	{
		StartMenuMusic();
		return;
	}
	IntroHasAlreadyBeenPlayed=1;

	ResetFrameCounter();
	Show_CopyrightInfo();

//	ThisFramesRenderingHasBegun();

	/* play the Fox Interactive FMV */
//	ClearScreenToBlack();

//	ThisFramesRenderingHasBegun();
//	ClearScreenToBlack();

//	PlayBinkedFMV("FMVs/logos.bik");
#ifdef _XBOX
	//PlayBinkedFMV("D:\\FMVs\\logos.ogv");
#else
	PlayBinkedFMV("FMVs/logos.ogv");
#endif

//	PlayFMV("FMVs/rebellion.smk");

//	ThisFramesRenderingHasFinished();

//	FlipBuffers();

	StartMenuMusic();
	ResetFrameCounter();

	Show_Presents();
	#if ALLOW_SKIP_INTRO
	if (!GotAnyKey) Show_ARebellionGame();
	if (!GotAnyKey) Show_AvPLogo();
	#else
	Show_ARebellionGame();
	Show_AvPLogo();
	#endif

}
extern void ShowSplashScreens(void)
{
	LoadAllSplashScreenGfx();
	int i;
	enum AVPMENUGFX_ID graphic[] =
	{
		AVPMENUGFX_SPLASH_SCREEN1,AVPMENUGFX_SPLASH_SCREEN2,AVPMENUGFX_SPLASH_SCREEN3,
		AVPMENUGFX_SPLASH_SCREEN4,AVPMENUGFX_SPLASH_SCREEN5,
	};
	for (i=0; i<5; i++)
	{
		int timeRemaining = 5*ONE_FIXED;
		do
		{
			ThisFramesRenderingHasBegun();

			int a = timeRemaining*2;
			if (a>ONE_FIXED) a=ONE_FIXED;

			if (i!=4)
			{
			  	DrawAvPMenuGfx_CrossFade(graphic[i],graphic[i+1], a);
				timeRemaining-=NormalFrameTime;
			}
			else
			{
				if (a==ONE_FIXED)
				{
				  	DrawAvPMenuGfx(graphic[i], 0, 0, ONE_FIXED+1,AVPMENUFORMAT_LEFTJUSTIFIED);
				}
				else
				{
				  	DrawAvPMenuGfx_Faded(graphic[i], 0, 0, a,AVPMENUFORMAT_LEFTJUSTIFIED);
					DrawFadeQuad(0, 0, a);
				}
				timeRemaining-=NormalFrameTime/2;
			}
			CheckForWindowsMessages();

			ThisFramesRenderingHasFinished();
			FlipBuffers();

		  	DirectReadKeyboard();
			FrameCounterHandler();
		}
		while(timeRemaining>=0 && !DebouncedGotAnyKey);
	}
	ThisFramesRenderingHasBegun();
	ClearScreenToBlack();
	ThisFramesRenderingHasFinished();
	FlipBuffers();

//	ClearScreenToBlack();
//	FlipBuffers();
}

extern void Show_WinnerScreen(void)
{
	LoadAvPMenuGfx(AVPMENUGFX_WINNER_SCREEN);

	int timeRemaining = 10*ONE_FIXED;
	do
	{
		ThisFramesRenderingHasBegun();

		int a = timeRemaining*2;
		if (a>ONE_FIXED)
		{
		  	DrawAvPMenuGfx(AVPMENUGFX_WINNER_SCREEN, 0, 0, ONE_FIXED+1,AVPMENUFORMAT_LEFTJUSTIFIED);
		}
		else
		{
		  	DrawAvPMenuGfx_Faded(AVPMENUGFX_WINNER_SCREEN, 0, 0, a,AVPMENUFORMAT_LEFTJUSTIFIED);
		}

		CheckForWindowsMessages();

		ThisFramesRenderingHasFinished();
		FlipBuffers();

	  	DirectReadKeyboard();
		FrameCounterHandler();
		timeRemaining-=NormalFrameTime;
	}
	while(timeRemaining>=0 && !DebouncedGotAnyKey);

	ThisFramesRenderingHasBegun();
	ClearScreenToBlack();
	ThisFramesRenderingHasFinished();
	FlipBuffers();
//	ClearScreenToBlack();
//	FlipBuffers();
}

void Show_CopyrightInfo(void)
{
	int timeRemaining = ONE_FIXED/2;
	do
	{
		CheckForWindowsMessages();
		{
			ThisFramesRenderingHasBegun();
			DrawAvPMenuGfx_Faded(AVPMENUGFX_COPYRIGHT_SCREEN, 0, 0, ONE_FIXED-timeRemaining*2,AVPMENUFORMAT_LEFTJUSTIFIED);
			ThisFramesRenderingHasFinished();
			FlipBuffers();
		}
		FrameCounterHandler();
		timeRemaining-=NormalFrameTime;
	}
	while(timeRemaining>0);

	timeRemaining = ONE_FIXED*2;
	do
	{
		CheckForWindowsMessages();
		{
			ThisFramesRenderingHasBegun();
			DrawAvPMenuGfx_Faded(AVPMENUGFX_COPYRIGHT_SCREEN, 0, 0, ONE_FIXED,AVPMENUFORMAT_LEFTJUSTIFIED);
			ThisFramesRenderingHasFinished();
			FlipBuffers();
		}
		FrameCounterHandler();
		timeRemaining-=NormalFrameTime;
	}
	while(timeRemaining>0);

	timeRemaining = ONE_FIXED/2;
	do
	{
		CheckForWindowsMessages();
		{
			ThisFramesRenderingHasBegun();
			DrawAvPMenuGfx_Faded(AVPMENUGFX_COPYRIGHT_SCREEN, 0, 0, timeRemaining*2,AVPMENUFORMAT_LEFTJUSTIFIED);
			ThisFramesRenderingHasFinished();
			FlipBuffers();
		}
		FrameCounterHandler();
		timeRemaining-=NormalFrameTime;
	}
	while(timeRemaining>0);
}

void Show_Presents(void)
{
	int timeRemaining = 8*ONE_FIXED-ONE_FIXED/2;

	do
	{
		ThisFramesRenderingHasBegun();

		CheckForWindowsMessages();
		{
			char *textPtr = GetTextString(TEXTSTRING_FOXINTERACTIVE);
			int y = (480-AvPMenuGfxStorage[AVPMENUGFX_PRESENTS].Height)/2;
			PlayMenuMusic();
			DrawMainMenusBackdrop();

			if (timeRemaining > 6*ONE_FIXED)
			{
				DrawFadeQuad(0, 0, (15*ONE_FIXED-timeRemaining*2)/3);
			}
			else if (timeRemaining > 5*ONE_FIXED)
			{
				RenderMenuText(textPtr, MENU_CENTREX, y, 6*ONE_FIXED-timeRemaining, AVPMENUFORMAT_CENTREJUSTIFIED);
			}
			else if (timeRemaining > 4*ONE_FIXED)
			{
				RenderMenuText(textPtr, MENU_CENTREX, y, ONE_FIXED, AVPMENUFORMAT_CENTREJUSTIFIED);
			}
			else if (timeRemaining > 3*ONE_FIXED)
			{
				RenderMenuText(textPtr, MENU_CENTREX, y, timeRemaining-3*ONE_FIXED, AVPMENUFORMAT_CENTREJUSTIFIED);
			}

			ThisFramesRenderingHasFinished();
			FlipBuffers();
		}
		#if ALLOW_SKIP_INTRO
		DirectReadKeyboard();
		#endif
		FrameCounterHandler();
		timeRemaining-=NormalFrameTime;
	}
	#if ALLOW_SKIP_INTRO

	while((timeRemaining>0) && !GotAnyKey);

	#else
	while(timeRemaining>0);// && !GotAnyKey);
	#endif
}

void Show_ARebellionGame(void)
{
	int timeRemaining = 7*ONE_FIXED;
	do
	{
		ThisFramesRenderingHasBegun();

		CheckForWindowsMessages();
		{
			char *textPtr = GetTextString(TEXTSTRING_PRESENTS);
			int y = (480-AvPMenuGfxStorage[AVPMENUGFX_AREBELLIONGAME].Height)/2;
			DrawMainMenusBackdrop();
//			DrawAvPMenuGfx(AVPMENUGFX_BACKDROP, 0, 0, ONE_FIXED+1,AVPMENUFORMAT_LEFTJUSTIFIED);
			PlayMenuMusic();

			if (timeRemaining > 13*ONE_FIXED/2)
			{
//				DrawAvPMenuGfx(AVPMENUGFX_AREBELLIONGAME, MENU_CENTREX, y, 14*ONE_FIXED-timeRemaining*2,AVPMENUFORMAT_CENTREJUSTIFIED);
				RenderMenuText(textPtr,MENU_CENTREX,y,14*ONE_FIXED-timeRemaining*2,AVPMENUFORMAT_CENTREJUSTIFIED);
//				DrawGraphicWithAlphaChannel(&RebellionLogo,timeRemaining*2-13*ONE_FIXED);
 			}
			else if (timeRemaining > 5*ONE_FIXED)
			{
//				DrawAvPMenuGfx(AVPMENUGFX_AREBELLIONGAME, MENU_CENTREX, y, ONE_FIXED,AVPMENUFORMAT_CENTREJUSTIFIED);
				RenderMenuText(textPtr,MENU_CENTREX,y,ONE_FIXED,AVPMENUFORMAT_CENTREJUSTIFIED);
//				DrawGraphicWithAlphaChannel(&RebellionLogo,0);
			}
			else if (timeRemaining > 3*ONE_FIXED)
			{
//				DrawAvPMenuGfx(AVPMENUGFX_AREBELLIONGAME, MENU_CENTREX, y, (timeRemaining-3*ONE_FIXED)/2,AVPMENUFORMAT_CENTREJUSTIFIED);
				RenderMenuText(textPtr,MENU_CENTREX,y,(timeRemaining-3*ONE_FIXED)/2,AVPMENUFORMAT_CENTREJUSTIFIED);
//				DrawGraphicWithAlphaChannel(&RebellionLogo, ONE_FIXED - (timeRemaining-3*ONE_FIXED)/2);
			}

			ThisFramesRenderingHasFinished();
			FlipBuffers();
		}
		DirectReadKeyboard();
		FrameCounterHandler();
		timeRemaining-=NormalFrameTime;
	}
	#if ALLOW_SKIP_INTRO
	while((timeRemaining>0) && !GotAnyKey);
	#else
	while(timeRemaining>0);// && !GotAnyKey);
	#endif
}
void Show_AvPLogo(void)
{
	int timeRemaining = 5*ONE_FIXED;
	do
	{
		ThisFramesRenderingHasBegun();

		CheckForWindowsMessages();
		{
			int y = (480-AvPMenuGfxStorage[AVPMENUGFX_ALIENSVPREDATOR].Height)/2;
			DrawMainMenusBackdrop();
//			DrawAvPMenuGfx(AVPMENUGFX_BACKDROP, 0, 0, ONE_FIXED+1,AVPMENUFORMAT_LEFTJUSTIFIED);
			PlayMenuMusic();

			if (timeRemaining > 9*ONE_FIXED/2)
			{
				DrawAvPMenuGfx(AVPMENUGFX_ALIENSVPREDATOR, MENU_CENTREX, y, -timeRemaining*2+10*ONE_FIXED,AVPMENUFORMAT_CENTREJUSTIFIED);
 			}
			else if (timeRemaining > 4*ONE_FIXED)
			{
				DrawAvPMenuGfx(AVPMENUGFX_ALIENSVPREDATOR, MENU_CENTREX, y, ONE_FIXED,AVPMENUFORMAT_CENTREJUSTIFIED);
			}
			else
			{
				DrawAvPMenuGfx(AVPMENUGFX_ALIENSVPREDATOR, MENU_CENTREX, y, timeRemaining/4,AVPMENUFORMAT_CENTREJUSTIFIED);
				timeRemaining-=NormalFrameTime/4;
			}

			ThisFramesRenderingHasFinished();
			FlipBuffers();
		}
		DirectReadKeyboard();
		FrameCounterHandler();
		timeRemaining-=NormalFrameTime;
	}
	#if ALLOW_SKIP_INTRO
	while((timeRemaining>0) && !GotAnyKey);
	#else
	while(timeRemaining>0);// && !GotAnyKey);
	#endif
}

};

