/*
 * "SeamCarvingApp"
 */

#pragma once
#include <windows.h>
#include <winuser.h>
#include "Image.h"
#include <iostream>
#include <string>
#include <cmath>
#include "SeamCarving.h"
#define uc unsigned char
#define FOR(a,b,c) for(int a=(int)(b);a<(int)(c);a++)
#define RFOR(a,b,c) for(int a=(int)(b);a>(int)(c);a--)
#define ID_FILE_OPEN 1
#define ID_FILE_SAVE 2
#define ID_FILE_EXIT 3

static HANDLE INIT_HANDLE;

class SeamCarvingApp
{
public:		

	/****************************************************************
	*						  Useful Stuff							*
	****************************************************************/

	// ctors and dtor	
    SeamCarvingApp(Apollo::SimpleImage<BYTE>* img);
	~SeamCarvingApp();	

	//load and save
	int load(const std::string& path);
	int save(const std::string& path);
	void openFile(); //opens an open file dialog
	void saveFile(); //opens a save file dialog
    void setImage(Apollo::SimpleImage<BYTE>* img){ m_image = img; }				
    Apollo::SimpleImage<BYTE>* getImage(){ return m_image; }

	//stuff
	void forceRedraw();
	void setWindowLocation(int x, int y); //handy if you're gonna run multiple canvases
	int getWidth(){ return m_width; }
	int getHeight(){ return m_height; }
	HANDLE getThread(){ return m_thread; }


private:

	//event callbacks, override these in a subclass	
	virtual void onResize(int nw, int nh, int ow, int oh);
	virtual void onKeyDown(int key);
	virtual void onKeyUp(int key);
	virtual void onLeftMouseDown(int row, int col);
	virtual void onLeftMouseDrag(int row, int col);
	virtual void onLeftMouseUp(int row, int col);
	virtual void onRightMouseDown(int row, int col);
	virtual void onRightMouseDrag(int row, int col);
	virtual void onRightMouseUp(int row, int col);	


	/***************************************************************
	*				SeamCarvingApp internal stuff				   *
	****************************************************************/

	void initCanvas(Apollo::SimpleImage<BYTE>* img); //handles most of the setup procedure
	
	// the window procedure (event handler)
	LRESULT CALLBACK windowProc(HWND, UINT, WPARAM, LPARAM);		

	// stub for said window procedure (since it is a member function)
	static LRESULT CALLBACK windowProc_static(	HWND, UINT, 
												WPARAM, LPARAM);

	//these three are just wrappers around ugly WINAPI code
	int configWindow(WNDCLASSEX &wcex); 
	HWND createWindow(int w, int h);		

	//the function called by the event handling thread. repeatedly
	//waits for, processes and acts on messages
	long msgLoop();

	//again, stub for the thread function since its a member
	static long msgLoop_static(LPVOID args);		

	//used to convert from window to screen coordinates since
	//the drawing functions want screen coordinates
	void WindowToScreenCoords(POINT &p);

	//same in reverse
	void ScreenToWindowCoords(POINT &p);


	//member variables	
	HINSTANCE m_thisInstance; // the current instancs	
	char *m_windowName; // window's actual name
	char *m_className; // class name (for registering with the OS)
	MSG m_messages; // to hold messages for processing	
	bool m_leftMouseDown, m_rightMouseDown;	// for drags
	HANDLE m_thread; // the event handling thread
    HANDLE m_initEvent;
	bool m_isConstructed; // to prevent calling of onResize too early	
	HMENU m_menu; //the main menu

protected:	
	virtual void paint();

	//what a pain in the ass. window size include menu and frame
	//so this function adjusts for that
	void fixWindowSize();

	// pointer to the window. probably unnecessary
	// but it may allow for some customization
	Apollo::SimpleImage<BYTE>* m_image;
	HWND m_windowHandle;
	
	int m_width, m_height; // width and height of the window	

    Apollo::SeamCarving* m_engine;
};




//inlines 

inline 
void
SeamCarvingApp::WindowToScreenCoords(POINT &p)
{
	RECT r; 
	GetWindowRect(m_windowHandle, &r);
	p.x += r.left;
	int a1 = GetSystemMetrics(SM_CYSIZEFRAME);
	int a2 = GetSystemMetrics(SM_CYSIZE);
	int a3 = GetSystemMetrics(SM_CYMENU);
	p.y += r.top + a1 + a2 + a3;
}

inline 
void
SeamCarvingApp::ScreenToWindowCoords(POINT &p)
{
	RECT r; 
	GetWindowRect(m_windowHandle, &r);
	p.x -= (r.left + GetSystemMetrics(SM_CXSIZEFRAME));
	int a1 = GetSystemMetrics(SM_CYSIZEFRAME);
	int a2 = GetSystemMetrics(SM_CYSIZE);
	int a3 = GetSystemMetrics(SM_CYMENU);
	p.y -= (r.top + a1 + a2 + a3);
}

inline 
void
SeamCarvingApp::forceRedraw(){		
	InvalidateRect(m_windowHandle, NULL, 0);
}



//implementation begins here


char *file_extensions = 	"JPEG (.jpg)\0*.jpg\0\
							PNG (.png)\0*.png\0\
							BITMAP (.bmp)\0*.bmp\0\
							GIF (*gif)\0*.gif\0\
							TARGA (.tga)\0*.tga\0\
							All Files (*)\0*";

SeamCarvingApp::SeamCarvingApp(Apollo::SimpleImage<BYTE>* img) : m_leftMouseDown(0), m_rightMouseDown(0),
											m_width(0), m_height(0), m_isConstructed(0)
{ 	
    // load the image if there is one, else allocate a new empty bitmap
	if(img){
        m_image = new Apollo::SimpleImage<BYTE>(img->GetWidth(), img->GetHeight());
	} else {
		m_image = new Apollo::SimpleImage<BYTE>(640, 480);	        
    }

    m_width = m_image->GetWidth();
	m_height = m_image->GetHeight();

#if NDEBUG
    // Precomputing takes a long time on debug
    m_engine = new Apollo::SeamCarving(img, m_image, Apollo::SeamCarving::SeamCachePrecompute);
#else
    m_engine = new Apollo::SeamCarving(img, m_image, Apollo::SeamCarving::SeamCacheDynamic);
#endif
    
	initCanvas(img);
    forceRedraw();
}

SeamCarvingApp::~SeamCarvingApp() {
    delete m_engine;
    delete m_image;
}

void
SeamCarvingApp::initCanvas(Apollo::SimpleImage<BYTE>* img){		
	m_windowName = "Seam!";
	m_className = "Windows App";

    m_isConstructed = false;
	long (*fptr)(LPVOID) = &SeamCarvingApp::msgLoop_static;	

	//since all a window does is repeatedly wait for and process messages,
	//it needs its own thread. mmmmm... threads	

    m_initEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_thread = CreateThread( NULL, //default security settings 
						0, //default stack size
						(LPTHREAD_START_ROUTINE) fptr, // function to call
						this, //pass self to msgLoop_static so it may call a member function
						0, // no creation flags = run right away
						NULL //don't care about thread id
	);	

    WaitForSingleObject(m_initEvent, INFINITE);
    m_isConstructed = true;
}

long
SeamCarvingApp::msgLoop_static(LPVOID args){
	((SeamCarvingApp*)(args))->msgLoop();
	return 0;
}

long SeamCarvingApp::msgLoop(){

	WNDCLASSEX wcex;  // wcex is a DS that stores window creation parameters
	configWindow(wcex);

	WNDCLASSEX p;
	if (!GetClassInfoEx(m_thisInstance, "Windows App", &p))

		if (!RegisterClassEx (&wcex)){			
			int err = GetLastError();
			char buf[500], msg[500];
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,0,0,0,buf,500,0);
			sprintf(msg, "Could notg register window with the OS\n Error was: \"%s\"\n", buf);
			MessageBox(NULL, msg, "Oh Crap!", MB_OK);			
		}
   
	m_windowHandle = createWindow(m_width, m_height);

	if (!m_windowHandle){
		int err = GetLastError();
		char buf[500], msg[500];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,0,0,0,buf,500,0);
		sprintf(msg, "Could not register window with the OS\n Error was: \"%s\"\n", buf);
		MessageBox(NULL, msg, "Oh Crap!", MB_OK);					
	}

	SetWindowLong(m_windowHandle, GWL_USERDATA, (LONG)this);						

	m_menu = CreateMenu();
	HMENU hSubMenu = CreatePopupMenu();
    AppendMenu(hSubMenu, MF_STRING, ID_FILE_OPEN, "&Open");
    AppendMenu(hSubMenu, MF_STRING, ID_FILE_SAVE, "&Save");
    AppendMenu(hSubMenu, MF_SEPARATOR, 0, 0);
    AppendMenu(hSubMenu, MF_STRING, ID_FILE_EXIT, "&Quit");
    AppendMenu(m_menu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&File");
	int res = SetMenu(m_windowHandle, m_menu);			

	// Make that shit show up!
	ShowWindow (m_windowHandle, 1);
	UpdateWindow(m_windowHandle);

	//window is now constructed and will enter message loop
    SetEvent(m_initEvent);
	
	while (GetMessage (&m_messages, NULL, 0, 0)){		
		TranslateMessage(&m_messages); // get the message
		DispatchMessage(&m_messages); // handle the message via windowProc
	}
	return 0;
}

int
SeamCarvingApp::configWindow(WNDCLASSEX &wcex)
{	
	wcex.hInstance =		m_thisInstance;
	wcex.lpszClassName =	m_className;
	wcex.lpfnWndProc =		windowProc_static; // The window message handler
	wcex.style =			CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW; // these do something
	wcex.cbSize =			sizeof (WNDCLASSEX); 
	wcex.hIcon =			LoadIcon (NULL, IDI_APPLICATION); // icon
	wcex.hIconSm =			LoadIcon (NULL, IDI_APPLICATION); // taskbar icon
	wcex.hCursor =			LoadCursor (NULL, IDC_CROSS); // default cursor
	wcex.lpszMenuName =		"MENU_1";
	wcex.cbClsExtra =		0;    
	wcex.cbWndExtra =		0;    
	wcex.hbrBackground =	(HBRUSH) GetStockObject(BLACK_BRUSH);
	return 0;
}

HWND
SeamCarvingApp::createWindow(int w, int h){
	int realWidth = m_width		+ 2	* GetSystemMetrics(SM_CXSIZEFRAME);
	int a1 = 2 * GetSystemMetrics(SM_CYSIZEFRAME);
	int a2 = GetSystemMetrics(SM_CYSIZE);
	int a3 = GetSystemMetrics(SM_CYMENU);
	int realHeight = m_height + a1 + a2 + a3 + 1;

	HWND ret = CreateWindowEx (
				WS_EX_APPWINDOW,		// basic window
				m_className,			// Classname 
				m_windowName,			// Title Text 
				WS_OVERLAPPEDWINDOW | WS_VISIBLE,	
				CW_USEDEFAULT,			// let windows decide where to put it
				CW_USEDEFAULT,        
				realWidth,				// width
				realHeight,				// height
				NULL,					 
				NULL,					
				m_thisInstance,			
				NULL					
	);  	
	return ret;
}

//This is a hack to get around an issue with using a member function as the WndProc
LRESULT CALLBACK 
SeamCarvingApp::windowProc_static(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	SeamCarvingApp* actualCanvas = (SeamCarvingApp*) GetWindowLong( hWnd, GWL_USERDATA );
	if (actualCanvas)
		return actualCanvas->windowProc(hWnd, message, wParam, lParam);
	else
		return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK 
SeamCarvingApp::windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	POINT p;	

	int newWidth, newHeight, oldWidth, oldHeight;	

	switch (message)
	{
	case WM_ERASEBKGND:
		return (LRESULT) 1;
	case WM_COMMAND:				
		switch(LOWORD(wParam))
		{
		case ID_FILE_OPEN:
			openFile();
	        break;
        case ID_FILE_EXIT:
			PostQuitMessage(0);
			break;		
		}
		break;		
	case WM_PAINT:	
		paint();		
		break;	
	case WM_SIZE:		
		newWidth = LOWORD(lParam);
		newHeight = HIWORD(lParam);
		oldWidth = m_width;
		oldHeight = m_height;
		m_width = newWidth;
		m_height = newHeight;
		if(!m_isConstructed) break;
		this->onResize(newWidth, newHeight, oldWidth, oldHeight);		
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	//input stuff

	case WM_KEYDOWN:
		onKeyDown(wParam);
		break;

	case WM_KEYUP:
		onKeyUp(wParam);
		break;

	case WM_LBUTTONDOWN:		      
		GetCursorPos(&p);
		ScreenToWindowCoords(p);
		onLeftMouseDown(p.y, p.x);
		m_leftMouseDown = 1;
		break;	
	case WM_LBUTTONUP:		
		GetCursorPos(&p);
		ScreenToWindowCoords(p);
		onLeftMouseUp(p.y, p.x);
		m_leftMouseDown = 0;
		break;

	case WM_RBUTTONDOWN:		      
		GetCursorPos(&p);
		ScreenToWindowCoords(p);
		onRightMouseDown(p.y, p.x);
		m_rightMouseDown = 1;
		break;	
	case WM_RBUTTONUP:		
		GetCursorPos(&p);
		ScreenToWindowCoords(p);
		onRightMouseUp(p.y, p.x);
		m_rightMouseDown = 0;
		break;

	case WM_MOUSEMOVE:
 		GetCursorPos(&p);		
		ScreenToWindowCoords(p);
		if(m_leftMouseDown)
			onLeftMouseDrag(p.y, p.x);
		if(m_rightMouseDown)
			onRightMouseDrag(p.y, p.x);
		break;	
		
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}	

	return 0;
}

void
SeamCarvingApp::setWindowLocation(int x, int y)
{
	int realWidth = m_width		+ 2	* GetSystemMetrics(SM_CXSIZEFRAME);
	int a1 = 2 * GetSystemMetrics(SM_CYSIZEFRAME);
	int a2 = GetSystemMetrics(SM_CYSIZE);
	int a3 = GetSystemMetrics(SM_CYMENU);
	int realHeight = m_height + a1 + a2 + a3 + 1;

	int ret = SetWindowPos(	m_windowHandle, 
							NULL, 
							x, 
							y, 
							realWidth, 
							realHeight, 
							SWP_NOZORDER | SWP_NOACTIVATE
	);
}

void
SeamCarvingApp::fixWindowSize()
{
	int realWidth = m_width		+ 2	* GetSystemMetrics(SM_CXSIZEFRAME);
	int a1 = 2 * GetSystemMetrics(SM_CYSIZEFRAME);
	int a2 = GetSystemMetrics(SM_CYSIZE);
	int a3 = GetSystemMetrics(SM_CYMENU);
	int realHeight = m_height + a1 + a2 + a3 + 12;

	m_isConstructed = 0;
	SetWindowPos(	m_windowHandle, 
					HWND_TOP, 
					0, 
					0, 
					realWidth, 
					realHeight, 
					SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE
	);
}

int
SeamCarvingApp::load(const std::string& path)
{	
    Apollo::Image* src_img = Apollo::ImageIO::Load(path);
    Apollo::SimpleImage<BYTE>* img = src_img->ToSimpleImage<BYTE>();
	if(m_image) delete m_image;
	m_image = img;
	m_width = m_image->GetWidth();
	m_height = m_image->GetHeight();
	fixWindowSize();
	forceRedraw();
	return 0;
}


void 
SeamCarvingApp::paint(){	
	PAINTSTRUCT ps;
	RECT r;	
	BeginPaint(m_windowHandle, &ps);

    int w = m_engine->GetWidth();
    int h = m_engine->GetHeight();
    

	SetStretchBltMode(ps.hdc, COLORONCOLOR);

	GetClientRect(m_windowHandle, &r);	

    BITMAPINFO info = {0};
	info.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth			= m_image->GetWidth();
    info.bmiHeader.biHeight			= -m_image->GetHeight();
	info.bmiHeader.biPlanes			= 1;
    info.bmiHeader.biBitCount		= 8 * sizeof(Apollo::SimpleImage<BYTE>::Pixel);
	info.bmiHeader.biCompression	= BI_RGB;
	
	StretchDIBits(ps.hdc, 
				  r.left, 
                  r.top,
				  w,
				  h,
                  0, 
                  0, 
                  w,
				  h,
                  m_image->GetData(), 
				  &info, 
                  DIB_RGB_COLORS, 
                  SRCCOPY
	);	
	EndPaint(m_windowHandle, &ps);
}

	

void 
SeamCarvingApp::openFile()
{
	m_image;
	OPENFILENAME ofn;
    char respath[MAX_PATH] = "";    
	memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_windowHandle;
    ofn.lpstrFilter = file_extensions;
    ofn.lpstrFile = respath;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "bmp";

	if (!GetOpenFileName(&ofn) || respath == "" || load(respath) < 0){		
		MessageBox(NULL, "Could not load image :(", "Oh Crap!", MB_OK);
	}
}




//Event Callbacks. Subclass and override

void
SeamCarvingApp::onResize(int newWidth, int newHeight, int oldWidth, int oldHeight)
{
    m_engine->SeamCarve(newWidth, newHeight);
}

void
SeamCarvingApp::onKeyDown(int key)
{
}

void
SeamCarvingApp::onKeyUp(int key)
{
	if (key == VK_ESCAPE) PostQuitMessage(0);	
	else if (key == 'O') openFile();
}

void
SeamCarvingApp::onLeftMouseDown(int row, int col)
{
    /*
	Pixel p; p.r = 255; p.g = 0; p.b = 0;
	FOR(i,max(row-6,0),min(row+6,m_height)) FOR(j,max(col-6,0),min(col+6,m_width))
		if((i-row)*(i-row)+(j-col)*(j-col) < 20)
			m_image->setPixel(i, j, p);
	forceRedraw();		
    */
}

void
SeamCarvingApp::onLeftMouseDrag(int row, int col)
{
    /*
	Pixel p; p.r = 255; p.g = 0; p.b = 0;
	FOR(i,max(row-6,0),min(row+6,m_height)) FOR(j,max(col-6,0),min(col+6,m_width))
		if((i-row)*(i-row)+(j-col)*(j-col) < 20)
			m_image->setPixel(i, j, p);
	forceRedraw();			
    */
}

void
SeamCarvingApp::onLeftMouseUp(int row, int col)
{
}

void
SeamCarvingApp::onRightMouseDown(int row, int col)
{
    /*
    Pixel p; p.r = 0; p.g = 255; p.b = 0;
	FOR(i,max(row-6,0),min(row+6,m_height)) FOR(j,max(col-6,0),min(col+6,m_width))
		if((i-row)*(i-row)+(j-col)*(j-col) < 20)
			m_image->setPixel(i, j, p);
	forceRedraw();	
    */
}

void
SeamCarvingApp::onRightMouseDrag(int row, int col)
{
    /*
	Pixel p; p.r = 0; p.g = 255; p.b = 0;
	FOR(i,max(row-6,0),min(row+6,m_height)) FOR(j,max(col-6,0),min(col+6,m_width))
		if((i-row)*(i-row)+(j-col)*(j-col) < 20)
			m_image->setPixel(i, j, p);
	forceRedraw();		
    */
}

void
SeamCarvingApp::onRightMouseUp(int row, int col)
{
}
