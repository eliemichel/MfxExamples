cmake_minimum_required(VERSION 3.14...3.24)
project(MfxExamples)

include(cmake/FetchOpenMfx.cmake) # get OpenMfx targets
include(cmake/utils.cmake) # for target_treat_warnings_as_errors

# Define the plugin target, called for instance mfx_examples
# You may list additional source files after "plugin.cpp"
set(SRC
	utils.hpp
	TranslateEffect.hpp
	ArrayEffect.hpp
	BoxUvEffect.hpp
	CopyEffect.hpp
	BoxGeneratorEffect.hpp
	ExplodeEffect.hpp
	plugin.cpp
)
add_library(mfx_examples SHARED ${SRC})

# Set up the target to depend on the SDK and output a file
# called .ofx (rather than the standard .dll or .so)
target_link_libraries(mfx_examples PRIVATE OpenMfx::Sdk::Cpp::Plugin)
set_target_properties(mfx_examples PROPERTIES SUFFIX ".ofx")

target_treat_warnings_as_errors(mfx_examples)
