// ASTER.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "resource.h"
#include "Input.h"
#include "Grid.h"
#include <windowsx.h>
#include "JPS.h"
#include <cstdlib>
#include <ctime>  

#define MAX_LOADSTRING 100

HBRUSH g_hTileBrush;
HPEN g_hGridPen;
Input input;
Grid grid;
Astar astar;

// 더블 버퍼링 변수
RECT ClientRect;


RECT g_hMemDCRect;
HDC g_hMemDC;
HBITMAP g_hMemDC_Bitmap;
HBITMAP g_hMemDC_BitmapOld;

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);


	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_JPS, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	srand(static_cast<unsigned int>(time(0)));

	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_JPS));

	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDC_JPS));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_JPS);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	switch (message)
	{
	case WM_CREATE:
	{
		HDC hdc = GetDC(hWnd);
		g_hGridPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
		g_hTileBrush = CreateSolidBrush(RGB(100, 100, 100));
		GetClientRect(hWnd, &g_hMemDCRect);
		g_hMemDC_Bitmap = CreateCompatibleBitmap(hdc, g_hMemDCRect.right, g_hMemDCRect.bottom);
		g_hMemDC = CreateCompatibleDC(hdc);
		ReleaseDC(hWnd, hdc);
		g_hMemDC_BitmapOld = (HBITMAP)SelectObject(g_hMemDC, g_hMemDC_Bitmap);
	}
	break;
	case WM_SIZE:

		break;

	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_SPACE:
		{
			while (!astar.found)
			{
				if (astar.SearchStep(grid.startPoint, grid.endPoint)) {
					printf("Path found\n");
					break;
				}
				InvalidateRect(hWnd, NULL, true);
			}
		

		}
		break;
		case 'Q':

			astar.found = false;
			astar.initializeNodeMap();
			astar.initialized = false;
			InvalidateRect(hWnd, NULL, true);
			break;
		case 'A':
		{
			if (!astar.found) {
				if (astar.SearchStep(grid.startPoint, grid.endPoint)) {
					printf("Path found\n");
				}
				InvalidateRect(hWnd, NULL, true);
			}
		}
		break;
		case 'E':
		{
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(hWnd, &pt);
			int xPos = pt.x;
			int yPos = pt.y;
			int iTileX = (xPos - grid.offsetX) / grid.GridSize;
			int iTileY = (yPos - grid.offsetY) / grid.GridSize;
			if (iTileX >= 0 && iTileX < Grid_WIDTH && iTileY >= 0 && iTileY < Grid_HEIGHT)
			{
				grid.endPoint = { iTileX, iTileY };
			}
			InvalidateRect(hWnd, NULL, true);
		}
		break;
		case 'S':
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(hWnd, &pt);
			int xPos = pt.x;
			int yPos = pt.y;
			int iTileX = (xPos - grid.offsetX) / grid.GridSize;
			int iTileY = (yPos - grid.offsetY) / grid.GridSize;
			if (iTileX >= 0 && iTileX < Grid_WIDTH && iTileY >= 0 && iTileY < Grid_HEIGHT)
			{
				grid.startPoint = { iTileX, iTileY };

			}
			InvalidateRect(hWnd, NULL, true);
			break;
		}
	}
	break;

	case WM_PAINT:
	{
		//hdc = BeginPaint(hWnd, &ps);
		PatBlt(g_hMemDC, 0, 0, g_hMemDCRect.right, g_hMemDCRect.bottom, WHITENESS);

		const TCHAR* instructions =
			TEXT("Instructions:\n")
			TEXT("마우스 오른쪽 드래그로  카메라 이동\n")
			TEXT("마우스 왼쪽으로 벽 제작\n")
			TEXT("마우스 휠로 확대 축소\n")
			TEXT("E : End 설정\n")
			TEXT("S : Start 설정\n")
			TEXT("SPACE 길찾기 시행\n")
			TEXT("Q : 리셋 A :독립시행\n");


		RECT textRect = { 10, 10, g_hMemDCRect.right - 10, 300 };
		DrawText(g_hMemDC, instructions, -1, &textRect, DT_LEFT | DT_TOP | DT_WORDBREAK);


		HBRUSH hOpenBrush = CreateSolidBrush(RGB(178, 235, 244)); // 이쁜 파랑색
		HBRUSH hCloseBrush = CreateSolidBrush(RGB(255, 255, 0)); // 노란색

		for (auto& nodePair : PathListNodes) {
			POINT node = nodePair.first;
			COLORREF color = nodePair.second;

			HBRUSH hBrush = CreateSolidBrush(color);
			RECT rect;
			rect.left = grid.offsetX + node.x * grid.GridSize;
			rect.top = grid.offsetY + node.y * grid.GridSize;
			rect.right = rect.left + grid.GridSize;
			rect.bottom = rect.top + grid.GridSize;

			FillRect(g_hMemDC, &rect, hBrush);
			DeleteObject(hBrush);
		}

		for (int y = 0; y < Grid_HEIGHT; ++y) {
			for (int x = 0; x < Grid_WIDTH; ++x) {

				if (astar.isInOpenList(POINT{ x, y })) {
					RECT rect;
					rect.left = grid.offsetX + x * grid.GridSize;
					rect.top = grid.offsetY + y * grid.GridSize;
					rect.right = rect.left + grid.GridSize;
					rect.bottom = rect.top + grid.GridSize;
					FillRect(g_hMemDC, &rect, hOpenBrush);
				}
				else if (astar.isInCloseList(POINT{ x, y })) {
					RECT rect;
					rect.left = grid.offsetX + x * grid.GridSize;
					rect.top = grid.offsetY + y * grid.GridSize;
					rect.right = rect.left + grid.GridSize;
					rect.bottom = rect.top + grid.GridSize;
					FillRect(g_hMemDC, &rect, hCloseBrush);
				}
			}
		}


		grid.DrawObstacle(g_hMemDC);
		grid.DrawGrid(g_hMemDC);

		if (grid.GridSize > 80)
		{
			for (int y = 0; y < Grid_HEIGHT; ++y) {
				for (int x = 0; x < Grid_WIDTH; ++x) {
					Node& node = nodeMap[y][x];

					if (node.f != INF) {
						TCHAR text[100];
						TCHAR text2[100];
						TCHAR text3[100];
						wsprintf(text, TEXT("g:%ld"), node.g);
						wsprintf(text2, TEXT("h:%ld"), node.h);
						wsprintf(text3, TEXT("f:%ld"), node.f);
						RECT rect;
						rect.left = grid.offsetX + x * grid.GridSize + 5;
						rect.top = grid.offsetY + y * grid.GridSize;
						rect.right = rect.left + grid.GridSize;
						rect.bottom = rect.top + grid.GridSize;
						SetBkMode(g_hMemDC, TRANSPARENT);
						DrawText(g_hMemDC, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
						DrawText(g_hMemDC, text2, -1, &rect, DT_TOP | DT_VCENTER | DT_SINGLELINE);
						DrawText(g_hMemDC, text3, -1, &rect, DT_BOTTOM | DT_VCENTER | DT_SINGLELINE);
					}
					if (node.parent != nullptr) {
						int startX = grid.offsetX + x * grid.GridSize + grid.GridSize / 2;
						int startY = grid.offsetY + y * grid.GridSize + grid.GridSize / 2;
						int endX = grid.offsetX + node.parent->pos.x * grid.GridSize + grid.GridSize / 2;
						int endY = grid.offsetY + node.parent->pos.y * grid.GridSize + grid.GridSize / 2;
						grid.DrawArrow(g_hMemDC, startX, startY, endX, endY);
					}

				}
			}

		}

		if (astar.found)
		{
			if (!finalPath.empty()) {
				HPEN hPen = CreatePen(PS_SOLID, 5, RGB(0, 0, 255)); 
				HPEN hOldPen = (HPEN)SelectObject(g_hMemDC, hPen);

				Node* firstNode = finalPath[0];
				int startX = grid.offsetX + firstNode->pos.x * grid.GridSize + grid.GridSize / 2;
				int startY = grid.offsetY + firstNode->pos.y * grid.GridSize + grid.GridSize / 2;

				MoveToEx(g_hMemDC, startX, startY, nullptr);

				for (size_t i = 0; i < finalPath.size(); ++i) {
					Node* node = finalPath[i];

					int x = grid.offsetX + node->pos.x * grid.GridSize + grid.GridSize / 2;
					int y = grid.offsetY + node->pos.y * grid.GridSize + grid.GridSize / 2;

					LineTo(g_hMemDC, x, y);
				}

				HPEN hPenPathNodes = CreatePen(PS_SOLID, 2, RGB(255, 0, 0)); 
				SelectObject(g_hMemDC, hPenPathNodes);

				POINT firstPOINT = finalPathNodes[0];
				int startX2 = grid.offsetX + firstPOINT.x * grid.GridSize + grid.GridSize / 2;
				int startY2 = grid.offsetY + firstPOINT.y * grid.GridSize + grid.GridSize / 2;

				MoveToEx(g_hMemDC, startX2, startY2, nullptr);

				for (size_t i = 0; i < finalPathNodes.size(); ++i) {
					POINT node = finalPathNodes[i];

					int x = grid.offsetX + node.x * grid.GridSize + grid.GridSize / 2;
					int y = grid.offsetY + node.y * grid.GridSize + grid.GridSize / 2;

					LineTo(g_hMemDC, x, y);
				}


				SelectObject(g_hMemDC, hOldPen); 
				DeleteObject(hPen);              
				DeleteObject(hPenPathNodes);      
			}
		}


		DeleteObject(hOpenBrush);
		DeleteObject(hCloseBrush);
		EndPaint(hWnd, &ps);

		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, g_hMemDCRect.right, g_hMemDCRect.bottom, g_hMemDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
	}
	break;

	/// 마우스 이벤트
	case WM_RBUTTONDOWN:
		input.OnRButtonDown();
		input.mousePos.x = GET_X_LPARAM(lParam);
		input.mousePos.y = GET_Y_LPARAM(lParam);
		break;
	case WM_RBUTTONUP:
		input.OnRButtonUp();
		break;
	case WM_MOUSEMOVE:

		if (input.isRButtonPress)
		{
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);

			int deltaX = xPos - input.mousePos.x;

			int deltaY = yPos - input.mousePos.y;

			grid.MoveGrid(deltaX, deltaY);

			input.mousePos.x = xPos;
			input.mousePos.y = yPos;
		}
		input.OnMouseMove({ GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) });
		if (input.isLButtonPress)
		{
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);
			int iTileX = (xPos - grid.offsetX) / grid.GridSize;
			int iTileY = (yPos - grid.offsetY) / grid.GridSize;
			if (iTileX < 0 || iTileY < 0 || iTileX>Grid_HEIGHT || iTileY>Grid_WIDTH)
				break;
			if (grid.g_bErase)
			{
				g_Tile[iTileY][iTileX] = TileState::Block;
			}
			else
			{
				g_Tile[iTileY][iTileX] = TileState::NoneBlock;
			}
		}
		InvalidateRect(hWnd, NULL, false);
		break;
	case WM_LBUTTONDOWN:
		input.OnLButtonDown();
		if (input.isLButtonPress)
		{
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);
			int iTileX = (xPos - grid.offsetX) / grid.GridSize;
			int iTileY = (yPos - grid.offsetY) / grid.GridSize;
			if (iTileX >= 0 && iTileX < Grid_WIDTH && iTileY >= 0 && iTileY < Grid_HEIGHT)
			{
				if (g_Tile[iTileY][iTileX] == TileState::NoneBlock)
				{
					grid.g_bErase = true;
					g_Tile[iTileY][iTileX] = TileState::Block;
				}
				else
				{
					grid.g_bErase = false;
					g_Tile[iTileY][iTileX] = TileState::NoneBlock;
				}
			}
		}
		break;
	case WM_LBUTTONUP:
		input.OnLButtonUp();
		break;
	case WM_MOUSEWHEEL:
	{
		int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		POINT pt;
		pt.x = GET_X_LPARAM(lParam);
		pt.y = GET_Y_LPARAM(lParam);
		ScreenToClient(hWnd, &pt);
		int mouseX = pt.x;
		int mouseY = pt.y;

		int oldGridSize = grid.GridSize;

		if (zDelta > 0)
			grid.ChangeGridSize(5);
		else
			grid.ChangeGridSize(-5);

		int newGridSize = grid.GridSize;

		grid.offsetX = mouseX - (mouseX - grid.offsetX) * newGridSize / oldGridSize;
		grid.offsetY = mouseY - (mouseY - grid.offsetY) * newGridSize / oldGridSize;

		InvalidateRect(hWnd, NULL, TRUE);
	}
	break;

	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 메뉴 선택을 구문 분석합니다:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;

	case WM_DESTROY:
	{
		SelectObject(g_hMemDC, g_hMemDC_BitmapOld);
		DeleteObject(g_hMemDC_Bitmap);
		DeleteDC(g_hMemDC);
		DeleteObject(g_hGridPen);
		DeleteObject(g_hTileBrush);
		PostQuitMessage(0);
	}
	break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
