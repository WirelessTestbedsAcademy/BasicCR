FROM ubuntu:14.04
MAINTAINER "Mikolaj Chwalisz" <chwalisz@tkn.tu-berlin.de>


# TinyOS toolchain
#  It was created following http://tinyos.stanford.edu/tinyos-wiki/index.php/Automatic_installation
RUN gpg --keyserver keyserver.ubuntu.com --recv-keys A9B913B9
RUN gpg -a --export A9B913B9 | apt-key add -

RUN echo "deb http://tinyprod.net/repos/debian wheezy main" >> /etc/apt/sources.list.d/tinyprod-debian.list
RUN echo "deb http://tinyprod.net/repos/debian msp430-46 main" >> /etc/apt/sources.list.d/tinyprod-debian.list

RUN apt-get update
RUN apt-get install -y nesc tinyos-tools-devel gcc-msp430

# Other packages
RUN apt-get install -y git-core build-essential

RUN locale-gen en_US.UTF-8

# SSH
RUN apt-get install -y openssh-server sudo
RUN useradd --create-home --shell /bin/bash wta
RUN echo 'wta ALL=(ALL) NOPASSWD:ALL' > /etc/sudoers.d/10-wta-sudo
RUN mkdir -p /var/run/sshd

RUN echo "wta:crew" | chpasswd

EXPOSE 22


# TinyOS

RUN git clone --depth=1 https://github.com/tinyos/tinyos-main.git /home/wta/tinyos-main
RUN chown wta:wta -R /home/wta/tinyos-main

ENV TOSROOT="/home/wta/tinyos-main"
ENV TOSDIR="$TOSROOT/tos"
ENV CLASSPATH=$CLASSPATH:$TOSROOT/support/sdk/java
ENV MAKERULES="$TOSROOT/support/make/Makerules"
ENV PYTHONPATH=$PYTHONPATH:$TOSROOT/support/sdk/python


# Wireless Testbed Academy
RUN git clone https://github.com/WirelessTestbedsAcademy/BasicCR.git /home/wta/BasicCR
RUN chown wta:wta -R /home/wta/BasicCR

CMD  /usr/sbin/sshd -D
