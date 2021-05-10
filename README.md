# Minecraft

A Minecraft clone written in C leveraging Vulkan.

# Building #defines

| Operating Systems | CPU Architectures | Desktop Window Managers (API) | Memory Interfaces |
| :---------------: | :---------------: | :---------------------------: | :---------------- |
| MC_OS_WINDOWS     | MC_CPU_X86_32     | MC_DWM_WIN32                  | MC_MEM_WIN32      |
| MC_OS_LINUX       | MC_CPU_X86_64     | MC_DWM_XLIB                   | MC_MEM_LIBC       |
|                   |                   |                               | MC_MEM_POSIX      |
