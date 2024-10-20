#pragma once
#include <Windows.h>
#include <math.h>
#define Grid_HEIGHT 100
#define Grid_WIDTH 100

extern HBRUSH g_hTileBrush;
extern HPEN g_hGridPen;

enum class TileState
{
	NoneBlock,
	Block,
	Start,
	End,
	Open,
	Close,
	Path,
};
TileState g_Tile[Grid_HEIGHT][Grid_WIDTH];

class Grid
{
public:
	Grid() { memset(g_Tile, 0, sizeof(g_Tile)); };

	int offsetX = 0;  // �׸����� X �� ������
	int offsetY = 0;  // �׸����� Y �� ������
	int GridSize = 50;
	POINT startPoint = { -1, -1 };  // ��ŸƮ ����
	POINT endPoint = { -1, -1 };  // ��ŸƮ ����


	bool g_bErase = false;

	void ChangeGridSize(int delta) {
		GridSize += delta;
		if (GridSize < 5) GridSize = 5;  // �ּ� �׸��� ũ�� ����
		if (GridSize > 100) GridSize = 100;  // �ִ� �׸��� ũ�� ����
	}
	void MoveGrid(int deltaX, int deltaY) {
		offsetX += deltaX;
		offsetY += deltaY;
	}

	void DrawGrid(HDC hdc) {
		HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
		HPEN oldPen = (HPEN)SelectObject(hdc, pen);
		for (int iCntW = 0; iCntW <= Grid_WIDTH; ++iCntW)
		{
			MoveToEx(hdc, offsetX + iCntW * GridSize, offsetY, NULL);
			LineTo(hdc, offsetX + iCntW * GridSize, offsetY + Grid_HEIGHT * GridSize);
		}
		for (int iCntH = 0; iCntH <= Grid_HEIGHT; ++iCntH)
		{
			MoveToEx(hdc, offsetX, offsetY + iCntH * GridSize, NULL);
			LineTo(hdc, offsetX + Grid_WIDTH * GridSize, offsetY + iCntH * GridSize);
		}
		SelectObject(hdc, oldPen);
		DeleteObject(pen);
	}


	void DrawArrow(HDC hdc, int startX, int startY, int endX, int endY)
	{
		HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
		HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

		// Draw the line part of the arrow
		MoveToEx(hdc, startX, startY, NULL);
		LineTo(hdc, endX, endY);

		// Calculate the angle of the arrow
		double angle = atan2(endY - startY, endX - startX);

		// Draw the arrowhead
		int arrowLength = 10;
		int arrowWidth = 5;
		POINT arrowPoints[3] = {
			{ endX, endY },
			{ endX - (int)(arrowLength * cos(angle - 0.5)), endY - (int)(arrowLength * sin(angle - 0.5)) },
			{ endX - (int)(arrowLength * cos(angle + 0.5)), endY - (int)(arrowLength * sin(angle + 0.5)) }
		};
		Polygon(hdc, arrowPoints, 3);

		SelectObject(hdc, hOldPen);
		DeleteObject(hPen);
	}


	void DrawObstacle(HDC hdc)
	{
		HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, g_hTileBrush);
		SelectObject(hdc, GetStockObject(NULL_PEN));
		for (int iCntW = 0; iCntW < Grid_WIDTH; ++iCntW)
		{
			for (int iCntH = 0; iCntH < Grid_HEIGHT; ++iCntH)
			{
				if (g_Tile[iCntH][iCntW] == TileState::Block)
				{
					int iX = offsetX + iCntW * GridSize;
					int iY = offsetY + iCntH * GridSize;
					Rectangle(hdc, iX, iY, iX + GridSize, iY + GridSize);
				}
			}
		}

		// ��ŸƮ ����Ʈ �����ϱ�
		HBRUSH hhStartBrush = CreateSolidBrush(RGB(107, 102, 255)); // ��ŸƮ ������ �ʷϻ����� ǥ��
		HBRUSH hhOldBrush = (HBRUSH)SelectObject(hdc, hhStartBrush);
		SelectObject(hdc, hhStartBrush);
		for (int iCntW = 0; iCntW < Grid_WIDTH; ++iCntW)
		{
			for (int iCntH = 0; iCntH < Grid_HEIGHT; ++iCntH)
			{
				if (g_Tile[iCntH][iCntW] == TileState::Close)
				{
					int iX = offsetX + iCntW * GridSize;
					int iY = offsetY + iCntH * GridSize;
					Rectangle(hdc, iX, iY, iX + GridSize, iY + GridSize);
				}
			}
		}
		SelectObject(hdc, hOldBrush);
		DeleteObject(hhStartBrush);

		if (startPoint.x != -1 && startPoint.y != -1) {
			HBRUSH hStartBrush = CreateSolidBrush(RGB(0, 255, 0)); // ��ŸƮ ������ �ʷϻ����� ǥ��
			HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hStartBrush);
			Rectangle(hdc, offsetX + startPoint.x * GridSize,
				offsetY + startPoint.y * GridSize,
				offsetX + (startPoint.x + 1) * GridSize,
				offsetY + (startPoint.y + 1) * GridSize);
			g_Tile[startPoint.y][startPoint.x] = TileState::Start;
			SelectObject(hdc, hOldBrush);
			DeleteObject(hStartBrush);
		}
		// ���� ����Ʈ �����ϱ�
		if (endPoint.x != -1 && endPoint.y != -1) {
			HBRUSH hStartBrush = CreateSolidBrush(RGB(255, 0, 0)); // ���� ������ ���������� ǥ��
			HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hStartBrush);
			Rectangle(hdc, offsetX + endPoint.x * GridSize,
				offsetY + endPoint.y * GridSize,
				offsetX + (endPoint.x + 1) * GridSize,
				offsetY + (endPoint.y + 1) * GridSize);
			g_Tile[endPoint.y][endPoint.x] = TileState::End;
			SelectObject(hdc, hOldBrush);
			DeleteObject(hStartBrush);
		}

	}
};

