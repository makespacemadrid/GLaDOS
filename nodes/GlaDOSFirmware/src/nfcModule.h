#ifndef RFIDMOD
#define RFIDMOD

void array_to_string(byte array[], unsigned int len, char buffer[])
{
    for (unsigned int i = 0; i < len; i++)
    {
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}

MFRC522 rfid(D8,D3);

class nfcModule
{
public:
	nfcModule()
	{
	}

	virtual void	  setup()									= 0;
	virtual void	  readIdBytes(byte* dest) = 0;

	virtual String  readCard()
	{
		byte cardid[8];
		char cardstr[9];

		for(int i = 0 ; i < 8 ; i++)
		{
			cardid[i] = 0;
		}
		for(int i = 0 ; i < 9 ; i++)
		{
			cardstr[i] = 0;
		}

		readIdBytes(cardid);
		array_to_string(cardid, 8, cardstr);
		return String(cardstr);
	}

protected:

};


class nfcModuleMFRC522 : public nfcModule
{
public:
	nfcModuleMFRC522() : nfcModule()
	{
	}
	virtual void	setup()
	{
		SPI.begin(); // Init SPI bus
		rfid.PCD_Init(); // Init MFRC522
	}


	virtual void	readIdBytes(byte* dest)
	{
		for(int i = 0 ; i < 8 ; i++)
			dest[i] = 0;

		if( (rfid.PICC_IsNewCardPresent()) && (rfid.PICC_ReadCardSerial()))
		{
			Serial.print(F("PICC type: "));
			MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
			Serial.println(rfid.PICC_GetTypeName(piccType));

			Serial.println(F("The NUID tag is:"));
			Serial.print(F("In hex: "));
			dest[0] = rfid.uid.size;
			for (byte i = 0; i < rfid.uid.size; i++) {
				dest[i+1] = rfid.uid.uidByte[i];
				Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
				Serial.print(rfid.uid.uidByte[i], HEX);
			}
			Serial.println();

			// Halt PICC
			rfid.PICC_HaltA();

			// Stop encryption on PCD
			rfid.PCD_StopCrypto1();
		}
//    else
//    {
//        setup();
//    }
	}

protected:

};

#endif
