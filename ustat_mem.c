#include "ustat.h"
#include "djb/fmt.h"
#include <unistd.h>
#include <stdint.h>
#include <sys/param.h>

static uint64_t _mem_total = 0;
static uint64_t _mem_free = 0;

// usefull:
//
// http://nadeausoftware.com/articles/2012/09/c_c_tip_how_get_physical_memory_size_system

static int _get_total_free(uint64_t* mem_total, uint64_t* mem_free);


int mem_init(struct ustat_module* m, const char* s, size_t l) {

    if (_mem_total > 0 && _mem_free > 0) {
        return 1;
    }

    _get_total_free(&_mem_total, &_mem_free);

    m->ready = 1;
    return 1;
}


#if defined(BSD)

#if defined(__APPLE__)

#include <mach/mach.h>

static int _get_total_free(uint64_t* mem_total, uint64_t* mem_free) {

    size_t  len;
    int     mib[2];
    long    ps = sysconf(_SC_PAGESIZE);

    struct vm_statistics64 vm;
    mach_port_t            port = mach_host_self();;
    mach_msg_type_number_t count = sizeof(vm) / sizeof(natural_t);

    if (host_statistics64(port, HOST_VM_INFO64, (host_info_t)&vm, &count) != KERN_SUCCESS) {
        return 0;
    }

    *mem_total = (uint64_t)vm.free_count
               + (uint64_t)vm.active_count
               + (uint64_t)vm.inactive_count
               + (uint64_t)vm.wire_count;
    *mem_total *= (uint64_t)ps;

    *mem_free = (uint64_t)vm.free_count * (uint64_t)ps;

    return 1;
}

#else // *BSD

#include <sys/sysctl.h>
#include <sys/vmmeter.h>

static int _get_total_free(uint64_t* mem_total, uint64_t* mem_free) {

    size_t  len;
    int     mib[2];
    long    total = 0;
    long    ps = sysconf(_SC_PAGESIZE);

    if (ps == -1) {
        return 0;
    }

#if defined (CTL_HW) && (defined(HW_MEMSIZE) || defined(HW_PHYSMEM64))
    {
        int64_t t;
        mib[0] = CTL_HW;
#if defined (HW_MEMSIZE)
        mib[1] = HW_MEMSIZE;
#elif defined (HW_PHYSMEM64)
        mib[1] = HW_PHYSMEM64;
#endif
        len = sizeof(t);
        if (sysctl(mib, 2, &t, &len, 0, 0) == 0) {
            total = t;
        }
    }
#endif // CTL_HW


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
#endif

    *mem_total = (uint64_t)total * (uint64_t)ps;

    return 0;
}

#endif // *BSD

#else  // LINUX (or other)

#include <sys/sysinfo.h>

static int _get_total_free(uint64_t* mem_total, uint64_t* mem_free) {

    struct sysinfo si;
    if (sysinfo(&si) == -1) {
        return 0;
    }

    *mem_total = (uint64_t)si.totalram * si.mem_unit;
    *mem_free = (uint64_t)si.freeram * si.mem_unit;

    return 1;
}
#endif



static int mem_print(int fd, uint64_t val) {
    char buf[32];
    int n = fmt_8longlong(buf, val);
    write(fd, buf, n);
    return 1;
}

int mem_total_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return mem_print(fd, _mem_total);
}

int mem_free_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return mem_print(fd, _mem_free);
}

int mem_total_human_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return print_8longlong_human(fd, 1, _mem_total);
}

int mem_free_human_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return print_8longlong_human(fd, 1, _mem_free);
}

int mem_ratio_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return print_double(fd, (double)_mem_free / (double)_mem_total, 2);
}

