
#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <unordered_map>

#define PAGE_SIZE 4096
#define NODE_HEADER_SIZE (sizeof(int) * 3)
#define AVAILABLE_SPACE (PAGE_SIZE - NODE_HEADER_SIZE)
#define MAX_KEYS 10
#define t ((MAX_KEYS + 1) / 2)
#define CHILD_PTR_SPACE ((MAX_KEYS + 1) * sizeof(int))
#define KEYS_SPACE (MAX_KEYS * sizeof(int))
#define AVAILABLE_DATA_SPACE (AVAILABLE_SPACE - KEYS_SPACE - CHILD_PTR_SPACE)
#define DATA_SIZE (AVAILABLE_DATA_SPACE / MAX_KEYS)

#define ROOT_INDEX_SIZE sizeof(int)
#define BITMAP_SIZE (PAGE_SIZE - ROOT_INDEX_SIZE)
#define BITS_PER_BYTE 8
#define MAX_PAGES (BITMAP_SIZE * BITS_PER_BYTE)

#define MAX_CACHE_SIZE 20

class BTree;
class BTreeNode;
class Pager;
class Header;
class NodeCache;

struct KeyValue
{
    int key;
    char data[DATA_SIZE];

    KeyValue()
    {
        key = -1;
        memset(data, 0, DATA_SIZE);
    }

    KeyValue(int k, const char *d)
    {
        key = k;
        memset(data, 0, DATA_SIZE);
        strncpy(data, d, DATA_SIZE - 1);
    }

    bool operator<(const KeyValue &other) const
    {
        return key < other.key;
    }

    bool operator==(const KeyValue &other) const
    {
        return key == other.key;
    }
};

#define KEY_VALUE_SIZE (sizeof(KeyValue))
#define CHILD_PTR_SIZE sizeof(int)

class Pager
{
public:
    Pager() = default;
    ~Pager() { cleanup(); }

    std::fstream file;

    void getPage(char buffer[PAGE_SIZE], int index);
    void writePage(int index, char *buffer);
    void writePages(int index, char *buffer);
    void flush();
    void deleteFile();
    void cleanup();
    bool open(const char *filename);
};

class Header
{
public:
    Header(Pager &pager) : pager(pager), rootIndex(0)
    {
        memset(bitmap, 0, BITMAP_SIZE);
    }

    int rootIndex;
    uint8_t bitmap[BITMAP_SIZE];

    void deserializeHeader();
    void serializeHeader(char buffer[PAGE_SIZE]);
    void writeHeader();
    void freeIndex(int index);
    void setIndex(int index);
    int nextFree();
    bool isAllocated(int index);
    void setRootIndex(int index);
    void setBit(int index, bool value);
    bool getBit(int index);

private:
    Pager &pager;
};

class BTreeNode
{
public:
    KeyValue keys[MAX_KEYS];
    int children[MAX_KEYS + 1];
    int numKeys;
    int index;
    bool isLeaf;

    BTreeNode(bool leaf, int idx, BTree &btree);
    ~BTreeNode();

    void traverse();
    BTreeNode *search(int k);
    int findKey(int k);
    void insertNonFull(int k, char data[DATA_SIZE]);
    void splitChild(int i, BTreeNode *y);
    void remove(int k);
    void removeFromLeaf(int idx);
    void removeFromNonLeaf(int idx);
    KeyValue getPred(int idx);
    KeyValue getSucc(int idx);
    void fill(int idx);
    void borrowFromPrev(int idx);
    void borrowFromNext(int idx);
    void merge(int idx);

private:
    BTree &btree;
};

class NodeCache
{
public:
    NodeCache(Pager &pager, Header &header)
        : pager(pager), header(header), btreePtr(nullptr), isInMemMode(false), lruCount(0) {}

    NodeCache(const NodeCache &) = delete;
    NodeCache &operator=(const NodeCache &) = delete;

    bool isInMemMode = false;

    void init(bool inMem, BTree &b);
    void add(int index, struct BTreeNode *node);
    void remove(int index);
    struct BTreeNode *get(int index);
    void markDirty(int index);
    void sync();

    void setBTree(BTree *btree) { btreePtr = btree; }

private:
    typedef struct
    {
        BTreeNode *node;
        int nodeIndex;
        bool isDirty;
    } CacheEntry;

    CacheEntry cache[MAX_CACHE_SIZE];
    std::unordered_map<int, int> nodeIndexToCachePos;
    int lruList[MAX_CACHE_SIZE];
    int lruCount = 0;
    Pager &pager;
    Header &header;
    BTree *btreePtr;

    void serializeNode(BTreeNode *node, char *buffer);
    BTreeNode *deserializeNode(char *buffer);
    int findInLru(int cachePos);
    void updateLru(int cachePos);
    int evictLruIfNeeded();
    int findFreeCacheSlot();
};

class BTree
{
public:
    BTree();
    ~BTree() = default;

    BTree(const BTree &) = delete;
    BTree &operator=(const BTree &) = delete;

    friend class BTreeNode;
    friend class NodeCache;

    void traverse();
    BTreeNode *search(int k);
    void insert(int k, char data[DATA_SIZE]);
    void remove(int k);
    void init(bool newDb, bool inMem);
    bool get(int k, char *result);

    bool openFile(const char *filename);

    inline NodeCache &nodeCache() { return cache; }
    inline Header &header() { return headerObj; }
    inline Pager &pager() { return pagerObj; }

private:
    BTreeNode *root;
    Pager pagerObj;
    Header headerObj;
    NodeCache cache;
};