/********************************************************************************************/
/*                                     narrow_wide_string.h                                 */
/*                                                                                          */
/*   Author          :  Blake Madden                                                        */
/*   Creation Date   :  12.11.2003	                                                        */
/*   Purpose         :  string class that can be assigned to either narrow or				*/
/*						wide strings.							                            */
/********************************************************************************************/    

#ifndef __NARROW_WIDE_STRING_H__
#define __NARROW_WIDE_STRING_H__

#include <string>
#include <locale>
#include <cstdarg>

//narrow_wide_string: string class that can be assigned to either narrow or wide strings.
template<typename Tcodecvt = std::codecvt<wchar_t,char,mbstate_t> >
class narrow_wide_string
	{
public:
	//empty ctor
	narrow_wide_string()
		: m_locale(std::locale::empty(), new Tcodecvt), m_string(NULL), m_wstring(NULL), m_size(0)
		{}
	//copy ctor
	narrow_wide_string(const narrow_wide_string& that)
		: m_locale(std::locale::empty(), new Tcodecvt), m_string(NULL), m_wstring(NULL)
		{
		m_size = that.m_size;
		m_wstring = new wchar_t[m_size];
		m_string = new char[m_size];
		std::wmemset(m_wstring, 0, m_size);
		std::memset(m_string, 0, m_size);
		//no need to do expensive conversion--just copy over the raw text
		std::wcsncpy(m_wstring, that.m_wstring, m_size);
		std::strncpy(m_string, that.m_string, m_size);
		}

	narrow_wide_string(const wchar_t* wide_text)
		: m_locale(std::locale::empty(), new Tcodecvt), m_string(NULL), m_wstring(NULL)
		{
		m_size = wcslen(wide_text)+1;
		m_wstring = new wchar_t[m_size];
		m_string = new char[m_size];
		std::wmemset(m_wstring, 0, m_size);
		std::memset(m_string, 0, m_size);
		std::wcsncpy(m_wstring, wide_text, m_size);
		convert();
		}
	//ctor designed to take a listing a single unicode characters
	//parameters are the size of string followed by individual characters (including null terminator)
	narrow_wide_string(size_t string_size, ...)
		: m_locale(std::locale::empty(), new Tcodecvt), m_string(NULL), m_wstring(NULL)
		{
		va_list ap = NULL;
		va_start(ap, string_size);

		m_size = string_size;
		m_wstring = new wchar_t[string_size];
		std::wmemset(m_wstring, 0, string_size);
		m_string = new char[string_size];
		std::memset(m_string, 0, string_size);

		wchar_t* wideBuffer = m_wstring;
		wchar_t t;

		while ((t = va_arg(ap, wchar_t) != NULL) )
			{
			*wideBuffer++ = t;
			}
		//get the null terminator
		*wideBuffer++ = t;

		va_end(ap);
		convert();
		}

	~narrow_wide_string()
		{
		delete [] m_string;
		delete [] m_wstring;
		}

	//assignment
	void operator=(const narrow_wide_string& that)
		{
		m_size = that.m_size;
		delete [] m_string;
		delete [] m_wstring;
		m_wstring = new wchar_t[m_size];
		m_string = new char[m_size];
		std::wmemset(m_wstring, 0, m_size);
		std::memset(m_string, 0, m_size);
		//no need to do expensive conversion--just copy over the raw text
		std::wcsncpy(m_wstring, that.m_wstring, m_size);
		std::strncpy(m_string, that.m_string, m_size);
		}
	void operator=(const wchar_t* wide_text)
		{
		m_size = wcslen(wide_text)+1;
		delete [] m_string;
		delete [] m_wstring;
		m_wstring = new wchar_t[m_size];
		m_string = new char[m_size];
		std::wmemset(m_wstring, 0, m_size);
		std::memset(m_string, 0, m_size);
		std::wcsncpy(m_wstring, wide_text, m_size);
		convert();
		}

	inline size_t length() const { return m_size-1; }

	//simple comparison and implicit conversion functions
	inline operator const char*() const { return m_string; }
	inline operator const wchar_t*() const { return m_wstring; }
///@todo should allow user to specify a collation interface
	inline bool operator==(const char* that) const { return std::strcmp(m_string, that) == 0; }
	inline bool operator==(const wchar_t* that) const { return std::wcscmp(m_wstring, that) == 0; }
	inline bool operator==(const narrow_wide_string& that) const { return std::wcscmp(m_wstring, that.m_wstring) == 0; }
	inline bool operator<(const char* that) const { return std::strcmp(m_string, that) < 0; }
	inline bool operator<(const wchar_t* that) const { return std::wcscmp(m_wstring, that) < 0; }
	inline bool operator<(const narrow_wide_string& that) const { return std::wcscmp(m_wstring, that.m_wstring) < 0; }
private:
	void convert()
		{
		char* toNext = NULL;
		const wchar_t* fromNext = NULL;
		typename Tcodecvt::state_type statet;

		/*convert unicode text to narrow counterpart
		(dependent on the code conversion class provided)*/
		std::codecvt_base::result conversionResult =
			std::use_facet<Tcodecvt>
				(m_locale).out(statet,
					const_cast<const wchar_t*>(m_wstring),
					const_cast<const wchar_t*>(m_wstring+m_size),
					fromNext,
					m_string, m_string+m_size, toNext);
		/*ok and nocovert are expected return values, but partial
		and error should raise*/
		if (conversionResult == std::codecvt_base::error ||
			conversionResult == std::codecvt_base::partial)
			{
			throw std::runtime_error("wide to narrow conversion failure.");
			}
		}
	std::locale m_locale;
	char* m_string;
	wchar_t* m_wstring;
	size_t m_size;
	};

#endif //__NARROW_WIDE_STRING_H__
