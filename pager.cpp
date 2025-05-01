#include "btree.h"

bool Pager::open(const char *filename)
{
    std::ifstream checkFile(filename);
    bool fileExists = checkFile.good();
    checkFile.close();

    file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!file)
    {
        file.clear();
        file.open(filename, std::ios::out | std::ios::binary);
        file.close();
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    }
    return fileExists;
}

void Pager::getPage(char buffer[PAGE_SIZE], int index)
{
    file.seekg(PAGE_SIZE * index, std::ios::beg);
    file.read(buffer, PAGE_SIZE);
}

void Pager::writePage(int index, char *buffer)
{
    file.seekp(PAGE_SIZE * index, std::ios::beg);
    file.write(buffer, PAGE_SIZE);
    file.flush();
}

/*
for when we mark multiple pages as dirty in a single op
*/
void Pager::writePages(int index, char *buffer)
{
    file.seekp(PAGE_SIZE * index, std::ios::beg);
    file.write(buffer, PAGE_SIZE);
}

void Pager::flush()
{
    file.flush();
}

void Pager::cleanup()
{
    if (file.is_open())
    {
        file.close();
    }
}

void Pager::deleteFile()
{
    file.close();
}