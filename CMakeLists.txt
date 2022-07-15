cmake_minimum_required(VERSION 3.0...3.18.4)
# Name of the project
project(MfxExamples)

# Add OpenMfx to define CppPluginSupport, the C++ helper API
add_subdirectory(OpenMfx)

function(target_treat_warnings_as_errors target)
	if(MSVC)
		target_compile_options(${target} PRIVATE /W4 /WX)
	else()
		target_compile_options(${target} PRIVATE -Wall -Wextra -pedantic -Werror)
	endif()
endfunction()

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

# Set up the target to depend on CppPluginSupport and output a file
# called .ofx (rather than the standard .dll or .so)
target_link_libraries(mfx_examples PRIVATE OpenMfx::CppPluginSupport)
set_target_properties(mfx_examples PROPERTIES SUFFIX ".ofx")
target_treat_warnings_as_errors(mfx_examples)

