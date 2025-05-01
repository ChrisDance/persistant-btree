#include "btree.h"

void NodeCache::init(bool inMem, BTree &b)
{
    btreePtr = &b;
    /* in memory for testing btree ops */
    isInMemMode = inMem;
    for (int i = 0; i < MAX_CACHE_SIZE; i++)
    {
        cache[i].node = nullptr;
        cache[i].nodeIndex = -1;
        cache[i].isDirty = false;
        lruList[i] = -1;
    }

    nodeIndexToCachePos.clear();
    lruCount = 0;
}

int NodeCache::findInLru(int cachePos)
{
    for (int i = 0; i < lruCount; i++)
    {
        if (lruList[i] == cachePos)
        {
            return i;
        }
    }
    return -1;
}

void NodeCache::updateLru(int cachePos)
{
    int pos = findInLru(cachePos);

    if (pos != -1)
    {
        for (int i = pos; i > 0; i--)
        {
            lruList[i] = lruList[i - 1];
        }
    }
    else
    {
        if (lruCount < MAX_CACHE_SIZE)
        {
            for (int i = lruCount; i > 0; i--)
            {
                lruList[i] = lruList[i - 1];
            }
            lruCount++;
        }
        else
        {
            std::cerr << "LRU list is full but node not found" << std::endl;
            return;
        }
    }

    lruList[0] = cachePos;
}

int NodeCache::findFreeCacheSlot(void)
{
    for (int i = 0; i < MAX_CACHE_SIZE; i++)
    {
        if (cache[i].node == nullptr)
        {
            return i;
        }
    }

    return evictLruIfNeeded();
}

int NodeCache::evictLruIfNeeded(void)
{
    if (lruCount == 0)
    {
        std::cerr << "Error: Cannot evict from empty cache" << std::endl;
        return -1;
    }

    int lruCachePos = lruList[lruCount - 1];
    int nodeIndex = cache[lruCachePos].nodeIndex;

    if (!isInMemMode && cache[lruCachePos].isDirty && cache[lruCachePos].node != nullptr)
    {
        char buffer[PAGE_SIZE];
        serializeNode(cache[lruCachePos].node, buffer);
        pager.writePage(nodeIndex, buffer);
    }

    if (cache[lruCachePos].node != nullptr)
    {
        cache[lruCachePos].node = nullptr;
    }

    nodeIndexToCachePos.erase(nodeIndex);

    cache[lruCachePos].nodeIndex = -1;
    cache[lruCachePos].isDirty = false;

    lruCount--;

    return lruCachePos;
}

BTreeNode *NodeCache::get(int nodeIndex)
{
    if (nodeIndex <= 0)
    {
        std::cerr << "Invalid node index: " << nodeIndex << std::endl;
        return nullptr;
    }

    auto it = nodeIndexToCachePos.find(nodeIndex);
    if (it != nodeIndexToCachePos.end())
    {
        int cachePos = it->second;
        updateLru(cachePos);
        return cache[cachePos].node;
    }

    if (isInMemMode)
    {
        throw std::runtime_error("Too many nodes requested");
        return nullptr;
    }

    char pageBuffer[PAGE_SIZE];
    pager.getPage(pageBuffer, nodeIndex);

    BTreeNode *node = deserializeNode(pageBuffer);
    if (node == nullptr)
    {
        return nullptr;
    }

    int cachePos = findFreeCacheSlot();
    if (cachePos < 0)
    {
        std::cerr << "Failed to find cache slot" << std::endl;
        return nullptr;
    }

    cache[cachePos].node = node;
    cache[cachePos].nodeIndex = nodeIndex;
    cache[cachePos].isDirty = false;

    nodeIndexToCachePos[nodeIndex] = cachePos;
    updateLru(cachePos);

    return node;
}

void NodeCache::add(int nodeIndex, BTreeNode *node)
{
    if (nodeIndex <= 0 || node == nullptr)
    {
        std::cerr << "Invalid node index or nullptr node" << std::endl;
        return;
    }

    auto it = nodeIndexToCachePos.find(nodeIndex);
    if (it != nodeIndexToCachePos.end())
    {
        int cachePos = it->second;
        cache[cachePos].node = node;
        cache[cachePos].isDirty = true;
        updateLru(cachePos);
        return;
    }

    int cachePos = findFreeCacheSlot();
    if (cachePos < 0)
    {
        std::cerr << "Failed to find cache slot" << std::endl;
        return;
    }

    cache[cachePos].node = node;
    cache[cachePos].nodeIndex = nodeIndex;
    cache[cachePos].isDirty = true;

    nodeIndexToCachePos[nodeIndex] = cachePos;
    updateLru(cachePos);
}

void NodeCache::remove(int nodeIndex)
{
    auto it = nodeIndexToCachePos.find(nodeIndex);
    if (it == nodeIndexToCachePos.end())
    {
        return;
    }

    int cachePos = it->second;

    if (!isInMemMode && cache[cachePos].isDirty && cache[cachePos].node != nullptr)
    {
        char buffer[PAGE_SIZE];
        serializeNode(cache[cachePos].node, buffer);
        pager.writePage(nodeIndex, buffer);
    }

    if (cache[cachePos].node != nullptr)
    {
        cache[cachePos].node = nullptr;
    }

    cache[cachePos].nodeIndex = -1;
    cache[cachePos].isDirty = false;

    nodeIndexToCachePos.erase(nodeIndex);

    int lruPos = findInLru(cachePos);
    if (lruPos != -1)
    {
        for (int i = lruPos; i < lruCount - 1; i++)
        {
            lruList[i] = lruList[i + 1];
        }
        lruCount--;
    }
}

void NodeCache::markDirty(int nodeIndex)
{
    if (isInMemMode)
    {
        return;
    }
    auto it = nodeIndexToCachePos.find(nodeIndex);
    if (it != nodeIndexToCachePos.end())
    {
        int cachePos = it->second;
        cache[cachePos].isDirty = true;
    }
}

void NodeCache::sync()
{
    if (isInMemMode)
    {
        return;
    }

    for (int i = 0; i < MAX_CACHE_SIZE; i++)
    {
        if (cache[i].isDirty && cache[i].node != nullptr)
        {
            char buffer[PAGE_SIZE];
            serializeNode(cache[i].node, buffer);
            pager.writePage(cache[i].nodeIndex, buffer);
            cache[i].isDirty = false;
        }
    }

    pager.flush();
}

void NodeCache::serializeNode(BTreeNode *node, char *buffer)
{
    std::memset(buffer, 0, PAGE_SIZE);

    int numChildren = node->isLeaf ? 0 : node->numKeys + 1;

    std::memcpy(buffer, &(node->index), sizeof(int));
    std::memcpy(buffer + sizeof(int), &(node->numKeys), sizeof(int));
    std::memcpy(buffer + 2 * sizeof(int), &numChildren, sizeof(int));

    char *kvStart = buffer + NODE_HEADER_SIZE;
    std::memcpy(kvStart, node->keys, sizeof(KeyValue) * MAX_KEYS);

    if (!node->isLeaf)
    {
        char *childStart = kvStart + (sizeof(KeyValue) * MAX_KEYS);
        std::memcpy(childStart, node->children, sizeof(int) * (MAX_KEYS + 1));
    }
}

BTreeNode *NodeCache::deserializeNode(char *buffer)
{
    int index, numKeys, numChildren;

    std::memcpy(&index, buffer, sizeof(int));
    std::memcpy(&numKeys, buffer + sizeof(int), sizeof(int));
    std::memcpy(&numChildren, buffer + 2 * sizeof(int), sizeof(int));

    if (!btreePtr)
    {
        std::cerr << "Error: NodeCache has no associated BTree" << std::endl;
        return nullptr;
    }

    BTreeNode *node = new BTreeNode((numChildren == 0), index, *btreePtr);

    node->index = index;
    node->numKeys = numKeys;

    char *kvStart = buffer + NODE_HEADER_SIZE;

    std::memcpy(node->keys, kvStart, sizeof(KeyValue) * MAX_KEYS);

    if (!node->isLeaf)
    {
        char *childStart = kvStart + (sizeof(KeyValue) * MAX_KEYS);
        std::memcpy(node->children, childStart, sizeof(int) * (MAX_KEYS + 1));
    }

    return node;
}