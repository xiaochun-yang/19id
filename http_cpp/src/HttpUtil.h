#ifndef __HttpUtil_h__
#define __HttpUtil_h__

/**
 * @file HttpUtil.h
 * Header file for the HttpUtil class.
 */

#include <string>
#include <map>

/**
 * @class HttpUtil
 * Utility class for a collection of miscellaneous static functions related to HTTP.
 */

class HttpUtil
{

public:

    /**
     * @brief Guess the Content-Encoding, Transfer-Encoding and content format
     * of the input buffer.
     * Content-Encoding is defaulted to "identity".
     *
     * First, guess if the content is text or binary:
     * - text
     *     - format: plain/text or text/html or application/postscript or image/x-xbitmap
     *     - Content-Encoding: plain/text or macbinhex or base64
     *     - Content-Transfer-Encoding: 8bit or 7bit
     * - binary
     *     - format: application/octet-stream or image/gif, image/jpeg, image/tiff, image/png, audio/basic
     *     - Content-Encoding: compress or gzip
     *     - Content-Transfer-Encoding:
     *
     * @param buf Input buffer
     * @param size Size of the buffer
     * @param contentEncoding Returned Content-Encoding
     * @param contentTransferEncoding Returned Transfer-Encoding
     * @param format Format of the content.
     * @todo Rewrite this func
     */
    static void guessFromContent(const char* buf, int size,
        std::string& contentEncoding,
        std::string& contentTransferEncoding,
        std::string& format);

    /**
     * @brief Guess the content type from file extension.
     * @param filename Filename with extension. Assuming the file extension conforms to the MIME types.
     * @param contentType Returned Content-Type header values corresponding to the file extension
     */
    static void guessFromFileExtension(const std::string& filename, std::string& contentType);


    /**
     * @brief Converts the URI encoded string back to its original string.
     * @param str URL encoded input string
     * @param decodedString Decoded returned string
     */
     static bool decodeURI(const char* str, char * decodedString);

    /**
     * @brief Decodes the URI encoded string back to its original string.
     * @param in URL encoded input string
     * @param out Decoded returned string
     */
     static bool decodeURI(const std::string& in, std::string& out);

    /**
     * @brief Encodes the URI string, following RFC2396.
     * @param str Input string in URI encoded format
     * @param ret Decoded returned string
     **/
    static bool encodeURI(const std::string& str, std::string& ret);

	/**
	 * @brief Parses the first line of the http response.
	 * 
	 * The response line contains the HTTP version info, 
	 * response status code and phrase.
	 *
	 * @param str Input string to parse
	 * @param version Returned HTTP version info
	 * @param code Returned status code
	 * @param phrase Returned status phrase
	 * @param reason The reason why the func fails (if the func returns false).
	 * @return True if the func parses the string successfully. The returned 
	 *         variables should only be used if the func returns true.
	 *         Returns false if the parsing fails, in which case, the 
	 *         reason why it fails is passed back in last argument.
	 */
	static bool parseResponseLine(const std::string& str,
								std::string& version,
								int& code,
								std::string& phrase,
								std::string& reason);

	/**
	 * @brief Parses url-encoded form data which is typically found
	 * in the query part of the URI or the request body.
	 *
	 * @param str Input string in the url-encoded form
	 * @param params Returned list of parameter names and values
	 * @param reason Returned error string if the func returns false.
	 * @return True if the func parses the form data successfully.
	 */
	static bool parseFormData(const std::string& str,
							  std::map<std::string, std::string>& params,
							  std::string& reason);

	/**
	 * @brief Parses the URI. Extracts host, port, and
	 * list of parameter name and values (from the query part).
	 *
	 * @param uri Input URI to parse
	 * @param host Returned host name
	 * @param port Returned port number
	 * @param resource Returned resource part of the URI
	 * @param params Returned list of parameter names and 
	 *        values from the query part of the URI.
	 * @param reason Returned error string if the func returns false.
	 * @return True if the func parses the URI successfully. 
	 *         If the func returns false, the error string is also returned. 
	 */
	static bool parseURI(const std::string& uri,
					std::string& host,
					std::string& port,
					std::string& resource,
					std::map<std::string, std::string>& params,
					std::string& reason);

};

#endif // __HttpUtil_h__

