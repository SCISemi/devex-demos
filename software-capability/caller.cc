// Copyright Microsoft and CHERIoT Contributors.
// SPDX-License-Identifier: MIT

#include "../include/example-error-handler.hh"
#include "monotonic_counter.hh"
#include <stdio.h>

// Declare and define a counter for this compartment to use.
DECLARE_AND_DEFINE_COUNTER(aCounter)

DECLARE_AND_DEFINE_COUNTER(aSecondCounter)

void __cheriot_compartment("caller") entry()
{
	// Get a pointer to the valid counter.
	auto validCounter  = STATIC_SEALED_VALUE(aCounter);
	auto validCounter2 = STATIC_SEALED_VALUE(aSecondCounter);
	// Create an unsealed value of the correct type
	MonotonicCounterState invalidCounterState;
	auto                  invalidCounter =
	  reinterpret_cast<MonotonicCounter>(&invalidCounterState);
	auto invalidSealedCounter =
	  reinterpret_cast<MonotonicCounter>(MALLOC_CAPABILITY);

	// Try the valid capability
	printf("Valid counter increment returned %lld\n",
	       monotonic_counter_increment(validCounter));
	printf("Valid counter increment returned %lld\n",
	       monotonic_counter_increment(validCounter));
	printf("Valid counter increment of second counter returned %lld\n",
	       monotonic_counter_increment(validCounter2));
	// Try the invalid ones
	printf("Invalid counter increment returned %lld\n",
	       monotonic_counter_increment(invalidCounter));
	printf("Invalid counter increment returned %lld\n",
	       monotonic_counter_increment(invalidSealedCounter));
	// Try manipulating the counter directly
	CHERIOT_DURING
	auto underlyingCounter =
	  reinterpret_cast<MonotonicCounterState *>(validCounter);
	(*underlyingCounter)++;
	CHERIOT_HANDLER
	printf("Failed to directly modify counter\n");
	CHERIOT_END_HANDLER
}
