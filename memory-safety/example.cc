#include <compartment.h>
#include <debug.hh>
#include <unwind.h>
#include <priv/riscv.h>

using Debug = ConditionalDebug<true, "Memory safety">;

extern "C" ErrorRecoveryBehaviour
compartment_error_handler(ErrorState *frame, size_t mcause, size_t mtval)
{
	if (mcause == priv::MCAUSE_CHERI)
	{
		// An unexpected error -- log it and end the simulation with error.
		// Note: handle CZR differently as `get_register_value` will return a
		// nullptr which we cannot dereference.

		auto [exceptionCode, registerNumber] =
		  CHERI::extract_cheri_mtval(mtval);

		Debug::log(
		  "{} error at {} (return address: {}), with capability register "
		  "{}: {}",
		  exceptionCode,
		  frame->pcc,
		  frame->get_register_value<CHERI::RegisterNumber::CRA>(),
		  registerNumber,
		  registerNumber == CHERI::RegisterNumber::CZR
		    ? nullptr
		    : *frame->get_register_value(registerNumber));
		cleanup_unwind();
	}
	return ErrorRecoveryBehaviour::ForceUnwind;
}

/**
 * Helper that does an out-of-bounds access.  Marked as weak so that the
 * optimiser doesn't look inside it and can't elide the access.
 */
__attribute__((weak))
__noinline void bounds_error(char *buffer, size_t offset)
{
	buffer[offset] = '\0';
}

/**
 * Helper that tries an out-of-bounds access in an error handler, demonstrating
 * that it is recoverable.
 */
void spatial_safety_error(std::string_view kind, char *buffer, size_t offset)
{
	Debug::log("Trying {} buffer overflow:", kind);
	CHERIOT_DURING
		bounds_error(buffer, offset);
	CHERIOT_HANDLER
		Debug::log("Recovered from {} buffer overflow", kind);
	CHERIOT_END_HANDLER
}

/**
 * Thread entry point.
 */
__cheriot_compartment("example") void entry()
{
	char stack[] = "short buffer";
	static char global[] = "short buffer";

	// Try an our-of-bounds write to a stack allocation.
	spatial_safety_error("stack", stack, sizeof(stack));

	// Try an our-of-bounds write to a global.
	spatial_safety_error("global", global, sizeof(global));

	// Try an our-of-bounds write to a heap allocation.
	char *heap = new char[16];
	spatial_safety_error("heap", heap, 16);
	free(heap);

	// Try a use after free.
	Debug::log("Freed pointer (note invalid tag): {}", heap);
	CHERIOT_DURING
		heap[0] = 'H';
	CHERIOT_HANDLER
		Debug::log("Recovered from use after free");
	CHERIOT_END_HANDLER

	Debug::log("Done");
}
