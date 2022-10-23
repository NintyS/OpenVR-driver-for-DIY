#!/usr/bin/bash
rm -R /home/nintyswinty/.local/share/Steam/steamapps/common/SteamVR/drivers/sample
echo "cmake\n"
cmake .
echo "make instal\n"
make install
echo "Coping\n"
sudo cp -R /usr/local/drivers/sample /home/nintyswinty/.local/share/Steam/steamapps/common/SteamVR/drivers/
echo "Setting accessability\n"
chmod 777 -R /home/nintyswinty/.local/share/Steam/steamapps/common/SteamVR/drivers/sample
echo "Done"