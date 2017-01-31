/*
 * Nicholas Marasco
 * CIS 421 - AI
 * Assignment 3
 * Due Oct. 24
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "node.h"

#define DATA_LINE_SIZE 128
#define GWEIGHT 0.1
#define QWEIGHT 0.1
#define DWEIGHT 0.8

// change for type of search 1 for distance, 2 for other
#define STEP 1

// prototypes
void search(Node**,int);
Node** buildGraph();
void buildPath(int,Node**);
int getLowest(int*,int,Node**);
int getDistVal(int,int,int);
int parseStart(char*,Node**);
unsigned long hash(char*);

// debug/utility prototypes
int indexOf(int,int*,int);
void removeArr(int,int*,int);
void destroyGraph(Node**);
void printGraph(Node**);

int main(int argc, char *argv[]){
  Node **graph = buildGraph();
  int start;
  if (argc == 1){
    start = 0;
  }
  else if(argc == 2){
    start = parseStart(argv[1],graph);
    if(start == -1){
      fprintf(stderr,"Error: city name: %s not found\n",argv[1]);
      return 1;
    }
  }
  else{
    fprintf(stderr,"usage: <program-name> [<city-name>]\n");
    return 1;
  }
//   printGraph(graph);
  search(graph,start);
  destroyGraph(graph);
  return 0;
}

// perform A* search on the graph for Iron Hills
// parameters:
//  graph - the graph to search on
//  start - city index to start at
// returns: array of node indexes for optimal path
void search(Node **graph,int start){
  int *path;
  int *closed = malloc(sizeof(int)*32);
  int *open = malloc(sizeof(int)*32);
  int cSize = 0;
  int oSize = 0;
  int cur = start;
  graph[cur]->g = 0;
  graph[cur]->f = graph[cur]->h;
  graph[cur]->pathParent = -1;
  open[oSize++] = cur;

  while(oSize){
    cur = getLowest(open,oSize,graph);
//     printf("%d\n",graph[cur]->f);
    if(cur == 23){
      free(closed);
      free(open);
      buildPath(cur,graph);
      printf("%d\n",graph[cur]->f);
      break;
    }
    removeArr(cur,open,oSize--);
    closed[cSize++] = cur;
    Noderef **localLinks = graph[cur]->links;
    int i = 0;
    while(localLinks[i] != NULL){
      int neighbor = localLinks[i]->idx;
      if(indexOf(neighbor,closed,cSize) != -1){
        i++; continue;
      }
      int dVal = getDistVal(localLinks[i]->dist,
                            localLinks[i]->rQuality,
                            localLinks[i]->dangerZone);
      int tempG = graph[cur]->g + dVal;
      if(indexOf(neighbor,open,oSize) == -1){
        open[oSize++] = neighbor;
      }
      else if(tempG >= graph[neighbor]->g){
        i++; continue;
      }
      graph[neighbor]->pathParent = cur;
      graph[neighbor]->g = tempG;
      graph[neighbor]->f = tempG + graph[neighbor]->h;
    }
  }
}

// build the initial graph
// returns: array of Node pointers representing nodes and their links
Node** buildGraph(){

  char *line = malloc(DATA_LINE_SIZE);
  Node **graph = malloc(sizeof(Node*)*32);
  int pos, goalDist, idx, dist, qual, danger;
  char *name;
  Noderef **links;
  int offset, bytes;

  FILE *input = fopen("data.txt","r");

  while(fgets(line,DATA_LINE_SIZE,input)){
    name = malloc(32);
    links = malloc(sizeof(Noderef*)*16);
    sscanf(line,"%d:%[^:]:%d%n",&pos,name,&goalDist,&offset);
    int i = 0;
    while(sscanf(line + offset,":%d,%d,%d,%d%n",&idx,&dist,&qual,&danger,&bytes)){
      offset += bytes;
      links[i] = malloc(sizeof(Noderef));
      links[i]->idx = idx;
      links[i]->dist = dist;
      links[i]->rQuality = qual;
      links[i++]->dangerZone = danger;
    }
    links[i] = (Noderef*)NULL;
    graph[pos] = malloc(sizeof(Node));
    graph[pos]->name = name;
    graph[pos]->hashName = hash(name);
//     graph[pos]->f = INT_MAX;
    graph[pos]->h = goalDist;
    graph[pos]->links = links;
  }
  graph[++pos] = (Node*)NULL;
  free(line);
  fclose(input);
  return graph;
}

// build the final path
// parameters:
//  cur - current path
//  graph - the graph
void buildPath(int cur,Node **graph){
  char** path = malloc(sizeof(char*)*32);
  int i = 0;
  while(cur != -1){
    path[i] = malloc(32);
    path[i++] = graph[cur]->name;
    cur = graph[cur]->pathParent;
  }
  for(i = i-1; i >= 0; i--){
    printf("%s\n",path[i]);
  }

}

// remove and retrieve the lowest cost Node index in list
// parameters:
//  list - array to find lowest value in
//  size - size of the list array
//  graph - graph of nodes
// returns: the lowest value in list
int getLowest(int *list, int size, Node **graph){
  int low = 0;
  int i;
  for(i = 1; i < size; i++){
    int f1 = graph[list[i]]->g + graph[list[i]]->h;
    int f2 = graph[list[low]]->g + graph[list[low]]->h;
    (f1 < f2) && (low = i);
  }
  low = list[low];
  return low;
}

// get the modified dist value
// parameters:
//  dist - point to point dist
//  qual - road quality
//  danger - amount of danger
// returns: calculated dist value
int getDistVal(int dist, int qual, int danger){
  if(STEP == 1){
//     printf("%d\n",dist);
    return dist;
  }
  else{
    // Values of road and danger quality are multiplied by 10 to have an
    // influence similar to that of distance
    int val = (GWEIGHT*(dist) + 10*(QWEIGHT*(100-qual)) + 10*(DWEIGHT*(danger)));
//     printf("%d\n",val);
    return val;
  }
}

// parse the args for a start place
// parameters:
//  arr - char pointer to check
// returns: index of the starting city
int parseStart(char *arr, Node** graph){
  unsigned long arg = hash(arr);
  int i;
  while(graph[i] != NULL){
    if(graph[i]->hashName == arg){
      return i;
    }
    i++;
  }
  return -1;
}

// hash string for faster processing
// parameters:
//  s - string to hash
// returns: long hash value
unsigned long hash(char *s){
  unsigned long hash = 5381;
  int c;
  while( c = *s++){
    hash = ((hash << 5) + hash) + c;
  }
  return hash;
}

// check array for given value
// parameters:
//  val - value to search for
//  arr - array to search through
//  size - size of array
// returns: index if found, -1 otherwise
int indexOf(int val, int* arr, int size){
  int i;
  for(i = 0; i < size; i++)
    if(arr[i] == val){ return i; }
  return -1;
}

// remove value from array
// parameters:
//  val - value to remove
//  arr - array to remove from
//  size - size of array
void removeArr(int val, int* arr, int size){
  int i = indexOf(val,arr,size);
  if(i != -1){
    for(i; i < size-1; i++){
      arr[i] = arr[i+1];
    }
  }
  else{
    printf("Value does not exist\n");
  }
}

// destroy the graph and everything it loves
// parameters:
//  graph - graph to destroy
void destroyGraph(Node **graph){
  int i = 0;
  while(graph[i] != NULL){
    int j = 0;
    while(graph[i]->links[j] != NULL){
      free(graph[i]->links[j++]);
    }
    free(graph[i]->links);
    free(graph[i++]);
  }
  free(graph);
}

// print out the graph
// parameters:
//  graph - graph to print
void printGraph(Node **graph){
  int i = 0;
  Noderef** conn;
  while(graph[i] != NULL){
    printf("Node Name: %s\n",graph[i]->name);
    printf("  Goal Dist: %d\n",graph[i]->h);
    printf("  Connections:\n");
    conn = graph[i++]->links;
    int j = 0;
    while(conn[j] != NULL){
      printf("    Name: %s\n",graph[conn[j]->idx]->name);
      printf("      Dist: %d\n",conn[j]->dist);
      printf("      Qual: %d\n",conn[j]->rQuality);
      printf("      Danger: %d\n",conn[j]->dangerZone);
      j++;
    }
  }
}
