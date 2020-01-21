#ifndef _PACKMSG_HPP_
#define _PACKMSG_HPP_

#include "packdata.hpp"

#include "new.hpp"

class PackedMsg : public PackedData
{
public:
	PackedMsg() {
		// put the header
		putLong ( 0 );
		putLong ( 0 );
	};

	PackedMsg ( char *ptr, int size ) : PackedData ( ptr, size ) {
	};

	// put special NAK format data
	void putNAKInfo ( int nak, int info = 0 ) {
		putLong ( nak );
		putLong ( info );
	};

	// put special ACK format data
	void putACKInfo ( int ack, int info = 0 ) {
		putLong ( ack );
		putLong ( info );
	};

	// put special ACK format data
	void putBLE_ACKInfo ( int ack, int info = 0 ) {
		putBLE_Long ( ack );
		putBLE_Long ( info );
	};
};

#endif
