# cmake upgrad
sudo apt remove cmake cmake-curses-gui
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" | sudo tee /etc/apt/sources.list.d/kitware.list
sudo apt update
sudo apt install cmake cmake-curses-gui
cmake --version

#niaja upgrad
sudo apt remove ninja-build
sudo rm /usr/bin/ninja

#clang upgrad


#boost

