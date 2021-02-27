"""
Author: tushushu
Date: 2020-10-01 16:23:51
"""
import os
from distutils.core import setup
from Cython.Build import cythonize


def compile_cpp11_file(file_name: str) -> None:
    """通过c++将.pyx文件编译为.so文件。

    Arguments:
        file_name {str}
    """
    file_name = file_name.split(
        ".")[0] if file_name.endswith(".pyx") else file_name
    if os.path.exists(file_name + ".so"):
        os.remove(file_name + ".so")
    ext_modules = cythonize(file_name + ".pyx")
    ext_modules[0].extra_compile_args.append("-stdlib=libc++")
    ext_modules[0].extra_link_args.append("-stdlib=libc++")
    name = file_name.split(".")[0] if "." in file_name else file_name
    setup(name=name, ext_modules=ext_modules)
    if os.path.exists(file_name + ".cpp"):
        os.remove(file_name + ".cpp")
    if os.path.exists(file_name + ".cpython-36m-darwin.so"):
        os.rename(file_name + ".cpython-36m-darwin.so", file_name + ".so")


if __name__ == "__main__":
    compile_cpp11_file("chapter_4.pyx")
# python setup.py build_ext --inplace
