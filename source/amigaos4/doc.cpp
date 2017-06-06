/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 * Program WebSite: http://methane.sourceforge.net/index.html              *
 * Project Email: rombust@postmaster.co.uk                                 *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>

#include "SDL.h"
#include "doc.h"

#ifdef METHANE_MIKMOD
#include "../mikmod/audiodrv.h"
#endif

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

static void main_code(void);

static CMethDoc Game;

static SDL_Surface * SdlScreen = 0;
static SDL_Window * SdlWindow;
static SDL_Renderer * SdlRenderer;
static SDL_Texture * SdlTexture;

static char TheScreen[SCR_WIDTH * SCR_HEIGHT];
//static int SampleChannel = 5;	  // Used by CMethDoc::PlaySample

static SDL_bool fullscreen = SDL_TRUE;
static SDL_bool vsync = SDL_TRUE;
static const char* renderer_name = "compositing";

static char HighScoreFileName[] = "Methane.HiScores";
#define HighScoreLoadBufferSize (MAX_HISCORES * 64)

void readArgs(int argc, char** argv)
{
    int i = 1;
    while (i < argc) {
        printf("arg %d - %s\n", i, argv[i]);

        if (!strcmp(argv[i], "nosync")) {
            vsync = SDL_FALSE;
        }

        if (!strcmp(argv[i], "window")) {
            fullscreen = SDL_FALSE;
        }

        i++;
    }
}

int main(int argc, char** argv)
{
	if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO) < 0) {
		fprintf (stderr, "Can't init SDL : %s", SDL_GetError());
		return 1;
	}

    readArgs(argc, argv);

	SDL_JoystickOpen(0);

	main_code();

    SDL_Quit();

	return (0);
}

static void print_info()
{
	puts("The GNU General Public License V2 applies to this game.\n");
	puts("See: http://methane.sourceforge.net");
	puts("Instructions:\n");
	puts("Press X to start game (You can't enter player names).");
	puts("Press A to fire gas from the gun.");
	puts("Press X to jump.");
	puts("Hold A to suck a trapped baddie into the gun.");
	puts("Release A to throw the trapped baddie from the gun.");
	puts("Throw baddies at the wall to destroy them.\n");
	puts("START = Quit (and save high scores)");
	puts("SELECT = Change player graphic during game");
	//puts("F1 = Next Level (DISABLED)\n");
}

static SDL_bool alloc_resources()
{
    SDL_bool result = SDL_FALSE;

    printf("Fullscreen %d, vsync %d, renderer name '%s'\n",
        fullscreen, vsync, renderer_name);

    // ARGB(8888) surface
    SdlScreen = SDL_CreateRGBSurface(0, SCR_WIDTH, SCR_HEIGHT, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	if (!SdlScreen)
	{
		fprintf(stderr, "Couldn't create surface: %s\n", SDL_GetError());
		goto out;
	}

    SdlWindow = SDL_CreateWindow("Super Methane Bros SDL2",
       SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
       WINDOW_WIDTH, WINDOW_HEIGHT,
       fullscreen ? SDL_WINDOW_FULLSCREEN : 0);

    if (!SdlWindow)
    {
        fprintf(stderr, "Couldn't create window: %s\n", SDL_GetError());
        goto out;
    }

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, renderer_name);
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, vsync ? "1" : "0");

    SdlRenderer = SDL_CreateRenderer(SdlWindow, -1, SDL_RENDERER_ACCELERATED);

    if (!SdlRenderer)
    {
        fprintf(stderr, "Couldn't create renderer: %s\n", SDL_GetError());
        goto out;
    }

    SdlTexture = SDL_CreateTexture(SdlRenderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, SCR_WIDTH, SCR_HEIGHT);

    if (!SdlTexture)
    {
        fprintf(stderr, "Couldn't create texture: %s\n", SDL_GetError());
        goto out;
    }

    result = SDL_TRUE;

out:
    return result;
}

void free_resources()
{
    if (SdlTexture) SDL_DestroyTexture(SdlTexture);
    if (SdlRenderer) SDL_DestroyRenderer(SdlRenderer);
    if (SdlWindow) SDL_DestroyWindow(SdlWindow);
    if (SdlScreen) SDL_FreeSurface(SdlScreen);
}

//------------------------------------------------------------------------------
//! \brief The program main code
//------------------------------------------------------------------------------
static void main_code(void)
{
	JOYSTICK *jptr1;
//	JOYSTICK *jptr2;

    if (!alloc_resources()) {
        free_resources();
        return;
    }

	SDL_ShowCursor(SDL_DISABLE);

    print_info();

	Game.InitSoundDriver();
	Game.InitGame ();
	Game.LoadScores();
	Game.StartGame();
	jptr1 = &Game.m_GameTarget.m_Joy1;
//	jptr2 = &Game.m_GameTarget.m_Joy2;

    SDL_bool running = SDL_TRUE;

	while (running)
	{
		SDL_PumpEvents();

        const Uint8 *state = SDL_GetKeyboardState(NULL);

		if (state[SDL_SCANCODE_ESCAPE]) running = SDL_FALSE;
		
        if (state[SDL_SCANCODE_TAB])
		{
			Game.m_GameTarget.m_Game.TogglePuffBlow();
		}

//		jptr1->key = key;
//		jptr2->key = key;

        jptr1->right = state[SDL_SCANCODE_RIGHT];
		jptr1->left  = state[SDL_SCANCODE_LEFT];
		jptr1->up    = state[SDL_SCANCODE_UP];
		jptr1->down  = state[SDL_SCANCODE_DOWN];
		jptr1->fire  = state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL];
		jptr1->key = 13; // Fake key press (required for high score table)

        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT) running = SDL_FALSE;
        }

		// (CHEAT MODE DISABLED) --> jptr1->next_level = 0;
		Game.MainLoop(0);
	}

	Game.SaveScores();
    free_resources();
}

//------------------------------------------------------------------------------
//! \brief Initialise Document
//------------------------------------------------------------------------------
CMethDoc::CMethDoc()
{
#ifdef METHANE_MIKMOD
	SMB_NEW(m_pMikModDrv,CMikModDrv);
#endif
	m_GameTarget.Init(this);
}

//------------------------------------------------------------------------------
//! \brief Destroy Document
//------------------------------------------------------------------------------
CMethDoc::~CMethDoc()
{
#ifdef METHANE_MIKMOD
	if (m_pMikModDrv)
	{
		delete(m_pMikModDrv);
		m_pMikModDrv = 0;
	}
#endif
}

//------------------------------------------------------------------------------
//! \brief Initialise the game
//------------------------------------------------------------------------------
void CMethDoc::InitGame(void)
{
	m_GameTarget.InitGame (TheScreen);
	m_GameTarget.PrepareSoundDriver ();
}

//------------------------------------------------------------------------------
//! \brief Initialise the sound driver
//------------------------------------------------------------------------------
void CMethDoc::InitSoundDriver(void)
{
#ifdef METHANE_MIKMOD
	m_pMikModDrv->InitDriver();
#endif
}

//------------------------------------------------------------------------------
//! \brief Remove the sound driver
//------------------------------------------------------------------------------
void CMethDoc::RemoveSoundDriver(void)
{
#ifdef METHANE_MIKMOD
	m_pMikModDrv->RemoveDriver();
#endif
}

//------------------------------------------------------------------------------
//! \brief Start the game
//------------------------------------------------------------------------------
void CMethDoc::StartGame(void)
{
	m_GameTarget.StartGame();
}

//------------------------------------------------------------------------------
//! \brief Redraw the game main view
//!
//! 	\param pal_change_flag : 0 = Palette not changed
//------------------------------------------------------------------------------
void CMethDoc::RedrawMainView( int pal_change_flag )
{
	// Function not used
}

//------------------------------------------------------------------------------
//! \brief Draw the screen
//!
//! 	\param screen_ptr = UNUSED
//------------------------------------------------------------------------------
void CMethDoc::DrawScreen( void *screen_ptr )
{
    //SDL_Color colors[PALETTE_SIZE];
    Uint32 colors[PALETTE_SIZE];

	// Set the game palette
	METHANE_RGB *pptr = m_GameTarget.m_rgbPalette;

	for (int cnt=0; cnt < PALETTE_SIZE; cnt++, pptr++)
	{
        /*
		colors[cnt].r = pptr->red;
		colors[cnt].g = pptr->green;
		colors[cnt].b = pptr->blue;
        colors[cnt].a = 255;
        */
        colors[cnt] = 255 << 24 | pptr->red << 16 | pptr->green << 8 | pptr->blue; // ARGB
	}

    if (SDL_MUSTLOCK (SdlScreen))
	{
		SDL_LockSurface (SdlScreen);
	}

	// Update surface
	Uint32 * dptr = (Uint32 *) SdlScreen->pixels;
	char * sptr = TheScreen;

	for (int y = 0; y < SCR_HEIGHT; y++) {
		for (int x = 0; x < SCR_WIDTH; x++) {
			dptr[x] = colors[(Uint8) *sptr++];
		}
		dptr += (SdlScreen->pitch / 4);
	}

    if (SDL_MUSTLOCK (SdlScreen))
	{
		SDL_UnlockSurface (SdlScreen);
	}

    SDL_UpdateTexture(SdlTexture, NULL, SdlScreen->pixels, SdlScreen->pitch);
    SDL_RenderCopy(SdlRenderer, SdlTexture, NULL, NULL);
    SDL_RenderPresent(SdlRenderer);
}

//------------------------------------------------------------------------------
//! \brief The Game Main Loop
//!
//! 	\param screen_ptr = UNUSED
//------------------------------------------------------------------------------
void CMethDoc::MainLoop( void *screen_ptr )
{
	m_GameTarget.MainLoop();
	DrawScreen( screen_ptr );
#ifdef METHANE_MIKMOD
	m_pMikModDrv->Update();
#endif
}

//------------------------------------------------------------------------------
//! \brief Play a sample (called from the game)
//!
//! 	\param id = SND_xxx id
//!	\param pos = Sample Position to use 0 to 255
//!	\param rate = The rate
//------------------------------------------------------------------------------
void CMethDoc::PlaySample(int id, int pos, int rate)
{
#ifdef METHANE_MIKMOD
	m_pMikModDrv->PlaySample(id, pos, rate);
#endif
}

//------------------------------------------------------------------------------
//! \brief Stop the module (called from the game)
//------------------------------------------------------------------------------
void CMethDoc::StopModule(void)
{
#ifdef METHANE_MIKMOD
	m_pMikModDrv->StopModule();
#endif
}

//------------------------------------------------------------------------------
//! \brief Play a module (called from the game)
//!
//! 	\param id = SMOD_xxx id
//------------------------------------------------------------------------------
void CMethDoc::PlayModule(int id)
{
#ifdef METHANE_MIKMOD
	m_pMikModDrv->PlayModule(id);
#endif
}

//------------------------------------------------------------------------------
//! \brief Update the current module (ie restart the module if it has stopped) (called from the game)
//!
//! 	\param id = SMOD_xxx id (The module that should be playing)
//------------------------------------------------------------------------------
void CMethDoc::UpdateModule(int id)
{
#ifdef METHANE_MIKMOD
	m_pMikModDrv->UpdateModule(id);
#endif
}

//------------------------------------------------------------------------------
//! \brief Load the high scores
//------------------------------------------------------------------------------
void CMethDoc::LoadScores(void)
{
	FILE *fptr = fopen(HighScoreFileName, "r");
	if (!fptr) return;	// No scores available

	// Allocate file memory, which is cleared to zero
	char *mptr = (char *) calloc(1, HighScoreLoadBufferSize);
	if (!mptr)		// No memory
	{
		fclose(fptr);
		return;
	}
	
    fread(mptr, 1, HighScoreLoadBufferSize - 2, fptr);	 // Get the file

	// (Note: mptr is zero terminated)
	char *tptr = mptr;
	
    for (int cnt = 0; cnt < MAX_HISCORES; cnt++)	// For each highscore
	{
		if (!tptr[0]) break;

		m_GameTarget.m_Game.InsertHiScore(atoi(&tptr[4]), tptr);

    	char let;

		do	// Find next name
		{
			let = *(tptr++);
		} while (!( (let == '$') || (!let) ));

		if (!let) break;	// Unexpected EOF
	}

	free(mptr);

	fclose(fptr);
}

//------------------------------------------------------------------------------
//! \brief Save the high scores
//------------------------------------------------------------------------------
void CMethDoc::SaveScores(void)
{
	FILE *fptr = fopen(HighScoreFileName, "w");
	
    if (!fptr) return;	// Cannot write scores

    int cnt;
	HISCORES *hs;
	
    for (cnt = 0, hs = m_GameTarget.m_Game.m_HiScores; cnt < MAX_HISCORES; cnt++, hs++)
	{
		fprintf(fptr, "%c%c%c%c%d$", hs->name[0], hs->name[1], hs->name[2], hs->name[3], hs->score);
	}

	fclose(fptr);

}

