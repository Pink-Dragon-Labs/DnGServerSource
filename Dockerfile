FROM cmhulbert/dwarvesandgiants:latest

RUN apt install -y gcc g++ gcc-multilib g++-multilib make cmake gdb valgrind git 
#libstdc++5:i386

ENV SERVER_SOURCE /root/serversource
ENV SERVER_BUILD $SERVER_SOURCE/bin

COPY . $SERVER_SOURCE

WORKDIR $SERVER_SOURCE/integrationScripts
ENTRYPOINT ["bash", "buildCopyRun.sh"]
