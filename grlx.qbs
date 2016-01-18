import qbs

Product {
    name: "grlx"
    Depends { name: "cpp" }

    cpp.includePaths: [
        ".",
        "./rapidjson/include"
    ]
    files: ["**/*.h"]

    Export {
        Depends { name: "cpp" }

        cpp.includePaths: [
            ".",
            "./rapidjson/include"
        ]
        cpp.defines: ["RAPIDJSON_HAS_STDSTRING", "RAPIDJSON_SSE42", "RAPIDJSON_HAS_CXX11_RVALUE_REFS"]
        cpp.cxxFlags: "-std=c++0x"
    }

}
