

#pragma once
#include "Windows.h"
#include <queue>
#include <cmath>
#include <algorithm>
#include <vector>
#include <set>
#include "Grid.h"
#include <utility>

extern TileState g_Tile[Grid_HEIGHT][Grid_WIDTH];
extern Grid grid;
constexpr long INF = 1e9 + 7;

class Node {
public:
	Node() : g(0), h(0), f(INF), pos(POINT{ 0,0 }), parent(nullptr) {};
	Node(POINT p) : g(0), h(0), f(INF), pos(p), parent(nullptr) {};
	Node(long _g, long _h, POINT _pos, Node* _parent = nullptr)
		: g(_g), h(_h), f(_g + _h), pos(_pos), parent(_parent) {};

	long g;
	long f;
	long h;
	POINT pos;
	Node* parent;

	bool operator==(const Node& other) const
	{
		return pos.x == other.pos.x && pos.y == other.pos.y;
	}
};

struct CompareNode
{
	bool operator()(const Node& lhs, const Node& rhs) const
	{
		return lhs.f > rhs.f;
	}
};

struct ComparePoint
{
	bool operator()(const POINT& lhs, const POINT& rhs) const
	{
		if (lhs.y == rhs.y) {
			return lhs.x < rhs.x;
		}
		return lhs.y < rhs.y;
	}
};

long getDistance(const POINT& current, const POINT& neighbor)
{
	int dx = abs(current.x - neighbor.x);
	int dy = abs(current.y - neighbor.y);
	return dx + dy;
}

float heuristic(const POINT& from, const POINT& to)
{
	int dx = abs(from.x - to.x);
	int dy = abs(from.y - to.y);
	return sqrt(dx * dx + dy * dy);
}

std::vector<std::vector<Node>> nodeMap(Grid_HEIGHT, std::vector<Node>(Grid_WIDTH));
std::vector<std::pair<POINT, COLORREF>> PathListNodes;
std::vector<Node*> finalPath;
std::vector<POINT> finalPathNodes;
class Astar {
public:
	std::set<POINT, ComparePoint> openListPoints;
	std::set<POINT, ComparePoint> closeListPoints;
	std::priority_queue<Node, std::vector<Node>, CompareNode> openList;
	bool initialized = false;
	bool found = false;
	int iterationCount = 0;


	Node* findNode = nullptr;

	bool isValid(int x, int y)
	{
		return (x > -1) && (x < Grid_WIDTH) && (y > -1) && (y < Grid_HEIGHT);
	}

	bool isUnBlocked(int x, int y)
	{
		return g_Tile[y][x] == TileState::NoneBlock || g_Tile[y][x] == TileState::End || g_Tile[y][x] == TileState::Start;
	}
	void initializeNodeMap() {
		for (int y = 0; y < Grid_HEIGHT; ++y)
		{
			for (int x = 0; x < Grid_WIDTH; ++x) {
				nodeMap[y][x].pos = { x, y };
				nodeMap[y][x].parent = nullptr;
				nodeMap[y][x].f = INF;
				nodeMap[y][x].g = 0;
				nodeMap[y][x].h = 0;
			}
		}
		for (int y = 0; y < Grid_HEIGHT; ++y)
		{
			for (int x = 0; x < Grid_WIDTH; ++x) {
				if (g_Tile[y][x] != TileState::Block)
					g_Tile[y][x] = TileState::NoneBlock;
			}
		}
		g_Tile[grid.startPoint.y][grid.startPoint.x] = TileState::Start;
		g_Tile[grid.endPoint.y][grid.endPoint.x] = TileState::End;
		openListPoints.clear();
		closeListPoints.clear();
		PathListNodes.clear();
		while (!openList.empty())
		{
			openList.pop();
		}
		initialized = true;
		found = false;
	}

	bool isGoal(POINT current, POINT end)
	{
		return current.y == end.y && current.x == end.x;
	}

	bool SearchStep(POINT start, POINT end)
	{
		if (!initialized) {
			initializeNodeMap();
			Node& node = nodeMap[start.y][start.x];
			node.f = node.g = node.h = 0.0f;
			openList.push(node);
			openListPoints.insert(node.pos);
		}

		if (found || openList.empty())
		{
			return found;
		}

		Node current = openList.top();

		if (isGoal(current.pos, end))
		{
			found = true;
			reconstructPath(&nodeMap[current.pos.y][current.pos.x]);
			return true;
		}
		else
		{
			openList.pop();
			openListPoints.erase(current.pos);
			iterationCount++;
			closeListPoints.insert(current.pos);
		}


		std::vector<std::pair<int, int>> directions;
		getNeighborsPruned(current, directions);

		for (auto& dir : directions)
		{
			int dx = dir.first;
			int dy = dir.second;
			std::vector<std::pair<POINT, COLORREF>> jumpedPoints;
			Node* jumpPoint = jump(current.pos, dx, dy, end, jumpedPoints);
			if (jumpPoint != nullptr)
			{
				if (closeListPoints.find(jumpPoint->pos) != closeListPoints.end()) {
					continue;
				}

				if (openListPoints.find(jumpPoint->pos) == openListPoints.end()) {
					long tentative_g = current.g + heuristic(current.pos, jumpPoint->pos);

					if (tentative_g < jumpPoint->g || openListPoints.find(jumpPoint->pos) == openListPoints.end()) {
						jumpPoint->g = tentative_g;
						jumpPoint->h = heuristic(jumpPoint->pos, end);
						jumpPoint->f = jumpPoint->g + jumpPoint->h;
						jumpPoint->parent = &nodeMap[current.pos.y][current.pos.x];
						openList.push(*jumpPoint);
						openListPoints.insert(jumpPoint->pos);
					}
				}
			}
			for (auto& posColorPair : jumpedPoints)
			{
				PathListNodes.push_back(posColorPair);
			}
		}
		return false;
	}
	void getNeighborsPruned(Node& current, std::vector<std::pair<int, int>>& directions)
	{
		directions.clear();

		std::vector<std::pair<int, int>> allDirections =
		{
			{1, 0}, {0, 1}, {-1, 0}, {0, -1},
			{1, 1}, {1, -1}, {-1, -1}, {-1, 1}
		};

		if (current.parent == nullptr) {
			directions = allDirections;
			return;
		}

		int dx = current.pos.x - current.parent->pos.x;
		int dy = current.pos.y - current.parent->pos.y;

		dx = dx / max(abs(dx), 1);
		dy = dy / max(abs(dy), 1);

		// 이동 중인 방향에 따른 가지치기
		if (dx != 0 && dy != 0)
		{
			if (isValid(current.pos.x, current.pos.y + dy) && isValid(current.pos.x + dx, current.pos.y) &&
				isUnBlocked(current.pos.x + dx, current.pos.y) && isUnBlocked(current.pos.x, current.pos.y + dy))
			{
				directions.push_back({ dx, dy });
			}
			// 대각선 이동 시 상하좌우도 확인
			if (isValid(current.pos.x + dx, current.pos.y) && isUnBlocked(current.pos.x + dx, current.pos.y))
			{
				directions.push_back({ dx, 0 });
			}
			if (isValid(current.pos.x, current.pos.y + dy) && isUnBlocked(current.pos.x, current.pos.y + dy))
			{
				directions.push_back({ 0, dy });
			}
			//보조 탐색
			if (isValid(current.pos.x - dx, current.pos.y + dy) && isUnBlocked(current.pos.x - dx, current.pos.y + dy))
			{
				directions.push_back({ -dx, dy });
			}
			if (isValid(current.pos.x + dx, current.pos.y - dy) && isUnBlocked(current.pos.x + dx, current.pos.y - dy))
			{
				directions.push_back({ dx, -dy });
			}
		}
		else if (dx != 0)
		{
			// 수평 이동인 경우, 수평과 대각선만 탐색
			directions.push_back({ dx, 0 });
			if (isValid(current.pos.x, current.pos.y + 1))
				directions.push_back({ dx, 1 });
			if (isValid(current.pos.x, current.pos.y - 1))
				directions.push_back({ dx, -1 });
		}
		else if (dy != 0)
		{
			// 수직 이동인 경우, 수직과 대각선만 탐색
			directions.push_back({ 0, dy });
			if (isValid(current.pos.x + 1, current.pos.y))
				directions.push_back({ 1, dy });
			if (isValid(current.pos.x - 1, current.pos.y))
				directions.push_back({ -1, dy });
		}

		auto it = directions.begin();
		while (it != directions.end())
		{
			int newX = current.pos.x + it->first;
			int newY = current.pos.y + it->second;

			if (!isValid(newX, newY) || !isUnBlocked(newX, newY))
			{
				it = directions.erase(it);
			}
			else
			{
				++it;
			}
		}
	}


	COLORREF calculateColor(int iteration)
	{
		unsigned int seed = iteration * 1234567;
		int red = (seed >> 16) & 0xFF;
		int green = (seed >> 8) & 0xFF;
		int blue = seed & 0xFF;

		return RGB(red, green, blue);
	}
	Node* jump(POINT current, int dx, int dy, POINT end, std::vector<std::pair<POINT, COLORREF>>& jumpedPoints)
	{
		int nextX = current.x + dx;
		int nextY = current.y + dy;

		if (!isValid(nextX, nextY) || !isUnBlocked(nextX, nextY))
			return nullptr;

		POINT nextPoint = { nextX, nextY };
		if (isUnBlocked(nextPoint.x, nextPoint.y))
		{
			COLORREF color = calculateColor(iterationCount);
			jumpedPoints.push_back({ nextPoint, color });
		}

		if (isGoal(nextPoint, end))
		{
			return &nodeMap[nextY][nextX];
		}

		if (hasForcedNeighbor(nextPoint, dx, dy))
		{
			return &nodeMap[nextY][nextX];
		}

		if (dx != 0 && dy != 0) {
			if (jump(nextPoint, dx, 0, end, jumpedPoints) != nullptr ||
				jump(nextPoint, 0, dy, end, jumpedPoints) != nullptr) {
				return &nodeMap[nextY][nextX];
			}
		}
		return jump(nextPoint, dx, dy, end, jumpedPoints);
	}

	bool hasForcedNeighbor(POINT current, int dx, int dy)
	{
		// 대각선 이동인 경우
		if (dx != 0 && dy != 0) {
			return (isValid(current.x - dx, current.y + dy) && isUnBlocked(current.x - dx, current.y + dy) && !isUnBlocked(current.x - dx, current.y)) ||
				(isValid(current.x + dx, current.y - dy) && isUnBlocked(current.x + dx, current.y - dy) && !isUnBlocked(current.x, current.y - dy));
		}
		// 가로 이동인 경우
		else if (dx != 0) {
			return (isValid(current.x + dx, current.y + 1) && isUnBlocked(current.x + dx, current.y + 1) && !isUnBlocked(current.x, current.y + 1)) ||
				(isValid(current.x + dx, current.y - 1) && isUnBlocked(current.x + dx, current.y - 1) && !isUnBlocked(current.x, current.y - 1));
		}
		// 세로 이동인 경우
		else if (dy != 0) {
			return (isValid(current.x + 1, current.y + dy) && isUnBlocked(current.x + 1, current.y + dy) && !isUnBlocked(current.x + 1, current.y)) ||
				(isValid(current.x - 1, current.y + dy) && isUnBlocked(current.x - 1, current.y + dy) && !isUnBlocked(current.x - 1, current.y));
		}
		return false;
	}

	bool isDirectPathClear(const POINT& start, const POINT& end)
	{
		std::vector<POINT> line = BresenhamLine(start, end);
		for (const auto& point : line)
		{
			if (!isUnBlocked(point.x, point.y)) {
				return false;
			}
		}
		return true;
	}
	void reconstructPath(Node* current)
	{
		finalPath.clear();
		finalPathNodes.clear();

		while (current != nullptr)
		{
			finalPath.push_back(current);
			current = current->parent;
		}

		std::reverse(finalPath.begin(), finalPath.end());

		Node* startNode = finalPath[0];
		Node* longestNode = finalPath[0]; 
		int longestLine_length = 0;
		finalPathNodes.push_back(finalPath[0]->pos);

		for (int i = 1; i < finalPath.size(); ++i)
		{
			if (isDirectPathClear(startNode->pos, finalPath[i]->pos))
			{
				int length = getDistance(startNode->pos, finalPath[i]->pos);
				if (longestLine_length < length)
				{
					longestLine_length = length;
					longestNode = finalPath[i]; 
				}
			}
			else
			{
				finalPathNodes.push_back(longestNode->pos);

				--i;
				startNode = finalPath[i];
				longestNode = startNode;
				longestLine_length = 0;
			}
		}
		finalPathNodes.push_back(finalPath.back()->pos);
	}
	bool isInOpenList(POINT pt)
	{
		return openListPoints.find(pt) != openListPoints.end();
	}

	bool isInCloseList(POINT pt)
	{
		return closeListPoints.find(pt) != closeListPoints.end();
	}
	std::vector<POINT> BresenhamLine(const POINT& start, const POINT& end) {
		std::vector<POINT> line;
		int x1 = start.x, y1 = start.y;
		int x2 = end.x, y2 = end.y;

		int dx = abs(x2 - x1), dy = abs(y2 - y1);
		int sx = (x1 < x2) ? 1 : -1;
		int sy = (y1 < y2) ? 1 : -1;
		int err = dx - dy;

		while (true) {
			line.push_back({ x1, y1 });
			if (x1 == x2 && y1 == y2) break;

			int e2 = 2 * err;
			if (e2 > -dy) {
				err -= dy;
				x1 += sx;
			}
			if (e2 < dx) {
				err += dx;
				y1 += sy;
			}
		}

		return line;
	}

};


