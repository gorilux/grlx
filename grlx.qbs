import qbs

Product {
    name: "grlx"
    Depends { name: "cpp" }

    cpp.cxxLanguageVersion: "c++14";
    //cpp.cxxStandardLibrary: "libstdc++";



    //type: ["dynamiclibrary"]
    type: ["staticlibrary"]

    cpp.includePaths: [
        "include",
        "include/grlx/service"
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
        cpp.cxxLanguageVersion: "c++14";        
        //cpp.cxxStandardLibrary: "libstdc++";

        //cpp.libraryPath: xyzPath + "/lib"
        cpp.staticLibraries: "zmq"
    }

}
