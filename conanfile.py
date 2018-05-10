from conans import ConanFile, CMake, tools


class ConcurrentutilsConan(ConanFile):
    name = "concurrent-utils"
    version = "1.0"
    license = "MIT"
    author = "Max Plutonium"
    url = "https://github.com/max-plutonium/concurrent-utils"
    description = "Utils for concurrent programming"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"

    def source(self):
        self.run("git clone https://github.com/max-plutonium/concurrent-utils.git")
        self.run("cd concurrent-utils && git submodule update --init --checkout --recursive")

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder="concurrent-utils")
        cmake.build()
        
    def package_id(self):
        self.info.header_only()
    
    def package(self):
        self.copy("concurrent-utils/*.h", dst="include", src="concurrent-utils/src")
        self.copy("concurrent-utils/*.tcc", dst="include", src="concurrent-utils/src")

    def package_info(self):
        self.cpp_info.libs = ["concurrent-utils"]
