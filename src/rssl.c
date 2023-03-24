//
// Created by Einc on 2022/4/16.
//

#include "global.h"
#include "rssl.h"
#include "log.h"


SSL_CTX * initSSL (bool true_for_server_false_for_client)
{
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    if (true_for_server_false_for_client)
        return SSL_CTX_new (SSLv23_server_method ());
    else return SSL_CTX_new (SSLv23_client_method ());
}

void setVerifyPeer (SSL_CTX * sslCtx, bool verifyPeer)
{
    if (verifyPeer)
        SSL_CTX_set_verify (sslCtx, SSL_VERIFY_PEER, NULL);
    else SSL_CTX_set_verify (sslCtx, SSL_VERIFY_NONE, NULL);
}

bool loadCA (SSL_CTX * sslCtx, const char * CA_path)
{
    int errno_save = errno;
    int status = SSL_CTX_load_verify_locations (sslCtx, CA_path, NULL);
    if (status <= 0)
    {
        perr (true, rssl_logLevel, "CA cert load failed");
        ERR_print_errors_fp (stdout);
        return false;
    }
    errno = errno_save;
    return true;
}

bool loadCert (SSL_CTX * sslCtx, const char * Cert_path)
{
    int errno_save = errno;
    int status = SSL_CTX_use_certificate_file (sslCtx, Cert_path, X509_FILETYPE_PEM);
    if (status <= 0)
    {
        perr (true, rssl_logLevel, "User cert load failed");
        ERR_print_errors_fp (stdout);
        return false;
    }
    errno = errno_save;
    return true;
}

bool loadKey (SSL_CTX * sslCtx, const char * Key_path)
{
    int errno_save = errno;
    int status = SSL_CTX_use_PrivateKey_file (sslCtx, Key_path, SSL_FILETYPE_PEM);
    if (status <= 0)
    {
        perr (true, rssl_logLevel, "User key load failed");
        ERR_print_errors_fp (stdout);
        return false;
    }
    errno = errno_save;
    return true;
}

bool checkKey (SSL_CTX * sslCtx)
{
    int errno_save = errno;
    int status = SSL_CTX_check_private_key (sslCtx);
    if (status <= 0)
    {
        perr (true, rssl_logLevel, "User Private key check failed");
        ERR_print_errors_fp (stdout);
        return false;
    }
    errno = errno_save;
    return true;
}

void showPeerCert (SSL * ssl_fd)
{
    int errno_save = errno;
    X509 * cert;
    char * line;
    int show_level = LOG_INFO;

    cert = SSL_get_peer_certificate (ssl_fd);
    if (cert != NULL)
    {
        perr (true, show_level, "Digital certificate information:");
        line = X509_NAME_oneline (X509_get_subject_name (cert), 0, 0);
        perr (true, show_level, "Certificate: %s", line);
        free (line);
        line = X509_NAME_oneline (X509_get_issuer_name (cert), 0, 0);
        perr (true, show_level, "Issuer: %s", line);
        free (line);
        X509_free (cert);
    } else perr (true, show_level, "No certificate information!");
    errno = errno_save;
}

void showSelfCert (SSL * ssl_fd)
{
    int errno_save = errno;
    X509 * cert;
    char * line;
    int show_level = LOG_INFO;

    cert = SSL_get_certificate (ssl_fd);
    if (cert != NULL)
    {
        perr (true, show_level, "Digital certificate information:");
        line = X509_NAME_oneline (X509_get_subject_name (cert), 0, 0);
        perr (true, show_level, "Certificate: %s", line);
        free (line);
        line = X509_NAME_oneline (X509_get_issuer_name (cert), 0, 0);
        perr (true, show_level, "Issuer: %s", line);
        free (line);
        X509_free (cert);
    } else perr (true, show_level, "No certificate information!");
    errno = errno_save;
}

SSL * SSL_fd (SSL_CTX * sslCtx, int fd)
{
    int errno_save = errno;
    SSL * ssl_fd = SSL_new (sslCtx);
    int status = SSL_set_fd (ssl_fd, fd);
    if (status == -1)
    {
        perr (true, rssl_logLevel, "SSL fd create failed");
        ERR_print_errors_fp (stdout);
        return NULL;
    }
    errno = errno_save;
    return ssl_fd;
}