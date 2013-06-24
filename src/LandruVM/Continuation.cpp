
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "LandruVM/Continuation.h"

#include <map>
#include <string>

namespace Landru
{
    
struct QueueRecord
{
    std::string  name;
    ContinuationList* queue;
};

std::vector<QueueRecord>& queues()
{
    static std::vector<QueueRecord>* _queues = new std::vector<QueueRecord>();
    return *_queues;		
}

unsigned int Queue(const char* type, const char* eventName)
{
	char* temp = (char*) alloca(strlen(type) + strlen(eventName) + 1);
	strcpy(temp, type);
	strcat(temp, eventName);

    int index = 0;
    std::vector<QueueRecord>& q = queues();
    for (std::vector<QueueRecord>::const_iterator i = q.begin(); i != q.end(); ++i, ++index) {
        if (!strcmp((*i).name.c_str(), temp))
            return index;
    }
    
    return -1;
}
        
ContinuationList* Queue(unsigned int index)
{
    return queues()[index].queue;
}

void QueueRegister(ContinuationList* oib, const char* type, const char* eventName)
{
	char* temp = (char*) alloca(strlen(type) + strlen(eventName) + 1);
	strcpy(temp, type);
	strcat(temp, eventName);
    std::vector<QueueRecord>& q = queues();
	q.push_back(QueueRecord());
    q.back().name.assign(temp);
    q.back().queue = oib;
}
    
void QueueDeregister(ContinuationList* cl)
{
    // QueueRegister allows duplicate registrations, this removes every instance of cl.
    std::vector<QueueRecord>& q = queues();
    std::vector<QueueRecord>::iterator i = q.begin();
    while (i != q.end()) {
        if ((*i).queue == cl) {
            q.erase(i);
            i = q.begin();
        }
        else
            ++i;
    }
}

void UpdateQueues(Engine* engine, float elapsedTime)
{
    std::vector<QueueRecord>& q = queues();
	for (std::vector<QueueRecord>::iterator i = q.begin(); i != q.end(); ++i)
		(*i).queue->update(engine, elapsedTime);
}

void ClearQueues(VarObjPtr* f)
{
    std::vector<QueueRecord>& q = queues();
	for (std::vector<QueueRecord>::iterator i = q.begin(); i != q.end(); ++i)
		(*i).queue->clearContinuation(f);
}
	
    
void Continuation::deallocate()
{
    for (auto i = _children.begin(); i != _children.end(); ++i)
        (*i)->_continuationList->clearContinuation(*i);
    
    if (_f != 0 && _varPool != 0) {
        _varPool->releaseStrongRef(_f);
        _f = 0;
    }
}
    
	
} // Landru
