/*
 * Copyright (c) 2015-2016 brainpower <fbaumgae at haw-landshut dot de>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
*/

#ifndef XSEC_CORE_H
#define XSEC_CORE_H

#ifdef __cplusplus__
#define EXTC extern "C" {
#define CTXE }
#else
#define EXTC
#define CTXE
#endif

#include <string>
#include <vector>


namespace XSec {

#include <xmlsec/keysmngr.h>

class Core;

typedef struct _xsec_sign_options_t    sign_options_t;
typedef struct _xsec_verify_options_t  verify_options_t;
typedef struct _xsec_encrypt_options_t encrypt_options_t;
typedef struct _xsec_decrypt_options_t decrypt_options_t;

typedef struct reference_t {
	int hash;
	int transform;
	std::string uri;
	std::string xpath_intersect;
	std::string xpath_subtract;
	std::string xpath_union;
} Reference;

enum C14NAlgo {
	C14N_UNSET = 0,
	C14N_11_INCLUSIVE,
	C14N_INCLUSIVE,
	C14N_EXCLUSIVE
};

enum SignFormat {
	SF_UNSET = 0,
	SF_ENVELOPED,
	SF_ENVELOPING,
	SF_DETACHED
};

enum EncFormat {
	EF_UNSET = 0,
	EF_ELEMENT,
	EF_CONTENT,
	EF_ROOT
};

enum SignAlgo {
	SA_UNSET=0,
	SA_RSA_SHA1,
	SA_RSA_SHA224,
	SA_RSA_SHA256,
	SA_RSA_SHA384,
	SA_RSA_SHA512,
	SA_ECDSA_SHA1,
	SA_ECDSA_SHA224,
	SA_ECDSA_SHA256,
	SA_ECDSA_SHA384,
	SA_ECDSA_SHA512
};

enum EncAlgo {
	EA_UNSET = 0,
	EA_AES128_CBC,
	EA_AES192_CBC,
	EA_AES256_CBC,
	EA_3DES_CBC
};

enum HashAlgo {
	HA_UNSET = 0,
	HA_SHA1,
	HA_SHA224,
	HA_SHA256,
	HA_SHA384,
	HA_SHA512
};

xmlSecTransformId get_hash_id(int hash_algo);
xmlSecTransformId get_sign_id(int sign_algo);
xmlSecTransformId get_c14n_id(int c14n_algo);
xmlSecTransformId get_enc_id(int enc_algo);
xmlSecKeyDataId get_key_id(int enc_algo);
xmlSecSize get_key_size(int enc_algo);

bool is_ancestor_of(xmlNodePtr anc, xmlNodePtr node);

class Core {
	static const int32_t core_version = 0x00000100;

public:
	Core();
	~Core();

	/**
	 * creates a signature
	 * @param document path to file containing a xml document (default) or document itself
	 * @param options  a set of options
	 */
	int
	sign(const std::string &document, std::string &result, const sign_options_t &options);

	int
	verify( const std::string &document, bool &result, const verify_options_t &options );

	int
	encrypt( const std::string &document, std::string &result, const encrypt_options_t &options );

	int
	decrypt( const std::string &document, std::string &result, const decrypt_options_t &options );

	/*void
	setDefaultKeypair(const std::string &pubkey, const std::string &privkey );

	void
	setDefaultSignAlgoritm(const int algo);

	void
	setDefaultC11nAlgorithm(const std::string &algo);*/

	int32_t
	version() const { return core_version; }

	std::string
	error_message() const { return error_msg; }

	static std::string
	serror_message() { return serror_msg; }

	static std::string
	serror_reset() { serror_msg = std::string(); };

	static bool
	hasSignature( const std::string &file);

	static bool
	isEncrypted(const std::string &file);

private:

	int xerror(const int code, const std::string &msg){
		//FIXME: DEBUG:
		fprintf(stderr, msg.c_str());
		error_msg = msg;
		return error_code = code;
	}

	int default_format = SF_ENVELOPED,
			default_c14n   = C14N_11_INCLUSIVE,
	    default_hash   = HA_SHA256,
	    default_sign   = SA_RSA_SHA256,
	    default_enc_format = EF_ROOT,
	    default_enc    = EA_AES256_CBC;

	std::string error_msg;
	static std::string serror_msg;
	int         error_code;

	xmlSecKeysMngrPtr mngr;

	friend void core_set_error(const char *file, int line, const char *func, const char *errobj, const char *errsbj, int reason, const char *msg);
};

void core_set_error(const char *file, int line, const char *func, const char *errobj, const char *errsbj, int reason, const char *msg);

struct _xsec_verify_options_t {
	bool doc_in_memory = false;
	bool public_key_is_cert = false;
	bool public_key_is_p12 = false;
	bool trust_selfsigned_cert = false;
	std::string base_url;
	std::string public_key;
	//std::string key_password;
};

struct _xsec_sign_options_t {
	/* tells XSecCore if documents and results shall be treated as paths (default; false)
	 * or if they're the actual documents already loaded into memory (true)
	 * In the second case, the content of the result parameter will be overwritten! */
	bool doc_in_memory = false;
	bool public_key_is_cert = false;
	bool keys_in_p12  = false;
	int  format        = 0;
	int  c14n_algorithm= 0;
	int  sign_algorithm= 0;
	int  hash_algorithm= 0;
	std::string private_key;
	std::string public_key;
	std::string key_password;
	std::string base_url;
	std::vector<Reference*> references;
};

struct _xsec_encrypt_options_t {
	int  encryption_algorithm = 0;
	int  encryption_form = 0;
	bool public_key_is_cert = false;
	bool keys_in_p12  = false;
	bool trust_selfsigned_cert = false;

	std::string public_key;
	std::string key_password;
	std::vector<std::string> xpaths;
};

struct _xsec_decrypt_options_t {
	bool private_key_is_p12 = false;
	bool trust_selfsigned_cert = false;
	std::string private_key;
	std::string key_password;
};

} // namespace XSec
#endif
