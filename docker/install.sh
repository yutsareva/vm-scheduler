set -e -x

apt-get update

DEBIAN_FRONTEND="noninteractive" apt-get -y install tzdata

apt-get install -y \
	wget \
	gpg \
	software-properties-common

# Kitware APT repository
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main'
apt-get update

# TODO: remove unnecessary
apt-get install -y \
	cmake \
	ssh \
	clang-12 \
	curl \
	git \
	zip \
	unzip \
	tar \
	pkg-config \
	clang-format-12 \
	clang-tidy-12 \
	lldb-12 \
	python3 \
	python3-pip \
	python3-venv \
	rsync \
	vim \
	gdb \
	ca-certificates \
	openssh-server \
	ninja-build \
	autoconf \
	iputils-ping \
	binutils-dev \
	libboost-all-dev

git clone https://github.com/microsoft/vcpkg
./vcpkg/bootstrap-vcpkg.sh
./vcpkg/vcpkg install "aws-sdk-cpp[ec2]" --recurse
