#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2022-present, Zejun Wang (wangzejunscut@126.com).
# All rights reserved.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import sys
import setuptools
import os
import subprocess
import platform
import io

__version__ = "0.2.0"
EASYTOKENIZER_SRC = "src"


# Based on https://github.com/pybind/python_example
class GetPyBindInclude(object):
    """Helper class to determine the pybind11 include path
    The purpose of this class is to postpone importing pybind11
    until it is actually installed, so that the ``get_include()``
    method can be invoked. """

    def __init__(self, user=False):
        try:
            import pybind11
        except ImportError:
            if subprocess.call([sys.executable, "-m", "pip", "install", "pybind11"]):
                raise RuntimeError("pybind11 install failed.")

        self.user = user

    def __str__(self):
        import pybind11
        return pybind11.get_include(self.user)


ext_modules = [
    Extension(
        str("easytokenizer"),
        [
            str("pybind/easytokenizer/pybind.cc"),
            str("src/tokenizer.cc"),
            str("src/utf8proc.c")
        ],
        include_dirs=[
            # Path to pybind11 headers
            GetPyBindInclude(),
            GetPyBindInclude(user=True),
            # Path to easytokenizer source code
            EASYTOKENIZER_SRC,
        ],
        language="c++",
        extra_compile_args=["-O3"]
    ),
]


# As of Python 3.6, CCompiler has a `has_flag` method.
# cf http://bugs.python.org/issue26689
def has_flag(compiler, flags):
    """Return a boolean indicating whether a flag name is supported on
    the specified compiler.
    """
    import tempfile
    with tempfile.NamedTemporaryFile(mode="w", suffix=".cpp") as f:
        f.write("int main (int argc, char **argv) { return 0; }")
        try:
            compiler.compile([f.name], extra_postargs=flags)
        except setuptools.distutils.errors.CompileError:
            return False
    return True


def cpp_flag(compiler):
    """Return the -std=c++[11/14] compiler flag.
    The c++14 is preferred over c++11 (when it is available).
    """
    standards = ["-std=c++11"]
    for standard in standards:
        if has_flag(compiler, [standard]):
            return standard
    raise RuntimeError(
        "Unsupported compiler -- at least C++11 support is needed!"
    )


class BuildExt(build_ext):
    """A custom build extension for adding compiler-specific options."""
    c_opts = {"msvc": ["/EHsc"], "unix": []}

    def build_extensions(self):
        if sys.platform == "darwin":
            mac_osx_version = float('.'.join(platform.mac_ver()[0].split('.')[:2]))
            os.environ["MACOSX_DEPLOYMENT_TARGET"] = str(mac_osx_version)
            all_flags = ["-stdlib=libc++", "-mmacosx-version-min=10.7"]
            if has_flag(self.compiler, [all_flags[0]]):
                self.c_opts["unix"] += [all_flags[0]]
            elif has_flag(self.compiler, all_flags):
                self.c_opts["unix"] += all_flags
            else:
                raise RuntimeError(
                    "libc++ is needed! Failed to compile with {} and {}.".
                    format(" ".join(all_flags), all_flags[0])
                )
        ct = self.compiler.compiler_type
        opts = self.c_opts.get(ct, [])
        extra_link_args = []
        
        if ct == 'unix':
            opts.append('-DVERSION_INFO="%s"' % self.distribution.get_version())
            opts.append(cpp_flag(self.compiler))
            if has_flag(self.compiler, ["-fvisibility=hidden"]):
                opts.append("-fvisibility=hidden")
        elif ct == "msvc":
            opts.append(
                '/DVERSION_INFO=\\"%s\\"' % self.distribution.get_version()
            )

        for ext in self.extensions:
            ext.extra_compile_args.extend(opts)
            ext.extra_link_args = extra_link_args
        build_ext.build_extensions(self)


def _get_readme():
    """
    Use pandoc to generate rst from md.
    pandoc --from=markdown --to=rst --output=README.rst README.md
    """
    with io.open("README.rst", encoding="utf-8") as fid:
        return fid.read()


setup(
    name="easytokenizer",
    version=__version__,
    author='wangzejun',
    author_email="wangzejunscut@126.com",
    description="easytokenizer Python bindings",
    long_description=_get_readme(),
    ext_modules=ext_modules,
    url="https://github.com/zejunwang1/easytokenizer",
    license="MIT",
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3.7",
        "Topic :: Software Development",
        "Topic :: Scientific/Engineering",
        "Operating System :: Unix",
    ],
    install_requires=["pybind11 >= 2.2", "setuptools >= 0.7.0"],
    cmdclass={"build_ext": BuildExt},
    packages=[str("easytokenizer")],
    package_dir={str(""): str("pybind")},
    zip_safe=False,
)
