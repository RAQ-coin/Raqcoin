from distutils.core import setup
setup(name='abcspendfrom',
      version='1.0',
      description='Command-line utility for raqcoin "coin control"',
      author='Gavin Andresen',
      author_email='gavin@raqcoinfoundation.org',
      requires=['jsonrpc'],
      scripts=['spendfrom.py'],
      )
