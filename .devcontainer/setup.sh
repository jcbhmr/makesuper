#!/usr/bin/env bash
set -ex

cmake_version=$(cmake --version | grep -Pom1 '\d+\.\d+\.\d+')
cmake_version_major=$(echo "$cmake_version" | cut -d. -f1)
cmake_version_minor=$(echo "$cmake_version" | cut -d. -f2)
if (( cmake_version_major != 3 )) || (( cmake_version_minor < 29 )); then
    echo "cmake version $cmake_version is <3.29"
    # https://askubuntu.com/questions/355565/how-do-i-install-the-latest-version-of-cmake-from-the-command-line
    sudo apt remove --purge --auto-remove cmake
    # OR sudo apt purge --auto-remove cmake
    sudo apt update && \
    sudo apt install -y software-properties-common lsb-release && \
    sudo apt clean all
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
    sudo apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main"
    sudo apt update
    sudo apt install kitware-archive-keyring
    sudo rm /etc/apt/trusted.gpg.d/kitware.gpg
    sudo apt update
    sudo apt install cmake
fi

if ! command -v cosmocc; then
    pushd ~
    mkdir -p cosmocc
    cd cosmocc
    wget https://cosmo.zip/pub/cosmocc/cosmocc.zip
    unzip cosmocc.zip
    echo 'export PATH="$PATH:$HOME/cosmocc/bin"' >> ~/.bashrc
    popd
    source ~/.bashrc

    if grep binfmt /proc/mounts; then
        sudo wget -O /usr/bin/ape https://cosmo.zip/pub/cosmos/bin/ape-$(uname -m).elf
        sudo chmod +x /usr/bin/ape
        sudo sh -c "echo ':APE:M::MZqFpD::/usr/bin/ape:' >/proc/sys/fs/binfmt_misc/register"
        sudo sh -c "echo ':APE-jart:M::jartsr::/usr/bin/ape:' >/proc/sys/fs/binfmt_misc/register"
    else
        echo no binfmt_misc
    fi
fi

if ! command -v clang-format; then
    cd ~/.local/bin
    sudo wget https://cosmo.zip/pub/cosmos/bin/clang-format
    chmod +x clang-format
    echo 'PATH="$PATH:$HOME/.local/bin"' >> ~/.bashrc
    source ~/.bashrc
fi
