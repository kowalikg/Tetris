#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glut.h>
#include <gl\glaux.h>
#include "draw_scene.h"

extern int score;//zmienne typu extern zmuszaj¹ kompilator do wyszukania ich w do³¹czonych plikach
extern int level;

HDC hDC = NULL; // graphic device rendering context
HGLRC hRC = NULL; // OpenGL rendering context
HWND hWnd = NULL; // window handle
HINSTANCE hInstance; // application instance
bool keys[256]; // keyboard routine
bool active = true; // application state
bool fullscreen = false;	// fullscreen mode flag

GLuint base; // Base Display List For The Font Set

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

GLvoid BuildFont(GLvoid)								// Build Our Bitmap Font
{
	HFONT	font;										// Windows Font ID
	HFONT	oldfont;									// Used For Good House Keeping

	base = glGenLists(96);								// Storage For 96 Characters

	font = CreateFont(	-15,							// Height Of Font
						0,								// Width Of Font
						0,								// Angle Of Escapement
						0,								// Orientation Angle
						FW_BOLD,						// Font Weight
						FALSE,							// Italic
						FALSE,							// Underline
						FALSE,							// Strikeout
						ANSI_CHARSET,					// Character Set Identifier
						OUT_TT_PRECIS,					// Output Precision
						CLIP_DEFAULT_PRECIS,			// Clipping Precision
						ANTIALIASED_QUALITY,			// Output Quality
						FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
						"Comic Sans Ms");					// Font Name

	oldfont = (HFONT)SelectObject(hDC, font);           // Selects The Font We Want
	wglUseFontBitmaps(hDC, 32, 96, base);				// Builds 96 Characters Starting At Character 32
	SelectObject(hDC, oldfont);							// Selects The Font We Want
	DeleteObject(font);									// Delete The Font
}

GLvoid KillFont(GLvoid)									// Delete The Font List
{
	glDeleteLists(base, 96);							// Delete All 96 Characters
}

GLvoid glPrint(const char *fmt, ...)					// Custom GL "Print" Routine
{
	char		text[256];								// Holds Our String
	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL)									// If There's No Text
		return;											// Do Nothing

	va_start(ap, fmt);									// Parses The String For Variables
	vsprintf(text, fmt, ap);						    // And Converts Symbols To Actual Numbers
	va_end(ap);											// Results Are Stored In Text

	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
	glListBase(base - 32);								// Sets The Base Character to 32
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits
}

/*
 * Destroys the OpenGL window.
 */
GLvoid KillGLWindow(GLvoid)
{
    // if the fullscreen mode is active restore display settings and show the cursor
	if (fullscreen)
	{
		ChangeDisplaySettings(NULL, 0);
		ShowCursor(true);
	}

    // delete rendering context
	if (hRC)
	{
		if (!wglMakeCurrent(NULL, NULL))
			MessageBox(NULL, "Release Of DC And RC Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);

		if (!wglDeleteContext(hRC))
			MessageBox(NULL, "Release Rendering Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hRC = NULL;
	}

    // release device context
	if (hDC && !ReleaseDC(hWnd, hDC))
	{
		MessageBox(NULL, "Release Device Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hDC = NULL;
	}

    // destroy the window
	if (hWnd && !DestroyWindow(hWnd))
	{
		MessageBox(NULL, "Could Not Release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;
	}

    // unregister the application class
	if (!UnregisterClass("OpenGL",hInstance))
	{
		MessageBox(NULL, "Could Not Unregister Class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;
	}

    KillFont();
}

/*
 * Initializes the OpenGL engine.
 */
int initGL(GLvoid)
{
    // initialize OpenGL
	glShadeModel(GL_SMOOTH);
	glClearColor(0, 0, 0, 0);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST | GL_LINE_SMOOTH);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	BuildFont();

	return true;
}

/*
 * Resizes the currently displayed scene.
 */
GLvoid resizeGLScene(GLsizei width, GLsizei height)
{
    // avoid division by zero
	if (height == 0)
		height = 1;

	glViewport(0, 0, width, height);

    float ratio = (float)width/height;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    glOrtho(0, 10*ratio, 0, 10, 0.0, 50.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/*
 * Creates a new OpenGL window.
 */
bool CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
    GLuint PixelFormat;
	WNDCLASS wc;
	DWORD dwExStyle;
	DWORD dwStyle;
	RECT WindowRect;
	WindowRect.left = (long)0;
	WindowRect.right = (long)width;
	WindowRect.top = (long)0;
	WindowRect.bottom = (long)height;

	fullscreen = fullscreenflag;

	hInstance = GetModuleHandle(NULL);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC) WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName	= NULL;
	wc.lpszClassName = "OpenGL";

    // register the application class
	if (!RegisterClass(&wc))
	{
		MessageBox(NULL, "Failed To Register The Window Class.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

    // prepare fullscreen mode settings
	if (fullscreen)
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = width;
		dmScreenSettings.dmPelsHeight = height;
		dmScreenSettings.dmBitsPerPel = bits;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			if (MessageBox(NULL, "The fullscreen mode is not supported by\nYour graphic card. Use the windowed mode instead?", "OpenGL Application", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				fullscreen = false;
			}
			else
			{
				MessageBox(NULL, "Program Will Now Close.", "ERROR", MB_OK | MB_ICONSTOP);
				return false;
			}
		}
	}

	if (fullscreen)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP;
		ShowCursor(false);
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle=WS_OVERLAPPEDWINDOW;
	}

    // adjust the application window to the requested size
	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

	// create application window
	if (!(hWnd=CreateWindowEx(dwExStyle, "OpenGL", title, dwStyle |	WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0,
        WindowRect.right-WindowRect.left, WindowRect.bottom-WindowRect.top, NULL, NULL, hInstance, NULL)))
	{
		KillGLWindow();
		MessageBox(NULL, "Window Creation Error.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

    // the pixelformat structure tells Windows how graphics should be rendered
	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR), // Size Of This Pixel Format Descriptor
		1, // Version Number
		PFD_DRAW_TO_WINDOW | // Format Must Support Window
		PFD_SUPPORT_OPENGL | // Format Must Support OpenGL
		PFD_DOUBLEBUFFER, // Must Support Double Buffering
		PFD_TYPE_RGBA, // Request An RGBA Format
		bits, // Select Our Color Depth
		0, 0, 0, 0, 0, 0, // Color Bits Ignored
		0, // No Alpha Buffer
		0, // Shift Bit Ignored
		0, // No Accumulation Buffer
		0, 0, 0, 0, // Accumulation Bits Ignored
		16, // 16Bit Z-Buffer (Depth Buffer)  
		0, // No Stencil Buffer
		0, // No Auxiliary Buffer
		PFD_MAIN_PLANE, // Main Drawing Layer
		0, // Reserved
		0, 0, 0	// Layer Masks Ignored
	};

	// obtain device context
	if (!(hDC = GetDC(hWnd)))
	{
		KillGLWindow();
		MessageBox(NULL, "Can't Create A GL Device Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

    // check whether Windows matched a pixel format
	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))
	{
		KillGLWindow();
		MessageBox(NULL, "Can't Find A Suitable PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

    // check whether pixel format can be set
	if(!SetPixelFormat(hDC, PixelFormat, &pfd))
	{
		KillGLWindow();
		MessageBox(NULL, "Can't Set The PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

    // check whether rendering context can be created
	if (!(hRC = wglCreateContext(hDC)))
	{
		KillGLWindow();
		MessageBox(NULL, "Can't Create A GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

    // activate rendering context
	if(!wglMakeCurrent(hDC,hRC))
	{
		KillGLWindow();
		MessageBox(NULL, "Can't Activate The GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

    // show the application window
	ShowWindow(hWnd,SW_SHOW);
	// move the window to the foreground
	SetForegroundWindow(hWnd);
	// set keyboard focus on the window
	SetFocus(hWnd);
	// adjust OpenGL scene to the window size
	resizeGLScene(width, height);

    // initialize newly created window
	if (!initGL())
	{
		KillGLWindow();
		MessageBox(NULL, "Initialization Failed.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	return true;
}


/*
 * Win32 API interruptions interceptor.
 */
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // switch by message type
	switch (uMsg)
	{
        // activation message (e.g. after program window restore)
		case WM_ACTIVATE:
		{
			if (!HIWORD(wParam))
                active = true;
			else
                active = false;

			return 0;
		}

        // system command
		case WM_SYSCOMMAND:
		{
			switch (wParam)
			{
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:
				return 0;
			}
			break;
		}

        // window close
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}

        // key down
		case WM_KEYDOWN:
		{//obs³uga klawiszy
			keys[wParam] = true;
			switch(wParam)
            {//je¿eli klawisz wciœniêty, wywo³ujemy odpowiednie funkcje:
              case VK_UP: 
                   rotate();
                   break;
              case VK_DOWN:
                   get_down();
                   break;
              case VK_LEFT:
                   go_left();
                   break;
              case VK_RIGHT:
                   go_right();
                   break;
     }
                   
                   
             
			return 0;
			
		}
      
        // key up
		case WM_KEYUP:
		{
			keys[wParam] = false;
			return 0;
		}

        // window resize
		case WM_SIZE:
		{
			resizeGLScene(LOWORD(lParam), HIWORD(lParam));
			return 0;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
         


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{   
    srand(GetTickCount());
    
	MSG	msg;
	bool done = false;

    // create OpenGL window
    char* title = "Tetris - K&K Productions, 2012";
    int width = 600;
    int height = 600;
    int depth = 32;
	if (!CreateGLWindow(title, width, height, depth, fullscreen))
		return 0;
 clear();
    // main game loop
	while (!done)
	{    
        // check existence of messages
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
                // finish the main game loop
				done = true;
            }
			else
			{
                // translate and dispatch message
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (active)
			{  
                       
                // if [ESC] key pressed finish the main loop
				if (keys[VK_ESCAPE])
					done = true;
				                    
				int flag = DrawGlScene();//wywo³anie funkcji rysuj¹cej
				
				// wypisanie napisów
				glColor3f(0,0,0);
				glRasterPos2f(7.1, 8); 
				glPrint("TETRIS 2012" , NULL);
				glRasterPos2f(7.1, 6.5); 
				glPrint("Next brick:" , NULL);
				glRasterPos2f(7.1, 3); 
				glPrint("Score: %d", score);
				glRasterPos2f(7.1, 2);
				glPrint("Level: %d", level);
				
                if (flag==0)//sprawdzamy czy gra jest zakoñczona
				{
                
                 glColor3f(1,1,1);                        
                 glRasterPos2f(3, 2); 
				 glPrint("TETRIS 2012" , NULL);
				 glRasterPos2f(3, 1); 
				 glPrint("K&K Productions, 2012",NULL);
				 glRasterPos2f(3, 9); 
				 glPrint("We are sorry. You lose!" ,NULL);
				 glRasterPos2f(3, 7); 
				 glPrint("Your score: %d", score);
				 glRasterPos2f(3, 5);
				 glPrint("Your level: %d", level);                
                }
			    
				SwapBuffers(hDC);
			}
		}
	}
    // shutdown program
	KillGLWindow();
	return (msg.wParam);
}


