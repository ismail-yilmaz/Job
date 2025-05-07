#include <Core/Core.h>
#include <Job/Job.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_FILE|LOG_COUT);
	
	auto Test = [](const String& name, Function<void()> fn) {
		String txt = "---" + name + ": ";
		try {
			fn();
			txt << "PASSED";
		}
		catch(...) {
			txt << "FAILED";
		}
		LOG(txt);
	};
	
	Test("Basic execution", [] {
		Job<int> job;
		job.Do([]{ return 42; });
		ASSERT(~job == 42);
	});
	
	Test("Void jobs", [] {
		Job<void> job;
		bool executed = false;
		job.Do([&]{ executed = true; });
		~job;
		ASSERT(executed);
	});
	
	Test("Exception handling", [] {
		Job<void> job;
		job.Do([]{ throw Exc("test"); });
		try { ~job; ASSERT(false); }
		catch(const Exc& e) { ASSERT(e == "test"); }
	});
	
	Test("Cancellation", [] {
		Job<void> job;
		bool canceled = false;
		job.Do([&]{ while(!JobWorker::IsCanceled()) Sleep(1); canceled = true; });
		Sleep(100);
		job.Cancel();
		ASSERT(canceled);
	});
	
	Test("Automatic scope cancellation", [] {
		bool canceled = false;
		{
			Job<void> job;
			job.Do([&]{ while(!JobWorker::IsCanceled()) Sleep(1); canceled = true; });
			Sleep(100);
		}
		ASSERT(canceled);
	});
	
	Test("Move semantics", [] {
		Job<int> job1;
		job1.Do([]{ return 123; });
		
		Job<int> job2 = pick(job1);
		ASSERT(~job2 == 123);
		ASSERT(job1.IsFinished());
	});

	Test("Cancellation during execution", [] {
		std::atomic<int> progress(0);
		Job<void> job;
		job.Do([&]{
			for(int i = 0; i < 10; i++) {
				if(JobWorker::IsCanceled()) break;
				progress = i + 1;
				Sleep(100);
			}
		});
		Sleep(250);
		job.Cancel();
		ASSERT(progress > 0 && progress < 10);
	});

	Test("Immediate cancellation", [] {
		Job<void> job;
		job.Cancel();
		job.Do([]{ ASSERT(false); }); // Should never execute
		ASSERT(job.IsFinished());
	});
	
	CheckLogEtalon();
}