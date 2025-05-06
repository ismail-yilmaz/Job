#include "Job.h"

namespace Upp {
	
#define LLOG(x) //  RLOG("Worker #" << this << ": " << x)

thread_local JobWorker *JobWorker::ptr = nullptr;

JobWorker::JobWorker()
{
	cb  = Null;
	exc = nullptr;
	cancel = false;
	shutdown = false;
	if(!work.RunNice([=]{ JobWorker::ptr = this; Loop(); }))
		throw Exc("Couldn't create new job.");
	LLOG("Initialized.");
}

JobWorker::~JobWorker()
{
	lock.Enter();
	cancel = true;
	shutdown = true;
	lock.Leave();
	Wait();
	ready.Signal();
	LLOG("Shut down signal sent...");
	work.Wait();
	LLOG("Joined.");
}

bool JobWorker::Start(Event<>&& fn)
{
	Mutex::Lock __(lock);
	
	if(cancel || cb) {
		LLOG("Couldn't start working. Worker is busy.");
		return false;
	}

	cb = pick(fn);
	exc = nullptr;
	cancel = false;
	LLOG("Starting to work...");
	ready.Signal();
	return true;
}

void JobWorker::Loop()
{
	Mutex::Lock __(lock);
	for (;;) {
	    while (!cb && !shutdown) {
	        LLOG("Waiting for work");
	        ready.Wait(lock); // This MUST be called with the lock held
	    }

	    if(shutdown) {
	        LLOG("Shut down signal received. Shutting down...");
	        return;
	    }
	
		lock.Leave();
	    try {
			cb();
	    }
	    catch (...) {
	        LLOG("Exception raised");
			exc = std::current_exception();
		}
	    lock.Enter();
	    cb = Null;
	    done.Signal();
	}
}

void JobWorker::Wait()
{
	Mutex::Lock __(lock);
	while(cb) {
		LLOG("Waiting for the worker to finish its job...");
		done.Wait(lock);
	}
}
}
