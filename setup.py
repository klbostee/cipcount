from setuptools import setup, Extension
setup(name='cipcount',
      version='0.3',
      author='Klaas Bosteels',
      author_email='klaas@last.fm',
      license = 'Apache Software License (ASF)',
      py_modules=['cipcount'],
      ext_modules=[Extension('cipcount_impl', ['cipcount_impl.c'])],
      install_requires = ['dumbo']
     )

