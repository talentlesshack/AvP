#include "3dc.h"
#include "inline.h"

#include "tallfont.hpp"
#include "strtab.hpp"

#include "awTexLd.h"
#include "alt_tab.h"

#include "chnktexi.h"
#include "hud_layout.h"

#define UseLocalAssert Yes
#include "ourasert.h"
#include "ffstdio.h"

#include "d3_func.h"

extern void D3D_RenderHUDString(char *stringPtr,int x,int y,int colour);
extern void DrawMenuQuad(int topX, int topY, int bottomX, int bottomY, int image_num, BOOL alpha);

extern "C"
{
#include "AvP_Menus.h"
extern unsigned char *ScreenBuffer;
extern long BackBufferPitch;
//extern DDPIXELFORMAT DisplayPixelFormat; // BJD
extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;

char AAFontWidths[256];

extern int CloudTable[128][128];
extern int CloakingPhase;

extern D3DInfo d3d;

AVPMENUGFX AvPMenuGfxStorage[MAX_NO_OF_AVPMENUGFXS] =
{
	{"Menus\\fractal.rim"},
	{"Common\\aa_font.rim"},// Warning! Texture from common used

	{"Menus\\copyright.rim"},

	{"Menus\\FIandRD.rim"},
	{"Menus\\presents.rim"},
	{"Menus\\AliensVPredator.rim"},
	
	{"Menus\\sliderbar.rim"},//AVPMENUGFX_SLIDERBAR,
	{"Menus\\slider.rim"},//AVPMENUGFX_SLIDER,

	{"Menus\\starfield.rim"},
	{"Menus\\aliens.rim"},
	{"Menus\\Alien.rim"},
	{"Menus\\Marine.rim"},
	{"Menus\\Predator.rim"},

	{"Menus\\glowy_left.rim"},
	{"Menus\\glowy_middle.rim"},
	{"Menus\\glowy_right.rim"},
	
	// Marine level
	{"Menus\\MarineEpisode1.rim"},
	{"Menus\\MarineEpisode2.rim"},
	{"Menus\\MarineEpisode3.rim"},
	{"Menus\\MarineEpisode4.rim"},
	{"Menus\\MarineEpisode5.rim"},
	{"Menus\\MarineEpisode6.rim"},
	{"Menus\\bonus.rim"},
	{"Menus\\bonus.rim"},
	{"Menus\\bonus.rim"},
	{"Menus\\bonus.rim"},
	{"Menus\\bonus.rim"},
	
	// Predator level
	{"Menus\\PredatorEpisode1.rim"},
	{"Menus\\PredatorEpisode2.rim"},
	{"Menus\\PredatorEpisode3.rim"},
	{"Menus\\PredatorEpisode4.rim"},
	{"Menus\\PredatorEpisode5.rim"},
	{"Menus\\PredatorEpisode5.rim"},
	{"Menus\\bonus.rim"},
	{"Menus\\bonus.rim"},
	{"Menus\\bonus.rim"},
	{"Menus\\bonus.rim"},
	{"Menus\\bonus.rim"},

	// Alien level
	{"Menus\\AlienEpisode2.rim"},
	{"Menus\\AlienEpisode4.rim"},
	{"Menus\\AlienEpisode1.rim"},
	{"Menus\\AlienEpisode3.rim"},
	{"Menus\\AlienEpisode5.rim"},
	{"Menus\\bonus.rim"},
	{"Menus\\bonus.rim"},
	{"Menus\\bonus.rim"},
	{"Menus\\bonus.rim"},
	{"Menus\\bonus.rim"},

//	{"Menus\\LoadingBar_Empty.rim"},
//	{"Menus\\LoadingBar_Full.rim"},

	// Splash screens
	#if MARINE_DEMO
	{"MarineSplash\\splash00.rim"},
	{"MarineSplash\\splash01.rim"},
	{"MarineSplash\\splash02.rim"},
	{"MarineSplash\\splash03.rim"},
	{"MarineSplash\\splash04.rim"},
	{"MarineSplash\\splash05.rim"},
	#elif ALIEN_DEMO
	{"AlienSplash\\splash00.rim"},
	{"AlienSplash\\splash01.rim"},
	{"AlienSplash\\splash02.rim"},
	{"AlienSplash\\splash03.rim"},
	{"AlienSplash\\splash04.rim"},
	{"AlienSplash\\splash05.rim"},
	#else
	{"PredatorSplash\\splash00.rim"},
	{"PredatorSplash\\splash01.rim"},
	{"PredatorSplash\\splash02.rim"},
	{"PredatorSplash\\splash03.rim"},
	{"PredatorSplash\\splash04.rim"},
	{"PredatorSplash\\splash05.rim"},
	#endif
/*
	{"Menus\\LoadingBar_Empty.rim"},
	{"Menus\\LoadingBar_Full.rim"},
*/
};

static void LoadMenuFont(void);
static void UnloadMenuFont(void);
static int RenderSmallFontString(char *textPtr,int sx,int sy,int alpha, int red, int green, int blue);
static void CalculateWidthsOfAAFont(void);
extern void DrawAvPMenuGlowyBar(int topleftX, int topleftY, int alpha, int length);
extern void DrawAvPMenuGlowyBar_Clipped(int topleftX, int topleftY, int alpha, int length, int topY, int bottomY);

extern void D3D_Rectangle(int x0, int y0, int x1, int y1, int r, int g, int b, int a);

#if 0 // moved to header file (AvP_MenuGfx.hpp)
typedef struct AVPIndexedFont
{
	AVPMENUGFX info;	/* graphic info */
	int swidth;		/* width for space */
	int ascii;		/* ascii code for initial character */
	int height;		/* height per character */
	
	int numchars;
	int FontWidth[256];
} AVPIndexedFont;
#endif

AVPIndexedFont IntroFont_Light;

static void LoadMenuFont(void)
{
	/* notes on converting to quad textured menu system:

		This function will load a REALLY tall image. When passed to our function to create a 
		power of 2 d3d texture, it'll try create a texture with the height of 8192, which 
		is both insane and not supported by any videocard out there that I know of.

		As we load the image as raw data, we COULD try re-order it into a square shaped
		texture instead of this single character wide madness.

		224 characters, each 33 pixels high?
		30 pixels wide

		16 x 16 grid?

		15 x 15 = 225, so better

		height:	495
		width : 450
	*/

	AVPMENUGFX *gfxPtr;
	char buffer[100];
	unsigned int fastFileLength;
	void const *pFastFileData;
	
	IntroFont_Light.height = 33;
	IntroFont_Light.swidth = 5;
	IntroFont_Light.ascii = 32;
		
	gfxPtr = &IntroFont_Light.info;
	gfxPtr->menuTexture = NULL;
	
	CL_GetImageFileName(buffer, 100, "Menus\\IntroFont.rim", LIO_RELATIVEPATH);
	
	pFastFileData = ffreadbuf(buffer, &fastFileLength);
	
	if (pFastFileData) {
		gfxPtr->ImagePtr = AwCreateSurface(
			"pxfXY",
			pFastFileData,
			fastFileLength,
			AW_TLF_TRANSP|AW_TLF_CHROMAKEY,
			&(gfxPtr->Width),
			&(gfxPtr->Height)
		);
	} else {
		gfxPtr->ImagePtr = AwCreateSurface(
			"sfXY",
			buffer,
			AW_TLF_TRANSP|AW_TLF_CHROMAKEY,
			&(gfxPtr->Width),
			&(gfxPtr->Height)
		);
	}
	
	GLOBALASSERT(gfxPtr->ImagePtr);
	GLOBALASSERT(gfxPtr->Width>0);
	GLOBALASSERT(gfxPtr->Height>0);
	
	gfxPtr->hBackup = 0;
	
	AvPTexture *image = (AvPTexture*)gfxPtr->ImagePtr;

	unsigned char *srcPtr = image->buffer;

	int c;
	
	if ((image->width != 30) || ((image->height % 33) != 0)) {
		// handle new texture
	}
	
	IntroFont_Light.numchars = image->height / 33;	
	IntroFont_Light.FontWidth[32] = 5;
	
	for (c=33; c<(32+IntroFont_Light.numchars); c++) {
		int x,y;
		int y1 = 1+(c-32)*33;
		
		IntroFont_Light.FontWidth[c]=31;
		
		for (x=29; x>0; x--) {
			int blank = 1;
			
			for (y=y1; y<y1+31; y++) {
				unsigned char *s = &srcPtr[(x + y*image->width) * 4];
				if (s[2]) {
					blank = 0;
					break;
				}
			}
			
			if (blank) {
				IntroFont_Light.FontWidth[c]--;
			} else {
				break;
			}
		}
	}

	// we're going to try create a square texture..
	gfxPtr->menuTexture = CreateD3DTallFontTexture(image);

#if 0 // bjd
	// image width - 30 pixels
	// image height -7392
	char buffer[100];
	IndexedFont_Kerned_Column :: Create
	(
		IntroFont_Light, // FontIndex I_Font_New,
		CL_GetImageFileName(buffer, 100, "Menus\\IntroFont.rim", LIO_RELATIVEPATH),
		33,//21, // int HeightPerChar_New,
		5, // int SpaceWidth_New,
		32 // ASCIICodeForInitialCharacter
	);
#endif
}

static void UnloadMenuFont(void)
{
#if 1
//	IndexedFont :: UnloadFont( IntroFont_Light );

	ReleaseDDSurface(IntroFont_Light.info.ImagePtr);
	IntroFont_Light.info.ImagePtr = NULL;

	SAFE_RELEASE(IntroFont_Light.info.menuTexture);
#endif
}
extern int LengthOfMenuText(const char *textPtr)
{
	int width = 0;
	
	while (textPtr && *textPtr) {
		width += IntroFont_Light.FontWidth[(unsigned int) *textPtr];
		
		textPtr++;
	}
	return width;
}

extern int RenderMenuText(const char *textPtr, int pX, int pY, int alpha, enum AVPMENUFORMAT_ID format) 
{
	int width = LengthOfMenuText(textPtr);
	int word_length = 0;

	switch(format)
	{
		default:
		GLOBALASSERT("UNKNOWN TEXT FORMAT"==0);
		case AVPMENUFORMAT_LEFTJUSTIFIED:
		{
			// supplied x is correct
			break;
		}
		case AVPMENUFORMAT_RIGHTJUSTIFIED:
		{
			//x -= (pFont->CalcSize(textPtr).w);
			pX -= width;
			break;
		}
		case AVPMENUFORMAT_CENTREJUSTIFIED:
		{
			//x -= (pFont->CalcSize(textPtr).w)/2;
			pX -= width / 2;
			break;
		}
	}

	LOCALASSERT(pX>0);
	
	extern enum MENUSSTATE_ID;

	if(GetAvPMenuState() == MENUSSTATE_INGAMEMENUS)
	{
		// do nothing here for now
	}
	else
	{
		if (alpha >BRIGHTNESS_OF_DARKENED_ELEMENT)
		{
			int size = width - 18;
			if (size<18) size = 18;

			DrawAvPMenuGfx(AVPMENUGFX_GLOWY_LEFT,pX+18,pY-8,alpha,AVPMENUFORMAT_RIGHTJUSTIFIED);
			DrawAvPMenuGlowyBar(pX+18,pY-8,alpha,size-18);
			DrawAvPMenuGfx(AVPMENUGFX_GLOWY_RIGHT,pX+size,pY-8,alpha,AVPMENUFORMAT_LEFTJUSTIFIED);
		}
	}

	// we need initial values for cloud effect
	int positionX = pX;
	int positionY = pY;

//	D3D_RenderHUDString(textPtr,pX,pY,0);

//#if 0 // bjd
/*
	unsigned char *srcPtr;
	unsigned short *destPtr;
	AVPMENUGFX *gfxPtr;
	D3DTexture *image;
	
	gfxPtr = &IntroFont_Light.info;
	image = (D3DTexture*)gfxPtr->ImagePtr;
*/
/*
	D3DInfo temp;

	temp = GetD3DInfo();

	D3DSURFACE_DESC surface_desc;
	temp.lpD3DBackSurface->GetDesc(&surface_desc);

	D3DLOCKED_RECT lock = {0,NULL};

	temp.lpD3DBackSurface->LockRect(&lock, NULL, 0); if (lock.pBits == NULL) return 0;
*/	
//	D3DInfo temp;
//	temp = GetD3DInfo();
/*
	temp.lpD3DDevice->Clear( 0, NULL, D3DCLEAR_STENCIL ,
		D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );
*/
	while( *textPtr ) {
		char c = *textPtr++;

		if (c>=' ') {

//			int topLeftU = 1;
//			int topLeftV = 1+(c-32)*33;
//			int x, y;
			int width = IntroFont_Light.FontWidth[(unsigned int) c];

			c = c - 32;

			int row = (int)(c / 15); // get row 
			int column = c % 15; // get column from remainder value

			int tex_y = row * 33;
			int tex_x = column * 30;

			int topLeftU = tex_x;
			int topLeftV = tex_y;

			/* bjd note
			need to fix the 'Cloud Table'
			ie the moving hazy smoke/cloud effect behind the large font text on the menus
			*/
			DrawTallFontCharacter(positionX, positionY, topLeftU, topLeftV, width, alpha);

//			DrawCloudTable(pX, pY, word_length, 255);

//			int r = CloudTable[(x+pX+CloakingPhase/64)&127][(y+CloakingPhase/128)&127];
/*			
			srcPtr = &image->buf[(topLeftU+topLeftV*image->w)*4];
			
			for (y=pY; y<33+pY; y++) {
				destPtr = (unsigned short *)(((unsigned char *)lock.pBits)+y*lock.Pitch) + pX;
				
				for (x=width; x>0; x--) {					
					if (srcPtr[0] || srcPtr[1] || srcPtr[2]) {
						unsigned int destR, destG, destB;
						
						int r = CloudTable[(x+pX+CloakingPhase/64)&127][(y+CloakingPhase/128)&127];
						r = MUL_FIXED(alpha, r);
						
						destR = (*destPtr & 0xF800)>>8;
						destG = (*destPtr & 0x07E0)>>3;
						destB = (*destPtr & 0x001F)<<3;
						
						destR += MUL_FIXED(r, srcPtr[0]);
						destG += MUL_FIXED(r, srcPtr[1]);
						destB += MUL_FIXED(r, srcPtr[2]);
						if (destR > 0x00FF) destR = 0x00FF;
						if (destG > 0x00FF) destG = 0x00FF;
						if (destB > 0x00FF) destB = 0x00FF;
						
						*destPtr =	((destR>>3)<<11) |
								((destG>>2)<<5 ) |
								((destB>>3));
					}
					srcPtr += 4;
					destPtr++;
				}
				srcPtr += (image->w - width) * 4;
			}
*/
			positionX += width;
			word_length += width;
		}
	}
//	DrawCloudTable(pX, pY, word_length, alpha);

//	temp.lpD3DBackSurface->UnlockRect();
//#endif
	return positionX;
}
extern int RenderMenuText_Clipped(char *textPtr, int pX, int pY, int alpha, enum AVPMENUFORMAT_ID format, int topY, int bottomY) 
{
	// this'll do for now :)
	RenderMenuText(textPtr, pX, pY, alpha, format); 
#if 0
	int width = LengthOfMenuText(textPtr);
	
	switch(format) {
		default:
			GLOBALASSERT("UNKNOWN TEXT FORMAT"==0);
			return 0;
		case AVPMENUFORMAT_LEFTJUSTIFIED:
			break;
		case AVPMENUFORMAT_RIGHTJUSTIFIED:
			pX -= width;
			break;
		case AVPMENUFORMAT_CENTREJUSTIFIED:
			pX -= width / 2;
			break;
	}
	
	LOCALASSERT(pX>0);
	
	if (alpha > BRIGHTNESS_OF_DARKENED_ELEMENT) {
		int size = width - 18;
		if (size<18) size = 18;
		
		DrawAvPMenuGfx_Clipped(AVPMENUGFX_GLOWY_LEFT,pX+18,pY-8,alpha,AVPMENUFORMAT_RIGHTJUSTIFIED,topY,bottomY);
		DrawAvPMenuGlowyBar_Clipped(pX+18,pY-8,alpha,size-18,topY,bottomY);
		DrawAvPMenuGfx_Clipped(AVPMENUGFX_GLOWY_RIGHT,pX+size,pY-8,alpha,AVPMENUFORMAT_LEFTJUSTIFIED,topY,bottomY);
	}
{
	unsigned char *srcPtr;
	unsigned short *destPtr;
	AVPMENUGFX *gfxPtr;
	D3DTexture *image;
	
	gfxPtr = &IntroFont_Light.info;
	image = (D3DTexture*)gfxPtr->ImagePtr;
	
	D3DInfo temp;

	temp = GetD3DInfo();

	D3DSURFACE_DESC surface_desc;
	temp.lpD3DBackSurface->GetDesc(&surface_desc);

	D3DLOCKED_RECT lock = {0,NULL};

	temp.lpD3DBackSurface->LockRect(&lock, NULL, 0); if (lock.pBits == NULL) return 0;

	while( *textPtr ) {
		char c = *textPtr++;

		if (c>=' ') {
			int topLeftU = 1;
			int topLeftV = 1+(c-32)*33;
			int x, y;
			int width = IntroFont_Light.FontWidth[(unsigned int) c];
			
			srcPtr = &image->buf[(topLeftU+topLeftV*image->w)*4];
			
			for (y=pY; y<33+pY; y++) {
				if(y>=topY && y<=bottomY) {
					destPtr = (unsigned short *)(((unsigned char *)lock.pBits)+y*lock.Pitch) + pX;
				
					for (x=width; x>0; x--) {
						if (srcPtr[0] || srcPtr[1] || srcPtr[2]) {
							unsigned int destR, destG, destB;
						
							int r = CloudTable[(x+pX+CloakingPhase/64)&127][(y+CloakingPhase/128)&127];
							r = MUL_FIXED(alpha, r);
							
							destR = (*destPtr & 0xF800)>>8;
							destG = (*destPtr & 0x07E0)>>3;
							destB = (*destPtr & 0x001F)<<3;
						
							destR += MUL_FIXED(r, srcPtr[0]);
							destG += MUL_FIXED(r, srcPtr[1]);
							destB += MUL_FIXED(r, srcPtr[2]);
							if (destR > 0x00FF) destR = 0x00FF;
							if (destG > 0x00FF) destG = 0x00FF;
							if (destB > 0x00FF) destB = 0x00FF;
						
							*destPtr =	((destR>>3)<<11) |
									((destG>>2)<<5 ) |
									((destB>>3));
						}
						srcPtr += 4;
						destPtr++;
					}
					srcPtr += (image->w - width) * 4;	
				} else {
					srcPtr += image->w * 4;
				}
			}
			pX += width;
		}
	}

	temp.lpD3DBackSurface->UnlockRect();
	}
#endif
return pX;
}


extern int RenderSmallMenuText(char *textPtr, int x, int y, int alpha, enum AVPMENUFORMAT_ID format) 
{
	int length;
	char *ptr;

	switch(format)
	{
		default:
		GLOBALASSERT("UNKNOWN TEXT FORMAT"==0);
		case AVPMENUFORMAT_LEFTJUSTIFIED:
		{
			// supplied x is correct
			break;
		}
		case AVPMENUFORMAT_RIGHTJUSTIFIED:
		{
			length = 0;
			ptr = textPtr;

			while(*ptr)
			{
				//length+=AAFontWidths[*ptr++];
				length+=AAFontWidths[(unsigned int) *ptr++];
			}

			x -= length;
			break;
		}
		case AVPMENUFORMAT_CENTREJUSTIFIED:
		{
			length = 0;
			ptr = textPtr;

			while(*ptr)
			{
				//length+=AAFontWidths[*ptr++];
				length+=AAFontWidths[(unsigned int) *ptr++];
			}

			x -= length/2;
			break;
		}
	}

	LOCALASSERT(x>0);

	x = RenderSmallFontString(textPtr,x,y,alpha,ONE_FIXED,ONE_FIXED,ONE_FIXED);
	return x;
}
extern int RenderSmallMenuText_Coloured(char *textPtr, int x, int y, int alpha, enum AVPMENUFORMAT_ID format, int red, int green, int blue) 
{
	switch(format)
	{
		default:
		GLOBALASSERT("UNKNOWN TEXT FORMAT"==0);
		case AVPMENUFORMAT_LEFTJUSTIFIED:
		{
			// supplied x is correct
			break;
		}
		case AVPMENUFORMAT_RIGHTJUSTIFIED:
		{
			int length = 0;
			char *ptr = textPtr;

			while(*ptr)
			{
				length+=AAFontWidths[*ptr++];
			}

			x -= length;
			break;
		}
		case AVPMENUFORMAT_CENTREJUSTIFIED:
		{
			int length = 0;
			char *ptr = textPtr;

			while(*ptr)
			{
				length+=AAFontWidths[*ptr++];
			}

			x -= length/2;
			break;
		}
	}

	LOCALASSERT(x>0);

	x = RenderSmallFontString(textPtr,x,y,alpha,red,green,blue);

	return x;
}

extern int Hardware_RenderSmallMenuText(char *textPtr, int x, int y, int alpha, enum AVPMENUFORMAT_ID format) 
{
	switch(format)
	{
		default:
		GLOBALASSERT("UNKNOWN TEXT FORMAT"==0);
		case AVPMENUFORMAT_LEFTJUSTIFIED:
		{
			// supplied x is correct
			break;
		}
		case AVPMENUFORMAT_RIGHTJUSTIFIED:
		{
			int length = 0;
			char *ptr = textPtr;

			while(*ptr)
			{
				length+=AAFontWidths[*ptr++];
			}

			x -= length;
			break;
		}
		case AVPMENUFORMAT_CENTREJUSTIFIED:
		{
			int length = 0;
			char *ptr = textPtr;

			while(*ptr)
			{
				length+=AAFontWidths[*ptr++];
			}

			x -= length/2;
			break;
		}	
	}

	LOCALASSERT(x>0);

	{
		unsigned int colour = alpha>>8;
		if (colour>255) colour = 255;
		colour = (colour<<24)+0xffffff;

		D3D_RenderHUDString(textPtr,x,y,colour);
	}
	return x;
}

extern int Hardware_RenderSmallMenuText_Coloured(char *textPtr, int x, int y, int alpha, enum AVPMENUFORMAT_ID format, int red, int green, int blue)
{
	switch(format)
	{
		default:
		GLOBALASSERT("UNKNOWN TEXT FORMAT"==0);
		case AVPMENUFORMAT_LEFTJUSTIFIED:
		{
			// supplied x is correct
			break;
		}
		case AVPMENUFORMAT_RIGHTJUSTIFIED:
		{
			int length = 0;
			char *ptr = textPtr;

			while(*ptr)
			{
				length+=AAFontWidths[*ptr++];
			}

			x -= length;
			break;
		}
		case AVPMENUFORMAT_CENTREJUSTIFIED:
		{
			int length = 0;
			char *ptr = textPtr;

			while(*ptr)
			{
				length+=AAFontWidths[*ptr++];
			}

			x -= length/2;
			break;
		}	
	}

	LOCALASSERT(x>0);

	{
		unsigned int colour = alpha>>8;
		if (colour>255) colour = 255;
		colour = (colour<<24);
		colour += MUL_FIXED(red,255)<<16;
		colour += MUL_FIXED(green,255)<<8;
		colour += MUL_FIXED(blue,255);
		D3D_RenderHUDString(textPtr,x,y,colour);
	}
	return x;
}

extern void Hardware_RenderKeyConfigRectangle(int alpha)
{
	extern void D3D_DrawRectangle(int x, int y, int w, int h, int alpha);
	D3D_DrawRectangle(10,ScreenDescriptorBlock.SDB_Height/2+25-115,ScreenDescriptorBlock.SDB_Width-20,250,alpha);
}
extern void RenderKeyConfigRectangle(int alpha)
{
	extern void D3D_DrawRectangle(int x, int y, int w, int h, int alpha);
	D3D_DrawRectangle(10,ScreenDescriptorBlock.SDB_Height/2+25-115,ScreenDescriptorBlock.SDB_Width-20,250,alpha);
//	return;
/*
	int x = 10;
	int y = ScreenDescriptorBlock.SDB_Height/2+25-115;
	int width = ScreenDescriptorBlock.SDB_Width-20;
	int height = 250;
*/
/*
	char buf[100];
	sprintf(buf, "\n x: %d y: %d width: %d height: %d", x,y,width,height);
	OutputDebugString(buf);
*/
//	extern void D3D_DrawRectangle(int x, int y, int w, int h, int alpha);
//	D3D_DrawRectangle(x, y, width, height, alpha);

#if 0
	int x,y;
	unsigned short c, *destPtr;

	c =	((MUL_FIXED(0xFF,alpha)>>3)<<11) |
		((MUL_FIXED(0xFF,alpha)>>2)<<5 ) |
		((MUL_FIXED(0xFF,alpha)>>3));

	D3DInfo temp;

	temp = GetD3DInfo();

	D3DSURFACE_DESC surface_desc;
	temp.lpD3DBackSurface->GetDesc(&surface_desc);

	D3DLOCKED_RECT lock = {0,NULL};

	temp.lpD3DBackSurface->LockRect(&lock, NULL, 0); if (lock.pBits == NULL) return;
		
	y = y1;
	{
		destPtr = (unsigned short *)(((unsigned char *)lock.pBits) + y*lock.Pitch) + x1;

		for (x=x1; x<=x2; x++)
		{
			*destPtr |= c;
			destPtr++;
		}
	}

	y = y2;
	{
		destPtr = (unsigned short *)(((unsigned char *)lock.pBits) + y*lock.Pitch) + x1;

		for (x=x1; x<=x2; x++)
		{
			*destPtr |= c;
			destPtr++;
		}
	}
	{

		for (y=y1+1; y<y2; y++)
		{
			destPtr = (unsigned short *)(((unsigned char *)lock.pBits) + y*lock.Pitch) + x1;
			*destPtr |= c;
		}
	}
	{

		for (y=y1+1; y<y2; y++)
		{
			destPtr = (unsigned short *)(((unsigned char *)lock.pBits) + y*lock.Pitch) + x2;
			*destPtr |= c;
		}
	}
	temp.lpD3DBackSurface->UnlockRect();
#endif
}
extern void Hardware_RenderHighlightRectangle(int x1,int y1,int x2,int y2,int r, int g, int b)
{
	D3D_Rectangle(x1, y1, x2, y2, r, g, b, 255);
}

extern void RenderHighlightRectangle(int x1,int y1,int x2,int y2, int r, int g, int b)
{
	// green rectangle highlight on load screen etc
	D3D_Rectangle(x1, y1, x2, y2, r, g, b, 255);
#if 0 
	int x,y;
	unsigned short c;

	c =	((r>>3)<<11) |
		((g>>2)<<5 ) |
		((b>>3));

	D3DInfo temp;

	temp = GetD3DInfo();

	D3DSURFACE_DESC surface_desc;
	temp.lpD3DBackSurface->GetDesc(&surface_desc);

	D3DLOCKED_RECT lock = {0,NULL};

	temp.lpD3DBackSurface->LockRect(&lock, NULL, 0); if (lock.pBits == NULL) return;

	for (y=y1; y<=y2; y++)
	{
		unsigned short *destPtr = (unsigned short *)(((unsigned char *)lock.pBits) + y*lock.Pitch) + x1;

		for (x=x1; x<=x2; x++)
		{
			*destPtr |= c;
			destPtr++;
		}
	}
	temp.lpD3DBackSurface->UnlockRect();
#endif
}

static int RenderSmallFontString(char *textPtr,int sx,int sy,int alpha, int red, int green, int blue)
{
//#if 0

//	unsigned short *destPtr;
//	unsigned char *srcPtr;

//	int extra = 0;

	int alphaR = MUL_FIXED(alpha, red);
	int alphaG = MUL_FIXED(alpha, green);
	int alphaB = MUL_FIXED(alpha, blue);
/*
	AVPMENUGFX *gfxPtr;
	AvPTexture *image;

	gfxPtr = &AvPMenuGfxStorage[AVPMENUGFX_SMALL_FONT];
	image = (AvPTexture*)gfxPtr->ImagePtr;
*/
/*
	D3DInfo temp;

	temp = GetD3DInfo();

	D3DSURFACE_DESC surface_desc;
	temp.lpD3DBackSurface->GetDesc(&surface_desc);

	D3DLOCKED_RECT lock = {0,NULL};

	temp.lpD3DBackSurface->LockRect(&lock, NULL, 0); if (lock.pBits == NULL) return 0;
*/
	while( *textPtr )
	{
		char c = *textPtr++;

		if (c>=' ')
		{
			int topLeftU = 1+((c-32)&15)*16;
			int topLeftV = 1+((c-32)>>4)*16;

			DrawSmallMenuCharacter(sx, sy, topLeftU, topLeftV, alphaR, alphaG, alphaB, alpha);

//			srcPtr = &image->buffer[(topLeftU+topLeftV*image->width)*4];

			for (int y=sy; y<HUD_FONT_HEIGHT+sy; y++)
			{
//				destPtr = (unsigned short *)(((unsigned char *)lock.pBits)+y*lock.Pitch) + sx;

				for (int x=0; x<HUD_FONT_WIDTH; x++)
				{
/*
					if (srcPtr[0] || srcPtr[1] || srcPtr[2])
					{
						unsigned int destR, destG, destB;
						
						destR = (*destPtr & 0xF800)>>8;
						destG = (*destPtr & 0x07E0)>>3;
						destB = (*destPtr & 0x001F)<<3;
						
						destR += MUL_FIXED(alphaR, srcPtr[0]);
						destG += MUL_FIXED(alphaG, srcPtr[1]);
						destB += MUL_FIXED(alphaB, srcPtr[2]);
						if (destR > 0x00FF) destR = 0x00FF;
						if (destG > 0x00FF) destG = 0x00FF;
						if (destB > 0x00FF) destB = 0x00FF;
						
						*destPtr =	((destR>>3)<<11) |
								((destG>>2)<<5 ) |
								((destB>>3));
					}
					destPtr++;
					srcPtr += 4;
*/
				} 
//				srcPtr += (image->width - HUD_FONT_WIDTH) * 4;	
			}

			sx += AAFontWidths[(unsigned int) c];
		}
	}
//	temp.lpD3DBackSurface->UnlockRect();
//#endif
	return sx;
}

extern void RenderSmallFontString_Wrapped(const char *textPtr,RECT* area,int alpha,int* output_x,int* output_y)
{
	// text on menus in bottom black space

	unsigned char *srcPtr;
//	unsigned short *destPtr;
	AVPMENUGFX *gfxPtr;
	AvPTexture *image;
	int wordWidth;
	int sx = area->left;
	int sy = area->top;
	
	gfxPtr = &AvPMenuGfxStorage[AVPMENUGFX_SMALL_FONT];
	image = (AvPTexture*)gfxPtr->ImagePtr;

/*
Determine area used by text , so we can draw it centrally
*/                        	
{
	const char *textPtr2=textPtr;
	while (*textPtr2) {
		int widthFromSpaces=0;
		int widthFromChars=0;
		
		while(*textPtr2 && *textPtr2==' ') {
			widthFromSpaces+=AAFontWidths[(unsigned int) *textPtr2++];
		}
		
		while(*textPtr2 && *textPtr2!=' ') {
			widthFromChars+=AAFontWidths[(unsigned int) *textPtr2++];
		}
		
		wordWidth=widthFromSpaces+widthFromChars;
		
		if(wordWidth> area->right-sx) {
			if(wordWidth >area->right-area->left) {
				int extraLinesNeeded=0;
				
				wordWidth-=(area->right-sx);
				
				sy+=HUD_FONT_HEIGHT;
				sx=area->left;
				
				extraLinesNeeded=wordWidth/(area->right-area->left);
				
				sy+=HUD_FONT_HEIGHT*extraLinesNeeded;
				wordWidth %= (area->right-area->left);
				
				if(sy+HUD_FONT_HEIGHT> area->bottom) break;
			} else {
				sy+=HUD_FONT_HEIGHT;
				sx=area->left;
				
				if(sy+HUD_FONT_HEIGHT> area->bottom) break;
				
				if(wordWidth> area->right-sx) break;
				
				wordWidth-=widthFromSpaces;
			}
		}
		sx+=wordWidth;
	}
	
	if(sy==area->top) {
		sx=area->left+ (area->right-sx)/2;
	} else {
		sx=area->left;
	}
	
	sy+=HUD_FONT_HEIGHT;
	if(sy<area->bottom) {
		sy=area->top + (area->bottom-sy)/2;
	} else {
		sy=area->top;
	}
}
/*
	D3DInfo temp;

	temp = GetD3DInfo();

	D3DSURFACE_DESC surface_desc;
	temp.lpD3DBackSurface->GetDesc(&surface_desc);

	D3DLOCKED_RECT lock = {0,NULL};

	temp.lpD3DBackSurface->LockRect(&lock, NULL, 0); if (lock.pBits == NULL) return;
*/	
	while ( *textPtr ) {
		const char* textPtr2=textPtr;
		wordWidth=0;
		
		while(*textPtr2 && *textPtr2==' ') {
			wordWidth+=AAFontWidths[(unsigned int) *textPtr2++];
		}
		
		while(*textPtr2 && *textPtr2!=' ') {
			wordWidth+=AAFontWidths[(unsigned int) *textPtr2++];
		}
		
		if(wordWidth> area->right-sx) {
			if(wordWidth>area->right - area->left) {
				/* 
				  word is too long too fit on one line 
				  so we'll just have to allow it to be split
				 */
			} else {
				sy+=HUD_FONT_HEIGHT;
				sx=area->left;
				
				if(sy+HUD_FONT_HEIGHT> area->bottom) break;
				
				if(wordWidth> area->right-sx) break;
				
				while(*textPtr && *textPtr==' ') {
					textPtr++;
				}
			}
		}
		
		while(*textPtr && *textPtr==' ') {
			sx+=AAFontWidths[(unsigned int) *textPtr++];
		}
		
		if(sx>area->right) {
			while(sx>area->right) {
				sx-=(area->right-area->left);
				sy+=HUD_FONT_HEIGHT;
			}
			
			if(sy+HUD_FONT_HEIGHT> area->bottom) break;
		}
		
		while(*textPtr && *textPtr!=' ') {
			char c = *textPtr++;
			int letterWidth = AAFontWidths[(unsigned int) c];
			
			if(sx+letterWidth>area->right) {
				sx=area->left;
				sy+=HUD_FONT_HEIGHT;
				
				if(sy+HUD_FONT_HEIGHT> area->bottom) break;
			}
			
			if (c>=' ' || c<='z') {
				int topLeftU = 1+((c-32)&15)*16;
				int topLeftV = 1+((c-32)>>4)*16;
				int x, y;
				
				srcPtr = &image->buffer[(topLeftU+topLeftV*image->width)*4];

				DrawSmallMenuCharacter(sx, sy, topLeftU, topLeftV, ONE_FIXED, ONE_FIXED, ONE_FIXED, alpha);
				
				for (y=sy; y<HUD_FONT_HEIGHT+sy; y++) {

//					destPtr = (unsigned short *)(((unsigned char *)lock.pBits) + y*lock.Pitch) + sx;
					
					for (x=0; x<HUD_FONT_WIDTH; x++) {
/*
						if (srcPtr[0] || srcPtr[1] || srcPtr[2]) {
							unsigned int destR, destG, destB;
						
							destR = (*destPtr & 0xF800)>>8;
							destG = (*destPtr & 0x07E0)>>3;
							destB = (*destPtr & 0x001F)<<3;
						
							destR += MUL_FIXED(alpha, srcPtr[0]);
							destG += MUL_FIXED(alpha, srcPtr[1]);
							destB += MUL_FIXED(alpha, srcPtr[2]);
							if (destR > 0x00FF) destR = 0x00FF;
							if (destG > 0x00FF) destG = 0x00FF;
							if (destB > 0x00FF) destB = 0x00FF;
						
							*destPtr =	((destR>>3)<<11) |
									((destG>>2)<<5 ) |
									((destB>>3));
						}
*/
//						srcPtr += 4;
//						destPtr++;
					}
//					srcPtr += (image->w - HUD_FONT_WIDTH) * 4;	
				}

				sx += AAFontWidths[(unsigned int) c];
			}
		}
	}
	
//	temp.lpD3DBackSurface->UnlockRect();
	
	if(output_x) *output_x=sx;
	if(output_y) *output_y=sy;
}

extern void LoadAvPMenuGfx(enum AVPMENUGFX_ID menuGfxID)
{
	AVPMENUGFX *gfxPtr;
	char buffer[100];
	
	GLOBALASSERT(menuGfxID < MAX_NO_OF_AVPMENUGFXS);

	gfxPtr = &AvPMenuGfxStorage[menuGfxID];

//#if 0 // bjd	
	CL_GetImageFileName(buffer, 100, gfxPtr->FilenamePtr, LIO_RELATIVEPATH);
	
	//see if graphic can be found in fast file
	unsigned int fastFileLength;
	void const * pFastFileData = ffreadbuf(buffer,&fastFileLength);

	if(pFastFileData)
	{
		//D3DTexture *ImagePtr; 

		/* bjd
		I think we use AwCreateSurface to tell the engine we'll be loading non-power of 2 
		or image files for "raw" pixel by pixel drawing.

		Eg, linux code:

			if (pixelFormat.texB) {
				Tex->buf = NULL; 
				CreateOGLTexture(Tex, buf); 
				free(buf);
			} else {
				Tex->buf = buf; 
				CreateIMGSurface(Tex, buf);
			}
			
		Only create a texture (d3d texture, ogl texture etc) IF pixelFormat.texB is true.
		.texB being true means its a TEXTURE, not surface creation function we called.
		We don't need to keep a copy of Tex->buf with this.

		If .textB is false and we're dealing with a surface, stick all the buffered image data 
		into tex->buf so we can do pixel by pixel drawing with it later
		*/

		//load from fast file
		gfxPtr->ImagePtr = AwCreateSurface
							(
								"pxfXYB",
								pFastFileData,
								fastFileLength,
								AW_TLF_TRANSP|AW_TLF_CHROMAKEY,
								&(gfxPtr->Width),
								&(gfxPtr->Height),
								&(gfxPtr->hBackup)
							);
	}
	else
	{
		//load graphic from rim file
		gfxPtr->ImagePtr = AwCreateSurface
							(
								"sfXYB",
								buffer,
								AW_TLF_TRANSP|AW_TLF_CHROMAKEY,
								&(gfxPtr->Width),
								&(gfxPtr->Height),
								&(gfxPtr->hBackup)
							);

	}

	if (gfxPtr->ImagePtr) 
	{
		AvPMenuGfxStorage[menuGfxID].menuTexture = CreateD3DTexturePadded((AvPTexture*)gfxPtr->ImagePtr, &gfxPtr->newHeight, &gfxPtr->newWidth);

		if( AvPMenuGfxStorage[menuGfxID].menuTexture == NULL) {
			OutputDebugString("Texture in AvPMenuGfxStorage was NULL!");
		}
	}

	GLOBALASSERT(gfxPtr->ImagePtr);
//	GLOBALASSERT(gfxPtr->hBackup);
	GLOBALASSERT(gfxPtr->Width>0);
	GLOBALASSERT(gfxPtr->Height>0);
	gfxPtr->hBackup=0;

//	ATIncludeSurface(gfxPtr->ImagePtr,gfxPtr->hBackup);
//#endif
}

static void ReleaseAvPMenuGfx(enum AVPMENUGFX_ID menuGfxID)
{
	AVPMENUGFX *gfxPtr;
	
	GLOBALASSERT(menuGfxID < MAX_NO_OF_AVPMENUGFXS);
		
	gfxPtr = &AvPMenuGfxStorage[menuGfxID];
	
	GLOBALASSERT(gfxPtr);
	GLOBALASSERT(gfxPtr->ImagePtr);

  //	ATRemoveSurface(gfxPtr->ImagePtr);
	ReleaseDDSurface(gfxPtr->ImagePtr);
	
	gfxPtr->ImagePtr = NULL;

	#if 0
	if (gfxPtr->hBackup)
	{
		AwDestroyBackupTexture(gfxPtr->hBackup);
		gfxPtr->hBackup = NULL;
	}
	#endif

	/* release d3d texture */
	SAFE_RELEASE(gfxPtr->menuTexture);
}

extern void LoadAllAvPMenuGfx(void)
{
	int i = 0;

	while(i < AVPMENUGFX_WINNER_SCREEN)
	{
		LoadAvPMenuGfx((enum AVPMENUGFX_ID)i++);
	}

	LoadMenuFont();	
	
	{	
		unsigned char *srcPtr;

		AVPMENUGFX *gfxPtr = &AvPMenuGfxStorage[AVPMENUGFX_CLOUDY];
		
		AvPTexture *image;

		image = (AvPTexture*)gfxPtr->ImagePtr;
		srcPtr = image->buffer;
	  		
		int x,y;
		extern int CloudTable[128][128];

			for (y = 0; y < gfxPtr->Height; y++)
			{
				for (x = 0; x < gfxPtr->Width; x++)
				{
					int r = srcPtr[0];

					r = DIV_FIXED(r, 0xFF);
					CloudTable[x][y] = r;
			
					srcPtr += 4;
				}
			}
		}

	CalculateWidthsOfAAFont();
}

extern void LoadAllSplashScreenGfx(void)
{
	int i = AVPMENUGFX_SPLASH_SCREEN1;
	while(i<MAX_NO_OF_AVPMENUGFXS)
	{
		LoadAvPMenuGfx((enum AVPMENUGFX_ID)i++);
	}
}

extern void InitialiseMenuGfx(void)
{
	int i=0;
	while(i<MAX_NO_OF_AVPMENUGFXS)
	{
		AvPMenuGfxStorage[i++].ImagePtr = 0;
	}
}

extern void ReleaseAllAvPMenuGfx(void)
{
	int i=0;
	while(i<MAX_NO_OF_AVPMENUGFXS)
	{
		if (AvPMenuGfxStorage[i].ImagePtr)
		{
			ReleaseAvPMenuGfx((enum AVPMENUGFX_ID)i);
		}
		i++;
	}	
	UnloadMenuFont();
}

extern void DrawAvPMenuGfx(enum AVPMENUGFX_ID menuGfxID, int topleftX, int topleftY, int alpha,enum AVPMENUFORMAT_ID format)
{		
	AVPMENUGFX *gfxPtr;
//	D3DTexture *image;
	
//	unsigned short *destPtr;
//	unsigned char *srcPtr;
	
	GLOBALASSERT(menuGfxID < MAX_NO_OF_AVPMENUGFXS);

	gfxPtr = &AvPMenuGfxStorage[menuGfxID];
//	image = (D3DTexture*)gfxPtr->ImagePtr; // image is our texture

	switch(format)
	{
		default:
		GLOBALASSERT("UNKNOWN TEXT FORMAT"==0);
		case AVPMENUFORMAT_LEFTJUSTIFIED:
		{
			// supplied x is correct
			break;
		}
		case AVPMENUFORMAT_RIGHTJUSTIFIED:
		{
			topleftX -= gfxPtr->Width;
			break;
		}
		case AVPMENUFORMAT_CENTREJUSTIFIED:
		{
			topleftX -= gfxPtr->Width/2;
			break;
		}
	}

	int length = gfxPtr->Width;
	if (ScreenDescriptorBlock.SDB_Width - topleftX < length)
	{
		length = ScreenDescriptorBlock.SDB_Width - topleftX;
	}
	if (length <= 0) return;

	if (alpha > ONE_FIXED) // ONE_FIXED = 65536
	{
		DrawMenuQuad(topleftX, topleftY, topleftX, topleftY, menuGfxID, FALSE);
	}
	else {
		DrawAlphaMenuQuad(topleftX, topleftY, topleftX, topleftY, menuGfxID, alpha);
	}
#if 0 // bjd
	D3DInfo temp;

	temp = GetD3DInfo();

	D3DSURFACE_DESC surface_desc;
	temp.lpD3DBackSurface->GetDesc(&surface_desc);

	D3DLOCKED_RECT lock = {0,NULL};

	temp.lpD3DBackSurface->LockRect(&lock, NULL, 0); if (lock.pBits == NULL) return;

	srcPtr = (unsigned char *)image->buf;
	int length = gfxPtr->Width;

	int BackBufferPitch = 0;

	if (ScreenDescriptorBlock.SDB_Width - topleftX < length)
	{
		length = ScreenDescriptorBlock.SDB_Width - topleftX;
	}
	if (length <= 0) return;

	if (alpha > ONE_FIXED) // ONE_FIXED = 65536
	{
		int x,y;

		for (y = topleftY; y < gfxPtr->Height+topleftY; y++)
		{
			destPtr = ((unsigned short *)(((unsigned char *)lock.pBits) + y*lock.Pitch)) + topleftX;
			//destPtr = (unsigned short *)(ScreenBuffer + topleftX*2 + y*BackBufferPitch);

			for (x=0; x<length; x++)
			{
				*destPtr =	((srcPtr[0]>>3)<<11) |
					((srcPtr[1]>>2)<<5 ) |
					((srcPtr[2]>>3));
				srcPtr += 4;
				destPtr++;
			}
			srcPtr += (image->w - length) * 4; 
		}
	}
	else
	{
		int x,y;

		for (y=topleftY; y<gfxPtr->Height+topleftY; y++)
		{
			//destPtr = (unsigned short *)(ScreenBuffer + topleftX*2 + y*BackBufferPitch);
			destPtr = ((unsigned short *)(((unsigned char *)lock.pBits) + y*lock.Pitch)) + topleftX;

			for (x=0; x<length; x++)
			{
				if (srcPtr[0] || srcPtr[1] || srcPtr[2]) {
					unsigned int destR, destG, destB;
						
					destR = (*destPtr & 0xF800)>>8;
					destG = (*destPtr & 0x07E0)>>3;
					destB = (*destPtr & 0x001F)<<3;
						
					destR += MUL_FIXED(alpha, srcPtr[0]);
					destG += MUL_FIXED(alpha, srcPtr[1]);
					destB += MUL_FIXED(alpha, srcPtr[2]);
					if (destR > 0x00FF) destR = 0x00FF;
					if (destG > 0x00FF) destG = 0x00FF;
					if (destB > 0x00FF) destB = 0x00FF;
						
					*destPtr =	((destR>>3)<<11) |
							((destG>>2)<<5 ) |
							((destB>>3));
				}

				srcPtr += 4;
				destPtr++;
			}
			srcPtr += (image->w - length) * 4; 
		}
	}

	temp.lpD3DBackSurface->UnlockRect();
#endif
}

extern void DrawAvPMenuGlowyBar(int topleftX, int topleftY, int alpha, int length)
{	
	enum AVPMENUGFX_ID menuGfxID = AVPMENUGFX_GLOWY_MIDDLE;										
/*
   	unsigned short *destPtr;
	unsigned char *srcPtr;
	AVPMENUGFX *gfxPtr;
	D3DTexture *image;
*/
	//GLOBALASSERT(menuGfxID < MAX_NO_OF_AVPMENUGFXS);
//	gfxPtr = &AvPMenuGfxStorage[menuGfxID];

	if (ScreenDescriptorBlock.SDB_Width - topleftX < length)
	{
		length = ScreenDescriptorBlock.SDB_Width - topleftX;
	}

	if (length<0) length = 0;

//	DrawMenuQuad(topleftX, topleftY, topleftX, topleftY, menuGfxID, alpha);

	// the image we're working with is only 1 pixel wide. we'll have to draw it a couple of 
	//times shifting the x position over 1 pixel each time until we reach the length of the bar
	for (int i = 0; i < length; i++) {
		if (alpha > ONE_FIXED) // ONE_FIXED = 65536
		{
			DrawMenuQuad(topleftX + i, topleftY, topleftX + i, topleftY, menuGfxID, FALSE);
		}
		else {
			DrawAlphaMenuQuad(topleftX + i, topleftY, topleftX + i, topleftY, menuGfxID, alpha);
		}
	}
#if 0 // bjd
	image = (D3DTexture*)gfxPtr->ImagePtr;
	srcPtr = image->buf;

	D3DInfo temp;

	temp = GetD3DInfo();

	D3DSURFACE_DESC surface_desc;
	temp.lpD3DBackSurface->GetDesc(&surface_desc);

	D3DLOCKED_RECT lock = {0,NULL};

	temp.lpD3DBackSurface->LockRect(&lock, NULL, 0); if (lock.pBits == NULL) return;

	if (ScreenDescriptorBlock.SDB_Width - topleftX < length)
	{
		length = ScreenDescriptorBlock.SDB_Width - topleftX;
	}

	if (length<0) length = 0;

	if (alpha>ONE_FIXED)
	{
		int x,y;

		for (y=topleftY; y<gfxPtr->Height+topleftY; y++)
		{
			//destPtr = (unsigned short *)(ScreenBuffer + topleftX*2 + y*BackBufferPitch);
			destPtr = ((unsigned short *)(((unsigned char *)lock.pBits) + y*lock.Pitch)) + topleftX;

			for (x=0; x<length; x++)
			{
				//*destPtr = *srcPtr;
				*destPtr =	((srcPtr[0]>>3)<<11) |
						((srcPtr[1]>>2)<<5 ) |
						((srcPtr[2]>>3));
				destPtr++;
			}
			//srcPtr += (ddsdimage.lPitch/2); 
			srcPtr += image->w * 4;
		}
	}
	else
	{
		int x,y;

		for (y=topleftY; y<gfxPtr->Height+topleftY; y++)
		{
			//destPtr = (unsigned short *)(ScreenBuffer + topleftX*2 + y*BackBufferPitch);
			destPtr = ((unsigned short *)(((unsigned char *)lock.pBits) + y*lock.Pitch)) + topleftX;

			for (x=0; x<length; x++)
			{
				//if (*srcPtr)
				if (srcPtr[0] || srcPtr[1] || srcPtr[2])
				{
					//unsigned int srcR,srcG,srcB;
					unsigned int destR,destG,destB;

					destR = (*destPtr & 0xF800)>>8;
					destG = (*destPtr & 0x07E0)>>3;
					destB = (*destPtr & 0x001F)<<3;
						
					destR += MUL_FIXED(alpha, srcPtr[0]);
					destG += MUL_FIXED(alpha, srcPtr[1]);
					destB += MUL_FIXED(alpha, srcPtr[2]);
					if (destR > 0x00FF) destR = 0x00FF;
					if (destG > 0x00FF) destG = 0x00FF;
					if (destB > 0x00FF) destB = 0x00FF;
						
					*destPtr =	((destR>>3)<<11) |
							((destG>>2)<<5 ) |
							((destB>>3));

				}
				destPtr++;
			}
			//srcPtr += (ddsdimage.lPitch/2); 
			srcPtr += image->w * 4;
		}
	}

	temp.lpD3DBackSurface->UnlockRect();
#endif
}

extern void DrawAvPMenuGlowyBar_Clipped(int topleftX, int topleftY, int alpha, int length, int topY, int bottomY)
{		
#if 0
	enum AVPMENUGFX_ID menuGfxID = AVPMENUGFX_GLOWY_MIDDLE;	
	GLOBALASSERT(menuGfxID < MAX_NO_OF_AVPMENUGFXS);

	if (length<0) length = 0;

//	DrawMenuQuad(topleftX, topleftY, topleftX, topleftY, menuGfxID, alpha);

	// the image we're working with is only 1 pixel wide. we'll have to draw it a couple of 
	//times shifting the x position over each time until we reach the length of the bar
	for (int i = 0; i < length; i++) {
		if (alpha > ONE_FIXED) // ONE_FIXED = 65536
		{
			DrawMenuQuad(topleftX + i, topleftY, topleftX + i, topleftY, menuGfxID, FALSE);
		}
		else {
			DrawAlphaMenuQuad(topleftX + i, topleftY, topleftX + i, topleftY, menuGfxID, alpha);
		}
	}

#endif
#if 0 // bjd
   	unsigned short *destPtr;
	unsigned char *srcPtr;
	AVPMENUGFX *gfxPtr;
	D3DTexture *image;

	GLOBALASSERT(menuGfxID < MAX_NO_OF_AVPMENUGFXS);
	gfxPtr = &AvPMenuGfxStorage[menuGfxID];
	image = (D3DTexture*)gfxPtr->ImagePtr;

	srcPtr = image->buf;

	D3DInfo temp;

	temp = GetD3DInfo();

	D3DSURFACE_DESC surface_desc;
	temp.lpD3DBackSurface->GetDesc(&surface_desc);

	D3DLOCKED_RECT lock = {0,NULL};

	temp.lpD3DBackSurface->LockRect(&lock, NULL, 0); if (lock.pBits == NULL) return;

	if (length<0) length = 0;

	if (alpha>ONE_FIXED)
	{
		int x,y;

		for (y=topleftY; y<gfxPtr->Height+topleftY; y++)
		{
			if(y>=topY && y<=bottomY)
			{
				//destPtr = (unsigned short *)(ScreenBuffer + topleftX*2 + y*BackBufferPitch);
				destPtr = (unsigned short *)(((unsigned char *)lock.pBits) + y*lock.Pitch) + topleftX;

				for (x=0; x<length; x++)
				{
					*destPtr =	((srcPtr[0]>>3)<<11) |
							((srcPtr[1]>>2)<<5 ) |
							((srcPtr[2]>>3));
					destPtr++;
				}
				//srcPtr += (ddsdimage.lPitch/2); 
				srcPtr += image->w * 4;
			}
		}
	}
	else
	{
		int x,y;

		for (y=topleftY; y<gfxPtr->Height+topleftY; y++)
		{
			if(y>=topY && y<=bottomY)
			{
				//destPtr = (unsigned short *)(ScreenBuffer + topleftX*2 + y*BackBufferPitch);
				destPtr = ((unsigned short *)(((unsigned char *)lock.pBits) + y*lock.Pitch)) + topleftX;

				for (x=0; x<length; x++)
				{
					//if (*srcPtr)
					if (srcPtr[0] || srcPtr[1] || srcPtr[2])
					{
						//unsigned int srcR,srcG,srcB;
						unsigned int destR,destG,destB;

						destR = (*destPtr & 0xF800)>>8;
						destG = (*destPtr & 0x07E0)>>3;
						destB = (*destPtr & 0x001F)<<3;
						
						destR += MUL_FIXED(alpha, srcPtr[0]);
						destG += MUL_FIXED(alpha, srcPtr[1]);
						destB += MUL_FIXED(alpha, srcPtr[2]);
						if (destR > 0x00FF) destR = 0x00FF;
						if (destG > 0x00FF) destG = 0x00FF;
						if (destB > 0x00FF) destB = 0x00FF;
						
						*destPtr =	((destR>>3)<<11) |
								((destG>>2)<<5 ) |
								((destB>>3));
					}
					destPtr++;
				}
			}
			//srcPtr += (ddsdimage.lPitch/2); 
			srcPtr += image->w * 4;
		}
	}
   	
	temp.lpD3DBackSurface->UnlockRect();
#endif
}


extern void DrawAvPMenuGfx_CrossFade(enum AVPMENUGFX_ID menuGfxID,enum AVPMENUGFX_ID menuGfxID2,int alpha)
{	

	OutputDebugString("\n does this even ever get called?");
#if 0 // bjd
//	LockSurfaceAndGetBufferPointer();

//	DDSURFACEDESC ddsdimage,ddsdimage2;

	char buf[100];
	sprintf(buf, "\n tex1: %d, tex2: %d", menuGfxID, menuGfxID2);
	OutputDebugString(buf);

   	unsigned short *destPtr;
	unsigned char  *srcPtr;
	unsigned char  *srcPtr2;
	
	AVPMENUGFX *gfxPtr;
	AVPMENUGFX *gfxPtr2;
	D3DTexture *image;
	D3DTexture *image2;

	GLOBALASSERT(menuGfxID < MAX_NO_OF_AVPMENUGFXS);
	gfxPtr = &AvPMenuGfxStorage[menuGfxID];
	image = (D3DTexture*)gfxPtr->ImagePtr;
	
	GLOBALASSERT(menuGfxID2 < MAX_NO_OF_AVPMENUGFXS);
	gfxPtr2 = &AvPMenuGfxStorage[menuGfxID2];
	image2 = (D3DTexture*)gfxPtr2->ImagePtr;

	D3DInfo temp;

	temp = GetD3DInfo();

	D3DSURFACE_DESC surface_desc;
	temp.lpD3DBackSurface->GetDesc(&surface_desc);

	D3DLOCKED_RECT lock = {0,NULL};

	temp.lpD3DBackSurface->LockRect(&lock, NULL, 0); if (lock.pBits == NULL) return;

	srcPtr = image->buffer;
	srcPtr2 = image2->buffer;

	int length = 640;

	if (alpha==ONE_FIXED)
	{
		int x,y;

		for (y=0; y<480; y++)
		{
			//destPtr = (unsigned int *)(ScreenBuffer + y*BackBufferPitch);
			destPtr = ((unsigned short *)(((unsigned char *)lock.pBits) + y*lock.Pitch));

			for (x=0; x<640; x++)
			{
				*destPtr =	((srcPtr[0]>>3)<<11) |
						((srcPtr[1]>>2)<<5 ) |
						((srcPtr[2]>>3));
				srcPtr += 4;
				destPtr++;

			}
			//srcPtr += (ddsdimage.lPitch/4) - 320; 
			srcPtr += (image->width - length) * 4;
		}
	}
	else
	{
		int x,y;

		for (y=0; y<480; y++)
		{
			//destPtr = (unsigned int *)(ScreenBuffer + y*BackBufferPitch);
			destPtr = ((unsigned short *)(((unsigned char *)lock.pBits) + y*lock.Pitch));

			for (x=0; x<640; x++)
			{
				unsigned int srcR1, srcR2;
				unsigned int srcG1, srcG2;
				unsigned int srcB1, srcB2;
				
				srcR1 = srcPtr[0];
				srcR2 = srcPtr2[0];
				srcG1 = srcPtr[1];
				srcG2 = srcPtr2[1];
				srcB1 = srcPtr[2];
				srcB2 = srcPtr2[2];
				
				srcR2 = MUL_FIXED(ONE_FIXED-alpha,srcR2)+MUL_FIXED(alpha,srcR1);
				srcG2 = MUL_FIXED(ONE_FIXED-alpha,srcG2)+MUL_FIXED(alpha,srcG1);
				srcB2 = MUL_FIXED(ONE_FIXED-alpha,srcB2)+MUL_FIXED(alpha,srcB1);
				
				*destPtr =	((srcR2>>3)<<11) |
						((srcG2>>2)<<5 ) |
						((srcB2>>3));
				srcPtr += 4;
				srcPtr2 += 4;
				destPtr++;

			}
			//srcPtr += (ddsdimage.lPitch/4) - 320; 
			//srcPtr2 += (ddsdimage2.lPitch/4) - 320; 
		}
	}
	temp.lpD3DBackSurface->UnlockRect();
#endif
}

extern void DrawAvPMenuGfx_Faded(enum AVPMENUGFX_ID menuGfxID, int topleftX, int topleftY, int alpha,enum AVPMENUFORMAT_ID format)
{	

	AVPMENUGFX *gfxPtr;
	gfxPtr = &AvPMenuGfxStorage[menuGfxID];

	switch(format)
	{
		default:
		GLOBALASSERT("UNKNOWN TEXT FORMAT"==0);
		case AVPMENUFORMAT_LEFTJUSTIFIED:
		{
			// supplied x is correct
			break;
		}
		case AVPMENUFORMAT_RIGHTJUSTIFIED:
		{
			topleftX -= gfxPtr->Width;
			break;
		}
		case AVPMENUFORMAT_CENTREJUSTIFIED:
		{
			topleftX -= gfxPtr->Width/2;
			break;
		}
	}
	
	int length = gfxPtr->Width;

	if (ScreenDescriptorBlock.SDB_Width - topleftX < length) {
		length = ScreenDescriptorBlock.SDB_Width - topleftX;
	}
	if (length <= 0) return;


	if (alpha > ONE_FIXED) // ONE_FIXED = 65536
	{
		DrawMenuQuad(topleftX, topleftY, topleftX, topleftY, menuGfxID, FALSE);
	}
	else {
		//DrawAlphaMenuQuad(topleftX, topleftY, topleftX, topleftY, menuGfxID, alpha);
//		DrawMenuQuad(topleftX, topleftY, topleftX, topleftY, menuGfxID, FALSE);
		DrawTexturedFadedQuad(topleftX, topleftY, menuGfxID, alpha);
	}

//	DrawTexturedFadedQuad(topleftX, topleftY, menuGfxID, alpha);

//	DrawMenuQuad(topleftX, topleftY, 0, 0, menuGfxID, FALSE);


#if 0 // bjd
   	unsigned short *destPtr;
	unsigned char *srcPtr;

	AVPMENUGFX *gfxPtr;
	D3DTexture *image;

	int length;

	GLOBALASSERT(menuGfxID < MAX_NO_OF_AVPMENUGFXS);
	gfxPtr = &AvPMenuGfxStorage[menuGfxID];
	image = (D3DTexture*)gfxPtr->ImagePtr;

	switch(format)
	{
		default:
		GLOBALASSERT("UNKNOWN TEXT FORMAT"==0);
		case AVPMENUFORMAT_LEFTJUSTIFIED:
		{
			// supplied x is correct
			break;
		}
		case AVPMENUFORMAT_RIGHTJUSTIFIED:
		{
			topleftX -= gfxPtr->Width;
			break;
		}
		case AVPMENUFORMAT_CENTREJUSTIFIED:
		{
			topleftX -= gfxPtr->Width/2;
			break;
		}
	}

	srcPtr = (unsigned char *)image->buf;
	length = gfxPtr->Width;

	if (ScreenDescriptorBlock.SDB_Width - topleftX < length) {
		length = ScreenDescriptorBlock.SDB_Width - topleftX;
	}
	if (length <= 0) return;

	D3DInfo temp;

	temp = GetD3DInfo();

	D3DSURFACE_DESC surface_desc;
	temp.lpD3DBackSurface->GetDesc(&surface_desc);

	D3DLOCKED_RECT lock = {0,NULL};

	temp.lpD3DBackSurface->LockRect(&lock, NULL, 0); if (lock.pBits == NULL) return;

	int x, y;

		for (y=topleftY; y<gfxPtr->Height+topleftY; y++)
		{
			destPtr = ((unsigned short *)(((unsigned char *)lock.pBits) + y*lock.Pitch)) + topleftX;

			for (x=0; x<gfxPtr->Width; x++)
			{
				unsigned int srcR,srcG,srcB;

				if (srcPtr[0] || srcPtr[1] || srcPtr[2])
				{
					srcR = MUL_FIXED(srcPtr[0], alpha);
					srcG = MUL_FIXED(srcPtr[1], alpha);
					srcB = MUL_FIXED(srcPtr[2], alpha);
					*destPtr =	((srcR>>3)<<11) |
							((srcG>>2)<<5 ) |
							((srcB>>3));
				}
				else
				{
					*destPtr = 0;
				}

				destPtr++;
				srcPtr += 4;
			}
			srcPtr += (image->w - length) * 4;
		}
	temp.lpD3DBackSurface->UnlockRect();
#endif
}


extern void DrawAvPMenuGfx_Clipped(enum AVPMENUGFX_ID menuGfxID, int topleftX, int topleftY, int alpha,enum AVPMENUFORMAT_ID format, int topY, int bottomY)
{	
	GLOBALASSERT(menuGfxID < MAX_NO_OF_AVPMENUGFXS);

	AVPMENUGFX *gfxPtr;
	gfxPtr = &AvPMenuGfxStorage[menuGfxID];

	switch(format)
	{
		default:
		GLOBALASSERT("UNKNOWN TEXT FORMAT"==0);
		case AVPMENUFORMAT_LEFTJUSTIFIED:
		{
			// supplied x is correct
			break;
		}
		case AVPMENUFORMAT_RIGHTJUSTIFIED:
		{
			topleftX -= gfxPtr->Width;
			break;
		}
		case AVPMENUFORMAT_CENTREJUSTIFIED:
		{
			topleftX -= gfxPtr->Width/2;
			break;
		}
	}

	int length = gfxPtr->Width;

	if (ScreenDescriptorBlock.SDB_Width - topleftX < length) {
		length = ScreenDescriptorBlock.SDB_Width - topleftX;
	}
	if (length <= 0) return;

//	DrawMenuQuad(topleftX, topleftY, topleftX, topleftY, menuGfxID, alpha);

	if (alpha > ONE_FIXED) // ONE_FIXED = 65536
	{
		DrawMenuQuad(topleftX, topleftY, topleftX, topleftY, menuGfxID, FALSE);
	}
	else {
//		DrawMenuQuad(topleftX, topleftY, topleftX, topleftY, menuGfxID, TRUE);
		DrawAlphaMenuQuad(topleftX, topleftY, topleftX, topleftY, menuGfxID, alpha);
	}

#if 0 // bjd
   	unsigned short *destPtr;
	unsigned char *srcPtr;
	AVPMENUGFX *gfxPtr;
	D3DTexture *image;

	GLOBALASSERT(menuGfxID < MAX_NO_OF_AVPMENUGFXS);
	gfxPtr = &AvPMenuGfxStorage[menuGfxID];
	image = (D3DTexture*)gfxPtr->ImagePtr;

	switch(format)
	{
		default:
		GLOBALASSERT("UNKNOWN TEXT FORMAT"==0);
		case AVPMENUFORMAT_LEFTJUSTIFIED:
		{
			// supplied x is correct
			break;
		}
		case AVPMENUFORMAT_RIGHTJUSTIFIED:
		{
			topleftX -= gfxPtr->Width;
			break;
		}
		case AVPMENUFORMAT_CENTREJUSTIFIED:
		{
			topleftX -= gfxPtr->Width/2;
			break;
		}
	}

	srcPtr = (unsigned char *)image->buf;

	int length = gfxPtr->Width;

	if (ScreenDescriptorBlock.SDB_Width - topleftX < length) {
		length = ScreenDescriptorBlock.SDB_Width - topleftX;
	}
	if (length <= 0) return;

	D3DInfo temp;

	temp = GetD3DInfo();

	D3DSURFACE_DESC surface_desc;
	temp.lpD3DBackSurface->GetDesc(&surface_desc);

	D3DLOCKED_RECT lock = {0,NULL};

	temp.lpD3DBackSurface->LockRect(&lock, NULL, 0); if (lock.pBits == NULL) return;


	if (alpha>ONE_FIXED)
	{
		int x,y;

		for (y=topleftY; y<gfxPtr->Height+topleftY; y++)
		{
			//destPtr = (unsigned short *)(ScreenBuffer + topleftX*2 + y*BackBufferPitch);
			destPtr = ((unsigned short *)(((unsigned char *)lock.pBits) + y*lock.Pitch)) + topleftX;
			if(y>=topY && y<=bottomY)
			{
				for (x=0; x<length; x++)
				{
					*destPtr =	((srcPtr[0]>>3)<<11) |
							((srcPtr[1]>>2)<<5 ) |
							((srcPtr[2]>>3));
					srcPtr += 4;
					destPtr++;
/*
					*destPtr = *srcPtr;
					destPtr++;
					srcPtr++;
*/
				}
				//srcPtr += (ddsdimage.lPitch/2) - gfxPtr->Width; 
				srcPtr += (image->w - length) * 4;
			}
			else
			{
				//srcPtr+=(ddsdimage.lPitch/2);
				srcPtr += image->w * 4;
			}
		}
	}
	else
	{
		int x,y;

		for (y=topleftY; y<gfxPtr->Height+topleftY; y++)
		{
			//destPtr = (unsigned short *)(ScreenBuffer + topleftX*2 + y*BackBufferPitch);
			destPtr = ((unsigned short *)(((unsigned char *)lock.pBits) + y*lock.Pitch)) + topleftX;

			if(y>=topY && y<=bottomY)
			{
				for (x=0; x<length; x++)
				{
					//if (*srcPtr)
					if (srcPtr[0] || srcPtr[1] || srcPtr[2])
					{
						unsigned int destR, destG, destB;
						
						destR = (*destPtr & 0xF800)>>8;
						destG = (*destPtr & 0x07E0)>>3;
						destB = (*destPtr & 0x001F)<<3;
						
						destR += MUL_FIXED(alpha, srcPtr[0]);
						destG += MUL_FIXED(alpha, srcPtr[1]);
						destB += MUL_FIXED(alpha, srcPtr[2]);
						if (destR > 0x00FF) destR = 0x00FF;
						if (destG > 0x00FF) destG = 0x00FF;
						if (destB > 0x00FF) destB = 0x00FF;
						
						*destPtr =	((destR>>3)<<11) |
								((destG>>2)<<5 ) |
								((destB>>3));

					}
					//destPtr++;
					//srcPtr++;
					srcPtr += 4;
					destPtr++;
				}
				//srcPtr += (ddsdimage.lPitch/2) - gfxPtr->Width; 
				srcPtr += (image->w - length) * 4;
			}
			else
			{
				//srcPtr += (ddsdimage.lPitch/2);
				srcPtr += image->w * 4;
			}
		}
	}  
	temp.lpD3DBackSurface->UnlockRect();
#endif
}

extern int HeightOfMenuGfx(enum AVPMENUGFX_ID menuGfxID)
{
	return AvPMenuGfxStorage[menuGfxID].Height; 
}

extern void ClearScreenToBlack(void)
{ 

	d3d.lpD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );

/*
	D3DSURFACE_DESC surface_desc;
	temp.lpD3DBackSurface->GetDesc(&surface_desc);

	D3DLOCKED_RECT lock = {0,NULL};

	temp.lpD3DBackSurface->LockRect(&lock, NULL, 0); if (lock.pBits == NULL) return;

	unsigned int x, y;
	unsigned short *ptr;

	for (y = 0; y < surface_desc.Height; y++) {
		ptr = (unsigned short *)(((unsigned char *)lock.pBits)+y*lock.Pitch);
		for (x = 0; x < surface_desc.Width; x++) {
			*ptr = 0;
			
			ptr++;
		}
	}

	temp.lpD3DBackSurface->UnlockRect();
*/
}

extern void FadedScreen(int alpha)
{	
#if 0 // bjd - replaced
	unsigned int x, y;
	unsigned short *ptr;

	D3DInfo temp;

	temp = GetD3DInfo();

	D3DSURFACE_DESC surface_desc;
	temp.lpD3DBackSurface->GetDesc(&surface_desc);

	D3DLOCKED_RECT lock = {0,NULL};

	temp.lpD3DBackSurface->LockRect(&lock, NULL, 0); if (lock.pBits == NULL) return;


	for (y = 60; y < surface_desc.Height-60; y++) {
		ptr = (unsigned short *)(((unsigned char *)lock.pBits)+y*lock.Pitch);
		for (x = 0; x < surface_desc.Width; x++) {
			unsigned int srcR, srcG, srcB;
			
			srcR = (*ptr & 0xF800) >> 11;
			srcG = (*ptr & 0x07E0) >> 5;
			srcB = (*ptr & 0x001F);
			
			srcR = MUL_FIXED(srcR, alpha);
			srcG = MUL_FIXED(srcG, alpha);
			srcB = MUL_FIXED(srcB, alpha);
			*ptr =	((srcR>>3)<<11) |
				((srcG>>2)<<5 ) |
				((srcB>>3));
			ptr++;
		}
	}

	temp.lpD3DBackSurface->UnlockRect();
#endif
}


static void CalculateWidthsOfAAFont(void)
{
	unsigned char *srcPtr;
	AVPMENUGFX *gfxPtr;
	AvPTexture *image;
	int c;
	
	gfxPtr = &AvPMenuGfxStorage[AVPMENUGFX_SMALL_FONT];
	image = (AvPTexture*)gfxPtr->ImagePtr;
	
	srcPtr = image->buffer;
	
	AAFontWidths[32]=3;
	
	for (c=33; c<255; c++) {
		int x,y;
		int x1 = 1+((c-32)&15)*16;
		int y1 = 1+((c-32)>>4)*16;
		
		AAFontWidths[c]=17;
		
		for (x=x1+HUD_FONT_WIDTH; x>x1; x--) {
			int blank = 1;
			
			for (y=y1; y<y1+HUD_FONT_HEIGHT; y++) {
				unsigned char *s = &srcPtr[(x + y*image->width) * 4];
				if (s[2] >= 0x80) {
					blank = 0;
					break;
				}
			}
			
			if (blank) {
				AAFontWidths[c]--;
			} else {
				break;
			}
		}
	}
}
	 
};