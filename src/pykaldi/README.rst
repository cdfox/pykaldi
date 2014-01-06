Install & Dependencies
============
 * ``cython 19.1+``, fork of `PyFST <https://github.com/UFAL-DSG/pyfst>`_ which transitively requires ``pyyaml``, ``pystache``
 * It should be enough to run ``make`` from ``pykaldi`` directory 

DEPRECATED- REWRITE - Running and building examples
-----------------------------
The Python [setuptools](http://pythonhosted.org/an_example_pypi_project/setuptools.html#installing-setuptools-and-easy-install) is used instead of Makefile
for "Installing" and running tests.

Surprisingly the most used `python setup.py install` command does not work.
It is so, because the `Pykaldi` depends on Kaldi libraries and they are not used as system libraries. You have probably noticed no 'make install' or anything similar.
If the it changes it will work. In fact, it does not matter.

Commands you SHOULD USE:
 * `python setup.py develop --user` an its undo `python setup.py develop --uninstall`

    The command links pykaldi directory to PYTHONPATH (at Ubuntu to `$HOME/.local/lib/python2.7/site-packages/`),
    so you can use pykaldi system wide.

    ```python
    import pykaldi
    print pykaldi.__version__
    ```
 * `python setup.py nosetests` runs all test for `pykaldi`


Commands you SHOULD NOT USE:
 * `python setup.py install [--user]` and its undo `pip uninstall pykaldi`

    This command fails due to missing headers of Kaldi. There are no headers for Kaldi in system paths.
