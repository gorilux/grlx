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
        cpp.cxxFlags: "-std=c++0x"
    }

}
