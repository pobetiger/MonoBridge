#ifndef __mono_bridge_hpp_
#define __mono_bridge_hpp_

#include <mono/jit/jit.h>
#include <mono/metadata/object.h>

#include <map>
#include <vector>
#include <string>

namespace MonoBridge {

class MonoBridge {
public:
    /* the init and destroy of the bridge */
    void Initialize();
    void Quit();

    /* launch and stop of an app domain */
    bool Launch();
    bool Stop();
    bool Restart();

    bool LoadAssembly(std::string file);
    bool LoadAssemblyPath(std::string path);
    MonoObject *CreateEx(std::string file, std::string ns, std::string name);
    MonoObject *Create(std::string ns, std::string name);
    MonoObject *Invoke(MonoObject *obj, std::string method_name, void **params);
    // MonoObject *GetPropValue(MonoObject *obj, std::string prop_name);
    // MonoObject *SetPropValue(MonoObject *obj, std::string prop_name, MonoObject *val);

    MonoDomain *getDomain();


private:
    bool isLoaded;
    MonoDomain *domain;
    // std::vector< MonoAssembly * > assemblies;
    // std::map< std::string, MonoImage* > images;
    std::vector< MonoObject* > instances;
};

}


#endif /* __mono_bridge_hpp_ */
