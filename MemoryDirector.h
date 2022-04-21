/*
 * An experemental memory administration booster
 * A template makes an empty memory blocks by the
 * type of containing memory and user size of an array
*/

#ifndef MEMORYDIRECTOR_H
#define MEMORYDIRECTOR_H
#include "predefines.h"

template<typename T, int block_size>
class MemoryDirector
{
private:
    unsigned int type_size;                     //Type size in bytes;
    handle_t handle_counter;                    //Counter for unique handles number
    struct nodeblock                            //Node block, single info cell with:
    {
        T * node_pointer;                       //Pointer to a main block memory part
        bool free_status;                       //Status of the sell (free/occupied)
    };

    struct memblock                             //One memory page structure with:
    {
        int block_num;                          //Block number.
        void * block_pointer;                   //Pointer to the memory of the block.
        int counter;                            //Occupation counter.
        int fragPg;                             //Fragmentation value (not nessesary but...)
        QVector<nodeblock> node_handler;        //Vector with Node cells that pointed to a
    };                                          //main block parts.

    struct handleblock                          //One handle record structure with:
    {
        int index_num;                          //Index number (index of node_handler in "memblock")
        int block_num;                          //Block number (index of page in "pages" linked list)
    };

    QLinkedList<memblock*> pages;               //List of the current memory blocks.
    QMap<handle_t, handleblock> handle_map;     //Handle map
    void Defragmentation();                                 //Defragmentation method;
    memblock * newBlock();                                  //Method creates a new block;
    void clearBlock(memblock*block);                        //Method clear block data;
    void * getFreeNode(memblock * block, int & index);      //Get's free memory cell and return pointer to it
    void freeOccNode(memblock* block, int index);           //Free occupy memory cell in the block
    bool blockIsFull(const memblock * block) const;         //Return true if block memory is occupy
    bool blockIsEmpty(const memblock * block) const;        //Return true if block memory is free
public:
    MemoryDirector();
    ~MemoryDirector();
    void * getAddr(handle_t & handle);                      //Method returns free memory cell and a handle in signature
    T* getSpace(const handle_t &handle);                    //Method returns memory cell by the handle;
    bool freeAddr(const handle_t & handle);                 //Method make free a memory cell by the handle;
};

//==================================================================================|
//**********************************************************************************|
//==================================================================================|


template<typename T, int block_size>
MemoryDirector<T,block_size>::MemoryDirector()
{
    type_size = sizeof(T);
    pages.append(newBlock());
    handle_counter=0;
}

template<typename T, int block_size>
MemoryDirector<T,block_size>::~MemoryDirector()
{
    typename QLinkedList<memblock*>::iterator iterator = pages.begin();
    while(iterator!=pages.end)
    {
        memblock * temp = *iterator;
        iterator++;
        clearBlock(temp);
        delete temp;
    }
    handle_map.clear();
}

template<typename T, int block_size>
void MemoryDirector<T,block_size>::clearBlock(memblock*block)
{
    free(block->block_pointer);
    block->node_handler.clear();
}

template<typename T, int block_size>
typename MemoryDirector<T,block_size>::memblock * MemoryDirector<T,block_size>::newBlock()
{
    memblock * temp = new memblock;
    temp->block_num=pages.size();
    temp->block_pointer = malloc(type_size*block_size);
    temp->counter=0;
    temp->fragPg=0;
    temp->node_handler.resize(block_size);
    for(int i = 0; i<block_size;i++)
    {
        temp->node_handler[i].node_pointer=static_cast<T*>(temp->block_pointer+(i*type_size));
        temp->node_handler[i].free_status=true;
    }
    return temp;
}

template<typename T, int block_size>
void * MemoryDirector<T,block_size>::getFreeNode(memblock * block,int &index)
{
    void * answer = nullptr;
    for(int i = 0; i<block_size;i++)
    {
        if(block->node_handler[i].free_status)
        {
            answer = block->node_handler[i].node_pointer;
            index = i;
            block->node_handler[i].free_status=false;
            block->counter++;
            break;
        }
    }
    return answer;
}

template<typename T, int block_size>
void MemoryDirector<T,block_size>::freeOccNode(memblock *block, int index)
{
    if(index<block->node_handler.size())
    {
        block->node_handler[index].free_status=true;
        block->counter--;
        block->fragPg++;
    }
}

template<typename T, int block_size>
bool MemoryDirector<T,block_size>::blockIsFull(const memblock * block) const
{
    return ((block->counter)==(block_size-1));
}

template<typename T, int block_size>
bool MemoryDirector<T,block_size>::blockIsEmpty(const memblock * block) const
{
    return !(block->counter);
}

template<typename T, int block_size>
void * MemoryDirector<T,block_size>::getAddr(handle_t &handle)
{
    void * answer = nullptr;
    int block_index=0;
    typename QLinkedList<memblock*>::iterator iterator;
    for(iterator = pages.begin();iterator!=pages.end();iterator++,block_index++)
    {
        if(blockIsFull(*iterator))
        {
            continue;
        }
        else
        {
            break;
        }
    }
    if(iterator==pages.end())
    {
        pages.append(newBlock());
        iterator = (pages.end()--);
    }
    int index=0;
    answer = getFreeNode(*iterator,index);
    if(answer)
    {
        handleblock new_var;
        new_var.index_num=index;
        new_var.block_num=block_index;
        handle=handle_counter;
        handle_map.insert(handle,new_var);
        handle_counter++;
    }
    return answer;
}

template<typename T, int block_size>
bool MemoryDirector<T,block_size>::freeAddr(const handle_t &handle)
{
    typename QMap<handle_t,handleblock>::iterator addrFree = handle_map.find(handle);
    if(addrFree==handle_map.end())
    {
        return false;
    }
    int block_index = (addrFree.value()).block_num;
    int index = (addrFree.value()).index_num;
    typename QLinkedList<memblock*>::iterator cur_block = ((pages.begin())+block_index);
    freeOccNode(*cur_block,index);
    handle_map.erase(addrFree);
    if(blockIsEmpty(*cur_block))
    {
        clearBlock(*cur_block);
        delete *cur_block;
        (*cur_block)=nullptr;
        pages.erase(cur_block);
    }
    return true;
}

template<typename T, int block_size>
T* MemoryDirector<T,block_size>::getSpace(const handle_t & handle)
{
    typename QMap<handle_t,handleblock>::iterator addrFree = handle_map.find(handle);
    if(addrFree==handle_map.end())
    {
        return nullptr;
    }
    int block_index = (addrFree.value()).block_num;
    int index = (addrFree.value()).index_num;
    typename QLinkedList<memblock*>::iterator cur_block = ((pages.begin())+block_index);
    memblock * block = *cur_block;
    return block->node_handler.at(index).node_pointer;
}

template<typename T, int block_size>
void MemoryDirector<T,block_size>::Defragmentation()
{
    typename QLinkedList<memblock*>::iterator block_iterator = (pages.rbegin());
    while(block_iterator!=pages.rend())
    {
        memblock * block = *block_iterator;
        int step = block->node_handler.size()-1;
        while(step>=0)
        {
            if(!(block->node_handler.at(step).free_status))
            {
                 typename QLinkedList<memblock*>::iterator target_iterator = (pages.begin());
                memblock * target_block = (*target_iterator);
                if(target_block!=block)
                {
                        //!Continue a defragmentation!
                }
            }
        }
    }
}

#endif // MEMORYDIRECTOR_H
