#include "module_info.h"
#include <link.h>

namespace moduleinfo
{
    static int callback(struct dl_phdr_info* info, size_t, void* data)
    {
        uintptr_t base = *(uintptr_t*)data;

        if (info->dlpi_addr == base)
        {
            size_t size = 0;
            for (int i = 0; i < info->dlpi_phnum; i++)
            {
                size += info->dlpi_phdr[i].p_memsz;
            }
            *(size_t*)data = size;
            return 1;
        }
        return 0;
    }

    size_t get_module_size(uintptr_t base)
    {
        size_t size = base;
        dl_iterate_phdr(callback, &size);
        return size == base ? 0 : size;
    }
}
