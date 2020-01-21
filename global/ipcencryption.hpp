//
// ipcEncryption
//

#ifndef _IPCENCRYPTION_HPP_
#define _IPCENCRYPTION_HPP_

// declare these classes so compile errors are avoided
class IPCClient;
class IPCMessage;
class PackedData;

//
// The IPCEncryption class represents the encryption scheme
//

class IPCEncryption {
protected:
	IPCClient*		m_pClient;

	unsigned int	m_nSeed;

public:
	IPCEncryption( IPCClient* pClient, bool bEncrypt );

	virtual void			Encode( IPCMessage* pMsg );
	virtual IPCMessage*		Decode( IPCMessage* pMsg );

	static IPCEncryption*	Secure( IPCClient* pClient, bool bEncrypt );
};

class IPCOldEncryption : public IPCEncryption {
protected:
	static unsigned char codeEncrypt[];
	static unsigned char codeDecrypt[];

	unsigned int	m_nGenerator;
	unsigned int	m_nIncrementor;

public:
	IPCOldEncryption( IPCClient* pClient, bool bEncrypt );

	virtual void			Encode( IPCMessage* pMsg );
	virtual IPCMessage*		Decode( IPCMessage* pMsg );

};

#endif
