// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/edk/system/mutex.h"

#include <stdlib.h>

#include <thread>

#include "mojo/edk/system/test/sleep.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace mojo {
namespace system {
namespace {

// Sleeps for a "very small" amount of time.
void EpsilonRandomSleep() {
  test::SleepMilliseconds(static_cast<unsigned>(rand()) % 20u);
}

// Basic test to make sure that Lock()/Unlock()/TryLock() don't crash ----------

TEST(MutexTest, Basic) {
  Mutex mutex;

  int thread_acquired = 0;
  auto thread = std::thread([&mutex, &thread_acquired]() {
    for (int i = 0; i < 10; i++) {
      mutex.Lock();
      mutex.AssertHeld();
      thread_acquired++;
      mutex.Unlock();
    }
    for (int i = 0; i < 10; i++) {
      mutex.Lock();
      mutex.AssertHeld();
      thread_acquired++;
      EpsilonRandomSleep();
      mutex.Unlock();
    }
    for (int i = 0; i < 10; i++) {
      if (mutex.TryLock()) {
        mutex.AssertHeld();
        thread_acquired++;
        EpsilonRandomSleep();
        mutex.Unlock();
      }
    }
  });

  int acquired = 0;
  for (int i = 0; i < 5; i++) {
    mutex.Lock();
    mutex.AssertHeld();
    acquired++;
    mutex.Unlock();
  }
  for (int i = 0; i < 10; i++) {
    mutex.Lock();
    mutex.AssertHeld();
    acquired++;
    EpsilonRandomSleep();
    mutex.Unlock();
  }
  for (int i = 0; i < 10; i++) {
    if (mutex.TryLock()) {
      mutex.AssertHeld();
      acquired++;
      EpsilonRandomSleep();
      mutex.Unlock();
    }
  }
  for (int i = 0; i < 5; i++) {
    mutex.Lock();
    mutex.AssertHeld();
    acquired++;
    EpsilonRandomSleep();
    mutex.Unlock();
  }

  thread.join();

  EXPECT_GE(acquired, 20);
  EXPECT_GE(thread_acquired, 20);
}

TEST(MutexTest, AssertHeld) {
  Mutex mutex;

#if defined(NDEBUG) && !defined(DCHECK_ALWAYS_ON)
  // For non-Debug builds, |AssertHeld()| should do nothing.
  mutex.AssertHeld();
#else
  EXPECT_DEATH_IF_SUPPORTED({ mutex.AssertHeld(); }, "Check failed");
#endif  // defined(NDEBUG) && !defined(DCHECK_ALWAYS_ON)

  // TODO(vtl): Should also test the case when the mutex is held by another
  // thread, though this is more annoying since it requires synchronization.
}

// Test that TryLock() works as expected ---------------------------------------

TEST(MutexTest, TryLock) MOJO_NO_THREAD_SAFETY_ANALYSIS {
  Mutex mutex;

  ASSERT_TRUE(mutex.TryLock());
  // We now have the mutex....

  {
    // This thread will not be able to get the mutex.
    auto thread = std::thread([&mutex]() { EXPECT_FALSE(mutex.TryLock()); });
    thread.join();
  }

  mutex.Unlock();
  // And now we don't.

  {
    // This thread will be able to get it (and then release it).
    auto thread = std::thread([&mutex]() {
      EXPECT_TRUE(mutex.TryLock());
      mutex.AssertHeld();
      mutex.Unlock();
    });
    thread.join();

    // And we can take it again.
    ASSERT_TRUE(mutex.TryLock());
  }

  mutex.Unlock();
}

// Tests that mutexes actually exclude -----------------------------------------

// We'll call this from both the main thread and secondary threads.
void DoStuffWithMutex(Mutex* mutex, int* value) {
  for (int i = 0; i < 40; i++) {
    mutex->Lock();
    int v = *value;
    EpsilonRandomSleep();
    *value = v + 1;
    mutex->Unlock();
  }
}

std::thread MakeMutexTestThread(Mutex* mutex, int* value) {
  return std::thread([mutex, value]() { DoStuffWithMutex(mutex, value); });
}

TEST(MutexTest, MutexTwoThreads) {
  Mutex mutex;
  int value = 0;

  std::thread thread = MakeMutexTestThread(&mutex, &value);

  DoStuffWithMutex(&mutex, &value);

  thread.join();

  EXPECT_EQ(2 * 40, value);
}

TEST(MutexTest, MutexFourThreads) {
  Mutex mutex;
  int value = 0;

  std::thread thread1 = MakeMutexTestThread(&mutex, &value);
  std::thread thread2 = MakeMutexTestThread(&mutex, &value);
  std::thread thread3 = MakeMutexTestThread(&mutex, &value);

  DoStuffWithMutex(&mutex, &value);

  thread1.join();
  thread2.join();
  thread3.join();

  EXPECT_EQ(4 * 40, value);
}

// MutexLocker -----------------------------------------------------------------

TEST(MutexTest, MutexLocker) {
  Mutex mutex;

  {
    MutexLocker locker(&mutex);
    mutex.AssertHeld();
  }

  // The destruction of |locker| should unlock |mutex|.
  ASSERT_TRUE(mutex.TryLock());
  mutex.AssertHeld();
  mutex.Unlock();
}

}  // namespace
}  // namespace system
}  // namespace mojo
