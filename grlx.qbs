import qbs

Product {
    name: "grlx"
    files: ["**/*.h"]
    Export {
        Depends { name: "cpp" }

        cpp.includePaths: [ "." ]
        cpp.cxxFlags: "-std=c++0x"
    }

}
