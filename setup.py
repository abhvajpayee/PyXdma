from Cython.Build import cythonize
from distutils.core import setup, Extension
import numpy

#C_module = Extension('PyXdma.libcxdma',
#                     sources = ['PyXdma/cxdma.c'],
#                     libraries=["pthread"],
#                     include_dirs=[ 'PyXdma/include' ],
#                     library_dirs = ['PyXdma'],
#                    )

cython_modules = cythonize([Extension("PyXdma.xdma", ["PyXdma/xdma.pyx"],
                                      #libraries=["cxdma.cpython-37m-x86_64-linux-gnu"],
                                      include_dirs=[ 'PyXdma', numpy.get_include() ],
                                      library_dirs=[ 'PyXdma' ],
                                     ),
                           ])

#modules = [C_module, cython_modules[0]]
modules = [cython_modules[0]]

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
