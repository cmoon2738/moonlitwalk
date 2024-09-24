# Possible supported platforms (<ABI | binary format>):
#
# Arch        | Linux/Unix/Android   | Windows            | MacOSX/iOS        | 
# ============|======================|===========================================================
# arm         | AAPCS | ELF          | AAPCS | PE         | AAPCS / MACH-0    | 
# arm64       | AAPCS | ELF          | --                 | AAPCS / MACH-0    | 
# i386        | SYSV | ELF           | MS | PE            | SYSV / MACH-0     | 
# loongarch64 | SYSV | ELF           |                    |                   |
# mips32      | O32 | ELF            | --                 | --                |
# mips64      | N64 | ELF            | --                 | --                |
# ppc32       | SYSV | ELF, XCOFF    | --                 | SYSV / MACH-0     | 
# ppc64       | SYSV | ELF, XCOFF    | --                 | SYSV / MACH-0     | 
# sparc       | SYSV | ELF           | --                 | --                |
# sparc64     | SYSV | ELF           | --                 | --                |
# x86_64      | SYSV,X32 | ELF       | MS | PE            | SYSV / MACH-0     | 
# wasm32/64   | Emscripten Toolchain, WebAssembly Binary Toolkit (WABT)
# --------------------------------------------------------------------------------------------

assembly = []
mw_cpu = 'null'
mw_os = 'null'
mw_abi = 'null'
mw_asm = 'null'

cpu_family = host_machine.cpu_family()

# Setting up the architecture
if cpu_family == 'x86_64'
    mw_cpu = 'x86_64'

elif cpu_family == 'x86' # i386
    mw_cpu = 'i386'

elif cpu_family == 'aarch64' # arm64
    mw_cpu = 'arm64'
    mw_os = 'aapcs'

elif cpu_family == 'arm' # arm32
    mw_cpu = 'arm'
    mw_os = 'aapcs'

elif cpu_family == 'loongarch64'
    mw_cpu = 'loongarch64'

elif cpu_family == 'mips'
    mw_cpu = 'mips32'
    mw_os = 'o32'

elif cpu_family == 'mips64'
    mw_cpu = 'mips64'
    mw_os = 'n64'

elif cpu_family == 'ppc'
    mw_cpu = 'ppc32'

elif cpu_family == 'ppc64'
    mw_cpu = 'ppc64'

elif cpu_family == 'sparc'
    mw_cpu = 'sparc'

elif cpu_family == 'sparc64'
    mw_cpu = 'sparc64'

elif cpu_family == 'wasm32' or host_machine.cpu_family() == 'wasm' or host_machine.cpu_family() == 'wasm64'
    mw_cpu = 'wasm'
    mw_os  = 'emscripten'
    mw_abi = 'webassembly'
    mw_asm = 'wabt'

else # cpu family not specified here
    error('Host CPU family architecture is not supported. Check the assembly sources and this meson.build, and implement if missing.')
endif


# Setting up the os, abi and assembler
if host_machine.system() == 'darwin'
    if mw_os.contains('null')
        mw_os = 'sysv' # will be aapcs for arm / arm64
    endif
    mw_abi = 'macho'
    mw_asm = 'gas.S'

elif host_machine.system() == 'windows'
    add_languages('masm')
    if mw_os.contains('null')
        mw_os = 'ms' # will be aapcs for arm / arm64
    endif
    mw_abi = 'pe'
    if cc.get_id() == 'msvc'
        if (mw_cpu == 'arm' or mw_cpu == 'arm64')
            mw_asm = 'armasm.asm'
        else
            mw_asm = 'masm.asm'
        endif
    else
        mw_asm = 'gas.asm'
    endif

elif host_machine.system() == 'aix' # IBM AIX PowerPC
    mw_abi = 'xcoff'
endif

if mw_os.contains('null')
    mw_os = 'sysv'
endif
if mw_abi.contains('null')
    mw_abi = 'elf'
endif
if mw_asm.contains('null')
    mw_asm = 'gas.S'
endif

if sys == 'emscripten'
    # We want to use WABT (the WebAssembly Binary Toolkit) to translate from
    # WebAssembly text format to the WebAssembly binary format.
    # https://github.com/WebAssembly/wabt
    
    # 'wasm2wat' is already included in the 'wasm.ini' cross file.
    assembly_config = 'wasm_emscripten_wabt.wat'
else
    assembly_config = mw_cpu + '_' + mw_os + '_' + mw_abi + '_' + mw_asm
endif
assembly += files(
    'jump_' + assembly_config,
    'make_' + assembly_config,
)
