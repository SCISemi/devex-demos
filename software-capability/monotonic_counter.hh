// Copyright Contributors.
// SPDX-License-Identifier: MIT

#include <atomic>
#include <compartment.h>

/**
 * Type that will be exposed to untrusted callers as a sealed type.
 */
using MonotonicCounterState = std::atomic<int64_t>;

/**
 * Macro for declaring a new counter.
 */
#define DECLARE_AND_DEFINE_COUNTER(name)                   \
	DECLARE_AND_DEFINE_STATIC_SEALED_VALUE(                  \
	  MonotonicCounterState, monotonic, CounterKey, name, 0)

/**
 * Type-safe sealed pointer to a monotonic counter.
 */
typedef MonotonicCounterState
  *__sealed_capability MonotonicCounter;

/**
 * Increments a monotonic counter and returns the new value.
 *
 * Returns a negative value for errors.
 */
int64_t __cheriot_compartment("monotonic")
  monotonic_counter_increment(
    MonotonicCounter allocatorCapability);
