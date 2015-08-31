#include "MonoBridge.hpp"

#include <iostream>
#include <fstream>
#include <vector>

#include <mono/jit/jit.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/debug-helpers.h>

// this is internal?
//#include <mono/metadata/threads-types.h>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

namespace MonoBridge {

static bool isDll(std::string file);
static std::vector<char> readFile(std::string filename);

static std::vector<char> readFile(std::string filename) {
    std::ifstream myfile(filename.c_str(), std::ios::binary|std::ios::ate);
    std::ifstream::pos_type pos = myfile.tellg();

    std::vector<char> result(pos);
    myfile.seekg(0, std::ios::beg);
    myfile.read(&result[0], pos);

    return result;
}

static void print_methods(MonoClass *k) {
    std::cout << "Getting a listing of methods:" << std::endl;
    void *iter = 0;
    MonoMethod *m;
    while ((m = mono_class_get_methods(k, &iter))) {
        std::string theName(mono_method_get_name(m));
        MonoMethodSignature *sig = mono_method_signature(m);
        std::string theSig(mono_signature_get_desc(sig, false));

        std::cout << " --> Found Method: " << theName << "(" << theSig << ")" << std::endl;
    }
}


void MonoBridge::Initialize() {

    mono_config_parse(NULL); /* default mono path */

    /* root domain, holds corlib */
    mono_jit_init_version("MonoBridge", "v4.0.30319");

    // for soft debugger
    mono_thread_set_main(mono_thread_current());
    isLoaded = false;
}

bool MonoBridge::Launch() {

    if (isLoaded) {
        this->Stop();
    }

    MonoDomain *nextDomain =
        mono_domain_create_appdomain((char*)"MonoBridge-sub", NULL);

    if (!nextDomain) {
        return false;
    }

    // above 3.12
    // mono_thread_push_appdomain_ref(nextDomain);

    if (!mono_domain_set(nextDomain, false)) {
        return false;
    }

    domain = nextDomain;
    isLoaded = true;
    return true;
}



bool MonoBridge::LoadAssembly(std::string file) {
    std::cout << "Loading assembly: " << file;

    // read the file into memory
    std::vector<char> data = readFile(file);

    // open as image
    MonoImageOpenStatus status;
    auto image = mono_image_open_from_data_with_name(
        (char*) &data[0], data.size(),
        true /* copy data */,
        &status,
        false /* ref only */,
        file.c_str()
    );
    if (status != MONO_IMAGE_OK || image == 0) {
        std::cout << " **IMG FAIL**" << std::endl;
        return false;
    }

    // from image create assembly
    auto assembly = mono_assembly_load_from_full(
        image, file.c_str(), &status, false);
    if (status != MONO_IMAGE_OK || assembly == 0) {
        std::cout << " **ASM FAIL**" << std::endl;
        return false;
    }

    // auto image_pair = std::make_pair(file, image);
    // images.insert(image_pair);
    // assemblies.push_back(assembly);

    std::cout << " *LOADED*" << std::endl;

    // char *asm_name = mono_assembly_name_get_name(assembly);
    // std::cout << " * Assembly name: " << asm_name << std::endl;

    return true;
}

static bool isDll(std::string file) {
    boost::regex isdll(".*\\.dll$");
    return boost::regex_match(file, isdll);
}

bool MonoBridge::LoadAssemblyPath(std::string path) {
    namespace fs = boost::filesystem;
    bool b_success = false;

    fs::path searchPath = fs::canonical(path);

    if (!fs::exists(searchPath)) return b_success;

    if (fs::is_directory(searchPath)) {
        fs::directory_iterator eod;
        for (fs::directory_iterator sod(searchPath); sod!=eod; ++sod) {

            if(!fs::is_regular_file(sod->status()))
                break;

            if (isDll(sod->path().string())) {
                b_success = LoadAssembly(sod->path().string());
            }
        }
    } else if (fs::is_regular_file(searchPath)) {
        if (isDll(searchPath.string()))
            b_success = LoadAssembly(searchPath.string());
    }

    return b_success;
}

MonoObject *MonoBridge::Create(std::string ns, std::string name) {

    std::string full_name = name;
    std::cout << "looking for assembly : " << full_name << std::endl;

    MonoAssemblyName *aname = mono_assembly_name_new (full_name.c_str());
    MonoAssembly *assm = mono_assembly_loaded(aname);
    if (!assm) {
        mono_free(aname);
        std::cout << "asm/class not found" << std::endl;
        return 0;
    }

    auto img = mono_assembly_get_image(assm);
    MonoClass *k = mono_class_from_name(img, ns.c_str(), name.c_str());
    if (!k) {
        mono_free(img);
        mono_free(aname);
        // mono_free(assm);
        std::cout << "class not found" << std::endl;
        return 0;
    }
    mono_free(aname);

    auto obj = mono_object_new(domain, k);
    if (!obj) {
        mono_free(img);
        mono_free(aname);
        mono_free(k);
        std::cout << "unable to create class " << ns << "." << name << std::endl;
    }
    mono_runtime_object_init(obj);
    instances.push_back(obj);

    return obj;
}

MonoObject *MonoBridge::CreateEx(std::string file, std::string ns, std::string name) {
    return 0;
}

MonoObject *MonoBridge::Invoke(
    MonoObject *obj,
    std::string method_name,
    void **params) {

    // MonoMethod *method;
    MonoObject *result = 0;
    MonoObject *exc = 0;

    MonoClass *k = mono_object_get_class(obj);

    // print_methods(k);
    MonoMethodDesc *mdesc = mono_method_desc_new((":" + method_name).c_str(), false);
    if (!mdesc) {
        std::cout << "unable to find method desc: " << method_name << std::endl;
        return 0;
    }
    MonoMethod *method = mono_method_desc_search_in_class (mdesc, k);
    if (!method) {
        mono_free(mdesc);
        std::cout << "unable to find method: " << method_name << std::endl;
        return 0;
    }

    // std::cout << "*** Invoking " << method_name << std::endl;
    result = mono_runtime_invoke(method, obj, params, 0);
    // std::cout << "** Invoking " << method_name << " complete" << std::endl;
    if (exc) {
        std::cout << "Exception: " << std::endl;
    }

    mono_free(mdesc);

    return result;
}


bool MonoBridge::Stop() {

    instances.clear();
    // images.clear();


    // NOTE: do not try to close all the opened images
    //       that will cause the domain unload to crash because
    //       the images have already been closed (double free)

    /* TODO: finalize instances of objects */
    for (unsigned int i=0; i<instances.size(); ++i) {
    }

    /* unload the app domain */
    MonoDomain *old = mono_domain_get();

    /* make sure we don't unload the root domain */
    if (old && old != mono_get_root_domain()) {
        if (!mono_domain_set(mono_get_root_domain(), false)) {
            // error setting to root domain, quit here
            return false;
        }

        // std::cout << "Attempting to free current AppDomain" << std::endl;
        // mono 3.12 above
        // mono_thread_pop_appdomain_ref();
        mono_domain_unload(old);
        old = 0;
        isLoaded = false;
        // std::cout << "Domain unloaded" << std::endl;
    } else {
        std::cout << "Attempting to stop root domain, no can do" << std::endl;
        return false;
    }

    return true;
}


void MonoBridge::Quit() {
    /* bring the whole thing down */
    mono_jit_cleanup(mono_domain_get());
}

MonoDomain * MonoBridge::getDomain() {
    return domain;
}

}



/* end of file */
