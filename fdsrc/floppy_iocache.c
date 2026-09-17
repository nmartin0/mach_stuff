
#include <mach/mach_types.h>
#define CACHE_LINE_SIZE 32 /* Bytes */

// @@@ implicit declaration of flush_dcache, invalidate_dcache

void
invalidate_cache_for_io(vm_offset_t area, unsigned count, boolean_t phys)
{ 
        vm_offset_t aligned_start, aligned_end, end;
   
        /* For unaligned reads we need to flush any
         * unaligned cache lines. We invalidate the
         * rest as this is faster
         */

        aligned_start = area & ~(CACHE_LINE_SIZE-1);
        if (aligned_start != area)
                flush_dcache(aligned_start, CACHE_LINE_SIZE, phys);

        end = area + count;
        aligned_end = (end & ~(CACHE_LINE_SIZE-1));
        if (aligned_end != end)
                flush_dcache(aligned_end, CACHE_LINE_SIZE, phys);

        invalidate_dcache(area, count, phys);
}

