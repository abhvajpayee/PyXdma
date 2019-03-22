cat install.log | xargs rm -rf
python3 setup.py clean
rm -rf dist
rm -rf build
python3 setup.py sdist
python3 setup.py bdist
python3 setup.py install --record install.log

