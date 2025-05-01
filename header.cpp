#include "btree.h"
bool Header::getBit(int index)
{
    if (index < 0 || index >= MAX_PAGES)
        return true;

    int byteIndex = index / BITS_PER_BYTE;
    int bitIndex = index % BITS_PER_BYTE;
    return (bitmap[byteIndex] & (1 << bitIndex)) != 0;
}

void Header::setBit(int index, bool value)
{
    if (index < 0 || index >= MAX_PAGES)
        return;

    int byteIndex = index / BITS_PER_BYTE;
    int bitIndex = index % BITS_PER_BYTE;

    if (value)
        bitmap[byteIndex] |= (1 << bitIndex);
    else
        bitmap[byteIndex] &= ~(1 << bitIndex);
}

void Header::deserializeHeader()
{
    char buffer[PAGE_SIZE];
    pager.getPage(buffer, 0);

    memcpy(&rootIndex, buffer, ROOT_INDEX_SIZE);
    memcpy(bitmap, buffer + ROOT_INDEX_SIZE, BITMAP_SIZE);
}

void Header::writeHeader()
{
    char buffer[PAGE_SIZE];
    serializeHeader(buffer);
    pager.writePage(0, buffer);
}

void Header::serializeHeader(char buffer[PAGE_SIZE])
{
    memset(buffer, 0, PAGE_SIZE);
    memcpy(buffer, &rootIndex, ROOT_INDEX_SIZE);
    memcpy(buffer + ROOT_INDEX_SIZE, bitmap, BITMAP_SIZE);
}

void Header::freeIndex(int index)
{
    if (index <= 0)
        return;

    setBit(index, false);
}

void Header::setIndex(int index)
{
    if (index < 0)
        return;

    setBit(index, true);
}

int Header::nextFree()
{
    for (int i = 1; i < MAX_PAGES; i++)
    {
        if (!getBit(i))
        {
            setIndex(i);
            return i;
        }
    }
    return -1;
}

bool Header::isAllocated(int index)
{
    return getBit(index);
}

void Header::setRootIndex(int index)
{
    rootIndex = index;
}