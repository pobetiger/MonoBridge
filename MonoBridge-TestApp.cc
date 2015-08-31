
#include "MonoBridge.hpp"

#include <iostream>

bool test_SingleObject(MonoBridge::MonoBridge *bridge) {

    std::cout << ">> Testing a single library" << std::endl;

    MonoObject *pt_fileiolib = bridge->Create("MonoBridgeTest", "FileIOLib");
    if (!pt_fileiolib) {
        return false;
    }

    MonoString *port_name = mono_string_new(bridge->getDomain(), "2009");
    MonoString *port_desc = mono_string_new(bridge->getDomain(), "Use for some data writing");
    void *params[2] = {
        port_name,
        port_desc,
    };
    bridge->Invoke(pt_fileiolib, "Setup(string,string)", params);

    bridge->Invoke(pt_fileiolib, "Dispose", 0);

    return true;
}

bool test_SeqSingleObject(MonoBridge::MonoBridge *bridge) {
    std::cout << ">> Testing a single library multiple times sequentially" << std::endl;

    MonoObject *pt_fileiolib1 = bridge->Create("MonoBridgeTest", "FileIOLib");
    if (!pt_fileiolib1) {
        return false;
    }
    MonoString *port_name1 = mono_string_new(bridge->getDomain(), "2009");
    void *params[1] = {
        port_name1,
    };
    bridge->Invoke(pt_fileiolib1, "Setup", params);

    bridge->Invoke(pt_fileiolib1, "Dispose", 0);

    // create the second instance after we're done with the first

    MonoObject *pt_fileiolib2 = bridge->Create("MonoBridgeTest", "FileIOLib");
    if (!pt_fileiolib2) {
        return false;
    }
    MonoString *port_name2 = mono_string_new(bridge->getDomain(), "2009");
    params[0] = port_name2;

    bridge->Invoke(pt_fileiolib2, "Setup", params);


    bridge->Invoke(pt_fileiolib2, "Dispose", 0);


    return true;
}

bool test_MultiObjectsNonConcurrent(MonoBridge::MonoBridge *bridge) {
    std::cout << ">> Testing a multiple library sequentially" << std::endl;


    return true;
}

bool test_MultiObjectConcurrent(MonoBridge::MonoBridge *bridge) {
    std::cout << ">> Testing a multiple library concurrently" << std::endl;
    return true;
}

bool test_AtonOfOperations(MonoBridge::MonoBridge *bridge) {
    std::cout << ">> Testing a ton of operations" << std::endl;

    MonoObject *logger = bridge->Create("MonoBridgeTest", "Logger");
    MonoObject *iolib = bridge->Create("MonoBridgeTest", "FileIOLib");

    if (logger == 0 || iolib==0) {
        std::cout << "Some operational library is not valid" << std::endl;
        return false;
    }

    auto log = [=] (std::string s_msg) {
        // std::string s_msg = "foo";
        MonoString *o_msg = mono_string_new(bridge->getDomain(), s_msg.c_str());
        void *params[] = {
            o_msg,
        };
        bridge->Invoke(logger, "Log", params); // void, no return
    };

    auto send_io = [=] (std::string s_packet) {
        log("Sending IO: " + s_packet);
        MonoString *o_packet = mono_string_new(bridge->getDomain(), s_packet.c_str());
        void *params[] = {
            o_packet,
        };
        bridge->Invoke(iolib, "Write", params); // void, no return
        log("Sending IO complete");
    };


    log("*** Starting up a ton of test ***");
    log("doing some obligatory logging stuff");
    log("system should be doing setup");
    send_io("send startup");
    send_io("recv");
    send_io("wait ack");
    int cmd_length = 1000;
    for (int i=0; i<cmd_length; ++i) {
        log ("cycle #" + std::to_string(i));
        send_io("do some cmd");
        send_io("recv");
        send_io("wait ack");
        log ("cycle #" + std::to_string(i) +" complte");
    }
    log("task complete, should do some cleanup");
    send_io("system shutdown");
    send_io("cleanup");
    log("test done");
    return true;
}

static bool test_leaky(MonoBridge::MonoBridge *bridge) {
    std::cout << ">> Testing a ton of operations" << std::endl;
    std::vector< MonoObject* > manyObjects;

    std::cout << "Press key when ready to start" << std::endl;
    std::cin.ignore();

    MonoObject *nonsuch = bridge->Create("MonoBridgeTest", "FileIOlib");

    for (int i=0;i<10000; i++) {
        MonoObject *iolib = bridge->Create("MonoBridgeTest", "FileIOLib");
        manyObjects.push_back(iolib);
    }
    std::cout << "Press key when ready" << std::endl;
    std::cin.ignore();

    std::cout << "Press key when ready again" << std::endl;
    std::cin.ignore();

    return true;
}

int runTest(MonoBridge::MonoBridge *bridge) {
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
    if (!test_AtonOfOperations(bridge)) {
        std::cout << "FAIL: A ton of operations fail" << std::endl;
        return -1;
    }
    if (!test_leaky(bridge)) {
        std::cout << "FAIL: leaky test fail" << std::endl;
        return -1;
    }
    return 0;
}

int main(int argc, char **argv) {

    std::string path = "./";
    if (argc >= 2) {
        path = argv[1];
    }

    MonoBridge::MonoBridge *bridge = new MonoBridge::MonoBridge();

    std::cout << "Initializing application" << std::endl;
    bridge->Initialize();

    bool isRunning = true;

    do {
        std::string s_nextstep;
        std::cout << "Action? ";
        std::cin >> s_nextstep;

        if (s_nextstep == "q") {
            bridge->Stop();
            isRunning = false;
            break;
        } else if (s_nextstep == "l") {
            std::cout << "Launching application" << std::endl;
            bridge->Launch();

            std::cout << "Loading assemblies from path: " << path << std::endl;
            if (bridge->LoadAssemblyPath(path) == 0) {
                std::cout << "FAIL: no library can be loaded" << std::endl;
                // TODO: maybe let user specify path instead
                bridge->Stop();
                break;
            }
        } else if (s_nextstep == "r") {
            if (runTest(bridge) < 0) {
                std::cout << "Test failed, back to command prompt" << std::endl;
            } else {
                std::cout << "All Tests Passed!" << std::endl;
            }
        } else if (s_nextstep == "s") {
            std::cout << "Stopping bridge" << std::endl;
            bridge->Stop();
        } else if (s_nextstep == "k") {
            std::cout << "Stopping bridge" << std::endl;
            bridge->Stop();
            std::cout << "Cleaning up" << std::endl;
            bridge->Quit();
            free(bridge);
            bridge = 0;
        } else {
            std::cout << "unimplemented command" << std::endl;
        }

    } while (isRunning);

    if (bridge) {
        std::cout << "Cleaning up" << std::endl;
        bridge->Quit();
        free(bridge);
        bridge = 0;
    }

    return 0;
}


/* end of file */
