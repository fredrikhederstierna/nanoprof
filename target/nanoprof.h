#ifndef __NANOPROF_H__
#define __NANOPROF_H__

// functions injected when using -finstrument-functions

void __cyg_profile_func_enter(void *this_fn, void *call_site) __attribute__((used)) __attribute__((no_instrument_function));

void __cyg_profile_func_exit(void *this_fn, void *call_site) __attribute__((used)) __attribute__((no_instrument_function));

// start address should be aligned 4-byte
void nanoprof_init(void* start_addr, void* end_addr);

#endif
