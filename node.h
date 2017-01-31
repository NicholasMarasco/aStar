typedef struct Noderef{
  int idx;
  int dist;
  int rQuality;
  int dangerZone;
} Noderef;

typedef struct Node{
  char *name;
  unsigned long hashName;
  int f,g,h;
  Noderef **links;
  int pathParent;
} Node;
