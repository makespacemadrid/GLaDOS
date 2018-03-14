#ifndef RFIDMOD
#define RFIDMOD

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
		byte id[8];
		readIdBytes(id);
		String val;
		return val;
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
