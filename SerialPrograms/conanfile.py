from conan import ConanFile

class SerialPrograms(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("dpp/10.0.35")
        self.requires("nlohmann_json/3.12.0", force=True)
        self.requires("opencv/4.12.0")