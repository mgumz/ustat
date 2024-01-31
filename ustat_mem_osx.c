#include <mach/mach.h>

static int _get_total_free(uint64_t* mem_total, uint64_t* mem_free) {

    size_t len;
    int mib[2];
    long ps = sysconf(_SC_PAGESIZE);

    struct vm_statistics64 vm;
    mach_port_t port = mach_host_self();
    ;
    mach_msg_type_number_t count = sizeof(vm) / sizeof(natural_t);

    if (host_statistics64(port, HOST_VM_INFO64, (host_info_t)&vm, &count) !=
        KERN_SUCCESS) {
        return 0;
    }

    *mem_total = (uint64_t)vm.free_count + (uint64_t)vm.active_count +
                 (uint64_t)vm.inactive_count + (uint64_t)vm.wire_count;
    *mem_total *= (uint64_t)ps;

    *mem_free = (uint64_t)vm.free_count * (uint64_t)ps;

    return 1;
}
