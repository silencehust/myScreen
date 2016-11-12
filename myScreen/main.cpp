#include<Windows.h>
 
#define ID_TIMER 1001
#define STRMAXLEN 35
#define STRMINLEN 15

typedef struct tarCharChain
{
	struct tarCharChain* prev;
	TCHAR ch;
	struct tarCharChain* next;
}CharChain,*pCharChain;

typedef struct tarCharColumn
{
	CharChain* head, *current, *point;
	int x, y, iStrlen;
	int iStopTimes, iMustStopTimes;
}CharColum,*pCharColumn;

TCHAR randomChar()
{
	return (TCHAR)(rand() % (126 - 33) + 33);
}
void init(CharColum* cc,int cyScreen,int x)
{
	int j;
	cc->iStrlen = rand() % (STRMAXLEN - STRMINLEN) + STRMINLEN;
	cc->x = x + 3;
	cc->y = rand() % 3 ? rand() % cyScreen : 0;
	cc->iMustStopTimes = rand() & 6;
	cc->iStopTimes = 0;
	cc->head = cc->current = (pCharChain)calloc(cc->iStrlen, sizeof(CharChain));

	for(j=0;j<cc->iStrlen-1;j++)
	{
		cc->current->prev = cc->point;
		cc->current->ch = 0;
		cc->current->next = cc->current + 1;
		cc->point = cc->current++;
	}
	cc->current->prev = cc->point;
	cc->current->ch = 0;
	cc->current->next = cc->head;
	cc->head->prev = cc->current;

	cc->current = cc->point = cc->head;
	cc->head->ch = randomChar();
	return;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	HDC hdc;
	INT i, j, temp, ctn;
	static HDC hdcMem;
	HFONT hFont;
	static HBITMAP hbitmap;
	static int cxscreen, cyscreen;
	static int iFontWidth = 12, iFontHeight = 16, iColumCount;
	static CharColum *ccChain;
	switch (message)
	{
		case WM_CREATE:
			cxscreen = GetSystemMetrics(SM_CXSCREEN);
			cyscreen = GetSystemMetrics(SM_CYSCREEN);

			SetTimer(hwnd, ID_TIMER, 10, NULL);

			hdc = GetDC(hwnd);
			hdcMem = CreateCompatibleDC(hdc);
			hbitmap = CreateCompatibleBitmap(hdc, cxscreen, cyscreen);
			SelectObject(hdcMem, hbitmap);
			ReleaseDC(hwnd, hdc);

			hFont=CreateFont(iFontHeight, iFontWidth - 4, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
							 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DRAFT_QUALITY, FIXED_PITCH | FF_SWISS,NULL);
			
			SelectObject(hdcMem, hFont);
			DeleteObject(hFont);
			SetBkMode(hdcMem,TRANSPARENT);
			iColumCount = cxscreen / (iFontWidth * 3 / 2);

			ccChain = (pCharColumn)calloc(iColumCount, sizeof(CharColum));

			for(i=0;i<iColumCount;i++)
			{
				init(ccChain + i, cyscreen, (iFontWidth * 3 / 2)*i);
			}

			break;
		case WM_TIMER:
			hdc = GetDC(hwnd);
			PatBlt(hdcMem, 0, 0, cxscreen, cyscreen, BLACKNESS);
			for(i=0;i<iColumCount;i++)
			{
				ctn = (ccChain + i)->iStopTimes++ > (ccChain + i)->iMustStopTimes;
				(ccChain + i)->point = (ccChain + i)->head;

				SetTextColor(hdcMem, RGB(255, 255, 255));
				TextOut(hdcMem, (ccChain + i)->x, (ccChain + i)->y, &((ccChain + i)->point->ch), 1);
				j = (ccChain + i)->y;
				(ccChain + i)->point = (ccChain + i)->point->next;

				temp = 0;
				while ((ccChain+i)->point!=(ccChain+i)->head&&(ccChain+i)->point->ch)
				{
					SetTextColor(hdcMem, RGB(0, 255 - (255 * (temp++) / (ccChain + i)->iStrlen), 0));
					TextOut(hdcMem, (ccChain + i)->x, j -= iFontHeight, &((ccChain + i)->point->ch), 1);
					(ccChain + i)->point = (ccChain + i)->point->next;
				}

				if(ctn)
				{
					(ccChain + i)->iStopTimes = 0;
				}
				else
				{
					continue;
				}
				(ccChain + i)->y += iFontHeight;
				if ((ccChain+i)->y-(ccChain+i)->iStrlen*iFontHeight>cyscreen)
				{
					free((ccChain + i)->current);
					init(ccChain + i, cyscreen, (iFontWidth * 3 / 2)*i);
				}
				(ccChain + i)->head = (ccChain + i)->head->prev;
				(ccChain + i)->head->ch = randomChar();
			}

			BitBlt(hdc, 0, 0, cxscreen, cyscreen, hdcMem, 0, 0, SRCCOPY);
			ReleaseDC(hwnd, hdc);
			break;
		case WM_RBUTTONDOWN:
			KillTimer(hwnd, ID_TIMER);
			break;
		case WM_RBUTTONUP:
			SetTimer(hwnd, ID_TIMER, 10, NULL);
			break;
		case WM_KEYUP:
		case WM_LBUTTONUP:
		case WM_DESTROY:
			KillTimer(hwnd, ID_TIMER);
			DeleteObject(hbitmap);
			DeleteDC(hdcMem);
			for (i=0;i<iColumCount;i++)
			{
				free((ccChain + i)->current);
			}
			free(ccChain);

			PostQuitMessage(0);
			break;
	}
	return DefWindowProc(hwnd,message,wparam,lparam);
}
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	static TCHAR *szAppName = TEXT  ("test");
	HWND hwnd;
	MSG msg;
	WNDCLASS wndClass;

	wndClass.style = CS_HREDRAW | CS_HREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = szAppName;

	if (!RegisterClass(&wndClass))
	{
		MessageBox(NULL, TEXT("×¢²á´°¿ÚÀàÊ§°Ü"), szAppName, MB_OK | MB_ICONERROR);
		return 0;
	}

	hwnd = CreateWindow(szAppName, NULL, WS_DLGFRAME | WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), 
						GetSystemMetrics(SM_CYSCREEN), NULL, NULL, hInstance, NULL);
	ShowWindow(hwnd, SW_SHOWMAXIMIZED);
	UpdateWindow(hwnd);
	ShowCursor(FALSE);

	srand((int)GetCurrentTime());
	while (GetMessage(&msg,NULL,0,0))
	{ 
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	ShowCursor(TRUE);
	return 0;
}