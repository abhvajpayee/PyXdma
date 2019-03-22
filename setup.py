#from distutils.core import setup
from Cython.Build import cythonize
#from distutils.extension import Extension
from distutils.core import setup, Extension
import numpy

C_module = Extension('PyXdma.libxdma',
                     sources = ['PyXdma/libxdma.c'],
                     libraries=["pthread"],
		     include_dirs=[ 'PyXdma/include' ],)

cython_modules = cythonize([Extension("PyXdma.pci", ["PyXdma/pci.pyx"],
                                      libraries=["libxdma"],
                                      include_dirs=[ 'PyXdma', numpy.get_include() ],
                                      library_dirs=[ 'PyXdma' ],
                                     ),
                           ])

modules = [C_module, cython_modules[0]]
#cython_modules.append(C_module)

setup(name='PyXdma',
      version='0.1.0',
      author='Abhishek Bajpai',
      author_email='abhvajpayee@gmail.com',
      packages=['PyXdma'],
      ext_modules = modules,
      #ext_modules = cythonize("PyXdma/*.pyx"),
      scripts=[ ],
      url=' ',
      license='LICENSE.txt',
      description='Useful stuff.',
      long_description=open('README.txt').read(),
      #install_requires=[    ],
      )
