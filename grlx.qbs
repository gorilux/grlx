import qbs

Product {
    name: "grlx"
    Depends { name: "cpp" }  

    Properties {
        condition: qbs.targetPlatform.contains("android")
        //qbs.architecture: "armv7a"
        architectures: !qbs.architecture ? ["x86", "armv7a"] : undefined
        Android.ndk.appStl: "c++_static"
        Android.ndk.platformVersion: 21
    }



    cpp.cxxLanguageVersion: "c++17";
    cpp.cxxFlags: ["-Wno-unused-parameter","-Wno-gnu-string-literal-operator-template"]

    //type: ["dynamiclibrary"]
    type: ["staticlibrary"]

    cpp.includePaths: [
        "include",
        "include/grlx/service",
        "include/grlx/rpc/transport/zmq",
        "include/grlx/rpc/transport"
    ]
    files: [       
        "include/**/*.h",
        "include/**/*.hpp",
        "cpp/**/*.cpp"
    ]



    Export {
        Depends { name: "cpp" }

        cpp.includePaths: [
            "include"
        ]

        cpp.defines: ["RAPIDJSON_HAS_STDSTRING", "RAPIDJSON_HAS_CXX11_RVALUE_REFS", "PETRA_USE_UDL"]

        cpp.cxxLanguageVersion: "c++17";
        cpp.cxxFlags: ["-Wno-unused-parameter", "-Wno-gnu-string-literal-operator-template"]
        //cpp.cxxStandardLibrary: "libstdc++";

        //cpp.libraryPath: xyzPath + "/lib"
        cpp.staticLibraries: "zmq"
    }





}
