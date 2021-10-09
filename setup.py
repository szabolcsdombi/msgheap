from setuptools import Extension, setup

ext = Extension(
    name='msgheap',
    sources=['msgheap.cpp'],
)

setup(
    name='msgheap',
    version='0.1.0',
    ext_modules=[ext],
)
