#include "btree.h"
#include <iostream>

BTree::BTree()
    : headerObj(pagerObj),
      cache(pagerObj, headerObj),
      root(nullptr)
{
}

bool BTree::openFile(const char *filename)
{
    return pagerObj.open(filename);
}

void BTree::init(bool newDb, bool inMem)
{
    cache.init(inMem, *this);

    if (newDb || inMem)
    {
        root = new BTreeNode(true, 1, *this);
        headerObj.setRootIndex(headerObj.nextFree());
        headerObj.writeHeader();
    }
    else
    {
        headerObj.deserializeHeader();
        root = cache.get(headerObj.rootIndex);
    }
}

void BTree::traverse()
{
    if (root != nullptr)
        root->traverse();
}

BTreeNode *BTree::search(int k)
{
    return (root == nullptr) ? nullptr : root->search(k);
}

bool BTree::get(int k, char *result)
{
    BTreeNode *node = search(k);
    if (node == nullptr)
    {
        return false;
    }

    int idx = node->findKey(k);
    if (idx < node->numKeys && node->keys[idx].key == k)
    {
        strncpy(result, node->keys[idx].data, DATA_SIZE);
        return true;
    }

    return false;
}

void BTree::insert(int k, char data[DATA_SIZE])
{
    if (root->numKeys == 2 * t - 1)
    {
        cache.markDirty(root->index);

        BTreeNode *s = new BTreeNode(false, headerObj.nextFree(), *this);
        s->children[0] = root->index;

        s->splitChild(0, root);

        int i = 0;
        if (s->keys[0].key < k)
            i++;

        cache.get(s->children[i])->insertNonFull(k, data);
        root = s;
        cache.markDirty(root->index);
        headerObj.setRootIndex(root->index);
        headerObj.writeHeader();
    }
    else
    {
        root->insertNonFull(k, data);
    }

    cache.sync();
}

void BTree::remove(int k)
{
    if (!root)
    {
        std::cout << "The tree is empty\n";
        return;
    }

    root->remove(k);

    if (root->numKeys == 0)
    {
        BTreeNode *tmp = root;
        if (root->isLeaf)
            root = nullptr;
        else
            root = cache.get(root->children[0]);

        delete tmp;

        if (root != nullptr)
        {
            /* if the root changes, we need to update the index the header points to */
            headerObj.setRootIndex(root->index);
            cache.markDirty(root->index);
            headerObj.writeHeader();
        }
    }

    cache.sync();
}
