# https://github.com/SCons/scons/wiki
# Import necessary modules from SCons


env = Environment(
#         ENV = os.environ
          CXX='clang++'
        , CXXFLAGS=['-std=c++20']
#       , CPPFLAGS=['-Wno-unused-value']
        # LINKFLAGS
        , CPPPATH=['src', 'lib/json/single_include/nlohmann/']
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

