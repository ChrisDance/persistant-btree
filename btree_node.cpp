#include "btree.h"

BTreeNode::BTreeNode(bool leaf, int idx, BTree &tree)
    : btree(tree), isLeaf(leaf), index(idx), numKeys(0)
{

    for (int i = 0; i < MAX_KEYS; i++)
    {
        keys[i] = KeyValue();
    }

    for (int i = 0; i <= MAX_KEYS; i++)
    {
        children[i] = -1;
    }

    btree.nodeCache().add(index, this);
    btree.nodeCache().markDirty(index);
}

BTreeNode::~BTreeNode()
{
    btree.nodeCache().remove(index);
    btree.header().freeIndex(index);
}

int BTreeNode::findKey(int k)
{
    int idx = 0;
    while (idx < numKeys && keys[idx].key < k)
        ++idx;
    return idx;
}

void BTreeNode::remove(int k)
{
    int idx = findKey(k);

    if (idx < numKeys && keys[idx].key == k)
    {
        if (isLeaf)
            removeFromLeaf(idx);
        else
            removeFromNonLeaf(idx);
    }
    else
    {
        if (isLeaf)
        {
            std::cout << "The key " << k << " is does not exist in the tree\n";
            return;
        }

        bool flag = ((idx == numKeys) ? true : false);

        if (btree.nodeCache().get(children[idx])->numKeys < t)
            fill(idx);

        if (flag && idx > numKeys)
            btree.nodeCache().get(children[idx - 1])->remove(k);
        else
            btree.nodeCache().get(children[idx])->remove(k);
    }
    return;
}

void BTreeNode::removeFromLeaf(int idx)
{
    for (int i = idx + 1; i < numKeys; ++i)
        keys[i - 1] = keys[i];

    numKeys--;
    btree.nodeCache().markDirty(index);
    return;
}

void BTreeNode::removeFromNonLeaf(int idx)
{
    int k = keys[idx].key;

    if (btree.nodeCache().get(children[idx])->numKeys >= t)
    {
        KeyValue pred = getPred(idx);
        keys[idx] = pred;
        btree.nodeCache().markDirty(index);
        btree.nodeCache().get(children[idx])->remove(pred.key);
    }
    else if (btree.nodeCache().get(children[idx + 1])->numKeys >= t)
    {
        KeyValue succ = getSucc(idx);
        keys[idx] = succ;
        btree.nodeCache().get(children[idx + 1])->remove(succ.key);
    }
    else
    {
        merge(idx);
        btree.nodeCache().get(children[idx])->remove(k);
    }
    return;
}

KeyValue BTreeNode::getPred(int idx)
{
    BTreeNode *cur = btree.nodeCache().get(children[idx]);
    while (!cur->isLeaf)
        cur = btree.nodeCache().get(cur->children[cur->numKeys]);

    return cur->keys[cur->numKeys - 1];
}

KeyValue BTreeNode::getSucc(int idx)
{
    BTreeNode *cur = btree.nodeCache().get(children[idx + 1]);
    while (!cur->isLeaf)
        cur = btree.nodeCache().get(cur->children[0]);

    return cur->keys[0];
}

void BTreeNode::fill(int idx)
{
    if (idx != 0 && btree.nodeCache().get(children[idx - 1])->numKeys >= t)
        borrowFromPrev(idx);
    else if (idx != numKeys && btree.nodeCache().get(children[idx + 1])->numKeys >= t)
        borrowFromNext(idx);
    else
    {
        if (idx != numKeys)
            merge(idx);
        else
            merge(idx - 1);
    }
    return;
}

void BTreeNode::borrowFromPrev(int idx)
{
    BTreeNode *child = btree.nodeCache().get(children[idx]);
    BTreeNode *sibling = btree.nodeCache().get(children[idx - 1]);

    for (int i = child->numKeys - 1; i >= 0; --i)
        child->keys[i + 1] = child->keys[i];

    if (!child->isLeaf)
    {
        for (int i = child->numKeys; i >= 0; --i)
            child->children[i + 1] = child->children[i];
    }

    child->keys[0] = keys[idx - 1];

    if (!child->isLeaf)
        child->children[0] = sibling->children[sibling->numKeys];

    keys[idx - 1] = sibling->keys[sibling->numKeys - 1];

    child->numKeys += 1;
    sibling->numKeys -= 1;

    btree.nodeCache().markDirty(index);
    btree.nodeCache().markDirty(child->index);
    btree.nodeCache().markDirty(sibling->index);

    return;
}

void BTreeNode::borrowFromNext(int idx)
{
    BTreeNode *child = btree.nodeCache().get(children[idx]);
    BTreeNode *sibling = btree.nodeCache().get(children[idx + 1]);

    child->keys[(child->numKeys)] = keys[idx];

    if (!(child->isLeaf))
        child->children[(child->numKeys) + 1] = sibling->children[0];

    keys[idx] = sibling->keys[0];

    for (int i = 1; i < sibling->numKeys; ++i)
        sibling->keys[i - 1] = sibling->keys[i];

    if (!sibling->isLeaf)
    {
        for (int i = 1; i <= sibling->numKeys; ++i)
            sibling->children[i - 1] = sibling->children[i];
    }

    child->numKeys += 1;
    sibling->numKeys -= 1;

    btree.nodeCache().markDirty(index);
    btree.nodeCache().markDirty(child->index);
    btree.nodeCache().markDirty(sibling->index);

    return;
}

void BTreeNode::merge(int idx)
{
    BTreeNode *child = btree.nodeCache().get(children[idx]);
    BTreeNode *sibling = btree.nodeCache().get(children[idx + 1]);

    child->keys[t - 1] = keys[idx];

    for (int i = 0; i < sibling->numKeys; ++i)
        child->keys[i + t] = sibling->keys[i];

    if (!child->isLeaf)
    {
        for (int i = 0; i <= sibling->numKeys; ++i)
            child->children[i + t] = sibling->children[i];
    }

    for (int i = idx + 1; i < numKeys; ++i)
        keys[i - 1] = keys[i];

    for (int i = idx + 2; i <= numKeys; ++i)
        children[i - 1] = children[i];

    child->numKeys += sibling->numKeys + 1;
    numKeys--;

    btree.nodeCache().markDirty(index);
    btree.nodeCache().markDirty(child->index);

    delete sibling;
}

void BTreeNode::insertNonFull(int k, char data[DATA_SIZE])
{
    if (isLeaf == true)
    {
        int existingPos = -1;
        for (int j = 0; j < numKeys; j++)
        {
            if (keys[j].key == k)
            {
                existingPos = j;
                break;
            }
        }

        if (existingPos != -1)
        {
            memcpy(keys[existingPos].data, data, DATA_SIZE);
            btree.nodeCache().markDirty(index);
            return;
        }

        int i = numKeys - 1;
        while (i >= 0 && keys[i].key > k)
        {
            keys[i + 1] = keys[i];
            i--;
        }

        keys[i + 1].key = k;
        memcpy(keys[i + 1].data, data, DATA_SIZE);
        numKeys++;
        btree.nodeCache().markDirty(index);
    }
    else
    {
        int i = numKeys - 1;
        while (i >= 0 && keys[i].key > k)
            i--;

        if (btree.nodeCache().get(children[i + 1])->numKeys == 2 * t - 1)
        {
            splitChild(i + 1, btree.nodeCache().get(children[i + 1]));

            if (keys[i + 1].key < k)
                i++;
        }
        btree.nodeCache().get(children[i + 1])->insertNonFull(k, data);
    }
}

void BTreeNode::splitChild(int i, BTreeNode *y)
{
    BTreeNode *z = new BTreeNode(y->isLeaf, btree.header().nextFree(), btree);
    z->numKeys = t - 1;

    for (int j = 0; j < t - 1; j++)
        z->keys[j] = y->keys[j + t];

    if (y->isLeaf == false)
    {
        for (int j = 0; j < t; j++)
            z->children[j] = y->children[j + t];
    }

    y->numKeys = t - 1;

    for (int j = numKeys; j >= i + 1; j--)
        children[j + 1] = children[j];

    children[i + 1] = z->index;

    for (int j = numKeys - 1; j >= i; j--)
        keys[j + 1] = keys[j];

    keys[i] = y->keys[t - 1];

    numKeys = numKeys + 1;

    btree.nodeCache().markDirty(index);
    btree.nodeCache().markDirty(y->index);
    btree.nodeCache().markDirty(z->index);
}

void BTreeNode::traverse()
{
    int i;
    for (i = 0; i < numKeys; i++)
    {
        if (isLeaf == false)
            btree.nodeCache().get(children[i])->traverse();
        std::cout << keys[i].key << " " << keys[i].data << "\n";
    }

    if (isLeaf == false)
        btree.nodeCache().get(children[i])->traverse();
}

BTreeNode *BTreeNode::search(int k)
{
    int i = 0;
    while (i < numKeys && k > keys[i].key)
        i++;

    if (keys[i].key == k)
        return this;

    if (isLeaf == true)
        return NULL;

    return btree.nodeCache().get(children[i])->search(k);
}