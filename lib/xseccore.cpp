#include <string.h>
#include "xseccore.hpp"

namespace XSec {

#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/uri.h>

#include <libxslt/xslt.h>
#include <libxslt/security.h>

#include <xmlsec/xmlsec.h>
#include <xmlsec/xmltree.h>
#include <xmlsec/xmldsig.h>
#include <xmlsec/xmlenc.h>
#include <xmlsec/keyinfo.h>
#include <xmlsec/templates.h>
#include <xmlsec/crypto.h>
#include <xmlsec/errors.h>

#include <openssl/evp.h>



#define PEM_BOUNDARY_FORMAT "-----BEGIN CERTIFICATE-----\n%s\n-----END CERTIFICATE-----\n"
#define PEM_BOUNDARY_SIZE sizeof(PEM_BOUNDARY_FORMAT)

Core::Core() {

	xmlInitParser();
	LIBXML_TEST_VERSION;

	xmlLoadExtDtdDefaultValue = XML_DETECT_IDS | XML_COMPLETE_ATTRS;
	xmlSubstituteEntitiesDefault( 1 );
	xmlIndentTreeOutput = 1;

	/* disable everything in xslt which might be harmful */
	auto xsltSecPrefs = xsltNewSecurityPrefs();
	xsltSetSecurityPrefs( xsltSecPrefs, XSLT_SECPREF_READ_FILE, xsltSecurityForbid );
	xsltSetSecurityPrefs( xsltSecPrefs, XSLT_SECPREF_WRITE_FILE, xsltSecurityForbid );
	xsltSetSecurityPrefs( xsltSecPrefs, XSLT_SECPREF_CREATE_DIRECTORY, xsltSecurityForbid );
	xsltSetSecurityPrefs( xsltSecPrefs, XSLT_SECPREF_READ_NETWORK, xsltSecurityForbid );
	xsltSetSecurityPrefs( xsltSecPrefs, XSLT_SECPREF_WRITE_NETWORK, xsltSecurityForbid );
	xsltSetDefaultSecurityPrefs( xsltSecPrefs );

	if( xmlSecInit() < 0 ) {
		fprintf(stderr, "ERROR: xmlsec init failed!" );
		return;
	}

	if( xmlSecCheckVersion() != 1 ) {
		fprintf(stderr, "ERROR: incompatible xmlsec library version" );
		return;
	}

#ifdef DISABLED
	if( xmlSecCryptoDLLoadLibrary(BAD_CAST XMLSEC_CRYPTO) < 0 ){
		fprintf(stderr, "Error: unable to load default xmlsec-crypto library. Make sure\n"
										"that you have it installed and check shared libraries path\n"
										"(LD_LIBRARY_PATH) envornment variable.\n");
		return;
	}
#endif

	if( xmlSecCryptoAppInit( nullptr ) < 0 ) {
		fprintf(stderr, "ERROR: crypto lib init failed!" );
		return;
	}

	if( xmlSecCryptoInit() < 0 ) {
		fprintf(stderr, "ERROR: xmlsec-crypto init failed" );
		return;
	}


	mngr = xmlSecKeysMngrCreate();
	if( mngr == nullptr ) {
		fprintf(stderr, "Error: failed to create keys manager.\n" );
		return;
	}


	// load default root certificates from system or bundled with crypto library
	if( xmlSecCryptoAppDefaultKeysMngrInit( mngr ) < 0 ) {
		fprintf(stderr, "Error: failed to initialize keys manager.\n" );
		xmlSecKeysMngrDestroy( mngr );
		return;
	}

}

Core::~Core() {
	xmlSecKeysMngrDestroy( mngr );
	xmlSecCryptoShutdown();
	xmlSecCryptoAppShutdown();
	xmlSecShutdown();
	xsltCleanupGlobals();
	xmlCleanupParser();
}

int
Core::sign(const std::string &document, std::string &result, const sign_options_t &options) {

	xmlDocPtr doc = nullptr;
	xmlNodePtr signNode = nullptr,
			keyInfoNode = nullptr;

	int format = options.format;
	error_code = 0;

	if( format == SF_UNSET ) {
		format = default_format;
	}

	xmlSecTransformId c14n_id;
	if( options.c14n_algorithm != C14N_UNSET)
		c14n_id = get_c14n_id(options.c14n_algorithm);
	else
		c14n_id = get_c14n_id(default_c14n);

	xmlSecTransformId sign_id;
	if( options.sign_algorithm != SA_UNSET)
		sign_id = get_sign_id(options.sign_algorithm);
	else
		sign_id = get_sign_id(default_sign);

	xmlSecTransformId hash_id;
	if( options.hash_algorithm != HA_UNSET)
		hash_id = get_hash_id(options.hash_algorithm);
	else
		hash_id = get_hash_id(default_hash);

	auto dsigCtx = xmlSecDSigCtxCreate( nullptr );
	if( !dsigCtx ) {
		xerror( -10, "Sign Context creation failed!" );
		goto done;
	}

	if( format == SF_ENVELOPED ) { // SF_ENVELOPED
		if( options.doc_in_memory ) {
			doc = xmlReadMemory(
					document.c_str(),
					document.size(),
					options.base_url.empty() ? "noname.xml" : options.base_url.c_str(),   /* base url */
					nullptr, /* encoding */
					0     /* parse options */
			);

			if(( doc == nullptr ) || ( xmlDocGetRootElement( doc ) == nullptr )) {
				xerror( -1, "Error: unable to parse xml document.\n" );
				goto done;
			}
		}
		else {
			doc = xmlParseFile( document.c_str());
			if(( doc == nullptr ) || ( xmlDocGetRootElement( doc ) == nullptr )) {
				xerror( -2, "Error: unable to parse file \"" + document + "\"\n" );
				goto done;
			}
		}
	} else {
		// create new empty doc
		doc = xmlNewDoc(BAD_CAST "1.0");

		if( doc == nullptr ) {
			xerror( -1, "Error: unable to create new xml document.\n" );
			goto done;
		}
	}

	signNode = xmlSecTmplSignatureCreate( doc, c14n_id, sign_id, nullptr );
	if( signNode == nullptr ) {
		xerror( -3, "Error: failed to create signature template\n" );
		goto done;
	}

	switch( format ) {
		default:
		case SF_ENVELOPED:
			xmlAddChild( xmlDocGetRootElement( doc ), signNode );
			{

				Reference *ref = nullptr;
				if(!options.references.empty())
					ref = options.references[0];

				auto refNode = xmlSecTmplSignatureAddReference(
						signNode,
						ref ? get_hash_id(ref->hash) : hash_id,
						nullptr,
						ref->uri.empty() ? BAD_CAST "" : BAD_CAST ref->uri.c_str(),
						nullptr );
				if( !refNode ) {
					xerror(-4, "Adding Reference failed!" );
					goto done;
				}
				if( xmlSecTmplReferenceAddTransform( refNode, xmlSecTransformEnvelopedId ) == nullptr ) {
					xerror(-5, "Error: failed to add enveloped transform to reference\n" );
					goto done;
				}
				if( ref && ( !ref->xpath_intersect.empty() || !ref->xpath_subtract.empty() )) {
					// dont need to check union as it is useless without any of the other!

					auto transNode = xmlSecTmplReferenceAddTransform( refNode, xmlSecTransformXPath2Id);
					if( !transNode ){
						xerror(-30, "Adding XPath transform failed!");
						goto done;
					}
					if( !ref->xpath_intersect.empty() ){
						if( xmlSecTmplTransformAddXPath2(transNode,BAD_CAST "intersect", BAD_CAST ref->xpath_intersect.c_str(), nullptr) != 0) {
							xerror( -31, "Adding XPath2 intersect transform failed!" );
							goto done;
						}
					}
					if( !ref->xpath_subtract.empty() ){
						if( xmlSecTmplTransformAddXPath2(transNode,BAD_CAST "subtract", BAD_CAST ref->xpath_subtract.c_str(), nullptr) != 0) {
							xerror( -31, "Adding XPath2 subtract transform failed!" );
							goto done;
						}
					}
					if( !ref->xpath_union.empty() ){
						if( xmlSecTmplTransformAddXPath2(transNode,BAD_CAST "union", BAD_CAST ref->xpath_union.c_str(), nullptr) != 0) {
							xerror( -31, "Adding XPath2 union transform failed!" );
							goto done;
						}
					}
				}
				if( ref && ref->transform != C14N_UNSET ){
					if( xmlSecTmplReferenceAddTransform( refNode, get_c14n_id(ref->transform)) == nullptr ) {
						xerror(-5, "Error: failed to add enveloped transform to reference\n" );
						goto done;
					}
				}
			}

			break;
		case SF_ENVELOPING:
			// the signature element is the root
			xmlDocSetRootElement(doc, signNode);
			{

			auto refs = options.references;
			int counter = 0;
			Reference *default_ref = nullptr;

			if( refs.empty()) {
				default_ref = new Reference;
				refs.push_back(default_ref);
			}

			for( auto &ref : refs ) {
				auto hash_id2 = ref->hash != HA_UNSET ? get_hash_id( ref->hash ) : hash_id;
				auto newuri = "#res" + std::to_string(counter);
				auto newid = "res" + std::to_string(counter);

				auto refNode = xmlSecTmplSignatureAddReference(
						signNode, hash_id2,
						nullptr, BAD_CAST newuri.c_str(), nullptr );

				if( !refNode ) {
					xerror(-7, "Adding Reference "+ref->uri+" failed!");
					if(default_ref)	delete default_ref;
					goto done;
				}
				if( !ref->xpath_intersect.empty() || !ref->xpath_subtract.empty() ) {
					// dont need to check union as it is useless without any of the other!

					auto transNode = xmlSecTmplReferenceAddTransform( refNode, xmlSecTransformXPath2Id);
					if( !transNode ){
						if(default_ref)	delete default_ref;
						xerror(-30, "Adding XPath transform failed!");
						goto done;
					}
					if( !ref->xpath_intersect.empty() ){
						if( xmlSecTmplTransformAddXPath2(transNode,BAD_CAST "intersect", BAD_CAST ref->xpath_intersect.c_str(), nullptr) != 0) {
							if(default_ref)	delete default_ref;
							xerror( -31, "Adding XPath2 intersect transform failed!" );
							goto done;
						}
					}
					if( !ref->xpath_subtract.empty() ){
						if( xmlSecTmplTransformAddXPath2(transNode,BAD_CAST "subtract", BAD_CAST ref->xpath_subtract.c_str(), nullptr) != 0) {
							if(default_ref)	delete default_ref;
							xerror( -31, "Adding XPath2 subtract transform failed!" );
							goto done;
						}
					}
					if( !ref->xpath_union.empty() ){
						if( xmlSecTmplTransformAddXPath2(transNode,BAD_CAST "union", BAD_CAST ref->xpath_union.c_str(), nullptr) != 0) {
							if(default_ref)	delete default_ref;
							xerror( -31, "Adding XPath2 union transform failed!" );
							goto done;
						}
					}
				}
				if( ref->transform != C14N_UNSET ){
					if( xmlSecTmplReferenceAddTransform( refNode, get_c14n_id(ref->transform)) == nullptr ) {
						if(default_ref)	delete default_ref;
						xerror(-5, "Error: failed to add enveloped transform to reference\n" );
						goto done;
					}
				}

				auto objNode = xmlSecTmplSignatureAddObject( signNode, BAD_CAST newid.c_str(), nullptr, nullptr);
				if( !objNode ) {
					if(default_ref)	delete default_ref;
					xmlFreeNode(refNode);
					xerror(-6, "Adding Object failed!");
					goto done;
				}

				// load uri and create an object from it
				auto refdoc = xmlParseFile( ref->uri.c_str() );
				if( !refdoc ) {
					if(default_ref)	delete default_ref;
					xmlFreeNode(refNode);
					xmlFreeNode(objNode);
					xerror(-6, "Parsing Object " +ref->uri+ " failed!");
					goto done;
				}
				auto refroot = xmlDocGetRootElement(refdoc);
				if( !refdoc ) {
					if(default_ref)	delete default_ref;
					xmlFreeNode(refNode);
					xmlFreeNode(objNode);
					xmlFreeDoc(refdoc);
					xerror(-6, "Unable to find root of Object " +ref->uri+ " failed!");
					goto done;
				}

				xmlUnlinkNode( refroot );
				xmlAddChild( objNode, refroot );

				xmlFreeDoc(refdoc);

				counter++;
			}
			if(default_ref)	delete default_ref;
			}
			break;
		case SF_DETACHED:
			// the signature element is the root
			xmlDocSetRootElement(doc, signNode);

			if( options.references.empty()) {
				xerror(-7, "Detached signatures require references!" );
				goto done;
			}

			for( auto &ref : options.references ) {
				auto hash_id2 = ref->hash != HA_UNSET ? get_hash_id( ref->hash ) : hash_id;
				auto refNode = xmlSecTmplSignatureAddReference(
						signNode, hash_id2,
						nullptr, BAD_CAST ref->uri.c_str(), nullptr );

				if( !refNode ) {
					xerror(-6, "Adding Reference '"+ref->uri+"' failed!");
					goto done;
				}
				if( !ref->xpath_intersect.empty() || !ref->xpath_subtract.empty() ) {
					// dont need to check union as it is useless without any of the other!

					auto transNode = xmlSecTmplReferenceAddTransform( refNode, xmlSecTransformXPath2Id);
					if( !transNode ){
						xerror(-30, "Adding XPath transform failed!");
						goto done;
					}
					if( !ref->xpath_intersect.empty() ){
						if( xmlSecTmplTransformAddXPath2(transNode,BAD_CAST "intersect", BAD_CAST ref->xpath_intersect.c_str(), nullptr) != 0) {
							xerror( -31, "Adding XPath2 intersect transform failed!" );
							goto done;
						}
					}
					if( !ref->xpath_subtract.empty() ){
						if( xmlSecTmplTransformAddXPath2(transNode,BAD_CAST "subtract", BAD_CAST ref->xpath_subtract.c_str(), nullptr) != 0) {
							xerror( -31, "Adding XPath2 subtract transform failed!" );
							goto done;
						}
					}
					if( !ref->xpath_union.empty() ){
						if( xmlSecTmplTransformAddXPath2(transNode,BAD_CAST "union", BAD_CAST ref->xpath_union.c_str(), nullptr) != 0) {
							xerror( -31, "Adding XPath2 union transform failed!" );
							goto done;
						}
					}
				}
				if( ref->transform != C14N_UNSET ){
					if( xmlSecTmplReferenceAddTransform( refNode, get_c14n_id(ref->transform)) == nullptr ) {
						xerror(-5, "Error: failed to add enveloped transform to reference\n" );
						goto done;
					}
				}
			}

			break;

	}

	keyInfoNode = xmlSecTmplSignatureEnsureKeyInfo( signNode, nullptr );
	if( !keyInfoNode ) {
		xerror( -19, "Adding KeyInfo failed!" );
		goto done;
	}

	if( options.private_key.empty()) {
		xerror( -20, "Signing requires a private key! None given!" );
		goto done;
	}


	//FIXME: for DEBUG
	dsigCtx->flags |= XMLSEC_DSIG_FLAGS_STORE_SIGNEDINFO_REFERENCES;

	if( !options.keys_in_p12 ) {
		dsigCtx->signKey = xmlSecCryptoAppKeyLoad( options.private_key.c_str(), xmlSecKeyDataFormatPem,
		                                           options.key_password.empty() ? nullptr : options.key_password.c_str(),
		                                           nullptr, nullptr );

	}
	else {
		dsigCtx->signKey = xmlSecCryptoAppKeyLoad( options.private_key.c_str(), xmlSecKeyDataFormatPkcs12,
		                                           options.key_password.empty() ? nullptr : options.key_password.c_str(),
		                                           nullptr, nullptr );
		// certificate is read automatically if one is inside the p12!
	}
	if( !dsigCtx->signKey ) {
		xerror( -30, "Could not load private key from \"" + options.private_key + "\"\n" );
		goto done;
	}

	if( options.public_key_is_cert && !options.public_key.empty()) {
		/* load certificate and add to the key */
		if( xmlSecCryptoAppKeyCertLoad( dsigCtx->signKey, options.public_key.c_str(), xmlSecKeyDataFormatPem ) < 0 ) {
			xerror( -31, "Error: failed to load pem certificate \"" + options.public_key + "\"\n" );
			goto done;
		}

		/* create X509Data in KeyInfo */
		if( xmlSecTmplKeyInfoAddX509Data( keyInfoNode ) == nullptr ) {
			xerror( -32, "Error: failed to add X509Data node\n" );
			goto done;
		}
	}
	else if( options.keys_in_p12 ){
		// a certificate should have been loaded automatically in this case, so add it to the signature!

		/* create X509Data in KeyInfo */
		if( xmlSecTmplKeyInfoAddX509Data( keyInfoNode ) == nullptr ) {
			xerror( -32, "Error: failed to add X509Data node\n" );
			goto done;
		}
	}
	else if( !options.public_key.empty()) {
		// embed the public key!
		auto keyValueNode = xmlSecTmplKeyInfoAddKeyValue(keyInfoNode);
		if(keyValueNode == nullptr) {
			xerror(-33,"Error: failed to add KeyValue node");
			goto done;
		}
	}
	else {
		//XTODO: public key empty, what now?
	}

	//TODO: DEBUG:
	xmlDocDump(stderr, doc);

	if( xmlSecDSigCtxSign( dsigCtx, signNode ) < 0 ) {
		xerror( -90, "Error: signing failed\n" );
		goto done;
	}

	if( options.doc_in_memory ) {
		xmlChar *xbuff;
		int buffsize;

		xmlDocDumpFormatMemory( doc, &xbuff, &buffsize, 1 );
		result = std::string((char *) xbuff, buffsize );

		xmlFree( xbuff );
		goto done;
	}
	else {
		if( xmlSaveFile( result.c_str(), doc ) < 0 ) {
			xerror( -80, "Error while writing file to " + result + "\n" );
			goto done;
		}

		//FIXME DEBUG:
		xmlSecDSigCtxDebugDump( dsigCtx, stdout);

		goto done;
	}


done:
	if( dsigCtx )
		xmlSecDSigCtxDestroy( dsigCtx );

	if( doc )
		xmlFreeDoc( doc );

	return error_code;
}

int Core::verify(const std::string &document, bool &result, const verify_options_t &options) {

	error_code = 0;

	xmlDocPtr doc = nullptr;
	xmlNodePtr node = nullptr;
	xmlSecDSigCtxPtr dsigCtx = nullptr;

	if( options.doc_in_memory ) {
		doc = xmlReadMemory(
				document.c_str(),
				document.size(),
				options.base_url.empty() ? "noname.xml" : options.base_url.c_str(),   /* base url */
				nullptr, /* encoding */
				0     /* parse options */
		);

		if(( doc == nullptr ) || ( xmlDocGetRootElement( doc ) == nullptr )) {
			xerror( -1, "Error: unable to parse xml document.\n" );
			goto done;
		}
	}
	else {
		doc = xmlParseFile( document.c_str());
		if(( doc == nullptr ) || ( xmlDocGetRootElement( doc ) == nullptr )) {
			xerror( -2, "Error: unable to parse file \"" + document + "\"\n" );
			goto done;
		}
	}

	node = xmlSecFindNode(
			xmlDocGetRootElement( doc ),
			xmlSecNodeSignature,
			xmlSecDSigNs );
	if( node == nullptr ) {
		xerror( -1, "ERROR: no signature found!" );
		goto done;
	}

	if( !options.public_key.empty()) {
		if( !options.public_key_is_cert ) {
			dsigCtx = xmlSecDSigCtxCreate( nullptr );
			dsigCtx->signKey = xmlSecCryptoAppKeyLoad(
					options.public_key.c_str(),
					xmlSecKeyDataFormatPem,
					nullptr, nullptr, nullptr );
			if( !dsigCtx->signKey ) {
				xerror( -30, "Could not load public key from \"" + options.public_key + "\"\n" );
				goto done;
			}
		}
		else if( options.public_key_is_p12 ) {
			//load public key from p12 somehow...
			dsigCtx = xmlSecDSigCtxCreate( mngr );
			if( options.trust_selfsigned_cert ) {
				xmlSecCryptoAppKeysMngrCertLoad( mngr, options.public_key.c_str(), xmlSecKeyDataFormatPkcs12,
				                                 xmlSecKeyDataTypeTrusted );
			} else {
				xmlSecCryptoAppKeysMngrCertLoad( mngr, options.public_key.c_str(), xmlSecKeyDataFormatPkcs12,
				                                 xmlSecKeyDataTypeUnknown );
			}
		}
		else {
			dsigCtx = xmlSecDSigCtxCreate( mngr );
			if( options.trust_selfsigned_cert ) {
				xmlSecCryptoAppKeysMngrCertLoad( mngr, options.public_key.c_str(), xmlSecKeyDataFormatCertPem,
				                                 xmlSecKeyDataTypeTrusted );
			}

			dsigCtx->signKey = xmlSecCryptoAppKeyLoad(
					options.public_key.c_str(),
					xmlSecKeyDataFormatCertPem,
					nullptr, nullptr, nullptr );
			xmlSecCryptoAppKeyCertLoad( dsigCtx->signKey, options.public_key.c_str(), xmlSecKeyDataFormatCertPem );
		}
	}
	else {
		dsigCtx = xmlSecDSigCtxCreate( mngr );
		// when certificate is embedded, this works already.
		// if certificate is self-signed, we need to add it as trusted to the key manager
		// do this only if explicitly wished!
		if( options.trust_selfsigned_cert ){
			auto keyInfo = xmlSecFindNode( node, xmlSecNodeKeyInfo, xmlSecDSigNs );
			auto x509Data = xmlSecFindNode( keyInfo, xmlSecNodeKeyInfo, xmlSecDSigNs );
			auto x509Cert = xmlSecFindNode( x509Data, xmlSecNodeX509Certificate, xmlSecDSigNs );
			//auto x509Cert = xmlSecFindNode( node, xmlSecNodeX509Certificate, xmlSecDSigNs );

			auto cert_raw = xmlNodeGetContent( x509Cert->children );
			if(cert_raw == nullptr){
				xerror(-120, "Could not allocate memory!!");
				goto done;
			}
			// calc size, + nullbyte + size of PEM boundary
			auto cert_pem_size = xmlStrlen(cert_raw) + PEM_BOUNDARY_SIZE;
			auto cert_pem = (xmlChar *) xmlMalloc( cert_pem_size );
			if(cert_pem == nullptr){
				xerror(-120, "Could not allocate memory!!");
				xmlFree(cert_raw);
				goto done;
			}
			xmlStrPrintf(cert_pem, cert_pem_size, BAD_CAST PEM_BOUNDARY_FORMAT, cert_raw);
			xmlFree(cert_raw); // free as soon as not needed anymore

			xmlSecCryptoAppKeysMngrCertLoadMemory( mngr, cert_pem, cert_pem_size, xmlSecKeyDataFormatCertPem, xmlSecKeyDataTypeTrusted );

			xmlFree(cert_pem);
		}
	}

	xmlSecErrorsSetCallback( core_set_error );
	xmlSecDSigCtxVerify( dsigCtx, node );
	xmlSecErrorsSetCallback( xmlSecErrorsDefaultCallback );

	if( dsigCtx->status == xmlSecDSigStatusSucceeded ) {
		result = true;
	}
	else {
		result = false;
	}

done:
	if( dsigCtx )
		xmlSecDSigCtxDestroy( dsigCtx );

	if( doc )
		xmlFreeDoc( doc );

	return error_code;
}

int Core::encrypt(const std::string &document, std::string &result, const encrypt_options_t &options) {
	xmlDocPtr doc = nullptr, docTpl = nullptr;
	xmlNodePtr encDataNode = nullptr;
	xmlNodePtr encKeyNode = nullptr;
	xmlNodePtr keyInfoNode = nullptr;
	xmlNodePtr keyInfoNode2 = nullptr;
	xmlNodePtr retrievalNode = nullptr;
	xmlSecEncCtxPtr encCtx = nullptr;
	xmlSecKeyPtr pubKey = nullptr;
	bool first_key_data = true;
	bool dtd_added = false;

	int format = options.encryption_form;
	error_code = 0;

	if( format == EF_UNSET ) {
		format = default_enc_format;
	}

	auto enc_type = xmlSecTypeEncContent;
	if( format == EF_ELEMENT )
		enc_type = xmlSecTypeEncElement;

	xmlSecTransformId enc_id;
	xmlSecKeyDataId key_id;
	xmlSecSize key_size;
	if( options.encryption_algorithm != EA_UNSET ) {
		enc_id = get_enc_id( options.encryption_algorithm );
		key_id = get_key_id( options.encryption_algorithm );
		key_size = get_key_size( options.encryption_algorithm );
	}
	else {
		enc_id = get_enc_id( default_enc );
		key_id = get_key_id( default_enc );
		key_size = get_key_size( default_enc );
	}

	doc = xmlParseFile( document.c_str());
	if(( doc == nullptr ) || ( xmlDocGetRootElement( doc ) == nullptr )) {
		xerror( -10, "Error: unable to parse file \"" + document + "\"\n" );
		goto done;
	}

	if( options.public_key.empty()) {
		xerror( -20, "Encrypting the session key requires a public key! None given!" );
		goto done;
	}

	if( !options.keys_in_p12 && !options.public_key_is_cert ){
		pubKey = xmlSecCryptoAppKeyLoad( options.public_key.c_str(), xmlSecKeyDataFormatPem,
		                                  options.key_password.empty() ? nullptr : options.key_password.c_str(),
				                              nullptr, nullptr );
	} else if( options.keys_in_p12 ){
		pubKey = xmlSecCryptoAppKeyLoad( options.public_key.c_str(), xmlSecKeyDataFormatPkcs12,
		                                  options.key_password.empty() ? nullptr : options.key_password.c_str(),
		                                  nullptr, nullptr );
		// certificate is read automatically if one is inside the p12!
	} else{
		pubKey = xmlSecCryptoAppKeyLoad( options.public_key.c_str(), xmlSecKeyDataFormatCertPem, nullptr, nullptr, nullptr );
	}
	if( pubKey == nullptr ) {
		xerror(-25, "Error: failed to load rsa key from file \""+options.public_key+"\"");
		goto done;
	}

	/* set key name to some name */
	if(xmlSecKeySetName(pubKey, BAD_CAST options.public_key.c_str()) < 0) {
		xerror(-26, "Error: failed to set key name for key from \""+options.public_key+"\"");
		xmlSecKeyDestroy(pubKey);
		goto done;
	}

	/* add key to keys manager, from now on keys manager is responsible
	 * for destroying key
	 */
	if(xmlSecCryptoAppDefaultKeysMngrAdoptKey(mngr, pubKey) < 0) {
		fprintf(stderr,"Error: failed to add private key to keys manager");
		xmlSecKeyDestroy(pubKey);
		goto done;
	}

	/* create encryption context */
	encCtx = xmlSecEncCtxCreate( mngr );
	if( encCtx == nullptr ) {
		xerror(-40, "Error: failed to create encryption context\n" );
		goto done;
	}

	// generate session key
	encCtx->encKey = xmlSecKeyGenerate(key_id, key_size, xmlSecKeyDataTypeSession);
	if(encCtx->encKey == NULL) {
		xerror(-50,"Error: failed to generate session key\n");
		goto done;
	}

	/* set key name to the file name, this is just an example! */
	if( xmlSecKeySetName( encCtx->encKey, BAD_CAST "key0") < 0 ) {
		xerror(-37, "Error: failed to set key name for session key");
		goto done;
	}

	if( !options.xpaths.empty() ){
		xmlXPathContextPtr xpathCtx = nullptr;
		xmlXPathObjectPtr xpathObject = nullptr;
		for( auto &xpath : options.xpaths ){
			xpathCtx = xmlXPathNewContext(doc);
			if(xpathCtx == nullptr) {
				xerror(-40,"Error: unable to create new XPath context\n");
				goto done;
			}

			/* Evaluate xpath expression */
			auto xpathObj = xmlXPathEvalExpression(BAD_CAST xpath.c_str(), xpathCtx);
			if(xpathObj == nullptr) {
				xerror(-41,"Error: unable to evaluate xpath expression "+xpath);
				xmlXPathFreeContext(xpathCtx);
				goto done;
			}

			/* store selected nodes */
			auto nodes = xpathObj->nodesetval;
			auto size = (nodes) ? nodes->nodeNr : 0;

			for(int i = size - 1; i >= 0; i--) {
				// check if an ancestor is already selected for encryption,
				// in that case, dont encrypt the child, it will be encrypted by the ancestors encryption
				bool anc_selected = false;

				for(int j = i-1; j>= 0; j--){
					if(is_ancestor_of(nodes->nodeTab[j], nodes->nodeTab[i])){
						anc_selected = true; break;
					}
				}

				if(!anc_selected) {

					if( format == EF_CONTENT ) {
						encDataNode = xmlSecTmplEncDataCreate( doc, enc_id, nullptr, xmlSecTypeEncContent, nullptr, nullptr );
					}
					else { // EF_ELEMENT
						encDataNode = xmlSecTmplEncDataCreate( doc, enc_id, nullptr, xmlSecTypeEncElement, nullptr, nullptr );
					}
					if( encDataNode == nullptr ) {
						xerror( -10, "Error: failed to create encryption template\n" );
						goto done;
					}

					/* we want to put encrypted data in the <enc:CipherValue/> node */
					if( xmlSecTmplEncDataEnsureCipherValue( encDataNode ) == nullptr ) {
						xerror( -20, "Error: failed to add CipherValue to EncryptedData" );
						goto done;
					}

					/* add <dsig:KeyInfo/> and <dsig:KeyName/> nodes to put key name in the document */
					keyInfoNode = xmlSecTmplEncDataEnsureKeyInfo( encDataNode, nullptr );
					if( keyInfoNode == nullptr ) {
						xerror( -30, "Error: failed to add KeyInfo to EncryptedData" );
						goto done;
					}

					if( xmlSecTmplKeyInfoAddKeyName( keyInfoNode, nullptr ) == nullptr ) {
						xerror( -35, "Error: failed to add KeyName to KeyInfo" );
						goto done;
					}

					if( first_key_data ){
						encKeyNode = xmlSecTmplKeyInfoAddEncryptedKey(keyInfoNode,
						                                              xmlSecTransformRsaPkcs1Id,
						                                              BAD_CAST "key0", NULL, NULL);
						if(encKeyNode == nullptr) {
							xerror(-36, "Error: failed to add EncryptedKey to KeyInfo");
							goto done;
						}

						if(xmlSecTmplEncDataEnsureCipherValue(encKeyNode) == NULL) {
							xerror(-37, "Error: failed to add CipherValue to EncryptedKey");
							goto done;
						}

						/* add <dsig:KeyInfo/> and <dsig:KeyName/> nodes to <enc:EncryptedKey/> */
						keyInfoNode2 = xmlSecTmplEncDataEnsureKeyInfo(encKeyNode, nullptr);
						if(keyInfoNode2 == nullptr) {
							xerror(-38, "Error: failed to add KeyInfo to EncryptedKey");
							goto done;
						}

						/* set key name so we can lookup key when needed */
						if(xmlSecTmplKeyInfoAddKeyName(keyInfoNode2, BAD_CAST options.public_key.c_str()) == NULL) {
							xerror(-39, "Error: failed to add KeyName to KeyInfo of EncryptedKey");
							goto done;
						}

						// keyInfoNode2 exists now, so add certificate or public key data into it.
						if( options.public_key_is_cert || options.keys_in_p12 ) {
							/* create X509Data in KeyInfo */
							if( xmlSecTmplKeyInfoAddX509Data( keyInfoNode2 ) == nullptr ) {
								xerror( -32, "Error: failed to add X509Data node\n" );
								goto done;
							}
						}
						else if( !options.public_key.empty()) {
							// embed the public key!
							auto keyValueNode = xmlSecTmplKeyInfoAddKeyValue(keyInfoNode2);
							if(keyValueNode == nullptr) {
								xerror(-33,"Error: failed to add KeyValue node");
								goto done;
							}
						}

						first_key_data = false;
					} else {
						/*if( !dtd_added ){
							dtd_added = true;
						}*/
						retrievalNode = xmlSecTmplKeyInfoAddRetrievalMethod( keyInfoNode, BAD_CAST "#key0",
						                       BAD_CAST "http://www.w3.org/2001/04/xmlenc#EncryptedKey" );
						if( retrievalNode == nullptr ) {
							xerror( -37, "Error: Failed to add RetrievalMethod to KeyInfo" );
							goto done;
						}
					}

					/* encrypt the selected node */
					if( xmlSecEncCtxXmlEncrypt( encCtx, encDataNode, nodes->nodeTab[i] ) < 0 ) {
						xerror( -70, "Error: encryption failed\n" );
						goto done;
					}

					{ // reset the encryption context as per https://www.aleksey.com/pipermail/xmlsec/2009/008665.html
						auto tmpkey = encCtx->encKey;
						encCtx->encKey = nullptr;
						xmlSecEncCtxReset( encCtx );
						encCtx->encKey = tmpkey;
					}
					encDataNode = nullptr; // nodes are now part of the doc!
					keyInfoNode = nullptr;
					retrievalNode = nullptr;
				}
				if( nodes->nodeTab[i]->type != XML_NAMESPACE_DECL )
					nodes->nodeTab[i] = nullptr;
			}

			/* Cleanup of XPath data */
			xmlXPathFreeObject(xpathObj);
			xmlXPathFreeContext(xpathCtx);
		}
		xpathCtx = nullptr; xpathObject = nullptr;

	} else {
		if( format == EF_CONTENT ){
			encDataNode = xmlSecTmplEncDataCreate( doc, enc_id, nullptr, xmlSecTypeEncContent, nullptr, nullptr );
		}
		else { // EF_ELEMENT
			encDataNode = xmlSecTmplEncDataCreate( doc, enc_id, nullptr, xmlSecTypeEncElement, nullptr, nullptr );
		}

		if( encDataNode == nullptr ) {
			xerror(-10, "Error: failed to create encryption template\n" );
			goto done;
		}

		/* we want to put encrypted data in the <enc:CipherValue/> node */
		if( xmlSecTmplEncDataEnsureCipherValue( encDataNode ) == nullptr ) {
			xerror( -20, "Error: failed to add CipherValue node\n" );
			goto done;
		}

		/* add <dsig:KeyInfo/> and <dsig:KeyName/> nodes to put key name in the signed document */
		keyInfoNode = xmlSecTmplEncDataEnsureKeyInfo( encDataNode, nullptr );
		if( keyInfoNode == nullptr ) {
			xerror(-30, "Error: failed to add key info\n" );
			goto done;
		}

		if( xmlSecTmplKeyInfoAddKeyName( keyInfoNode, nullptr ) == nullptr ) {
			xerror(-35, "Error: failed to add key name\n" );
			goto done;
		}

		encKeyNode = xmlSecTmplKeyInfoAddEncryptedKey(keyInfoNode,
		                                              xmlSecTransformRsaPkcs1Id,
		                                              BAD_CAST "key0", NULL, NULL);
		if(encKeyNode == nullptr) {
			xerror(-36, "Error: failed to add EncryptedKey to KeyInfo\n");
			goto done;
		}

		if(xmlSecTmplEncDataEnsureCipherValue(encKeyNode) == NULL) {
			xerror(-37, "Error: failed to add CipherValue to EncryptedKey");
			goto done;
		}

		/* add <dsig:KeyInfo/> and <dsig:KeyName/> nodes to <enc:EncryptedKey/> */
		keyInfoNode2 = xmlSecTmplEncDataEnsureKeyInfo(encKeyNode, nullptr);
		if(keyInfoNode2 == nullptr) {
			xerror(-38, "Error: failed to add KeyInfo to EncryptedKey");
			goto done;
		}

		/* set key name so we can lookup key when needed */
		if(xmlSecTmplKeyInfoAddKeyName(keyInfoNode2, BAD_CAST options.public_key.c_str()) == NULL) {
			xerror(-39, "Error: failed to add KeyName to KeyInfo of EncryptedKey");
			goto done;
		}

		// keyInfoNode2 exists now, so add certificate or public key data into it.
		if( options.public_key_is_cert || options.keys_in_p12) {
			/* create X509Data in KeyInfo */
			if( xmlSecTmplKeyInfoAddX509Data( keyInfoNode2 ) == nullptr ) {
				xerror( -32, "Error: failed to add X509Data node\n" );
				goto done;
			}
		}
		else if( !options.public_key.empty()) {
			// embed the public key!
			auto keyValueNode = xmlSecTmplKeyInfoAddKeyValue(keyInfoNode2);
			if(keyValueNode == nullptr) {
				xerror(-33,"Error: failed to add KeyValue node");
				goto done;
			}
		}

		if( xmlSecEncCtxXmlEncrypt( encCtx, encDataNode, xmlDocGetRootElement(doc) ) < 0 ) {
			xerror( -70, "Error: encryption failed\n" );
			goto done;
		}
		/* the template is inserted in the doc */
		encDataNode = nullptr;
		keyInfoNode = nullptr;
		retrievalNode = nullptr;
	}


	if( xmlSaveFile( result.c_str(), doc ) < 0 ) {
		xerror( -80, "Error while writing file to " + result + "\n" );
		goto done;
	}


done:

	/* cleanup */
	if( encCtx != nullptr ) {
		xmlSecEncCtxDestroy( encCtx );
	}

	if( encDataNode != nullptr ) {
		xmlFreeNode( encDataNode );
	}

	if( doc != nullptr ) {
		xmlFreeDoc( doc );
	}
	return error_code;
}

int Core::decrypt(const std::string &document, std::string &result, const decrypt_options_t &options) {
	xmlDocPtr doc = nullptr;
	xmlNodePtr node = nullptr;
	xmlSecEncCtxPtr encCtx = nullptr;
	error_code = 0;

	doc = xmlParseFile( document.c_str());
	if(( doc == nullptr ) || ( xmlDocGetRootElement( doc ) == nullptr )) {
		xerror(-10, "Error: unable to parse file \""+document+"\"");
		goto done;
	}

	if( !options.private_key.empty() ){
		if( !options.private_key_is_p12 ){
			auto privKey = xmlSecCryptoAppKeyLoad( options.private_key.c_str(), xmlSecKeyDataFormatPem,
			                                  nullptr, nullptr, nullptr );
			if( privKey == nullptr ) {
				xerror(-25, "Error: failed to load rsa key from file \""+options.private_key+"\"");
				goto done;
			}

			/* add key to keys manager, from now on keys manager is responsible
			 * for destroying key
			 */
			if(xmlSecCryptoAppDefaultKeysMngrAdoptKey(mngr, privKey) < 0) {
				fprintf(stderr,"Error: failed to add public key to keys manager");
				xmlSecKeyDestroy(privKey);
				goto done;
			}
		}
		else if( options.private_key_is_p12 ){
			if( options.trust_selfsigned_cert ) {
				xmlSecCryptoAppKeysMngrCertLoad( mngr, options.private_key.c_str(), xmlSecKeyDataFormatPkcs12,
				                                 xmlSecKeyDataTypeTrusted );
			}
			auto privKey = xmlSecCryptoAppKeyLoad( options.private_key.c_str(), xmlSecKeyDataFormatPkcs12,
			                                       options.key_password.empty() ? nullptr : options.key_password.c_str(), nullptr, nullptr );
			if( privKey == nullptr ) {
				xerror(-25, "Error: failed to load rsa key from file \""+options.private_key+"\"");
				goto done;
			}

			/* add key to keys manager, from now on keys manager is responsible
			 * for destroying key
			 */
			if(xmlSecCryptoAppDefaultKeysMngrAdoptKey(mngr, privKey) < 0) {
				fprintf(stderr,"Error: failed to add public key to keys manager");
				xmlSecKeyDestroy(privKey);
				goto done;
			}
		}
	}

	// little hack to get Id of EncryptedKey working, alternatively one would need a short DTD like this:
	// <!ATTLIST EncryptedKey Id ID #IMPLIED>
	node = xmlSecFindNode( xmlDocGetRootElement( doc ), xmlSecNodeEncryptedKey, xmlSecEncNs );
	if( node != nullptr){
		auto attr = xmlHasProp(node, BAD_CAST "Id");
		if((attr != nullptr) && (attr->children != nullptr)) {
			/* get the attribute (id) value */
			auto name = xmlNodeListGetString(doc, attr->children, 1);
			if(name != nullptr) {
				auto tmp = xmlGetID(doc, name);
				if(tmp == nullptr) { // if not registered as ID, do it
					xmlAddID(NULL, doc, name, attr);
					fprintf(stderr,"added node");
				}
				xmlFree(name);
			}
		}
	}

	encCtx = xmlSecEncCtxCreate( mngr );
	if( encCtx == nullptr ) {
		xerror(-30, "Error: failed to create encryption context" );
		goto done;
	}

	/* find start node */
	node = xmlSecFindNode( xmlDocGetRootElement( doc ), xmlSecNodeEncryptedData, xmlSecEncNs );
	if( node == nullptr ) {
		xerror(-20, "Error: no EncryptedData found in \""+document+"\"");
		goto done;
	}

	do {
		//xmlSecErrorsSetCallback( core_set_error );
		/* decrypt the data */
		if( xmlSecEncCtxDecrypt( encCtx, node ) < 0 ) {
			//xmlSecEncCtxDebugDump( encCtx, stderr);
			xerror( -50, "Error: decrypting failed." ); // error message set by callback
			goto done;
		}
		//xmlSecErrorsSetCallback( xmlSecErrorsDefaultCallback );

		if( encCtx->result == nullptr ) {
			xerror( -51, "Error: result is empty, nothing was decrypted." );
			goto done;
		}
		if( encCtx->resultReplaced == 0) {
			if( xmlSecBufferGetData( encCtx->result ) != nullptr ) {
				auto binout = fopen(result.c_str(), "wb");
				fwrite( xmlSecBufferGetData( encCtx->result ),
				        1,
				        xmlSecBufferGetSize( encCtx->result ),
				        binout);
				fclose(binout);
				goto done;
			}
		}

		{ // reset the encryption context as per https://www.aleksey.com/pipermail/xmlsec/2009/008665.html
			auto tmpkey = encCtx->encKey;
			encCtx->encKey = nullptr;
			xmlSecEncCtxReset( encCtx );
			encCtx->encKey = tmpkey;
		}
	} while((node = xmlSecFindNode( xmlDocGetRootElement( doc ), xmlSecNodeEncryptedData, xmlSecEncNs )) != nullptr);

	xmlSaveFile(result.c_str(), doc );


done:
	/* cleanup */
	if( encCtx != nullptr ) {
		xmlSecEncCtxDestroy( encCtx );
	}

	if( doc != nullptr ) {
		xmlFreeDoc( doc );
	}
	return error_code;
}

xmlSecTransformId get_hash_id(int hash_algo) {
	switch(hash_algo) {
		default:
		case HA_UNSET:
			return nullptr;

		case HA_SHA1:
			return xmlSecTransformSha1Id;
		case HA_SHA224:
			return xmlSecTransformSha224Id;
		case HA_SHA256:
			return xmlSecTransformSha256Id;
		case HA_SHA384:
			return xmlSecTransformSha384Id;
		case HA_SHA512:
			return xmlSecTransformSha512Id;
	}
}
xmlSecTransformId get_c14n_id(int c14n_algo) {
	switch(c14n_algo){
		default:
		case C14N_UNSET:
			return nullptr;
		case C14N_11_INCLUSIVE:
			return xmlSecTransformInclC14N11Id;
		case C14N_INCLUSIVE:
			return xmlSecTransformInclC14NId;
		case C14N_EXCLUSIVE:
			return xmlSecTransformExclC14NId;
	}
}

xmlSecTransformId get_sign_id(int sign_algo) {
	switch(sign_algo) {
		default:
		case SA_UNSET:
			return nullptr;
		case SA_RSA_SHA1:
			return xmlSecTransformRsaSha1Id;
		case SA_RSA_SHA224:
			return xmlSecTransformRsaSha224Id;
		case SA_RSA_SHA256:
			return xmlSecTransformRsaSha256Id;
		case SA_RSA_SHA384:
			return xmlSecTransformRsaSha384Id;
		case SA_RSA_SHA512:
			return xmlSecTransformRsaSha512Id;
		case SA_ECDSA_SHA1:
			return xmlSecTransformEcdsaSha1Id;
		case SA_ECDSA_SHA224:
			return xmlSecTransformEcdsaSha224Id;
		case SA_ECDSA_SHA256:
			return xmlSecTransformEcdsaSha256Id;
		case SA_ECDSA_SHA384:
			return xmlSecTransformEcdsaSha384Id;
		case SA_ECDSA_SHA512:
			return xmlSecTransformEcdsaSha512Id;
	}
}
xmlSecTransformId get_enc_id(int enc_algo){
	switch(enc_algo) {
		default:
		case EA_UNSET:
			return nullptr;
		case EA_AES128_CBC:
			return xmlSecTransformAes128CbcId;
		case EA_AES192_CBC:
			return xmlSecTransformAes192CbcId;
		case EA_AES256_CBC:
			return xmlSecTransformAes256CbcId;
		case EA_3DES_CBC:
			return xmlSecTransformDes3CbcId;
	}
}

xmlSecKeyDataId get_key_id(int enc_algo){
	switch(enc_algo) {
		default:
		case EA_UNSET:
			return nullptr;
		case EA_AES128_CBC:
		case EA_AES192_CBC:
		case EA_AES256_CBC:
			return xmlSecKeyDataAesId;
		case EA_3DES_CBC:
			return xmlSecKeyDataDesId;
	}
}

xmlSecSize get_key_size(int enc_algo) {
	switch(enc_algo) {
		default:
		case EA_UNSET:
			return 0;
		case EA_AES128_CBC:
			return 128;
		case EA_AES192_CBC:
			return 192;
		case EA_AES256_CBC:
			return 256;
		case EA_3DES_CBC:
			return 192;
	}
}

bool Core::hasSignature(const std::string &file) {
	bool ret = false;
	xmlDocPtr doc = xmlParseFile( file.c_str() );
	xmlNodePtr node = nullptr;

	if(( doc == nullptr ) || ( xmlDocGetRootElement( doc ) == nullptr )) {
		goto done;
	}

	node = xmlSecFindNode(
			xmlDocGetRootElement( doc ),
			xmlSecNodeSignature,
			xmlSecDSigNs );
	if( node == nullptr ){
		goto done;
	}

	ret = true;

done:
	if( doc != nullptr ) {
		xmlFreeDoc( doc );
	}
	return ret;
}

bool Core::isEncrypted(const std::string &file){
	bool ret = false;
	xmlNodePtr node = nullptr;
	xmlDocPtr doc = xmlParseFile( file.c_str());

	if(( doc == nullptr ) || ( xmlDocGetRootElement( doc ) == nullptr )) {
		goto done;
	}

	/* find start node */
	node = xmlSecFindNode( xmlDocGetRootElement( doc ), xmlSecNodeEncryptedData, xmlSecEncNs );
	if( node == nullptr ) {
		goto done;
	}

	ret = true; // at least one EncryptedData Node found, doc is encrypted

done:
	if(doc)
		xmlFreeDoc(doc);
	return ret;
}

void core_set_error(const char *file, int line, const char *func, const char *errobj, const char *errsbj,
                          int reason, const char *msg) {
	if( msg && strlen(msg) > 1 ) {
		Core::serror_msg = msg;
		fprintf(stderr, "'%s'\n", msg);
	}
}



bool is_ancestor_of(xmlNodePtr anc, xmlNodePtr node){
	// TODO: check if this works too:
	// xmlSecFindParent(node, xmlSecNodeGetName(anc), nullptr) ?

	xmlNodePtr check = node;
	while(check != nullptr){

		if( check == anc )
			return true;

		check = check->parent;
	}
	return false;
}

std::string Core::serror_msg;
} // namespace
