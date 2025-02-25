package counter

import future.keywords

# Check whether a capability is the correct type for being a capability
is_counter_capability(capability) {
	capability.kind == "SealedObject"
	capability.sealing_type.compartment == "monotonic"
	capability.sealing_type.key == "CounterKey"
}

is_valid_counter_capability(capability) {
	is_counter_capability(capability)
	integer_from_hex_string(capability.contents, 0, 4) == 0
	integer_from_hex_string(capability.contents, 4, 4) == 0
}

check_counters { 
	some counterCapabilities
	counterCapabilities = [ c |
		c = input.compartments[_].imports[_] ;
		is_counter_capability(c)
	]
	every c in counterCapabilities {
		is_valid_counter_capability(c)
	}   
}

counters_are_valid = result {
	check_counters
	result = true
}
counters_are_valid = result {
	not check_counters
	result = false
}

counter_owners = [ owner |
	c = input.compartments[owner].imports[_] ;
	is_counter_capability(c)
]

