#include <Arduino.h>
#include <Crypto.h>
#include <AES.h>
#include <GCM.h>

struct TestVector
{
    const char *name;
    uint8_t key[32];
    uint8_t authdata[20];
    uint8_t iv[12];
    uint8_t tag[16];
    size_t authsize;
    size_t tagsize;
    size_t ivsize;
};

static TestVector const testVectorGCM PROGMEM = {
    .name        = "AES-256 GCM",
    .key         = {0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
                    0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
                    0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
                    0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08},
    .authdata    = {0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
                    0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
                    0xab, 0xad, 0xda, 0xd2},
    .iv          = {0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad,
                    0xde, 0xca, 0xf8, 0x88},
    .tag         = {0x76, 0xfc, 0x6e, 0xce, 0x0f, 0x4e, 0x17, 0x68,
                    0xcd, 0xdf, 0x88, 0x53, 0xbb, 0x2d, 0x55, 0x1b},
    .authsize    = 20,
    .tagsize     = 16,
    .ivsize      = 12
};

TestVector testVector;
byte buffer[128];

void encrypt(AuthenticatedCipher *cipher, const struct TestVector *test, size_t datasize)
{
    memcpy_P(&testVector, test, sizeof(TestVector));
    test = &testVector;
    size_t posn, len;
    uint8_t tag[16];
    crypto_feed_watchdog();

    cipher->clear();
    cipher->setKey(test->key, cipher->keySize());
    cipher->setIV(test->iv, test->ivsize);
    for (posn = 0; posn < test->authsize; posn += datasize) {
        len = test->authsize - posn;
        if (len > datasize)
            len = datasize;
        cipher->addAuthData(test->authdata + posn, len);
    }
    Serial.print("Cisla: ");
    for(uint8_t i=0; i<60; i++) Serial.printf("%X",buffer[i]);
    Serial.println();

    for (posn = 0; posn < datasize; posn += datasize) {
        len = datasize - posn;
        if (len > datasize) len = datasize;
        crypto_feed_watchdog();
        cipher->encrypt((uint8_t*)buffer + posn, buffer + posn, len);
        crypto_feed_watchdog();
    }
    Serial.println("Vystup:\n");
    //      Decrypt
    for(uint8_t i=0; i<sizeof(buffer); i++) printf("0x%X,",buffer[i]);
}

void decrypt(AuthenticatedCipher *cipher, const struct TestVector *test, size_t datasize){
    bool ok;

    memcpy_P(&testVector, test, sizeof(TestVector));
    test = &testVector;
    size_t posn, len;
    uint8_t tag[16];
    crypto_feed_watchdog();
    cipher->clear();
    cipher->setKey(test->key, cipher->keySize());
    cipher->setIV(test->iv, test->ivsize);
    for (posn = 0; posn < test->authsize; posn += datasize) {
        len = test->authsize - posn;
        if (len > datasize)
            len = datasize;
        cipher->addAuthData(test->authdata + posn, len);
    }

    for (posn = 0; posn < datasize; posn += datasize) {
        len = datasize - posn;
        if (len > datasize)
            len = datasize;
        cipher->decrypt((uint8_t*)buffer + posn, buffer + posn, len);
    }

    Serial.print("\nVystup: ");
    for(uint8_t i=0; i<60;i++) Serial.printf("%c",(char)buffer[i]);
    Serial.println();

}


void setup(){
    Serial.begin(115200);
    Serial.println("Začínam\n");
    GCM <AES256> *gcm=0;
    gcm= new GCM<AES256>();
    gcm->setKey(testVectorGCM.key, 32);
    gcm->setIV(testVectorGCM.iv,testVectorGCM.ivsize);
    crypto_feed_watchdog();
    memset(buffer, (int)'\0', sizeof(buffer));
    char vstup[30]="Tak to netuším";     //String containing Non-Asci characters
    for(uint8_t i=0; i<30; i++) buffer[i]=vstup[i]; 
    encrypt(gcm,&testVectorGCM,30);
    decrypt(gcm,&testVectorGCM,30);
    delete gcm;
}

void loop(){
    delay(1000);
    Serial.print(".");
}
