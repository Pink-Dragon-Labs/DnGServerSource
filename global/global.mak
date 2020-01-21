GLOBALDIR 	= .
CPPINCS 	+= -I$(GLOBALDIR)

GLOBALOBJS = 	$(GLOBALDIR)/ipc.o $(GLOBALDIR)/file.o $(GLOBALDIR)/tools.o \
		$(GLOBALDIR)/fatal.o $(GLOBALDIR)/window.o $(GLOBALDIR)/ipcpollmgr.o \
		$(GLOBALDIR)/configmgr.o $(GLOBALDIR)/logmgr.o $(GLOBALDIR)/ipcclient.o\
		$(GLOBALDIR)/list.o $(GLOBALDIR)/crash.o $(GLOBALDIR)/ipcmsg.o \
		$(GLOBALDIR)/counter.o $(GLOBALDIR)/cgi.o $(GLOBALDIR)/html.o \
		$(GLOBALDIR)/player.o $(GLOBALDIR)/array.o $(GLOBALDIR)/ipcserver.o \
		$(GLOBALDIR)/packdata.o $(GLOBALDIR)/hash.o $(GLOBALDIR)/new.o \
		$(GLOBALDIR)/malloc.o $(GLOBALDIR)/sparse.o $(GLOBALDIR)/tree.o \
		$(GLOBALDIR)/string.o $(GLOBALDIR)/memorypool.o $(GLOBALDIR)/sql.o \
		$(GLOBALDIR)/script.o $(GLOBALDIR)/msgs.o $(GLOBALDIR)/ipcencryption.o \
		$(GLOBALDIR)/generator.o $(GLOBALDIR)/validate.o $(GLOBALDIR)/cencryptionstring.o \
		$(GLOBALDIR)/stringcache.o

LINKLINE += -lmysqlclient 
