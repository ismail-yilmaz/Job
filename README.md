# Job Package for U++

The `Job` template class provides a lightweight, scope-bound mechanism to run asynchronous tasks on a **dedicated single worker thread**. It is designed with RAII principles to ensure safety and ease of use, offering features similar to `std::future` / `std::promise`—including support for return values, `void` specialization, and exception propagation.

---

## Overview

`Job` represents a **hybrid model** between a *dedicated thread* and a *worker thread*. Each `Job` owns its thread and reuses it to execute tasks serially. This makes it ideal for scenarios requiring:

* Non-blocking task execution
* Reliable completion and result collection
* Simple cancellation and cleanup
* A fallback to synchronous behavior if needed

While `Job` is general-purpose, it is particularly well-suited for applications and libraries that need **occasional asynchronous behavior**, such as file I/O, network operations, or computation-heavy tasks that do not benefit from aggressive parallelism.

---

## Key Features

* **RAII-based lifecycle:** `Job` instances are scope-bound. When they go out of scope, their associated worker is safely stopped.
* **Thread reuse:** Threads are reused for each job run, avoiding creation overhead for each task.
* **Safe result retrieval:** Results (including `void`) can be retrieved synchronously through `Get()`, with proper exception forwarding.
* **Exception propagation:** Any exception thrown in the worker thread is caught and rethrown when `Get()` is called.
* **No thread pool required:** Unlike `CoWork`, `Job` does not use a shared worker pool.
* **Cancellation-aware:** The job function can check `Job::IsCanceled()` to terminate early if needed.

---

## Example Usage

```cpp
Job<int> job;
job.Do([] { return 42; });
int result = job.Get(); // Automatically waits for completion and retrieves result

```

---

## When to Use

Use `Job` when:

* You need a simple, reusable asynchronous execution mechanism
* Your tasks are long-running but infrequent
* You prefer not to manage a global thread pool
* You want reliable exception handling across thread boundaries

---

## Limitations

* Not intended for launching many concurrent jobs (use `CoWork` instead)
* Each `Job` uses a dedicated thread, which may be overkill for very short tasks

---

For more advanced control or performance-critical scenarios, refer to Ultimate++'s `CoWork` or traditional threading constructs like `Thread`.
