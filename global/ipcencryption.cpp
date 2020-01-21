//
// ipcEncryption
//
// This module contains the IPCEncryption class and supporting structures.
//
// author: Bryan Waters
//

#include "system.hpp"

struct charInt {
	union {
		unsigned char	nChar[4];
		int				nInt;
	};
};

//
// The IPCEncryption class represents the encryption scheme
//

IPCEncryption::IPCEncryption( IPCClient* pClient, bool bEncrypt ) {
	m_pClient = pClient;

	m_nSeed = rand();
}

void IPCEncryption::Encode( IPCMessage* pMsg ) {
}

IPCMessage* IPCEncryption::Decode( IPCMessage* pMsg ) {
	return pMsg;
}

IPCEncryption*	IPCEncryption::Secure( IPCClient* pClient, bool bEncrypt ) {
	static int nEncryption = 0;

	switch ( nEncryption & 1 ) {
		case 0:
			return new IPCOldEncryption( pClient, bEncrypt );

		case 1:
			return new IPCOldEncryption( pClient, bEncrypt );
	}

	return NULL;
}

//	----------------------------------------------------------------------------------
//	--
//	--	Old encryption scheme nSeed = nSeed * Generator + Incrementor
//	--
//	----------------------------------------------------------------------------------

// This is the code to encrypt a message for my decrypt routine.
unsigned char IPCOldEncryption::codeEncrypt[] = {
	//	Style of code 0 = encryption 1 = decryption
	0x00,

	//	Size of Code/Variable block ( multiple of 16 )
	0x06,

	//	Code starts at address 0x01 ( first byte is stripped at client side )
	0x55,									//		push	ebp
	0x8b, 0xec,								//		mov		ebp, esp
											
	0x50,									//		push	eax
	0x53,									//		push	ebx
	0x56,									//		push	esi
	0x57,									//		push	edi
											
	0x8b, 0xf1,								//		mov		esi, ecx
	0x8b, 0xfa,								//		mov		edi, edx
	0x8b, 0x0e,								//		mov		ecx, DWORD PTR [esi]
											
	0x89, 0x0f,								//		mov		DWORD PTR [edi], ecx
											
	0x83, 0xc6, 0x04,						//		add		esi, 4
	0x83, 0xc7, 0x04,						//		add		edi, 4
	0xc1, 0xe9, 0x02,						//		shr		ecx, 2
															
	0xbb, 0x00, 0x00, 0x00, 0x00,			//		mov		ebx, 0
															
	0xa1, 0x50, 0x00, 0x00, 0x00,			//		mov		eax, DWORD PTR g_nSendSeed
											
											//__Encrypt_Loop:
	0x33, 0x1e,								//		xor		ebx, DWORD PTR [esi]
	0xa5,									//		movsd
											
	0xf7, 0x25, 0x54, 0x00, 0x00, 0x00,		//		mul		DWORD PTR g_nGenerator
	0x05, 0xef, 0xe6, 0x18, 0x01,			//		add		eax, 0x0118e6ef
											
	0x31, 0x47, 0xfc,						//		xor		DWORD PTR [edi-4], eax
											
	0xe0, 0xed,								//		loopnz	__Encrypt_Loop
											
	0xa3, 0x50, 0x00, 0x00, 0x00,			//		mov		DWORD PTR g_nSendSeed, eax
	0x89, 0x1f,								//		mov		DWORD PTR [edi], ebx
															
	0x5f,									//		pop		edi
	0x5e,									//		pop		esi
	0x5b,									//		pop		ebx
	0x58,									//		pop		eax
															
	0x5d,									//		pop		ebp
	0xc3,									//		ret

	//	Pad the code to align the data on 16 byte boundry
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 

	//	Variables start at address 80

	//		g_nSendSeed
	0x00, 0x00, 0x00, 0x00,

	//		g_nGenerator
	0x00, 0x00, 0x00, 0x00,

	//	Pad the data to make the code/data block be a multiple of 16
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

	//	-----------------------------------------------------------------------------
	//	Number of fixups
	0x03,

	//	Offset of the variables in the code segment ( 16 bit word in Little Endian format )
	0x1e, 0x00,
	0x27, 0x00,
	0x36, 0x00,

	//	Number of Procedure calls calling outside the code
	0x00,

	//	Pad to make a multiple of 8
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// This is the code to decrypt a message for my encrypt routine.
unsigned char IPCOldEncryption::codeDecrypt[] = {
	//	Style of code 0 = encryption 1 = decryption
	0x01,

	//	Size of Code/Variable block ( multiple of 16 )
	0x0B,

	//	Code starts at address 0x01 ( first byte is stripped at client side )
	0x55,									//		push	ebp
	0x8B, 0xEC,								//		mov		ebp, esp
													
	0x50,									//		push	eax
	0x53,									//		push	ebx
	0x56,									//		push	esi
	0x57,									//		push	edi
													
	0xC1, 0xE9, 0x02,						//		shr		ecx, 2
															
	0xA1, 0x90, 0x00, 0x00, 0x00,			//		mov		eax, DWORD PTR g_nReceiveSeed
															
	0x8B, 0xf2,								//		mov		esi, edx
	0x8B, 0x1E,								//		mov		ebx, DWORD PTR [esi]
															
	0xF7, 0x25, 0x94, 0x00, 0x00, 0x00,		//		mul		DWORD PTR g_nGenerator
	0x05, 0xEF, 0xe6, 0x18, 0x01,			//		add		eax, 18409199

	0x31, 0x06,								//		xor		DWORD PTR [esi], eax

	0xF7, 0x25, 0x94, 0x00, 0x00, 0x00,		//		mul		DWORD PTR g_nGenerator
	0x05, 0xEF, 0xE6, 0x18, 0x01,			//		add		eax, 18409199

	0x31, 0x46, 0x04,						//		xor		DWORD PTR [esi+4], eax
	0x33, 0x5E, 0x04,						//		xor		ebx, DWORD PTR [esi+4]

	0xF7, 0x25, 0x94, 0x00, 0x00, 0x00,		//		mul		DWORD PTR g_nGenerator
	0x05, 0xEF, 0xE6, 0x18, 0x01,			//		add		eax, 18409199

	0x31, 0x46, 0x08,						//		xor		DWORD PTR [esi+8], eax
	0x33, 0x5E, 0x08,						//		xor		ebx, DWORD PTR [esi+8]

	0xA3, 0x90, 0x00, 0x00, 0x00,			//		mov		DWORD PTR g_nReceiveSeed, eax
	0x83, 0xF9, 0x04,						//		cmp		ecx, 4
	0x7C, 0x3d,								//		jl		SHORT __decode_done
															
	0xF7, 0x25, 0x94, 0x00, 0x00, 0x00,		//		mul		DWORD PTR g_nGenerator
	0x05, 0xEF, 0xE6, 0x18, 0x01,			//		add		eax, 18409199

	0x31, 0x46, 0x0C,						//		xor		DWORD PTR [esi+12], eax
	0x33, 0x5E, 0x0C,						//		xor		ebx, DWORD PTR [esi+12]

	0xA3, 0x90, 0x00, 0x00, 0x00,			//		mov		DWORD PTR g_nReceiveSeed, eax
	0x83, 0xF9, 0x05,						//		cmp		ecx, 5
	0x7C, 0x22,								//		jl		__decode_done
															
	0x83, 0xC6, 0x10,						//		add		esi, 16
	0x83, 0xE9, 0x05,						//		sub		ecx, 5
	0x74, 0x07,								//		je		__decode_skip
	
											//__decode_loop:
	0x33, 0x1E,								//		xor		ebx, DWORD PTR [esi]
	0x83, 0xC6, 0x04,						//		add		esi, 4
	0xE0, 0xF9,								//		loopnz	__decode_loop
											
											//__decode_skip:
	0x3B, 0x1E,								//		cmp		ebx, DWORD PTR [esi]
	0x74, 0x0F,								//		je		__decode_done
															
	0xB9, 0x10, 0x00, 0x00, 0x00,			//		mov		ecx, 16
	0xBA, 0x98, 0x00, 0x00, 0x00,			//		mov		edx, g_nFastMessage
	0xE8, 0x00, 0x00, 0x00, 0x00,			//		call	CNetFastSend
	
											//__decode_done:
	0x5F,									//		pop		edi
	0x5E,									//		pop		esi
	0x5B,									//		pop		ebx
	0x58,									//		pop		eax
															
	0x5D,									//		pop		ebp
	0xC3,									//		ret
	
	//	Pad the code to align the data on 16 byte boundry
	0x00,

	//	Variables start at address 160
	
	//		g_nSeed
	0x00, 0x00, 0x00, 0x00,
	
	//		g_nGenerator
	0x00, 0x00, 0x00, 0x00,

	//		g_nFastMessage
	0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

	//	Pad the code to align the data on 16 byte boundry
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

	//	-----------------------------------------------------------------------------
	//	Number of fixups
	0x08,

	//	Offset of the variables in the code segment ( 16 bit word in Little Endian format )
	0x0B, 0x00,
	0x15, 0x00,
	0x22, 0x00,
	0x33, 0x00,
	0x43, 0x00,
	0x4E, 0x00,
	0x5E, 0x00,
	0x80, 0x00,

	//	Number of Procedure calls calling outside the code
	0x01,

	//	Offset of the Procedure call in the code segment ( 16 bit word in Little Endian format ) / Which procedure
	0x85, 0x00, 0x01,

	//	Pad to make a multiple of 8
	0x00
};

IPCOldEncryption::IPCOldEncryption( IPCClient* pClient, bool bEncrypt ) : IPCEncryption( pClient, bEncrypt ) {
	charInt nValue;

//	m_nGenerator = g_nPrimes [ ( rand() & 0x0000ffff ) ];
//	m_nIncrementor = rand() | 1;

	m_nGenerator = 0x000180EF;
	m_nIncrementor = 1;
	
	PackedData response;

	if ( bEncrypt ) {
		nValue.nInt = m_nIncrementor;

		codeDecrypt[ 28 ] = codeDecrypt[ 41 ] = codeDecrypt[ 58 ] = codeDecrypt[ 85 ] = nValue.nChar[ 0 ];
		codeDecrypt[ 29 ] = codeDecrypt[ 42 ] = codeDecrypt[ 59 ] = codeDecrypt[ 86 ] = nValue.nChar[ 1 ];
		codeDecrypt[ 30 ] = codeDecrypt[ 43 ] = codeDecrypt[ 60 ] = codeDecrypt[ 87 ] = nValue.nChar[ 2 ];
		codeDecrypt[ 31 ] = codeDecrypt[ 44 ] = codeDecrypt[ 61 ] = codeDecrypt[ 88 ] = nValue.nChar[ 3 ];

		nValue.nInt = m_nSeed;

		codeDecrypt[ 146 ] = nValue.nChar[ 0 ];
		codeDecrypt[ 147 ] = nValue.nChar[ 1 ];
		codeDecrypt[ 148 ] = nValue.nChar[ 2 ];
		codeDecrypt[ 149 ] = nValue.nChar[ 3 ];

		nValue.nInt = m_nGenerator;

		codeDecrypt[ 150 ] = nValue.nChar[ 0 ];
		codeDecrypt[ 151 ] = nValue.nChar[ 1 ];
		codeDecrypt[ 152 ] = nValue.nChar[ 2 ];
		codeDecrypt[ 153 ] = nValue.nChar[ 3 ];

		response.putEncryptedString ( codeDecrypt, sizeof( codeDecrypt ) );
	} else {
		nValue.nInt = m_nIncrementor;

		codeEncrypt[ 46 ] = nValue.nChar[ 0 ];
		codeEncrypt[ 47 ] = nValue.nChar[ 1 ];
		codeEncrypt[ 48 ] = nValue.nChar[ 2 ];
		codeEncrypt[ 49 ] = nValue.nChar[ 3 ];

		nValue.nInt = m_nSeed;

		codeEncrypt[ 82 ] = nValue.nChar[ 0 ];
		codeEncrypt[ 83 ] = nValue.nChar[ 1 ];
		codeEncrypt[ 84 ] = nValue.nChar[ 2 ];
		codeEncrypt[ 85 ] = nValue.nChar[ 3 ];

		nValue.nInt = m_nGenerator;

		codeEncrypt[ 86 ] = nValue.nChar[ 0 ];
		codeEncrypt[ 87 ] = nValue.nChar[ 1 ];
		codeEncrypt[ 88 ] = nValue.nChar[ 2 ];
		codeEncrypt[ 89 ] = nValue.nChar[ 3 ];

		response.putEncryptedString ( codeEncrypt, sizeof( codeEncrypt ) );
	}

	IPCMessage* pMsg = new IPCMessage ( _IPC_SERVER_CONNECTED, (char *)response.data(), response.size(), m_pClient );

	m_pClient->addMsg ( pMsg );
}

void IPCOldEncryption::Encode( IPCMessage* pMsg ) {
	int* pBuf = (int*) pMsg->data();

	int nMask = ( pMsg->type() ^= ( m_nSeed = m_nSeed * m_nGenerator + m_nIncrementor ) );

	int nSize = pMsg->size();

	nMask ^= pBuf[ 0 ];
	pBuf[ 0 ] ^= ( m_nSeed = m_nSeed * m_nGenerator + m_nIncrementor );

	nMask ^= pBuf[ 1 ];
	pBuf[ 1 ] ^= ( m_nSeed = m_nSeed * m_nGenerator + m_nIncrementor );

	if ( nSize > 8 ) {
		nMask ^= pBuf[ 2 ];
		pBuf[ 2 ] ^= ( m_nSeed = m_nSeed * m_nGenerator + m_nIncrementor );

		if ( nSize > 12 ) {
			nSize = ( nSize >> 2 ) - 1;

			int nEncode;

			for (nEncode = 3;nEncode < nSize;nEncode++) {
				nMask ^= pBuf[ nEncode ];
			}

			pBuf[ nEncode ] = nMask;
		}
	}
}

IPCMessage* IPCOldEncryption::Decode( IPCMessage* pMsg ) {
	int nMask = pMsg->type() ^= ( m_nSeed = m_nSeed * m_nGenerator + m_nIncrementor );

	int nSize = ( pMsg->size() >> 2 );

	int* pBuf = (int*) pMsg->data();

	int nEncrypt;
	
	for (nEncrypt = 0;nEncrypt < nSize;nEncrypt++) {
		pBuf[ nEncrypt ] ^= ( m_nSeed = m_nSeed * m_nGenerator + m_nIncrementor );

		nMask ^= pBuf[ nEncrypt ];
	}

	if ( nSize > 4 && nMask != pBuf[ nEncrypt ] ) {
		delete pMsg;

		PackedData packet;
		packet.putByte ( 1 );

		int clientIP = m_pClient->IP();
		logInfo( _LOG_ALWAYS, "IPCOldEncryption::Decode - detected malformed packet. Checksum failure. IP is %d.%d.%d.%d", (clientIP & 0xFF000000) >> 24, (clientIP & 0x00FF0000) >> 16, (clientIP & 0x0000FF00) >> 8, (clientIP & 0x000000FF) );
 		
		pMsg = new IPCMessage ( _IPC_CLIENT_HACKED_MSG, (IPCPMMessage *)packet.data(), packet.size(), 0, this );
	}

	return pMsg;
}
