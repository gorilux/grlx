import qbs

Product {
    name: "grlx"
    Depends { name: "cpp" }

    cpp.includePaths: [ "." ]
    files: ["**/*.h"]

    Export {
        Depends { name: "cpp" }

        cpp.includePaths: [ "." ]
        cpp.cxxFlags: "-std=c++0x"
    }

}
