#include <sys/sysctl.h>
#include <vm/vm_param.h>
#include <sys/vmmeter.h>

static int _get_total_free(uint64_t* mem_total, uint64_t* mem_free) {

    size_t  len;
    int     mib[2];
    long    total = sysconf(_SC_PHYS_PAGES);
    long    ps = sysconf(_SC_PAGESIZE);

    if (ps == -1 || total == -1) {
        return 0;
    }

#if defined (CTL_VM) && (defined(VM_TOTAL) || defined(VM_METER))
    {
        struct vmtotal vmi;
        mib[0] = CTL_VM;
#if defined (VM_TOTAL)
        mib[1] = VM_TOTAL;
#else
        mib[1] = VM_METER;
#endif
        len = sizeof(vmi);
        if (sysctl(mib, 2, &vmi, &len, 0, 0) == 0) {
            *mem_total = (uint64_t)total * (uint64_t)ps;
            *mem_free = (uint64_t)vmi.t_free * (uint64_t)ps;
            return 1;
        }
    }
#else
#error "not implemented"
#endif

    return 0;
}

