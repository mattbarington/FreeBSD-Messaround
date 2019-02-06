echo "make buildkernel -DKERNFAST -j$1"
./copyFiles.sh; cd /usr/src/; make buildkernel -DKERNFAST -j$1;
