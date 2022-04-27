//
// Created by Einc on 2022/4/16.
//

#ifndef __MYSSL_H_
#define __MYSSL_H_

#include "head.h"

#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/x509.h"

enum SSL_mode
{
	server,
	client
};

/** Initialize with SSLv23, the parameter value: "server" or "client" */
extern SSL_CTX * initSSL(enum SSL_mode mode);

/** Load the CA certificate of the specified path */
extern bool loadCA(SSL_CTX * ctx,const char * CA_path);

/** Load the User certificate of the specified path */
extern bool loadCert(SSL_CTX * ctx,const char * Cert_path);

/** Load the User Private Key of the specified path */
extern bool loadKey(SSL_CTX * ctx,const char * Key_path);

/** Check whether the user certificate and private key match */
extern bool checkKey(SSL_CTX * ctx);

/** View counterpart certificate */
extern void showPeerCert(SSL * ssl_fd);

/** View Self certificate */
extern void showSelfCert(SSL * ssl_fd);

extern SSL * SSL_fd(SSL_CTX * ctx,int fd);

#endif //__MYSSL_H_
