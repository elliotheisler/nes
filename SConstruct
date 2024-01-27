# https://scons.org/documentation.html
import os

# https://scons.org/doc/production/HTML/scons-user/apa.html#cv-CXXFLAGS
env = Environment(
          ENV = os.environ
        , CXX='clang++'
        , CXXFLAGS=['-std=c++20']
#       , CPPFLAGS=['-Wno-unused-value']
        # LINKFLAGS
        , CPPPATH=['src/include', 'lib/json/single_include/nlohmann/']
        # LIBS
        )
# https://scons.org/doc/production/HTML/scons-user/ch27.html
env.Tool('compilation_db')
env.CompilationDatabase()

env_debug = env.Clone(
)
env_debug.Append(CXXFLAGS=['-g', '-O0'])

env.VariantDir('build', 'src', duplicate=0)

test_cpu_src = ['build/test/test_Cpu.cpp', 'build/instruction_database.cpp', 'build/Cpu.cpp', 'build/Cartridge.cpp', 'build/r16.cpp']
env.Program( target='bin/test_Cpu'
            , source = test_cpu_src
            )

objs_dbg = [
        env_debug.Object(ofile, srcfile) for ofile, srcfile in zip(
                ['test_Cpu-dbg', 'instruction_database-dbg', 'Cpu-dbg', 'Cartridge-dbg', 'r16-dbg'], test_cpu_src
                )
]
env_debug.Program(target='bin/test_Cpu-dbg', source=objs_dbg)

env.Program( target='bin/test_Cartridge'
            , source = ['build/test/test_Cartridge.cpp', 'build/Cartridge.cpp']
            )

env.Program( target='bin/test_r16'
, source = ['build/test/test_r16.cpp', 'build/r16.cpp']
)
