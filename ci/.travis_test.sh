cd ./build
./tests

echo "Running Feat Wrapper Tests"

export PATH=/home/travis/miniconda/bin:$PATH

cd ../python
python wrappertest.py -v 1
