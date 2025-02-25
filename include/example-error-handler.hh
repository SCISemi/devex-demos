#include <compartment.h>
#include <debug.hh>
#include <unwind.h>
#include <priv/riscv.h>

extern "C" ErrorRecoveryBehaviour
compartment_error_handler(ErrorState *frame, size_t mcause, size_t mtval)
{
	using Debug = ConditionalDebug<true, "Error handler">;
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
