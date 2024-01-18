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

env.Program( target='bin/test_cpu'
            , source = ['build/test/test_cpu.cpp', 'build/instruction_database.cpp', 'build/Cpu.cpp', 'build/common.cpp', 'build/CpuAndPpu.cpp']
            )
