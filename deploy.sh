#!/bin/bash
length=$(($#-1))
OPTIONS=${@:1:$length}
REPONAME="${!#}"
CWD=$PWD
echo -e "\n\nInstalling commons libraries...\n\n"
COMMONS="so-commons-library"
git clone "https://github.com/sisoputnfrba/${COMMONS}.git" $COMMONS
cd $COMMONS
sudo make uninstall
make all
sudo make install
cd $CWD
echo -e "\n\nBuilding projects...\n\n"
make -C ./app
make -C ./cliente
make -C ./restaurante
make -C ./comanda
make -C ./sindicato
echo -e "\n\nDeploy done!\n\n"