import qbs

Product {
    name: "grlx"
    Depends { name: "cpp" }

    cpp.cxxLanguageVersion: "c++14";
    //cpp.cxxStandardLibrary: "libstdc++";


    cpp.cxxFlags: ["-Wno-unused-parameter"]

    //type: ["dynamiclibrary"]
    type: ["staticlibrary"]

    cpp.includePaths: [
        "include",
        "include/grlx/service",
        "include/grlx/rpc/transport/zmq"
    ]
    files: [
        "include/**/*.h",
        "cpp/**/*.cpp"
    ]

    Export {
        Depends { name: "cpp" }

        cpp.includePaths: [
            "include"
        ]
        cpp.defines: ["RAPIDJSON_HAS_STDSTRING", "RAPIDJSON_SSE42", "RAPIDJSON_HAS_CXX11_RVALUE_REFS"]
        cpp.cxxLanguageVersion: "c++17";
        cpp.cxxFlags: ["-Wno-unused-parameter"]
        //cpp.cxxStandardLibrary: "libstdc++";

        //cpp.libraryPath: xyzPath + "/lib"
        cpp.staticLibraries: "zmq"
    }

}
