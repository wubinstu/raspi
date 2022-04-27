//
// Created by Einc on 2022/4/16.
//

#include "myssl.h"
#include "msgd.h"

static int SSL_logLevel = LOG_ERR;



SSL_CTX * initSSL(enum SSL_mode mode)
{
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	if(mode == server)
		return SSL_CTX_new(SSLv23_server_method());
	if(mode == client)
		return SSL_CTX_new(SSLv23_client_method());
	else return NULL;
}

bool loadCA(SSL_CTX * ctx,const char * CA_path)
{
	int status = SSL_CTX_load_verify_locations (ctx,CA_path,NULL);
	if(status <= 0)
	{
		perr_d (true,SSL_logLevel,"CA cert load failed");
		ERR_print_errors_fp(stdout);
		return false;
	}
	SSL_CTX_set_verify (ctx,SSL_VERIFY_PEER,NULL);
	return true;
}

bool loadCert(SSL_CTX * ctx,const char * Cert_path)
{
	int status = SSL_CTX_use_certificate_file(ctx,Cert_path,X509_FILETYPE_PEM);
	if(status <= 0)
	{
		perr_d (true,SSL_logLevel,"User cert load failed");
		ERR_print_errors_fp(stdout);
		return false;
	}
	return true;
}

bool loadKey(SSL_CTX * ctx,const char * Key_path)
{
	int status = SSL_CTX_use_PrivateKey_file(ctx,Key_path,SSL_FILETYPE_PEM);
	if(status <= 0)
	{
		perr_d (true,SSL_logLevel,"User key load failed");
		ERR_print_errors_fp(stdout);
		return false;
	}
	return true;
}

bool checkKey(SSL_CTX * ctx)
{
	int status = SSL_CTX_check_private_key (ctx);
	if(status <= 0)
	{
		perr_d (true,SSL_logLevel,"User Private key check failed");
		ERR_print_errors_fp(stdout);
		return false;
	}
	return true;
}

void showPeerCert(SSL * ssl_fd)
{
	X509 * cert;
	char * line;
	int show_level = LOG_INFO;

	cert = SSL_get_peer_certificate(ssl_fd);
	if (cert != NULL)
	{
		perr_d(true,show_level,"Digital certificate information:");
		line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
		perr_d(true,show_level,"Certificate: %s", line);
		free(line);
		line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
		perr_d(true,show_level,"Issuer: %s", line);
		free(line);
		X509_free(cert);
	}
	else
		perr_d(true,show_level,"No certificate information!");
}

void showSelfCert(SSL * ssl_fd)
{
	X509 * cert;
	char * line;
	int show_level = LOG_INFO;

	cert = SSL_get_certificate(ssl_fd);
	if (cert != NULL)
	{
		perr_d(true,show_level,"Digital certificate information:");
		line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
		perr_d(true,show_level,"Certificate: %s", line);
		free(line);
		line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
		perr_d(true,show_level,"Issuer: %s", line);
		free(line);
		X509_free(cert);
	}
	else
		perr_d(true,show_level,"No certificate information!");
}

SSL * SSL_fd(SSL_CTX * ctx,int fd)
{
	SSL * ssl_fd = SSL_new (ctx);
	int status = SSL_set_fd (ssl_fd,fd);
	if(status == -1)
	{
		perr_d (true,SSL_logLevel,"SSL fd create failed");
		ERR_print_errors_fp(stdout);
		return NULL;
	}
	return ssl_fd;
}