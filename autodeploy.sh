git reset --hard HEAD
git pull origin master
mkdir build
cd build
cmake ..
make
./leanclub