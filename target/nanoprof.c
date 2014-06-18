
/**
 * Minimalistic tracing and profiling utility
 * for bare metal embedded systems using GCC.
 *
 * by Fredrik Hederstierna 2014.
 *
 * Info on function instrumentation using GCC can be read at:
 *   http://balau82.wordpress.com/2010/10/06/trace-and-profile-function-calls-with-gcc/
 *   http://michael.hinespot.com/tutorials/gcc_trace_functions
 *
 * This source file is the public domain.
 */

/*
 from https://gcc.gnu.org/onlinedocs/gcc-4.9.0/gcc/Code-Gen-Options.html:

-finstrument-functions
Generate instrumentation calls for entry and exit to functions. Just after function entry and just before function exit, the following profiling functions are called with the address of the current function and its call site. (On some platforms, __builtin_return_address does not work beyond the current function, so the call site information may not be available to the profiling functions otherwise.)
          void __cyg_profile_func_enter (void *this_fn,
                                         void *call_site);
          void __cyg_profile_func_exit  (void *this_fn,
                                         void *call_site);
The first argument is the address of the start of the current function, which may be looked up exactly in the symbol table.

This instrumentation is also done for functions expanded inline in other functions. The profiling calls indicate where, conceptually, the inline function is entered and exited. This means that addressable versions of such functions must be available. If all your uses of a function are expanded inline, this may mean an additional expansion of code size. If you use ‘extern inline’ in your C code, an addressable version of such functions must be provided. (This is normally the case anyway, but if you get lucky and the optimizer always expands the functions inline, you might have gotten away without providing static copies.)

A function may be given the attribute no_instrument_function, in which case this instrumentation is not done. This can be used, for example, for the profiling functions listed above, high-priority interrupt routines, and any functions from which the profiling functions cannot safely be called (perhaps signal handlers, if the profiling routines generate output or allocate memory). 

-finstrument-functions-exclude-file-list=file,file,...
Set the list of functions that are excluded from instrumentation (see the description of -finstrument-functions). If the file that contains a function definition matches with one of file, then that function is not instrumented. The match is done on substrings: if the file parameter is a substring of the file name, it is considered to be a match.
For example:

          -finstrument-functions-exclude-file-list=/bits/stl,include/sys
excludes any inline function defined in files whose pathnames contain /bits/stl or include/sys.

If, for some reason, you want to include letter ',' in one of sym, write '\,'. For example, -finstrument-functions-exclude-file-list='\,\,tmp' (note the single quote surrounding the option). 

-finstrument-functions-exclude-function-list=sym,sym,...
This is similar to -finstrument-functions-exclude-file-list, but this option sets the list of function names to be excluded from instrumentation. The function name to be matched is its user-visible name, such as vector<int> blah(const vector<int> &), not the internal mangled name (e.g., _Z4blahRSt6vectorIiSaIiEE). The match is done on substrings: if the sym parameter is a substring of the function name, it is considered to be a match. For C99 and C++ extended identifiers, the function name must be given in UTF-8, not using universal character names. 
 */


//--------------------------------------------------

#include <stdint.h>

#include <nanoprof.h>
#include <nanoprof_clock.h>
#include <nanoprof_irq.h>

//--------------------------------------------------

#define TRACE_META_ENTER_BIT        (1 << 0)
#define TRACE_META_EXIT_BIT         (1 << 1)
#define TRACE_META_IRQ_ENABLED_BIT  (1 << 2)
#define TRACE_META_IRQ_CONTEXT_BIT  (1 << 3)

//--------------------------------------------------
struct nanoprof_trace
{
  uint32_t meta;       // enter/exit  irq on/off  type  mask
  uint64_t timestamp;  // often us
  uint32_t pc;
  uint32_t lr;
};

//-------------------------------------------------

static volatile struct nanoprof_trace *__trace_buffer_start = NULL;
static volatile struct nanoprof_trace *__trace_buffer_head  = NULL;
static volatile struct nanoprof_trace *__trace_buffer_end   = NULL;

//--------------------------------------------------
// move trace pointer forward
static volatile struct nanoprof_trace* __trace_next(void)
{
  uint32_t status = NANOPROF_IRQ_OFF;

  volatile struct nanoprof_trace *head = __trace_buffer_head;
  if (head == __trace_buffer_end) {
    // reset to start
    __trace_buffer_head = __trace_buffer_start;
  }
  else {
    // advance
    __trace_buffer_head++;
  }

  NANOPROF_IRQ_RESTORE(status);

  return head;
}

//--------------------------------------------------
void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
  if (trace_buffer_head) {

    // move trace pointer forward
    volatile struct nanoprof_trace* head = __trace_next();

    // get timestamp
    uint64_t ct = nanoprof_clock_get_timestamp();

    uint32_t meta = TRACE_META_ENTER_BIT;
    if (NANOPROF_IRQ_IS_ENABLED) {
      meta |= TRACE_META_IRQ_ENABLED_BIT;
    }
    if (NANOPROF_IRQ_CONTEXT) {
      meta |= TRACE_META_IRQ_CONTEXT_BIT;
    }
    head->meta      = meta;
    head->timestamp = ct;
    head->pc        = this_fn;
    head->lr        = call_site;
  }
}

//--------------------------------------------------
void __cyg_profile_func_exit(void *this_fn, void *call_site)
{
  if (trace_buffer_head) {

    // move trace pointer forward
    volatile struct nanoprof_trace* head = __trace_next();

    // get timestamp
    uint64_t ct = nanoprof_clock_get_timestamp();

    uint32_t meta = TRACE_META_EXIT_BIT;
    if (NANOPROF_IRQ_IS_ENABLED) {
      meta |= TRACE_META_IRQ_ENABLED_BIT;
    }
    if (NANOPROF_IRQ_CONTEXT) {
      meta |= TRACE_META_IRQ_CONTEXT_BIT;
    }
    head->meta      = meta;
    head->timestamp = ct;
    head->pc        = this_fn;
    head->lr        = call_site;
  }
}

#if 0
//------------------------------------
/*
  To resolve the addresses at run time you could also use the dladdr() function:
*/
#include <stdio.h>
#define __USE_GNU
#include <dlfcn.h>
void __cyg_profile_func_enter (void *this_fn, void *call_site)
{
  Dl_info info;
  if (dladdr(this_fn, &info) != 0) {
    printf("%s", info.dli_sname);
  }
}
#endif

//------------------------------------
void nanoprof_init(void* start_addr, void* end_addr)
{
  // check last position modulo trace size
  void *endpos = (end_addr / sizeof(struct nanoprof_trace)) * sizeof(struct nanoprof_trace);

  nanoprof_clock_init();
  
  __trace_buffer_end   = endpos;
  __trace_buffer_head  = start_addr;
  __trace_buffer_start = start_addr;
}
