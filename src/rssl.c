//
// Created by Einc on 2022/4/16.
//

#include "global.h"
#include "rssl.h"
#include "log.h"


SSL_CTX *initSSL ()
{
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    if (mode_serv_clnt == server)
        return SSL_CTX_new (SSLv23_server_method ());
    if (mode_serv_clnt == client)
        return SSL_CTX_new (SSLv23_client_method ());
    return NULL;
}

void setClientVerify (SSL_CTX *sslCtx)
{
    if (mode_ssl_client == client_empty || mode_ssl_client == client_only_ca)
        SSL_CTX_set_verify (sslCtx, SSL_VERIFY_NONE, NULL);
    if (mode_ssl_client == client_with_cert_key)
        SSL_CTX_set_verify (sslCtx, SSL_VERIFY_PEER, NULL);
}

bool loadCA (SSL_CTX *sslCtx, const char *CA_path)
{
    errno = 0;
    int status = SSL_CTX_load_verify_locations (sslCtx, CA_path, NULL);
    if (status <= 0)
    {
        perr_d (true, rssl_logLevel, "CA cert load failed");
        ERR_print_errors_fp (stdout);
        return false;
    }
    return true;
}

bool loadCert (SSL_CTX *sslCtx, const char *Cert_path)
{
    errno = 0;
    int status = SSL_CTX_use_certificate_file (sslCtx, Cert_path, X509_FILETYPE_PEM);
    if (status <= 0)
    {
        perr_d (true, rssl_logLevel, "User cert load failed");
        ERR_print_errors_fp (stdout);
        return false;
    }
    return true;
}

bool loadKey (SSL_CTX *sslCtx, const char *Key_path)
{
    errno = 0;
    int status = SSL_CTX_use_PrivateKey_file (sslCtx, Key_path, SSL_FILETYPE_PEM);
    if (status <= 0)
    {
        perr_d (true, rssl_logLevel, "User key load failed");
        ERR_print_errors_fp (stdout);
        return false;
    }
    return true;
}

bool checkKey (SSL_CTX *sslCtx)
{
    errno = 0;
    int status = SSL_CTX_check_private_key (sslCtx);
    if (status <= 0)
    {
        perr_d (true, rssl_logLevel, "User Private key check failed");
        ERR_print_errors_fp (stdout);
        return false;
    }
    return true;
}

void showPeerCert (SSL *ssl_fd)
{
    errno = 0;
    X509 *cert;
    char *line;
    int show_level = LOG_INFO;

    cert = SSL_get_peer_certificate (ssl_fd);
    if (cert != NULL)
    {
        perr_d (true, show_level, "Digital certificate information:");
        line = X509_NAME_oneline (X509_get_subject_name (cert), 0, 0);
        perr_d (true, show_level, "Certificate: %s", line);
        free (line);
        line = X509_NAME_oneline (X509_get_issuer_name (cert), 0, 0);
        perr_d (true, show_level, "Issuer: %s", line);
        free (line);
        X509_free (cert);
    } else perr_d (true, show_level, "No certificate information!");
}

void showSelfCert (SSL *ssl_fd)
{
    errno = 0;
    X509 *cert;
    char *line;
    int show_level = LOG_INFO;

    cert = SSL_get_certificate (ssl_fd);
    if (cert != NULL)
    {
        perr_d (true, show_level, "Digital certificate information:");
        line = X509_NAME_oneline (X509_get_subject_name (cert), 0, 0);
        perr_d (true, show_level, "Certificate: %s", line);
        free (line);
        line = X509_NAME_oneline (X509_get_issuer_name (cert), 0, 0);
        perr_d (true, show_level, "Issuer: %s", line);
        free (line);
        X509_free (cert);
    } else perr_d (true, show_level, "No certificate information!");
}

SSL *SSL_fd (SSL_CTX *sslCtx, int fd)
{
    errno = 0;
    SSL *ssl_fd = SSL_new (sslCtx);
    int status = SSL_set_fd (ssl_fd, fd);
    if (status == -1)
    {
        perr_d (true, rssl_logLevel, "SSL fd create failed");
        ERR_print_errors_fp (stdout);
        return NULL;
    }
    return ssl_fd;
}