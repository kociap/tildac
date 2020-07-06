cmake_minimum_required(VERSION 3.13)

project(crust CXX)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_executable(crust
    "${CMAKE_CURRENT_SOURCE_DIR}/compiler/tildac/ast.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/compiler/tildac/ast_printing.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/compiler/tildac/ast_printing.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/compiler/tildac/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/compiler/tildac/parser.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/compiler/tildac/parser.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/compiler/tildac/types.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/compiler/tildac/utility.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/compiler/tildac/codegen.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/compiler/tildac/codegen.cpp")
set_target_properties(crust PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF)

set(TARGET_WebAssembly WebAssemblyCodeGen WebAssemblyAsmParser WebAssemblyDesc WebAssemblyInfo)
set(TARGET_XCore XCoreCodeGen XCoreDesc XCoreInfo)
set(TARGET_SystemZ SystemZCodeGen SystemZAsmParser SystemZDesc SystemZInfo)
set(TARGET_Sparc SparcCodeGen SparcAsmParser SparcDesc SparcInfo)
set(TARGET_RISCV RISCVCodeGen RISCVAsmParser RISCVDesc RISCVUtils RISCVInfo)
set(TARGET_PowerPC PowerPCCodeGen PowerPCAsmParser PowerPCDesc PowerPCInfo)
set(TARGET_NVPTX NVPTXCodeGen NVPTXDesc NVPTXInfo)
set(TARGET_MSP430 MSP430CodeGen MSP430AsmParser MSP430Desc MSP430Info)
set(TARGET_Mips MipsCodeGen MipsAsmParser MipsDesc MipsInfo)
set(TARGET_Lanai LanaiCodeGen LanaiAsmParser LanaiDesc LanaiInfo)
set(TARGET_Hexagon HexagonCodeGen HexagonAsmParser HexagonDesc HexagonInfo)
set(TARGET_BPF BPFCodeGen BPFAsmParser BPFDesc BPFInfo)
set(TARGET_ARM ARMCodeGen ARMAsmParser ARMDesc ARMUtils ARMInfo)
set(TARGET_AMDGPU AMDGPUCodeGen AMDGPUAsmParser AMDGPUDesc AMDGPUUtils AMDGPUInfo)
set(TARGET_X86 X86CodeGen X86AsmParser X86Desc X86Utils X86Info)
set(TARGET_AArch64 AArch64CodeGen AArch64AsmParser AArch64Desc AArch64Utils AArch64Info)
set(TARGET_AVR AVRCodeGen AVRAsmParser AVRDesc AVRInfo)

set(TARGETS_TO_BUILD "WebAssembly" "XCore" "SystemZ" "Sparc" "RISCV" "PowerPC" "NVPTX" "MSP430" "Mips" "Lanai" "Hexagon" "BPF" "ARM" "AMDGPU" "X86" "AArch64" "AVR")

set(LLVM_TARGETS)
foreach (target IN ITEMS ${TARGETS_TO_BUILD})
    list(APPEND LLVM_TARGETS "${TARGET_${target}}")
endforeach(target)

llvm_map_components_to_libnames(LLVM_LIBS
    ${LLVM_TARGETS}
    TextAPI
    OrcJIT
    JITLink
    ObjectYAML
    WindowsManifest
    Coverage
    TableGen
    LTO
    Passes
    ObjCARCOpts
    Coroutines
    LibDriver
    XRay
    MIRParser
    ipo
    Instrumentation
    Vectorize
    Linker
    IRReader
    AsmParser
    Symbolize
    DebugInfoPDB
    FuzzMutate
    LineEditor
    MCA
    DebugInfoGSYM
    GlobalISel
    SelectionDAG
    AsmPrinter
    DebugInfoDWARF
    MCJIT
    Interpreter
    ExecutionEngine
    RuntimeDyld
    CodeGen
    Target
    ScalarOpts
    InstCombine
    AggressiveInstCombine
    TransformUtils
    BitWriter
    Analysis
    ProfileData
    DlltoolDriver
    Option
    Object
    MCParser
    MC
    DebugInfoCodeView
    DebugInfoMSF
    BitReader
    Core
    Remarks
    BinaryFormat
    BitstreamReader
    Support
    Demangle)

target_include_directories(crust PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/compiler")
target_compile_definitions(crust PRIVATE _CRT_SECURE_NO_WARNINGS)
target_compile_options(crust PRIVATE -Wall -Wextra -pedantic -Werror=return-type -Wnon-virtual-dtor)
target_link_libraries(crust PRIVATE ${LLVM_LIBS})