#include "MonoBridge.hpp"

#include <mono/jit/jit.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/environment.h>

namespace MonoBridge {

void MonoBridge::Initialize() {

    mono_config_parse(NULL); /* default mono path */

    /* root domain, holds corlib */
    mono_jit_init_version("MonoBridge", "v4.0.30319");

    // for soft debugger
    mono_thread_set_main(mono_thread_current());
}

bool MonoBridge::Launch() {
    MonoDomain *nextDomain = mono_domain_create_appdomain((char*)"MonoBridge-sub", NULL);
    if (!nextDomain) {
        return false;
    }

    // mono_thread_push_appdomain_ref(nextDomain);

    if (!mono_domain_set(nextDomain, false)) {
        return false;
    }

    domain = nextDomain;
    return true;
}

bool MonoBridge::LoadAssembly(std::string file) {

}

MonoObject *MonoBridge::CreateObject(std::string file, std::string ns, std::string name) {

}


bool MonoBridge::Stop() {

    /* TODO: close all asm images in use */

    /* TODO: clear all instances of objects */

    /* unload the app domain */

    MonoDomain *old = mono_domain_get();

    /* make sure we don't unload the root domain */
    if (old && old != mono_get_root_domain()) {
        if (!mono_domain_set(mono_get_root_domain(), false)) {
            // error setting to root domain, quit here
            return false;
        }

        mono_domain_unload(old);
        // run gc when we unload something
        mono_gc_collect(mono_gc_max_generation());
    }
}


void MonoBridge::Quit() {
    /* bring the whole thing down */
    mono_jit_cleanup(mono_domain_get());
}

}



/* end of file */
