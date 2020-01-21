.PRECIOUS: *.cpp *.hpp

# Macro definitions (switches and the like)
#
DFLAGS		=
CPPINCS		= -L./global -I./global -I/usr/include/mysql -L/usr/lib/mysql -I /usr/kerberos/include
CPPFLAGS	= -ggdb3 -Wwrite-strings

CPP		= g++ $(DFLAGS) $(CPPFLAGS) $(CPPINCS) $(CPPDEFINES)
PROOFEDCPP	= proof CC $(DFLAGS) $(CPPFLAGS) $(CPPINCS) $(CPPDEFINES)
CPPDEFINES	= -D _UNIX_ 

#
# Compilation rules
#
.SUFFIXES:
.SUFFIXES: .o .cpp

.cpp.o:
	$(CPP) -c $<
