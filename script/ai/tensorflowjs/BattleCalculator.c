#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Vector3Int {
  int x;
  int y;
  int z;
} Vector3Int;

typedef struct BaseActionNode {
  struct BaseActionNode *Prev; // 前置点
  Vector3Int Coordinate;       // 坐标
  int MovementsLeft;           // 剩余移动力
  int MovementCost;            // 移动消耗
  int AStarF;                  // 离目标位置系数
  int AStarG;                  // 走到当前格子消耗了多少
  int AStarH;                  // 当前位置到目标位置的曼哈顿距离
} BaseActionNode;

typedef struct BFSActionNode {
  BaseActionNode base;
  struct BFSActionNode **Suff; // 可前往位置
  int SuffCount;
} BFSActionNode;

Vector3Int Way_Node_Directions[] = {{-1, 0, 1}, {1, 0, -1}, {-1, 1, 0},
                                    {0, 1, -1}, {0, -1, 1}, {1, -1, 0}};

int GetManhattanDistanceBetween(Vector3Int from, Vector3Int to) {
  return abs(from.x - to.x) + abs(from.y - to.y);
}

typedef int (*GetMovementsFunc)(Vector3Int);
typedef bool (*CheckIsBlockFunc)(Vector3Int);

bool CheckInArea(Vector3Int start_pos, int radius, Vector3Int check_pos,
                 bool pIgnoreMovement) {
  Vector3Int *nodes;
  int pathLength;
  int getMovements(Vector3Int coord) {
    return 1; // Replace with actual movement cost  判断位置消耗
  }

  bool checkIsBlock(Vector3Int coord) {
    return false; // Replace with actual block check  判断是不是阻挡格子
  }

  nodes = FindPath(start_pos, radius, pIgnoreMovement, getMovements,
                   checkIsBlock, &pathLength);

  for (int i = 0; i < pathLength; i++) {
    if (nodes[i].x == check_pos.x && nodes[i].y == check_pos.y &&
        nodes[i].z == check_pos.z) {
      free(nodes);
      return true;
    }
  }

  free(nodes);
  return false;
}

// private static Dictionary<Vector3Int, xxxxx> dic_HexMap = new
// Dictionary<Vector3Int, xxxx>();

/// 初始化静态地图数据
// public static void Init(MapConfig mapConfig)
//{
//     //TODO 协议结构的地图静态数据
//
//     if (mapConfig.xxxx != null)
//     {
//         for (int i = 0; i < mapConfig.xxx.Length; i++)
//         {
//             var pos = new Vector3Int(mapConfig.SurfaceCfgs[i].Hex_X,
//             mapConfig.SurfaceCfgs[i].Hex_Y, mapConfig.SurfaceCfgs[i].Hex_Z);
//             SurfaceCfgs surface;
//             if (!dic_HexMap.TryGetValue(pos, out surface))
//             {
//                 dic_HexMap[pos] = mapConfig.SurfaceCfgs[i];
//             }
//         }
//     }
// }

void CalcAStarMoveNodeMap(BFSActionNode *root, bool pIgnoreMovement,
                          GetMovementsFunc getMovements,
                          CheckIsBlockFunc checkIsBlock) {
  size_t visitedSize = 1000;
  Vector3Int *visited = malloc(sizeof(Vector3Int) * visitedSize);
  memset(visited, 0, sizeof(Vector3Int) * visitedSize);
  size_t visitedCount = 0;

  BFSActionNode *open[1000];
  size_t openCount = 0;

  open[openCount++] = root;
  memcpy(&visited[visitedCount++], &root->base.Coordinate, sizeof(Vector3Int));

  while (openCount > 0) {
    BFSActionNode *parent_Node = open[--openCount];

    for (int i = 0; i < sizeof(Way_Node_Directions) / sizeof(Vector3Int); i++) {
      Vector3Int new_Coord = {
          parent_Node->base.Coordinate.x + Way_Node_Directions[i].x,
          parent_Node->base.Coordinate.y + Way_Node_Directions[i].y,
          parent_Node->base.Coordinate.z + Way_Node_Directions[i].z};

      // TODO 检查下 这个格子是不是在地图数据上出现过 没有出现
      //  if (!dic_HexMap.ContainsKey(new_Coord))//检查格子是否合法
      //{
      //    continue;
      // }
      //

      bool alreadyVisited = false;
      for (size_t j = 0; j < visitedCount; j++) {
        if (visited[j].x == new_Coord.x && visited[j].y == new_Coord.y &&
            visited[j].z == new_Coord.z) {
          alreadyVisited = true;
          break;
        }
      }
      if (alreadyVisited)
        continue;

      if (checkIsBlock && checkIsBlock(new_Coord))
        continue;

      int movementsLeft = pIgnoreMovement ? parent_Node->base.MovementsLeft - 1
                                          : parent_Node->base.MovementsLeft -
                                                (*getMovements)(new_Coord);
      if (movementsLeft < 0)
        continue;

      BFSActionNode *new_RootNode = malloc(sizeof(BFSActionNode));
      new_RootNode->base.Prev = parent_Node;
      new_RootNode->base.Coordinate = new_Coord;
      new_RootNode->base.MovementsLeft = movementsLeft;
      new_RootNode->Suff = NULL;
      new_RootNode->SuffCount = 0;

      if (!parent_Node->Suff) {
        parent_Node->Suff =
            malloc(sizeof(BFSActionNode *) * 10); // Assuming max 10 suff nodes
      }
      parent_Node->Suff[parent_Node->SuffCount++] = new_RootNode;
      memcpy(&visited[visitedCount++], &new_Coord, sizeof(Vector3Int));
      open[openCount++] = new_RootNode;
    }
  }

  free(visited);
}

Vector3Int *GetMoveWayPath(BFSActionNode *root, int *pathLength) {
  *pathLength = 0;
  Vector3Int *waypath =
      malloc(sizeof(Vector3Int) * 1000); // Assuming max path length of 1000

  BFSActionNode *nodes[1000];
  size_t nodesCount = 0;

  nodes[nodesCount++] = root;
  while (nodesCount > 0) {
    BFSActionNode *n = nodes[--nodesCount];

    memcpy(&waypath[*pathLength], &n->base.Coordinate, sizeof(Vector3Int));
    (*pathLength)++;

    if (n->SuffCount > 0) {
      for (int i = 0; i < n->SuffCount; i++) {
        nodes[nodesCount++] = n->Suff[i];
      }
    }
  }

  return waypath;
}

/// 毒圈 视野
/// <param name="start_pos">起点</param>
/// <param name="movements">移动力消耗 -1 无视移动力 >=0 采用实际移动力消耗
/// </param> <param
/// name="pIgnoreMovement">无视实际消耗，每个格子消耗为一</param> <param
/// name="getMovements">通过坐标，访问移动力消耗</param> <param
/// name="getMovements">通过坐标，访问是不是阻挡格子</param>
Vector3Int *FindPath(Vector3Int start_pos, int movements, bool pIgnoreMovement,
                     GetMovementsFunc getMovements,
                     CheckIsBlockFunc checkIsBlock, int *pathLength) {
  BFSActionNode *root = malloc(sizeof(BFSActionNode));
  root->base.Prev = NULL;
  root->base.Coordinate = start_pos;
  root->base.MovementsLeft = movements <= -1 ? INT_MAX : movements;
  root->Suff = NULL;
  root->SuffCount = 0;

  CalcAStarMoveNodeMap(root, pIgnoreMovement, getMovements, checkIsBlock);
  Vector3Int *waypath = GetMoveWayPath(root, pathLength);

  // Free the allocated memory for the BFS tree
  // This is a simplified version and does not handle all memory freeing
  free(root);

  return waypath;
}

void CalcAStarNodeMap(BaseActionNode *parent_Node, Vector3Int to_pos,
                      BaseActionNode **open, int *openCount,
                      BaseActionNode *visited[], int size, int pIgnoreMovement,
                      GetMovementsFunc getMovements,
                      CheckIsBlockFunc checkIsBlock) {
  if (parent_Node == NULL) {
    return;
  }

  for (int i = 0; i < sizeof(Way_Node_Directions) / sizeof(Vector3Int); i++) {
    Vector3Int new_Coord = {
        parent_Node->Coordinate.x + Way_Node_Directions[i].x,
        parent_Node->Coordinate.y + Way_Node_Directions[i].y,
        parent_Node->Coordinate.z + Way_Node_Directions[i].z};

    for (int j = 0; j < size; j++) {
      if (visited[j] && visited[j]->Coordinate.x == new_Coord.x &&
          visited[j]->Coordinate.y == new_Coord.y &&
          visited[j]->Coordinate.z == new_Coord.z) {
        break;
      }
    }

    // TODO 检查下 这个格子是不是在地图数据上出现过 没有出现
    //  if (!dic_HexMap.ContainsKey(new_Coord))//检查格子是否合法
    //{
    //    continue;
    // }
    //

    if (checkIsBlock && checkIsBlock(new_Coord)) {
      continue;
    }

    int movementsLeft =
        pIgnoreMovement ? parent_Node->MovementsLeft - 1
                        : parent_Node->MovementsLeft - getMovements(new_Coord);
    if (movementsLeft < 0)
      continue;

    BaseActionNode *new_Node = malloc(sizeof(BaseActionNode));
    new_Node->Prev = parent_Node;
    new_Node->Coordinate = new_Coord;
    new_Node->MovementsLeft = movementsLeft;
    new_Node->MovementCost = parent_Node->MovementsLeft - movementsLeft;

    new_Node->AStarG = parent_Node->AStarG + new_Node->MovementCost;
    new_Node->AStarH = GetManhattanDistanceBetween(new_Coord, to_pos);
    new_Node->AStarF = new_Node->AStarG + new_Node->AStarH * 2;

    for (int j = 0; j < *openCount; j++) {
      if (open[j]->Coordinate.x == new_Coord.x &&
          open[j]->Coordinate.y == new_Coord.y &&
          open[j]->Coordinate.z == new_Coord.z) {
        if (open[j]->AStarG > new_Node->AStarG) {
          free(open[j]);
          open[j] = new_Node;
        }
        break;
      }
    }

    if (*openCount < size) {
      open[*openCount] = new_Node;
      (*openCount)++;
    } else {
      qsort(open, *openCount, sizeof(BaseActionNode *), CompareNodes);
      if (open[*openCount - 1]->AStarF > new_Node->AStarF) {
        free(open[*openCount - 1]);
        open[*openCount - 1] = new_Node;
      }
    }
  }
}

Vector3Int *GetWayPath(Vector3Int start_pos, Vector3Int to,
                       BaseActionNode *visited[], int size, int *pathLength) {
  *pathLength = 0;
  Vector3Int *waypath = malloc(sizeof(Vector3Int) * size);

  BaseActionNode *starActionNode;
  for (int i = 0; i < size; i++) {
    if (visited[i] && visited[i]->Coordinate.x == to.x &&
        visited[i]->Coordinate.y == to.y && visited[i]->Coordinate.z == to.z) {
      starActionNode = visited[i];
      break;
    }
  }

  if (starActionNode == NULL) {
    printf("移动到地图外啦 ： (%d, %d, %d)\n", to.x, to.y, to.z);
    return waypath;
  }

  while (starActionNode != NULL) {
    waypath[*pathLength] = starActionNode->Coordinate;
    (*pathLength)++;
    starActionNode = starActionNode->Prev;
  }

  return waypath;
}

/// 获取移动路径
/// <param name="start_pos">起点</param>
/// <param name="to">目标点</param>
/// <param name="movements">移动力消耗 -1 无视移动力 >=0 采用实际移动力消耗
/// </param> <param
/// name="pIgnoreMovement">无视实际消耗，每个格子消耗为一</param> <param
/// name="getMovements">通过坐标，访问移动力消耗</param> <param
/// name="getMovements">通过坐标，访问是不是阻挡格子</param>
Vector3Int *FindPath(Vector3Int start_pos, Vector3Int to, int movements,
                     int pIgnoreMovement, GetMovementsFunc getMovements,
                     CheckIsBlockFunc checkIsBlock, int *pathLength) {
  BaseActionNode *root = malloc(sizeof(BaseActionNode));
  root->Prev = NULL;
  root->Coordinate = start_pos;
  root->MovementsLeft = movements <= -1 ? INT_MAX : movements;
  root->AStarF = 0;
  root->AStarG = 0;
  root->AStarH = GetManhattanDistanceBetween(to, start_pos);

  BaseActionNode *open[1000];
  int openCount = 0;
  BaseActionNode *visited[1000];
  int size = 1000;

  CalcAStarNodeMap(root, to, open, &openCount, visited, size, pIgnoreMovement,
                   getMovements, checkIsBlock);

  qsort(open, openCount, sizeof(BaseActionNode *), CompareNodes);

  while (openCount > 0) {
    BaseActionNode *node = open[0];
    openCount--;
    CalcAStarNodeMap(node, to, open, &openCount, visited, size, pIgnoreMovement,
                     getMovements, checkIsBlock);
  }

  Vector3Int *waypath = GetWayPath(start_pos, to, visited, size, pathLength);

  for (int i = 0; i < openCount; i++) {
    free(open[i]);
  }
  free(root);

  return waypath;
}

int CompareNodes(const void *a, const void *b) {
  BaseActionNode *node1 = *(BaseActionNode **)a;
  BaseActionNode *node2 = *(BaseActionNode **)b;
  return node1->AStarF - node2->AStarF;
}

int main() {
  Vector3Int start_pos = {0, 0, 0};
  Vector3Int to_pos = {1, 1, 1};
  int pathLength;
  int movements = 10;
  int pIgnoreMovement = 0;

  int getMovements(Vector3Int coord) {
    return 1; // Replace with actual movement cost calculation
  }

  int checkIsBlock(Vector3Int coord) {
    return 0; // Replace with actual block check
  }

  Vector3Int *path = FindPath(start_pos, to_pos, movements, pIgnoreMovement,
                              getMovements, checkIsBlock, &pathLength);
  if (path) {
    for (int i = 0; i < pathLength; i++) {
      printf("(%d, %d, %d)\n", path[i].x, path[i].y, path[i].z);
    }
    free(path);
  }

  Vector3Int start_pos = {0, 0, 0};
  int pathLength;
  int getMovements(Vector3Int coord) {
    return 1; // Replace with actual movement cost calculation
  }

  bool checkIsBlock(Vector3Int coord) {
    return false; // Replace with actual block check
  }

  Vector3Int *path =
      FindPath(start_pos, 10, false, getMovements, checkIsBlock, &pathLength);
  if (path) {
    for (int i = 0; i < pathLength; i++) {
      printf("(%d, %d, %d)\n", path[i].x, path[i].y, path[i].z);
    }
    free(path);
  }

  return 0;
}
