FROM cmhulbert/dwarvesandgiants:1.1

RUN apt install -y gcc g++ make cmake libstdc++5:i386

ENV SERVER_SOURCE /root/serversource
ENV SERVER_BUILD $SERVER_SOURCE/bin

COPY . $SERVER_SOURCE

WORKDIR $SERVER_SOURCE/integrationScripts
ENTRYPOINT ["bash", "buildCopyRun.sh"]
