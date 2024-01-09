# https://github.com/SCons/scons/wiki
import os


env = Environment(
          ENV = os.environ
        , CXX='clang++'
        , CXXFLAGS=['-std=c++20']
#       , CPPFLAGS=['-Wno-unused-value']
        # LINKFLAGS
        , CPPPATH=['src/include', 'lib/json/single_include/nlohmann/']
        # LIBS
        )

env.VariantDir('build', 'src', duplicate=0)

# https://scons.org/doc/production/HTML/scons-user/ch27.html
env.Tool('compilation_db')
env.CompilationDatabase()

env.Program( target='bin/test_check_bits'
           , source=['build/test_check_bits.cpp', 'build/common.cpp']
            )

env.Program( target='bin/json_test'
            , source = ['build/json_test.cpp']
            )

env.Program( target='bin/test_cpu'
            , source = ['build/instruction_database.cpp', 'build/test_cpu.cpp', 'build/Cpu.cpp', 'build/common.cpp', 'build/CpuAndPpu.cpp']
            )

env.Program( target='bin/test_instdb'
            , source = ['build/test_instdb.cpp', 'build/instruction_database.cpp']
            )
