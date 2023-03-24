//
// Created by Einc on 2022/4/16.
//

#ifndef __RSSL_H_
#define __RSSL_H_

#include "head.h"
#include "types.h"

#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/x509.h"


/** Initialize with SSLv23 */
extern SSL_CTX * initSSL (bool true_for_server_false_for_client);

/** server always load ca cert,user cert,user private key
 * but client need not,client can load 1.NONE, 2.ca, 3.ca_cert & user_cert & user_key */
extern void setVerifyPeer (SSL_CTX * sslCtx, bool verifyPeer);

/** Load the CA certificate of the specified path */
extern bool loadCA (SSL_CTX * sslCtx, const char * CA_path);

/** Load the User certificate of the specified path */
extern bool loadCert (SSL_CTX * sslCtx, const char * Cert_path);

/** Load the User Private Key of the specified path */
extern bool loadKey (SSL_CTX * sslCtx, const char * Key_path);

/** Check whether the user certificate and private key match */
extern bool checkKey (SSL_CTX * sslCtx);

/** View counterpart certificate */
extern void showPeerCert (SSL * ssl_fd);

/** View Self certificate */
extern void showSelfCert (SSL * ssl_fd);

extern SSL * SSL_fd (SSL_CTX * sslCtx, int fd);

#endif //__RSSL_H_
