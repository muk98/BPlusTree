#pragma once
#include "node.h"

class BPlusTree{
  public:
    BPlusTree(int numIndexPointers,int numDataPointers);
    IndexNode* toIndexNode(void *p);
    DataNode* toDataNode(void *p);
    void* insert(BPlusTree *tree,void* node,int key,int depth,void *newChildEntry,int &UpKey);
    DataNode* splitLeafChild(DataNode *x,int key,int &UpKey);
    IndexNode* splitNonLeafChild(IndexNode *x,void* newChildEntry,int &UpKey);
    void insertNonFull(void *node, int key, int depth);
    DataNode* search(void *node, int key, int depth);
    void LevelOrderTraversal(void *node, int depth);

    void* root;
    int numIndexPointers;
    int numDataPointers;
    int depth;
};