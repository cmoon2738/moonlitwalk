#include <moonlitwalk/os.h>

#include <dlfcn.h>

void *amw_load_dll(const char *libname) 
{
    void *handle = dlopen(libname, RTLD_NOW | RTLD_LOCAL);
    if (!handle)
        amw_log_error("dlopen '%s' failes: %s", libname, dlerror());
    return handle;
}

void *amw_get_proc_address(void *handle, const char *procname)
{
    const char *error;
    void *addr = dlsym((void *)handle, procname);
    if ((error = dlerror()) != NULL) {
        amw_log_error("dlsym '%s' failed: %s", procname, error);
    }
    return addr;
}

void amw_close_dll(void *handle)
{
    dlclose((void *)handle);
}
