#!/bin/bash
# https://github.com/pypa/python-manylinux-demo/blob/master/travis/build-wheels.sh
set -e -x

# Install a system package required by our library
#yum install -y atlas-devel

mkdir /io/wheelhouse_tmp
mkdir /io/wheelhouse

# Compile wheels
for PYBIN in /opt/python/*/bin; do
    "${PYBIN}/pip" install cython wheel
    "${PYBIN}/pip" wheel --wheel-dir /io/wheelhouse_tmp/ --build-option ${SETUP_PY_ARGS} /io/wrappers/Python
    #"${PYBIN}/pip" wheel /io/wrappers/Python -w /io/wheelhouse_tmp/
done

# Bundle external shared libraries into the wheels
for whl in /io/wheelhouse_tmp/*.whl; do
    auditwheel repair "$whl" -w /io/wheelhouse/
done

## Install packages and test
#for PYBIN in /opt/python/*/bin/; do
#    "${PYBIN}/pip" install python-manylinux-demo --no-index -f /io/wheelhouse
#    (cd "$HOME"; "${PYBIN}/nosetests" pymanylinuxdemo)
#done