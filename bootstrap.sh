#!/bin/bash

#####
#Dipendence: automake libtool subversion git-core bzr ruby1.8-full cmake zlib1g-dev libcloog-ppl0 wget g++
#####

pushd `dirname $0` > /dev/null
SCRIPTPATH=`pwd -P`
popd > /dev/null

OS=$(uname -s)
ARCH=$(uname -m)
KERNEL_VER=$(uname -r)
KERNEL_SHORT_VER=$(uname -r|cut -d\- -f1|tr -d '.'| tr -d '[A-Z][a-z]')

if [ -n "$CHAOS_BOOST_VERSION" ]; then
	BOOST_VERSION="1_"$CHAOS_BOOST_VERSION"_0"
	BOOST_VERSION_IN_PATH="1.$CHAOS_BOOST_VERSION.0"
else
	BOOST_VERSION=1_53_0
	BOOST_VERSION_IN_PATH=1.53.0
fi

BOOST_NUMBER_VERSION=$(echo $BOOST_VERSION_IN_PATH |sed "s/[^0-9]//g" )
LMEM_VERSION=1.0.16


if [ -n "$1" ]; then
    PREFIX=$1/usr/local
else
    PREFIX=$CHAOS_DIR/usr/local
fi

if [ -n "$CHAOS32" ]; then
export CFLAGS="-m32 -arch i386"
export CXXFLAGS="-m32 -arch i386"
echo "Force 32 bit binaries"
fi

if [ -n "$CHAOS_DEVELOPMENT" ]; then
	COMP_TYPE=Debug
#CHAOS_DIR=$SCRIPTPATH/../
	echo "Setup for chaos development folder structure"
#echo "Shared libray prefix -> $CHAOS_DIR"
else
	COMP_TYPE=Release
fi

CHAOS_DIR=$SCRIPTPATH
BASE_EXTERNAL=$CHAOS_DIR/external
PREFIX=$CHAOS_DIR/usr/local

echo "Operating system version: $OS"
echo "Current architecture: $ARCH"
echo "Current kernel version: $KERNEL_VER"
echo "Current short kernel version: $KERNEL_SHORT_VER"
echo "Using $CHAOS_DIR as chaos folder"
echo "Using $BASE_EXTERNAL as external library folder"
echo "Using $PREFIX as prefix folder"
echo "Using BOOST version $BOOST_VERSION/$BOOST_VERSION_IN_PATH ($BOOST_NUMBER_VERSION)"
echo "Compilation type -> $COMP_TYPE"

if [ ! -d "$BASE_EXTERNAL" ]; then
    mkdir -p $BASE_EXTERNAL
fi


if [ ! -d "$PREFIX/include/boost" ]; then
    if [ ! -e "$BASE_EXTERNAL/boost_$BOOST_VERSION.tar.gz" ]; then
        echo "Download boost source"
        wget --no-check-certificate -O $BASE_EXTERNAL/boost_$BOOST_VERSION.tar.gz "http://downloads.sourceforge.net/project/boost/boost/$BOOST_VERSION_IN_PATH/boost_$BOOST_VERSION.tar.gz?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Fboost%2Ffiles%2Fboost%2F$BOOST_VERSION_IN_PATH%2F&ts=1350734344&use_mirror=freefr"
        
    fi

    if [ ! -e $BASE_EXTERNAL/boost ]; then
        tar zxvf $BASE_EXTERNAL/boost_$BOOST_VERSION.tar.gz -C $BASE_EXTERNAL
        mv $BASE_EXTERNAL/boost_$BOOST_VERSION $BASE_EXTERNAL/boost
    fi

#install old version of boost log
	if [ $BOOST_NUMBER_VERSION -le 1530 ] && [ ! -d "$BASE_EXTERNAL/boost_log" ]; then
		git clone https://cvs.lnf.infn.it/boost_log $BASE_EXTERNAL/boost_log

		if [ ! -d "$BASE_EXTERNAL/boost/boost/log" ]; then
			echo "link $BASE_EXTERNAL/boost/boost/log -> $BASE_EXTERNAL/boost_log/boost/log"
			ln -s $BASE_EXTERNAL/boost_log/boost/log $BASE_EXTERNAL/boost/boost/log
		fi

		if [ ! -d "$BASE_EXTERNAL/boost/libs/log" ]; then
			echo "link $BASE_EXTERNAL/boost/libs/log -> $BASE_EXTERNAL/boost_log/libs/log"
			ln -s $BASE_EXTERNAL/boost_log/libs/log $BASE_EXTERNAL/boost/libs/log
		fi
	fi


    if [ ! -f "$BASE_EXTERNAL/boost/b2" ]; then
        echo "Boostrapping boost"
        cd $BASE_EXTERNAL/boost
        ./bootstrap.sh
    fi

    cd $BASE_EXTERNAL/boost
    echo "Compile and install boost libraries into $PREFIX/"

if [ -n "$CHAOS32" ]; then
    	echo "INSTALLING BOOST X86 32"
    	./b2 link=shared cflags=-m32 cxxflags=-m32 architecture=x86 address-model=32 --prefix=$PREFIX --with-iostreams --with-program_options --with-chrono --with-filesystem --with-log --with-regex --with-system --with-thread --with-atomic --with-timer install
    else
    	./b2 link=shared --prefix=$PREFIX --with-program_options --with-chrono --with-filesystem --with-iostreams --with-log --with-regex --with-system --with-thread --with-atomic --with-timer install
    fi
else
    echo "Boost Already present"
fi

### install libmodbus
if [ ! -d "$PREFIX/include/modbus" ]; then
        echo "Setup libmodbus library"
        if [ ! -d "$BASE_EXTERNAL/libmodbus" ]; then
                echo "Install libmodbus"
                git clone https://github.com/stephane/libmodbus.git $BASE_EXTERNAL/libmodbus
                cd $BASE_EXTERNAL/libmodbus
        else
                echo "Update libmodbus"
                cd $BASE_EXTERNAL/libmodbus/
                git pull
        fi
./autogen.sh
./configure --prefix=$PREFIX 
make clean
make
make install
echo "libmodbus done"
fi

if [ ! -d "$PREFIX/include/msgpack" ]; then
	echo "Setup MSGPACK"
	if [ ! -d "$BASE_EXTERNAL/msgpack-c" ]; then
		echo "Install msgpack-c"
		git clone https://github.com/msgpack/msgpack-c.git $BASE_EXTERNAL/msgpack-c
		cd $BASE_EXTERNAL/msgpack-c/
	else
		echo "Update msgpack-c"
		cd $BASE_EXTERNAL/msgpack-c/
		git pull
	fi
./bootstrap
./configure --prefix=$PREFIX 
make clean
make
make install
echo "MSGPACK Setupped"
fi

if [ ! -d "$PREFIX/include/mp" ]; then
	echo "Setup MPIO"
	if [ ! -d "$BASE_EXTERNAL/mpio" ]; then
		echo "Install mpio"
	#    git clone https://github.com/frsyuki/mpio.git $BASE_EXTERNAL/mpio
		git clone https://github.com/bisegni/mpio.git $BASE_EXTERNAL/mpio
		cd $BASE_EXTERNAL/mpio
	else
		echo "Update mpio"
		cd $BASE_EXTERNAL/mpio
		git pull
	fi
	./bootstrap
	if [ `echo $OS | tr [:upper:] [:lower:]` = `echo "linux" | tr [:upper:] [:lower:]` ] && [ $KERNEL_SHORT_VER -le 2625 ]; then
	./configure --prefix=$PREFIX
	else
	./configure --disable-timerfd --disable-signalfd --prefix=$PREFIX
	fi
	make clean
	make
	make install
	echo "MPIO Setupped"
fi

if [ ! -d "$PREFIX/include/msgpack/rpc" ]; then
	echo "Setup MSGPACK-RPC"
	if [ ! -d "$BASE_EXTERNAL/msgpack-rpc" ]; then
		echo "Install msgpack-rpc"
#git clone https://github.com/msgpack-rpc/msgpack-rpc-cpp.git $BASE_EXTERNAL/msgpack-rpc
		git clone https://github.com/bisegni/msgpack-rpc-cpp.git $BASE_EXTERNAL/msgpack-rpc
		cd $BASE_EXTERNAL/msgpack-rpc
	else
		echo "Update msgpack-rpc"
		cd $BASE_EXTERNAL/msgpack-rpc/
		git pull
		cd $BASE_EXTERNAL/msgpack-rpc/
	fi
	./bootstrap
	./configure --prefix=$PREFIX --with-mpio=$PREFIX --with-msgpack=$PREFIX
	make clean
	make
	make install
	echo "MSGPACK-RPC Setupped"
fi

echo "Setup LIBEVENT"
if [ ! -d "$PREFIX/include/event2" ]; then
	if [ ! -f "$BASE_EXTERNAL/libevent" ]; then
		echo "Installing LibEvent"
	#    git clone git://levent.git.sourceforge.net/gitroot/levent/libevent $BASE_EXTERNAL/libevent
		git clone http://git.code.sf.net/p/levent/libevent $BASE_EXTERNAL/libevent
		cd $BASE_EXTERNAL/libevent
	else
		cd $BASE_EXTERNAL/libevent
		git pull
	fi
	./autogen.sh
	./configure --prefix=$PREFIX
	make clean
	make
	make install
	echo "LIBEVENT Setupped"
fi

echo "Setup LIBMEMCACHED"
if [ ! -d "$PREFIX/include/libmemcached" ]; then
    echo "Install libmemcached into  $BASE_EXTERNAL/libmemcached"
    wget --no-check-certificate -O $BASE_EXTERNAL/libmemcached.tar.gz https://launchpad.net/libmemcached/1.0/$LMEM_VERSION/+download/libmemcached-$LMEM_VERSION.tar.gz
    tar zxvf $BASE_EXTERNAL/libmemcached.tar.gz -C $BASE_EXTERNAL
    cd $BASE_EXTERNAL/libmemcached-$LMEM_VERSION
    ./configure --disable-sasl --without-memcached --prefix=$PREFIX
    make clean
    make
    make install
fi
echo "LIBMEMCACHED Setupped"

echo "Setup ZMQ"
if [ ! -f "$PREFIX/include/zmq.h" ]; then
	if [ ! -d "$BASE_EXTERNAL/zeromq3-x" ]; then
	echo "Download zmq source"
	git clone https://github.com/zeromq/zeromq3-x.git $BASE_EXTERNAL/zeromq3-x
	else
	echo "Update zmq source"
	cd $BASE_EXTERNAL/zeromq3-x
	git pull
	fi
	cd $BASE_EXTERNAL/zeromq3-x
	./autogen.sh
	./configure --prefix=$PREFIX
	make
	make install
	echo "ZMQ Setupped"
fi

echo "Compile !CHAOS"
cd $SCRIPTPATH
if [ -n "$CHAOS32" ]; then
cmake -DCMAKE_BUILD_TYPE=$COMP_TYPE -DCMAKE_INSTALL_PREFIX:PATH=$PREFIX -DBUILD_FORCE_32=true -DBUILD_PREFIX=$PREFIX $SCRIPTPATH/.
else
cmake -DCMAKE_BUILD_TYPE=$COMP_TYPE -DCMAKE_INSTALL_PREFIX:PATH=$PREFIX -DBUILD_PREFIX=$PREFIX $SCRIPTPATH/.
fi
make -j4
make install
