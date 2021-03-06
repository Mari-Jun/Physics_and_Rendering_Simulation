# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

project('DirectX-Headers', 'cpp', version : '1.0.1',
        default_options : ['cpp_std=c++14'])

inc_dirs = [include_directories('include', is_system : true)]
install_inc_subdirs = ['']

if host_machine.system() != 'windows'
    inc_dirs += include_directories('include/wsl/stubs', is_system : true)
    install_inc_subdirs += ['', 'wsl/stubs', 'directx']
endif

guids_lib = static_library('DirectX-Guids', 'src/dxguids.cpp', include_directories : inc_dirs, install : true)

dep_dxheaders = declare_dependency(
    link_with : guids_lib,
    include_directories : inc_dirs)

if not meson.is_subproject() and get_option('build-test')
    subdir('test')
endif

pkg = import('pkgconfig')
pkg.generate(name : 'DirectX-Headers',
             description : 'Headers for using D3D12',
             libraries : [guids_lib],
             version : meson.project_version(),
             subdirs : install_inc_subdirs)
install_subdir('include', install_dir : '')
