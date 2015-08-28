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

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

namespace MonoBridge {

static bool isDll(std::string file);
static std::vector<char> readFile(std::string filename);

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

static std::vector<char> readFile(std::string filename) {
    std::ifstream myfile(filename.c_str(), std::ios::binary|std::ios::ate);
    std::ifstream::pos_type pos = myfile.tellg();

    std::vector<char> result(pos);
    myfile.seekg(0, std::ios::beg);
    myfile.read(&result[0], pos);

    return result;
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

    auto image_pair = std::make_pair(file, image);
    images.insert(image_pair);

    std::cout << " *LOADED*" << std::endl;

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

MonoObject *MonoBridge::CreateObject(std::string file, std::string ns, std::string name) {
    return 0;
}


bool MonoBridge::Stop() {

    /* close all asm images in use */
    for (auto &image_pair : images) {
        mono_image_close(image_pair.second);
    }

    /* TODO: clear all instances of objects */

    /* unload the app domain */
    MonoDomain *old = mono_domain_get();

    /* make sure we don't unload the root domain */
    if (old && old != mono_get_root_domain()) {
        if (!mono_domain_set(mono_get_root_domain(), false)) {
            // error setting to root domain, quit here
            return false;
        }
        std::cout << "Attempting to free current AppDomain" << std::endl;

        mono_domain_unload(old);
        // run gc when we unload something
        std::cout << "Garbage Collecting..." << std::endl;

        mono_gc_collect(mono_gc_max_generation());
    }

    return true;
}


void MonoBridge::Quit() {
    /* bring the whole thing down */
    mono_jit_cleanup(mono_domain_get());
}

}



/* end of file */
