Overview
========

The GPGME Python bindings passed through many hands and numerous phases
before, after a fifteen year journey, coming full circle to return to
the source. This is a short explanation of that journey.

.. _in-the-begining:

In the beginning
----------------

In 2002 John Goerzen released PyME; Python bindings for the GPGME module
which utilised the current release of Python of the time and SWIG. [1]_
Shortly after creating it and ensuring it worked he stopped supporting
it, though he left his work available on his Gopher site.

Keeping the flame alive
-----------------------

A couple of years later the project was picked up by Igor Belyi and
actively developed and maintained by him from 2004 to 2008. Igor's
whereabouts at the time of this document's creation are unknown, but the
current authors do hope he is well. We're assuming (or hoping) that life
did what life does and made continuing untenable.

Passing the torch
-----------------

In 2014 Martin Albrecht wanted to patch a bug in the PyME code and
discovered the absence of Igor. Following a discussion on the PyME
mailing list he became the new maintainer for PyME, releasing version
0.9.0 in May of that year. He remains the maintainer of the original
PyME release in Python 2.6 and 2.7 (available via PyPI).

.. _ouroboros:

Coming full circle
------------------

In 2015 Ben McGinnes approached Martin about a Python 3 version, while
investigating how complex a task this would be the task ended up being
completed. A subsequent discussion with Werner Koch led to the decision
to fold the Python 3 port back into the original GPGME release in the
languages subdirectory for non-C bindings under the module name of
``pyme3``.

In 2016 this PyME module was integrated back into the GPGME project by
Justus Winter. During the course of this work Justus adjusted the port
to restore limited support for Python 2, but not as many minor point
releases as the original PyME package supports. During the course of
this integration the package was renamed to more accurately reflect its
status as a component of GPGME. The ``pyme3`` module was renamed to
``gpg`` and adopted by the upstream GnuPG team.

In 2017 Justus departed G10code and the GnuPG team. Following this Ben
returned to maintain of gpgme Python bindings and continue building them
from that point.

.. _relics-past:

Relics of the past
==================

There are a few things, in addition to code specific factors, such as
SWIG itself, which are worth noting here.

The Annoyances of Git
---------------------

As anyone who has ever worked with git knows, submodules are horrible
way to deal with pretty much anything. In the interests of avoiding
migraines, that was skipped with addition of the PyME code to GPGME.

Instead the files were added to a subdirectory of the ``lang/``
directory, along with a copy of the entire git log up to that point as a
separate file within the ``lang/python/docs/`` directory. [2]_ As the
log for PyME is nearly 100KB and the log for GPGME is approximately 1MB,
this would cause considerable bloat, as well as some confusion, should
the two be merged.

Hence the unfortunate, but necessary, step to simply move the files. A
regular repository version has been maintained should it be possible to
implement this better in the future.

The Perils of PyPI
------------------

The early port of the Python 2 ``pyme`` module as ``pyme3`` was never
added to PyPI while the focus remained on development and testing during
2015 and early 2016. Later in 2016, however, when Justus completed his
major integration work and subsequently renamed the module from
``pyme3`` to ``gpg``, some prior releases were also provided through
PyPI.

Since these bindings require a matching release of the GPGME libraries
in order to function, it was determined that there was little benefit in
also providing a copy through PyPI since anyone obtaining the GPGME
source code would obtain the Python bindings source code at the same
time. Whereas there was the potential to sew confusion amongst Python
users installing the module from PyPI, only to discover that without the
relevant C files, header files or SWIG compiled binaries, the Python
module did them little good.

There are only two files on PyPI which might turn up in a search for
this module or a sample of its content:

#. gpg (1.8.0) - Python bindings for GPGME GnuPG cryptography library
#. pyme (0.9.0) - Python support for GPGME GnuPG cryptography library

.. _pypi-gpgme-180:

GPG 1·8·0 - Python bindings for GPGME GnuPG cryptography library
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This is the most recent version to reach PyPI and is the version of the
official Pyhon bindings which shipped with GPGME 1.8.0. If you have
GPGME 1.8.0 installed and *only* 1.8.0 installed, then it is probably
safe to use this copy from PyPI.

As there have been a lot of changes since the release of GPGME 1.8.0,
the GnuPG Project recommends not using this version of the module and
instead installing the current version of GPGME along with the Python
bindings included with that package.

.. _pypi-gpgme-90:

PyME 0·9·0 - Python support for GPGME GnuPG cryptography library
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This is the last release of the PyME bindings maintained by Martin
Albrecht and is only compatible with Python 2, it will not work with
Python 3. This is the version of the software from which the port from
Python 2 to Python 3 code was made in 2015.

Users of the more recent Python bindings will recognise numerous points
of similarity, but also significant differences. It is likely that the
more recent official bindings will feel "more pythonic."

For those using Python 2, there is essentially no harm in using this
module, but it may lack a number of more recent features added to GPGME.

Footnotes
=========

.. [1]
   In all likelihood thos would have been Python 2.2 or possibly Python
   2.3.

.. [2]
   The entire PyME git log and other preceding VCS logs are located in
   the ``gpgme/lang/python/docs/old-commits.log`` file.