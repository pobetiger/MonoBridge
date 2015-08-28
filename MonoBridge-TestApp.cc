
#include "MonoBridge.hpp"

#include <iostream>

bool test_SingleObject(MonoBridge::MonoBridge *bridge) {
    return true;
}
bool test_SeqSingleObject(MonoBridge::MonoBridge *bridge) {
    return true;
}
bool test_MultiObjectsNonConcurrent(MonoBridge::MonoBridge *bridge) {
    return true;
}
bool test_MultiObjectConcurrent(MonoBridge::MonoBridge *bridge) {
    return true;
}


int main(int argc, char **argv) {

    std::string path = "./";
    if (argc >= 2) {
        path = argv[1];
    }

    MonoBridge::MonoBridge *bridge = new MonoBridge::MonoBridge();

    std::cout << "Initializing application" << std::endl;
    bridge->Initialize();

    std::cout << "Launching application" << std::endl;
    bridge->Launch();

    std::cout << "Loading assemblies from path: " << path << std::endl;
    if (bridge->LoadAssemblyPath(path) == 0) {
        std::cout << "FAIL: no library can be loaded" << std::endl;
        return -1;
    }

    /* run the test */
    if (!test_SingleObject(bridge)) {
        std::cout << "FAIL: Single Object Test" << std::endl;
        return -1;
    }
    if (!test_SeqSingleObject(bridge)) {
        std::cout << "FAIL: Sequential Single Object Test" << std::endl;
        return -1;
    }
    if (!test_MultiObjectsNonConcurrent(bridge)) {
        std::cout << "FAIL: Multiple Object Non-Concurrent Test" << std::endl;
        return -1;
    }
    if (!test_MultiObjectConcurrent(bridge)) {
        std::cout << "FAIL: Multiple Object Concurrent Test" << std::endl;
        return -1;
    }

    std::cout << "All Tests Passed!" << std::endl;

    std::cout << "Stopping application" << std::endl;
    bridge->Stop();

    std::cout << "Cleaning up" << std::endl;
    bridge->Quit();

    return 0;
}


/* end of file */
