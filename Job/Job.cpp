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
	if(!work.RunNice([=]{ ptr = this; Loop(); }))
		throw Exc("Couldn't create new job.");
	LLOG("Initialized.");
}

JobWorker::~JobWorker()
{
	lock.Enter();
	exc = nullptr;
	ptr = nullptr;
	cancel = true;
	shutdown = true;
	lock.Leave();
	ready.Signal();
	Wait();
	LLOG("Shut down signal sent...");
	work.Wait();
	LLOG("Joined.");
}

bool JobWorker::Start(Event<>&& fn)
{
	lock.Enter();
	if(cancel || cb || shutdown) {
		lock.Leave();
		LLOG("Couldn't start working. Worker is busy.");
		return false;
	}

	cb = pick(fn);
	exc = nullptr;
	cancel = false;
	LLOG("Starting to work...");
	lock.Leave();
	ready.Signal();
	return true;
}

void JobWorker::Loop()
{
	for(;;) {
		lock.Enter();
	    while(!cb && !shutdown) {
	        LLOG("Waiting for work");
	        ready.Wait(lock);
	    }
		
		lock.Leave();
		
	    if(shutdown) {
	        LLOG("Shut down signal received. Shutting down...");
	        return;
	    }
	    
	    if(cancel) {
	        LLOG("Job canceled before start");
	        return;
	    }
	    
	    try {
			cb();
	    }
	    catch(...) {
	        LLOG("Exception raised");
	        Mutex::Lock __(lock);
			exc = std::current_exception();
		}

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